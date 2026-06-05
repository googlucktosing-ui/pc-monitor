import sys, serial, time
sys.stdout = open(sys.stdout.fileno(), mode="w", buffering=1)
print("START", flush=True)
ser = serial.Serial("/dev/ttyACM0", 115200, timeout=2)
ser.setDTR(False)
ser.setRTS(True)
time.sleep(0.2)
ser.setRTS(False)
print("RESET DONE", flush=True)
time.sleep(4)

# Read boot log
data = b""
for i in range(80):
    d = ser.read(200)
    if d: data += d
text = data.decode("utf-8", errors="replace")
print("Boot data:", len(data), "bytes", flush=True)
# Show key lines
for line in text.split("\n"):
    l = line.strip()
    if any(x in l for x in ["serial_cfg", "Send:", "Listening", "Received", "WiFi", "OK", "Reboot", "panic", "Guru", "uart driver"]):
        print("  >", l, flush=True)

# Send WIFI command
print("--- SENDING WIFI ---", flush=True)
ser.reset_input_buffer()
ser.write(b"WIFI:TestNet:pass123\n")
time.sleep(2)
resp = b""
for i in range(30):
    d = ser.read(100)
    if d: resp += d
r = resp.decode("utf-8", errors="replace").strip()
print("Response:", r if r else "(no response)", flush=True)
if "OK" in r:
    print("*** SUCCESS ***", flush=True)
else:
    # Check if there's any data
    print("Raw:", repr(resp[:200]), flush=True)
ser.close()
print("DONE", flush=True)
