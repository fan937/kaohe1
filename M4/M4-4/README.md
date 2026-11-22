mpu:SCL接PB10，SDA接PB11,
usbipd attach --wsl --busid 1-3(powershell)
make -C linux
sudo ./linux/build/serial_tool --dev /dev/ttyUSB0 --baud 115200 --csv out.csv   
sudo ./linux/build/serial_tool --dev /dev/ttyUSB0 --baud 115200 --send-rpm 60,0    
sudo ./linux/build/serial_tool --dev /tmp/ttyV0 --baud 115200 --csv data/out_junk.csv (脏字节)
sudo python3 inject_junk.py
usbipd attach --wsl --busid 1-3 --auto-attach(保持窗口打开即可自动挂载串口)   
sudo ./linux/build/serial_tool --dev /dev/ttyUSB0 --baud 115200 --csv data/out_reconn.csv --send-rpm 60,0
python3 frame_gen.py "AA 55 04 01 44 00 44 00 D4 BD" "AA 55 0E 02 44 00 44 00 F0 05 01 00 00 00 00 00 00 00 16 D2"
AA 55 04 01 3C 00 00 00 FF 1D
# 2.2
1.串口是流式字节，数据是一个一个间断到来的，中断接收不能阻塞等待整帧。如果等凑齐一帧再解析，串口中断里会卡住，新字节丢失，丢数据。环形缓冲区：中断只管快速把收到字节丢进缓冲区，中断立刻退出，耗时很短，不阻塞串口。状态机：在主循环里慢慢从缓冲区取字节、逐字节解析帧，一边取一边切换状态，不需要一次性拿到完整帧。 优势：实时性好、不容易丢字节，抗错位，帧出错后可以自动重新同步，适合嵌入式串口通信。
2.丢最新。保证当前正在解析的帧完整，控制指令优先保证已有帧正常解析。防止解析那帧残缺。
3.平时不清零，仅上位机下发清零指令时清零，用于累计统计链路错误。
# 2.3
处理方式：需要。检测到串口read错误后，调用serial_close关闭旧fd、标记断开；主循环持续循环尝试调用serial_open重新打开设备文件。USB插回，设备节点恢复，open成功，拿到新fd，通信自动恢复，无需重启程序。
# 2.4
1.坏帧计数应该是发生链路异常的事件数，因为waithead1情况下攒满8个字节才记一次，读到帧头但结构不对，坏帧只记一次。
2.会进状态机：垃圾里出现  AA ，状态机先把它当真帧头，进  WAIT_HEAD2 ；后面跟  55  就继续往下走。必被拦下：后面三个关卡（第二个帧头字节、 LEN  合法性、CRC16）至少有一处对不上 → 记 1 次坏帧 +  Parser_Reset ，回去重新找帧头。只牺牲当前一帧：因为找帧头是逐字节扫描，下一个真帧头一到立刻对齐 → 不会永久失步（实测：注入 8 次假帧头，坏帧计数正确）。
3.恢复时间约4437毫秒。mcu知道，加了超时检测，每收到一速度帧，就记录时间，若超过一秒没再收到速度帧说明上位机断开。