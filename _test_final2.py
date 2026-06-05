import sys, serial, time
sys.stdout = open(sys.stdout.fileno(), mode="w", buffering=1)
print("START", flush=True)

ser = serial.Serial("/dev/ttyACM0", 115200, timeout=3)

# Reset ESP32
ser.setDTR(False)
ser.setRTS(True)
time.sleep(0.3)
ser.setRTS(False)
print("RESET", flush=True)

# Wait for boot + app startup
time.sleep(5)

# Read all available data
data = ser.read(4096)  # Read up to 4KB with 3s timeout
text = data.decode("utf-8", errors="replace")
print(f"Got {len(data)} bytes", flush=True)

# Show key lines
found_serial = False
for line in text.split("\n"):
    l = line.strip()
    if any(x in l for x in ["serial_cfg", "Send:", "Listening", "Received", "WiFi", "OK", "ERROR", "Reboot", "panic", "Guru"]):
        print("  ", l, flush=True)
        found_serial = True

if not found_serial:
    print("  (no serial_cfg messages found)", flush=True)
    # Show last 500 chars for debugging
    print("  [last 500 chars]:", text[-500:].replace("\n", "\\n"), flush=True)

# Send WIFI command
print("--- SENDING WIFI:TestNet:pass123 ---", flush=True)
ser.reset_input_buffer()
ser.write(b"WIFI:TestNet:pass123\n")
time.sleep(2)

resp = ser.read(1024)
r = resp.decode("utf-8", errors="replace").strip()
print("Response:", r if r else "(no response)", flush=True)
if "OK" in r:
    print("*** SUCCESS! WiFi command received! ***", flush=True)
else:
    print(f"Raw ({len(resp)} bytes):", repr(resp[:200]), flush=True)

ser.close()
print("DONE", flush=True)
