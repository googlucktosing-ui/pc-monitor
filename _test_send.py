import sys, serial, time
sys.stdout = open(sys.stdout.fileno(), mode="w", buffering=1)
ser = serial.Serial("/dev/ttyACM0", 115200, timeout=2)
ser.setDTR(False)
ser.setRTS(True)
time.sleep(0.2)
ser.setRTS(False)
time.sleep(3)
data = b""
for i in range(30):
    d = ser.read(200)
    if d: data += d
# 显示serial_cfg相关行
text = data.decode("utf-8", errors="replace")
for line in text.split("\n"):
    if "serial_cfg" in line or "Send:" in line or "WIFI" in line.upper():
        print(line.strip())
if not any("serial_cfg" in l for l in text.split("\n")):
    print("(serial_cfg not found)")
print()

# 发送WIFI命令
print("--- Send WIFI ---", flush=True)
ser.reset_input_buffer()
ser.write(b"WIFI:TestWiFi:test123\n")
time.sleep(1)
resp = b""
for i in range(20):
    d = ser.read(100)
    if d: resp += d
r = resp.decode("utf-8", errors="replace").strip()
print("Response:", r if r else "(none)", flush=True)
print("OK:", "OK" in r, flush=True)
ser.close()
