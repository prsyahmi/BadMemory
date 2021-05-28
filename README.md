# BadMemory
Make portions of memory unusable for Windows. Equivalent to badram on linux but for windows.

It is possible to run windows with defective memory by telling windows not to use the faulty address by manually adding entry to badmemorylist on {badmemory} section of windows BCD.
For example:

  bcd /set {badmemory} badmemorylist PFN1 PFN2 PFN3 ...

However, there is limitation on the list which will causes bootmanager to fail with STATUS_BUFFER_TOO_SMALL and will not load the windows at all. Due to this and without succeed finding other solutions, this driver was created to overcome the issue.

## Warning
This driver will start early at the boot time and will try to block the bad RAM region. If somehow Windows/other drivers uses the bad RAM region before this driver able to block them, it will shows [FAIL] on the BadMemoryManager. So it offers no guarantee that the region will always be blocked by the driver.

However my computer have been running with defective ram 24/7 mostly for a year now with this driver and not a single BSODs, Application Crash, data corruption occurred.

Caution must be taken when doing major Windows update (eg. from Windows 10 Anniversary to Creators Update) since it will start another OS (WinPE) without the driver to do the update. In this case, it may end up creating a bad rollback files and when copying/installing new files to your drive it may ends up corrupted due to bad RAM. This will causes a boot-loop if you are unlucky, there is a way to exit the loop without reformat but I will not cover it here.

**Remember ALWAYS PULL OUT BAD RAM BEFORE DOING MAJOR WINDOWS UPDATE**.

## Download
https://github.com/prsyahmi/BadMemory/releases

## Compiling
To compile, make sure to have VS2015 with latest WDK installed. Then open the solution, configure the target platform and build the project.

The driver needs signing after compilation for windows to load the driver.

In case you don't have proper certificate for signing, you will need to have the test signing on and sign the driver with test-certificate (VS should already done this).
Refer here to turn on the settings: https://msdn.microsoft.com/en-us/library/windows/hardware/ff553484(v=vs.85).aspx

## Usage
1. Install the driver by right-clicking badmemory.inf file and then click Install.
2. Use BadMemoryManager.exe to create a list of bad memory region.
3. Restart the computer

Before using, move the faulty RAM to furthest bank (if you have more than 1 RAM installed), then run memtest and note down the faulty address. Then proceed to the installation above without defective RAM installed (if you have more than 1 RAM) and reinstall the RAM.

You may need to turn off RAM interleaving in the BIOS/UEFI if the bad region address resides on undesired location or if you want the bad region to be as little as possible.

UPDATE:
- This driver is not signed by microsoft, you may need to turn on test mode on later versions of windows
- RAMMap can be used to verify the region: It will show as System PTE

## License
This project is licensed under GPLv3. See LICENSE
