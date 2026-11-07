本项目使用  arm-none-eabi-gcc  + Makefile 作为构建工具链；使用 ST‑Link V2 + STM32CubeProgrammer 完成烧录与复位。
 
Makefile、链接脚本、启动文件参考了 CubeMX 的输出进行学习与验证；最终工程由空目录开始搭建，所有外设初始化、时钟配置、业务逻辑均手动实现。启动文件与链接脚本已经过阅读与确认，理解向量表、内存布局、链接地址与运行时初始化流程。
1. WSL 安装依赖

2. 获取代码，进入工程文件夹

3.  make   编译

4. ST‑LINK 接线，板子上电

5.  make flash 在Makefile里编译
flash:
    "/mnt/c/Program Files/STMicroelectronics/STM32Cube/STM32CubeProgrammer/bin/STM32_Programmer_CLI.exe" \
		-c port=SWD freq=4000 \
		-e all \
		-w build/$(TARGET).elf \
		-v
           使用 ST‑LINK V2，调用 STM32CubeProgrammer CLI，依次执行：全片擦除、固件写入、烧录校验。

6. 串口打开，观察输出、测试按键

7.  make clean  清理
 
 构建约定
 
   make ：一键编译，生成 elf / hex / bin

  make flash ：一键烧录（擦除、写入、校验）

  make clean ：删除 build