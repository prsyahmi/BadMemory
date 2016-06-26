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
PVOID GBadMemVAddr = NULL;

NTSTATUS
DriverEntry(
	IN PDRIVER_OBJECT  DriverObject,
	IN PUNICODE_STRING  RegistryPath
)
{
	NTSTATUS        ntStatus;
	UNICODE_STRING  ntUnicodeString;
	UNICODE_STRING  ntWin32NameString;
	PDEVICE_OBJECT  deviceObject = NULL;

	UNREFERENCED_PARAMETER(RegistryPath);

	PHYSICAL_ADDRESS lowAddr;
	PHYSICAL_ADDRESS highAddr;
	PHYSICAL_ADDRESS boundary;
	lowAddr.QuadPart = BADMEM_START;
	highAddr.QuadPart = BADMEM_END;
	boundary.QuadPart = 0;

	GBadMemVAddr = MmAllocateContiguousMemorySpecifyCache(BADMEM_SIZE, lowAddr, highAddr, boundary, MmNonCached);
	if (GBadMemVAddr == NULL) {
		DbgPrint("Unable to allocate bad memory");
		return STATUS_MEMORY_NOT_ALLOCATED;
	}

	RtlInitUnicodeString(&ntUnicodeString, NT_DEVICE_NAME);

	ntStatus = IoCreateDevice(
		DriverObject,                   // Our Driver Object
		0,                              // We don't use a device extension
		&ntUnicodeString,               // Device name "\Device\BadMemory"
		FILE_DEVICE_UNKNOWN,            // Device type
		FILE_DEVICE_SECURE_OPEN,     // Device characteristics
		FALSE,                          // Not an exclusive device
		&deviceObject);                // Returned ptr to Device Object

	if (!NT_SUCCESS(ntStatus))
	{
		//SDMA_KDPRINT(("Couldn't create the device object\n"));
		return ntStatus;
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

	ntStatus = IoCreateSymbolicLink(&ntWin32NameString, &ntUnicodeString);

	if (!NT_SUCCESS(ntStatus))
	{
		//
		// Delete everything that this routine has allocated.
		//
		IoDeleteDevice(deviceObject);
	}


	return ntStatus;
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

	if (GBadMemVAddr != NULL) {
		MmFreeContiguousMemorySpecifyCache(GBadMemVAddr, BADMEM_SIZE, MmNonCached);
	}
}