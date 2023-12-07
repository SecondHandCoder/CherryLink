# CherryLink
## Reference project
0. Thanks to the developers of the following project.
1. USB protocol stack: [CherryUSB](https://github.com/cherry-embedded/CherryUSB)
2. RTOS: [rt-thread](https://github.com/RT-Thread/rt-thread)
3. Transport protocol: [DAPLink](https://github.com/ARMmbed/DAPLink)
4. SPI simulation timing: [vllink](https://github.com/vllogic/vllink_lite)
## Development environment
### Windows
1. ARMCC V5.06 update 7 (build 960) of keil
2. ARMCLANG V6.19 of keil
3. IAR V9.40.1.364/W64 of IAR
### Linux
1. arm-none-eabi-gcc (Arm GNU Toolchain 13.2.rel1 (Build arm-13.7)) 13.2.1 20231009
## Compilation guide
### KEIL 
1. options for target->Target: Click on "ARM Compiler" to switch compiler versions.
2. options for target->User: Click "After Build/Rebuild" to switch to the instruction ARMCC/ARMLANG for generating bin files.
3. options for target->C/C++: Click "Optimization" to change optimization level. Usually, choose -Oz when compiling the boot, choose -Ofast when compiling the app for ARMCLANG. choose -O3 for ARMCC. 
4. Change the first precompile instruction in the /project/keil/linker/*.sct file. Use #! armlang for ARMCLANG, #! armcc for ARMCC.
### IAR
1. Compile boot and app separately without making any changes.
### GCC
1. cd project/gcc/build/boot && cmake -DCHERRY_BUILD_TYPE=BOOT ../../ && make
2. cd project/gcc/build/app && cmake -DCHERRY_BUILD_TYPE=APP ../../ && make
### After build
1. use merge_bin.exe(Windows environment)/merge_bin(Linux environment) tool in the tools directory to synthesize BOOT+APP files.
## Function description
### SWD/JTAG
1. Support universal debug/download tools such as Keil IAR or OpenOCD, use USB winusb interface, Windows10 and above do not require driver installation, Windows7 requires installation of WinUSB driver.
2. The maximum speed of SWD interface is 18Mb/s.
3. The maximum speed of JTAG interface is 9Mb/s, improving PCB layout can enhance to 18Mb/s. Currently, the code supports a maximum of 9Mb/s.
### Virtual serial port
1. USB CDC interface analog a virtual serial port, Windows10 and above do not require driver installation, Windows7 requires installation of USBCDC driver.
2. The maximum speed of virtual serial port is 9Mb/s(reference values given in the chip reference manual).
## TODO
1. A bootloader, due to USB-DFU requiring additional driver installation of upper computer, it may be necessary to develop a corresponding upper computer.
2. A spi flash download tool, it may be necessary to develop a corresponding upper computer too.