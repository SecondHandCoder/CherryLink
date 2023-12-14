# CherryLink
[English](https://github.com/SecondHandCoder/CherryLink/blob/main/README_EN.md)
## 参考项目
0. 感谢以下项目的开发人员.
1. USB 协议栈: [CherryUSB](https://github.com/cherry-embedded/CherryUSB)
2. 实时操作系统: [rt-thread](https://github.com/RT-Thread/rt-thread)
3. 传输协议: [DAPLink](https://github.com/ARMmbed/DAPLink)
4. SPI 模拟接口协议: [vllink](https://github.com/vllogic/vllink_lite)
## 开发环境
### Windows
1. 基于KEIL的ARMCC V5.06 update 7 (build 960)编译器
2. 基于KEIL的ARMCLANG V6.19编译器
3. 基于IAR的IAR V9.40.1.364/W64编译器
### Linux
1. arm-none-eabi-gcc (Arm GNU Toolchain 13.2.rel1 (Build arm-13.7)) 13.2.1 20231009
## 编译指南
### KEIL 
1. 选中options for target中的Target标签: 点击"ARM Compiler"切换编译器版本.
2. 选中options for target中的User标签: 点击"After Build/Rebuild"切换生成bin文件的指令.
3. 选中options for target中的C/C++标签: 点击"Optimization"切换优化等级. 通常, 在使用ARMCLANG编译boot时选择-Oz优化，编译app时选择-Ofast优化. 在使用ARMCC编译boot和app时选择-O3优化. 
4. 编辑/project/keil/linker/*.sct文件中的首行预编译指令, ARMCLANG使用#! armlang开头的指令 ARMCC使用#! armcc开头的指令.
### IAR
1. 直接编译boot和app即可.
### GCC
1. cd project/gcc/build/boot && cmake -DCHERRY_BUILD_TYPE=BOOT ../../ && make
2. cd project/gcc/build/app && cmake -DCHERRY_BUILD_TYPE=APP ../../ && make
### 编译完成
1. 使用tools文件下的merge_bin.exe(Windows开发环境)或merge_bin(Linux开发环境)工具合成BOOT+APP文件.
## 功能描述
### SWD/JTAG
1. 支持KEIL, IAR或OpenOCD等通用的编程工具, 使用USB的WinUSB接口, Windows10以上免驱, Windows7需自行安装WinUSB驱动.
2. SWD接口支持最高速度为18Mb/s.
3. JTAG接口支持最高速度为9Mb/s.
### 虚拟串口
1. USB-CDC接口虚拟串口, Windows10以上免驱, Windows7需自行安装USB-CDC驱动.
2. 虚拟串口支持最高速度为9Mb/s(基于芯片开发手册的参考值).
## 接口描述
| 接口名称 | 功能描述 |
| :---: | :---: |
| RX | 虚拟串口接收脚 |
| TX | 虚拟串口发送脚 |
| TDI | JTAG接口TDI脚 |
| TDO | JTAG接口TDO脚 |
| TCK | JTAG接口或SWD接口TCK脚 |
| TMS | JTAG接口或SWD接口TMS脚 |
| GND*2 | 地脚 |
| 3.3V | 3.3V输出脚 |
| RST | 复位脚 |
| Vref | 参考电压脚, 取自目标板或自身的输出脚. <br> 该脚电压值决定了其他脚的电压值, 最大值为 5V |
| 5V | 5V输出脚 |
## 硬件文件说明
| 文件名称 | 文件说明 |
| :---: | :---: |
| 3DShell_CherryLink.zip | 外壳生产文件, 分为上下外壳两部分 |
| appearance.jpg | 产品实物图 |
| Gerber_CherryLink.zip | PCB生产文件 |
| Panel_CherryLinkBoard,epanm | 上面板贴纸生产文件 |
| SCH_CherryLink.pdf | PCB原理图 |
## 未来展望
1. 开发bootloader, 因为USB-DFU需要安装额外的驱动, 所以可能会开发一款专用的上位机.
2. 增加SPI FLASH下载工具, 这个也需要专用的上位机配合.
