import sys, serial, time
sys.stdout = open(sys.stdout.fileno(), mode="w", buffering=1)
ser = serial.Serial("/dev/ttyACM0", 115200, timeout=2)
time.sleep(4)
data = ser.read(4096)
text = data.decode("utf-8", errors="replace")
# 检查崩溃
has_guru = "Guru" in text
print("Guru:", has_guru, flush=True)
for line in text.split("\n"):
    if "serial_cfg" in line.lower():
        print("  ", line.strip(), flush=True)
if has_guru: exit()

# 发送WIFI命令
print("--- Send WIFI ---", flush=True)
ser.reset_input_buffer()
ser.write(b"WIFI:TestNet:pass123\n")
time.sleep(2)
resp = ser.read(1024)
r = resp.decode("utf-8", errors="replace")
print("Response:", r[:200] if r else "(none)", flush=True)
print("OK:", "OK" in r, flush=True)
ser.close()
