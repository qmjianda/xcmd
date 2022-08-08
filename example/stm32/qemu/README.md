# 说明
本例程序是在qemu下模拟stm32并移植了xcmd
# 环境
win10 wsl2 ubuntu 18.04 LTS
# 安装
## 安装qemu
```bash
#1. 获取qemu-stm32, 如果clone失败请使用gitee转存
git clone https://github.com/beckus/qemu_stm32
#2. 安装基础环境
sudo apt-get install build-essential python zlib1g-dev libglib2.0-dev libpixman-1-dev libtool libfdt-dev
#3. 配置编译
cd qemu_stm32
./configure --enable-debug --disable-werror --target-list="arm-softmmu"
make
#4. 测试
arm-softmmu/qemu-system-arm --version
# 测试结果： QEMU emulator version 2.1.3, Copyright (c) 2003-2008 Fabrice Bellard
```
## 安装arm-none-eabi-gcc
```bash
#1. 安装
sudo apt-get install gcc-arm-none-eabi
#2. 测试
arm-none-eabi-gcc -v
#测试结果： gcc version 9.2.1 20191025 (release) [ARM/arm-9-branch revision 277599] (15:9-2019-q4-0ubuntu1)
```
## 安装arm-none-eabi-gdb
```bash
#1. 安装libreadline6
sudo apt-get install libreadline6
#2. 安装
wget http://archive.ubuntu.com/ubuntu/pool/universe/g/gdb-arm-none-eabi/gdb-arm-none-eabi_7.10-1ubuntu3+9_amd64.deb
sudo dpkg -i gdb-arm-none-eabi_7.10-1ubuntu3+9_amd64.deb
#3. 测试
arm-none-eabi-gdb --version
#测试结果： GNU gdb (7.10-1ubuntu3+9) 7.10
```
## 安装串口终端cu
```bash
sudo apt-get install cu
```

# 编译
```bash
make clean
make 
```
# 测试
**NOTE：** makefile中QEMU_ARM_DIR参数指定了qemu-system-arm的路径
- qemu run pty
```bash
#1. 启动qemu 
make main_QEMURUN_PTY
#输出：char device redirected to /dev/pts/11 (label serial0)
#2. 连接串口终端，串口设备见qemu输出
cu -l /dev/pts/11
#3. 退出cu
~.
```
- qemu debug pty
**NOTE** 
1. 启动qemu debug模式后程序不再自动运行，连接gdb后程序才会继续运行
2. 可以参考.vscode/launch.json中的配置"debug qemu"， 使用vscode调试程序
```bash
#1. 启动qemu 
make main_QEMUDBG_PTY 
#输出：qemu_stm32/arm-softmmu/qemu-system-arm -M stm32-p103 -gdb tcp::3333 -S -kernel demos/main/main.bin -serial pty
# char device redirected to /dev/pts/11 (label serial0)
#2. 连接gdb
arm-none-eabi-gdb -ex "target remote localhost: 3333" -ex "symbol-file demos/main/main.elf"
#运行
c
#启动调试界面
tui en
#3. 连接串口终端，串口设备见qemu输出
cu -l /dev/pts/11
#4. 退出cu
~.
```