import sys, serial, time
sys.stdout = open(sys.stdout.fileno(), mode="w", buffering=1)
ser = serial.Serial("/dev/ttyACM0", 115200, timeout=2)
ser.setDTR(False)
ser.setRTS(True)
time.sleep(0.2)
ser.setRTS(False)
time.sleep(2)
data = b""
for i in range(50):
    d = ser.read(200)
    if d: data += d
text = data.decode("utf-8", errors="replace")
# 打印最后1000个字符
print("--- LAST 1000 CHARS ---", flush=True)
print(text[-1000:], flush=True)
ser.close()
