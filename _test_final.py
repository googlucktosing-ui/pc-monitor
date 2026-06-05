import serial, time
ser = serial.Serial("/dev/ttyACM0", 115200, timeout=2)
time.sleep(3)
# 读启动日志
data = b""
for i in range(30):
    try:
        d = ser.read(200)
        if d: data += d
    except: pass
text = data.decode("utf-8", errors="replace")
for line in text.split("\n"):
    if "serial_cfg" in line or "Listening" in line or "Send:" in line or "WIFI" in line.upper():
        print(line.strip())
if "Guru" in text or "panic" in text:
    print("*** CRASHED ***")
    exit()
# 发送WIFI命令
print("\n--- Sending WIFI command ---")
ser.reset_input_buffer()
ser.write(b"WIFI:TestWiFi:test123\n")
time.sleep(1.5)
resp = b""
for i in range(20):
    try:
        d = ser.read(100)
        if d: resp += d
    except: pass
r = resp.decode("utf-8", errors="replace")
print("Response:", r.strip() if r else "(none)")
print("OK:", "OK" in r)
if "OK" in r:
    print("*** SUCCESS ***")
ser.close()
