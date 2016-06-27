# BadMemory
Make portions of memory unusable for Windows.

It is possible to run windows with defective memory by telling windows not to use the faulty address by manually adding entry to badmemorylist on {badmemory} section of windows BCD.
For example:

  bcd /set {badmemory} badmemorylist PFN1 PFN2 PFN3 ...

However, there is limitation on the list which will causes bootmanager to fail with STATUS_BUFFER_TOO_SMALL and will not load the windows at all. Due to this and without succeed finding other solutions, this driver was created to overcome the issue.

So far using this driver with defective RAM, I had never encountered any issues like BSODs, Application Crash, data corruption yet.

## Compiling
To compile, make sure to have VS2015 with latest WDK installed. Then open the solution, configure the target platform and build the project.

## Usage
1. Install the driver by right-clicking badmemory.inf file and then click Install.
2. Use BadMemoryManager.exe to create a list of bad memory region.
3. Restart the computer

Before using, move the faulty RAM to furthest bank (if you have more than 1 RAM installed), then run memtest and note down the faulty address. Then proceed to the installation above without defective RAM installed (if you have more than 1 RAM).

If you don't have the certificate to sign the driver or if you download the release from here, you will need to have the test signing on. Refer here to turn on the settings: https://msdn.microsoft.com/en-us/library/windows/hardware/ff553484(v=vs.85).aspx

## License
This project is licensed under GPLv3. See LICENSE
