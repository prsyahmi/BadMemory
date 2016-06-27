#include <ntddk.h>
#include "badmemory.h"

#define NT_DEVICE_NAME     L"\\Device\\BadMemory"
#define DOS_DEVICE_NAME    L"\\DosDevices\\BadMemory"

DRIVER_INITIALIZE DriverEntry;
DRIVER_UNLOAD BadMemUnloadDriver;

#ifdef ALLOC_PRAGMA
#pragma alloc_text( INIT, DriverEntry )
#pragma alloc_text( PAGE, BadMemUnloadDriver)
#endif // ALLOC_PRAGMA


#define BADMEM_SAFEPAGE 5
#define BADMEM_START		(0x1edfd80a0 - (BADMEM_SAFEPAGE * PAGE_SIZE)) / PAGE_SIZE  * PAGE_SIZE
#define BADMEM_END		(0x1effdc338 + (BADMEM_SAFEPAGE * PAGE_SIZE)) / PAGE_SIZE  * PAGE_SIZE
#define BADMEM_SIZE		BADMEM_END - BADMEM_START
PVOID* GBadMemAddresses = NULL;
ULONG GTotalRegions = 0;


NTSTATUS
DriverEntry(
	IN PDRIVER_OBJECT  DriverObject,
	IN PUNICODE_STRING RegistryPath
)
{
	NTSTATUS        Status;
	UNICODE_STRING  ntUnicodeString;
	UNICODE_STRING  ntWin32NameString;
	PDEVICE_OBJECT  deviceObject = NULL;

	UNICODE_STRING RegParamPath;
	OBJECT_ATTRIBUTES KeyAttr;
	HANDLE KeyHandle;
	UCHAR* ValueData = NULL;
	PBAD_REGION BadRegions = NULL;

	RegParamPath.Length = 0;
	RegParamPath.MaximumLength = RegistryPath->Length + 11 * sizeof(WCHAR);
	RegParamPath.Buffer = ExAllocatePoolWithTag(NonPagedPool, RegParamPath.MaximumLength, 'MdaB');
	if (RegParamPath.Buffer) {
		RtlCopyUnicodeString(&RegParamPath, RegistryPath);
		RtlAppendUnicodeToString(&RegParamPath, L"\\Parameters");

		InitializeObjectAttributes(&KeyAttr, &RegParamPath, OBJ_CASE_INSENSITIVE, NULL, NULL);
		Status = ZwOpenKey(&KeyHandle, KEY_READ, &KeyAttr);

		if (NT_SUCCESS(Status))
		{
			UNICODE_STRING ValueName;
			ULONG ValueLength = 0;

			RtlInitUnicodeString(&ValueName, L"BadRegions");

			Status = ZwQueryValueKey(KeyHandle, &ValueName, KeyValuePartialInformation, ValueData, 0, &ValueLength);
			if (Status == STATUS_BUFFER_OVERFLOW || Status == STATUS_BUFFER_TOO_SMALL)
			{
				ValueData = (UCHAR*)ExAllocatePoolWithTag(NonPagedPool, ValueLength, 'MdaB');
				if (ValueData)
				{
					Status = ZwQueryValueKey(KeyHandle, &ValueName, KeyValuePartialInformation, ValueData, ValueLength, &ValueLength);
					if (NT_SUCCESS(Status))
					{
						PKEY_VALUE_PARTIAL_INFORMATION KeyInfo = (PKEY_VALUE_PARTIAL_INFORMATION)ValueData;
						GTotalRegions = KeyInfo->DataLength / sizeof(BAD_REGION);
						BadRegions = (PBAD_REGION)&KeyInfo->Data[0];
					}
				}
			}

			ZwClose(KeyHandle);
		}

		ExFreePoolWithTag(RegParamPath.Buffer, 'MdaB');
	}

	GBadMemAddresses = (PVOID*)ExAllocatePoolWithTag(NonPagedPool, sizeof(PVOID) * GTotalRegions, 'MdaB');
	if (GBadMemAddresses == NULL) {
		DbgPrint("Unable to allocate bad memory holder");
	} else {
		RtlZeroMemory(GBadMemAddresses, sizeof(PVOID) * GTotalRegions);
	}

	for (ULONG i = 0; i < GTotalRegions; i++)
	{
		PHYSICAL_ADDRESS LowerBound;
		PHYSICAL_ADDRESS UpperBound;
		PHYSICAL_ADDRESS Boundary;

		LowerBound.QuadPart = BadRegions[i].LowerBound / PAGE_SIZE * PAGE_SIZE;
		UpperBound.QuadPart = BadRegions[i].UpperBound / PAGE_SIZE * PAGE_SIZE;
		Boundary.QuadPart = 0;

		PVOID BadAddr = MmAllocateContiguousMemorySpecifyCache(
			UpperBound.QuadPart - LowerBound.QuadPart,
			LowerBound,
			UpperBound,
			Boundary,
			MmNonCached);

		if (BadAddr == NULL) {
			DbgPrint("Unable to allocate bad memory at %I64d - %I64d", LowerBound.QuadPart, UpperBound.QuadPart);
		}

		if (GBadMemAddresses) {
			GBadMemAddresses[i] = BadAddr;
		}
	}

	if (ValueData) {
		ExFreePoolWithTag(ValueData, 'MdaB');
		ValueData = NULL;
		BadRegions = NULL;
	}

	RtlInitUnicodeString(&ntUnicodeString, NT_DEVICE_NAME);

	Status = IoCreateDevice(
		DriverObject,                   // Our Driver Object
		0,                              // We don't use a device extension
		&ntUnicodeString,               // Device name "\Device\BadMemory"
		FILE_DEVICE_UNKNOWN,            // Device type
		FILE_DEVICE_SECURE_OPEN,     // Device characteristics
		FALSE,                          // Not an exclusive device
		&deviceObject);                // Returned ptr to Device Object

	if (!NT_SUCCESS(Status))
	{
		return Status;
	}

	//
	// Initialize the driver object with this driver's entry points.
	//

	DriverObject->DriverUnload = BadMemUnloadDriver;

	//
	// Initialize a Unicode String containing the Win32 name
	// for our device.
	//

	RtlInitUnicodeString(&ntWin32NameString, DOS_DEVICE_NAME);

	//
	// Create a symbolic link between our device name  and the Win32 name
	//

	Status = IoCreateSymbolicLink(&ntWin32NameString, &ntUnicodeString);

	if (!NT_SUCCESS(Status))
	{
		//
		// Delete everything that this routine has allocated.
		//
		IoDeleteDevice(deviceObject);
	}


	return Status;
}

VOID
BadMemUnloadDriver(
    _In_ PDRIVER_OBJECT DriverObject
    )
/*++
Routine Description:
    This routine is called by the I/O system to unload the driver.
    Any resources previously allocated must be freed.
Arguments:
    DriverObject - a pointer to the object that represents our driver.
Return Value:
    None
--*/
{
    PDEVICE_OBJECT deviceObject = DriverObject->DeviceObject;
    UNICODE_STRING uniWin32NameString;

    PAGED_CODE();

    //
    // Create counted string version of our Win32 device name.
    //

    RtlInitUnicodeString( &uniWin32NameString, DOS_DEVICE_NAME );


    //
    // Delete the link from our device name to a name in the Win32 namespace.
    //

    IoDeleteSymbolicLink( &uniWin32NameString );

    if ( deviceObject != NULL )
    {
        IoDeleteDevice( deviceObject );
    }

	if (GBadMemAddresses != NULL) {
		for (ULONG i = 0; i < GTotalRegions; i++) {
			if (GBadMemAddresses[i] != NULL) MmFreeContiguousMemory(GBadMemAddresses[i]);
		}

		ExFreePoolWithTag(GBadMemAddresses, 'MdaB');
		GBadMemAddresses = NULL;
	}
}