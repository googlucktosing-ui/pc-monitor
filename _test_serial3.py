import serial, time
ser = serial.Serial("/dev/ttyACM0", 115200, timeout=2)
# 重置开发板
ser.setDTR(False)
ser.setRTS(True)
time.sleep(0.1)
ser.setRTS(False)
time.sleep(3)
# 读取启动日志
data = b""
for i in range(50):
    try:
        d = ser.read(200)
        if d:
            data += d
    except:
        pass
text = data.decode("utf-8", errors="replace")
# 显示关键信息
for line in text.split("\n"):
    l = line.strip()
    if any(x in l for x in ["serial_cfg", "listener", "Listening", "Send:", "Received", "Guru", "panic", "uart driver"]):
        print(l)
# 检查是否崩溃
if "Guru" in text or "panic" in text:
    print("\n*** FIRMWARE CRASHED ***")
else:
    print("\n*** FIRMWARE OK (no crash) ***")
# 发送WIFI命令并检查响应
print("\n[Sending WIFI:TEST:test123...]")
# 清空缓冲区
ser.reset_input_buffer()
ser.write(b"WIFI:TEST:test123\n")
time.sleep(1)
resp = b""
for i in range(20):
    try:
        d = ser.read(100)
        if d:
            resp += d
    except:
        pass
resp_text = resp.decode("utf-8", errors="replace")
print("Response:", resp_text if resp_text else "(none)")
print("OK received:", "OK" in resp_text)
ser.close()
