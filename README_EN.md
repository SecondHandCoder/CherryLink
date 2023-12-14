# CherryLink
[中文](https://github.com/SecondHandCoder/CherryLink/blob/main/README.md)
## Reference project
0. Thanks to the developers of the following project.
1. USB protocol stack: [CherryUSB](https://github.com/cherry-embedded/CherryUSB)
2. RTOS: [rt-thread](https://github.com/RT-Thread/rt-thread)
3. Transport protocol: [DAPLink](https://github.com/ARMmbed/DAPLink)
4. SPI simulation timing: [vllink](https://github.com/vllogic/vllink_lite)
## Development environment
### Windows
1. ARMCC V5.06 update 7 (build 960) of KEIL
2. ARMCLANG V6.19 of KEIL
3. IAR V9.40.1.364/W64 of IAR
### Linux
1. arm-none-eabi-gcc (Arm GNU Toolchain 13.2.rel1 (Build arm-13.7)) 13.2.1 20231009
## Compilation guide
### KEIL 
1. options for target->Target: Click on "ARM Compiler" to switch compiler versions.
2. options for target->User: Click on "After Build/Rebuild" to switch to the instruction ARMCC/ARMLANG for generating bin files.
3. options for target->C/C++: Click on "Optimization" to change optimization level. Usually, choose -Oz when compiling the boot, choose -Ofast when compiling the app for ARMCLANG. choose -O3 for ARMCC. 
4. Change the first precompile instruction in the /project/keil/linker/*.sct file. Use #! armlang for ARMCLANG, #! armcc for ARMCC.
### IAR
1. Compile boot and app separately without making any changes.
### GCC
1. cd project/gcc/build/boot && cmake -DCHERRY_BUILD_TYPE=BOOT ../../ && make
2. cd project/gcc/build/app && cmake -DCHERRY_BUILD_TYPE=APP ../../ && make
### After build
1. Use merge_bin.exe(Windows environment)/merge_bin(Linux environment) tool in the tools directory to synthesize BOOT+APP files.
## Function description
### SWD/JTAG
1. Support universal debug/download tools such as KEIL IAR or OpenOCD, use USB WinUSB interface, Windows10 and above do not require driver installation, Windows7 requires installation of WinUSB driver.
2. The maximum speed of SWD interface is 18Mb/s.
3. The maximum speed of JTAG interface is 9Mb/s.
### Virtual serial port
1. USB-CDC interface analog a virtual serial port, Windows10 and above do not require driver installation, Windows7 requires installation of USB-CDC driver.
2. The maximum speed of virtual serial port is 9Mb/s(reference values given in the chip reference manual).
## Interface description
| Interface name | Function description |
| :---: | :---: |
| RX | Virtual serial port receive port |
| TX | Virtual serial port transmit port |
| TDI | JTAG interface TDI port |
| TDO | JTAG interface TDO port |
| TCK | JTAG interface or SWD interface TCK port |
| TMS | JTAG interface or SWD interface TMS port |
| GND*2 | Ground port |
| 3.3V | 3.3V output port |
| RST | Reset port |
| Vref | Reference voltage port, taken from the target board or local board output port. <br> It determines the output voltage of other pins, maximum of 5V |
| 5V | 5V output port |
## Hardware description
| File name | Description |
| :---: | :---: |
| 3DShell_CherryLink.zip | The shell production documents, divided into the upper and lower shells |
| appearance.jpg | Product photos |
| Gerber_CherryLink.zip | PCB production documents |
| Panel_CherryLinkBoard,epanm | Production documents for upper shell stickers |
| SCH_CherryLink.pdf | PCB schematic file |
## TODO
1. A bootloader, due to USB-DFU requiring additional driver installation of upper computer, it may be necessary to develop a corresponding upper computer.
2. A SPI FLASH download tool, it may be necessary to develop a corresponding upper computer too.
