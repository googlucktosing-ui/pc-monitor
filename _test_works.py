import sys, serial, time
sys.stdout = open(sys.stdout.fileno(), mode="w", buffering=1)
print("Opening port...", flush=True)
ser = serial.Serial("/dev/ttyACM0", 115200, timeout=2)
print("Port opened, reading existing data...", flush=True)
time.sleep(1)
data = b""
for i in range(60):
    d = ser.read(200)
    if d: data += d
text = data.decode("utf-8", errors="replace")
print("Got", len(data), "bytes", flush=True)
for line in text.split("\n"):
    l = line.strip()
    if any(x in l for x in ["serial_cfg", "Send:", "Received", "OK", "ERROR", "WIFI", "listener", "Listening"]):
        print(l, flush=True)
print()
print("--- Now sending WIFI command ---", flush=True)
ser.reset_input_buffer()
ser.write(b"WIFI:MyWiFi:MyPass123\n")
print("Sent, waiting for response...", flush=True)
time.sleep(2)
resp = b""
for i in range(30):
    d = ser.read(100)
    if d: resp += d
r = resp.decode("utf-8", errors="replace").strip()
print("Response:", r if r else "(no response)", flush=True)
if "OK" in r:
    print("*** SUCCESS! ESP32 received the WiFi command! ***", flush=True)
else:
    print("*** No OK response ***", flush=True)
ser.close()
