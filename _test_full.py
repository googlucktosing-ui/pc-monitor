import sys, serial, time
sys.stdout = open(sys.stdout.fileno(), mode="w", buffering=1)
ser = serial.Serial("/dev/ttyACM0", 115200, timeout=2)
ser.setDTR(False)
ser.setRTS(True)
time.sleep(0.2)
ser.setRTS(False)
time.sleep(4)
# Read full boot log
data = b""
for i in range(80):
    d = ser.read(200)
    if d: data += d
text = data.decode("utf-8", errors="replace")
# Show serial_cfg lines
for line in text.split("\n"):
    if "serial_cfg" in line.lower() or "Send:" in line or "WIFI" in line:
        print(line.strip())
print()
# Send WIFI command
print("--- Sending WIFI:TestWiFi:test123 ---", flush=True)
ser.reset_input_buffer()
ser.write(b"WIFI:TestWiFi:test123\n")
time.sleep(2)
resp = b""
for i in range(30):
    d = ser.read(100)
    if d: resp += d
r = resp.decode("utf-8", errors="replace").strip()
print("Response:", r if r else "(none)", flush=True)
print("OK:", "OK" in r, flush=True)
ser.close()
