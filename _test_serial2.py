import serial, time, sys
ser = serial.Serial("/dev/ttyACM0", 115200, timeout=3)
time.sleep(2)
print("[READ] Boot log...")
data = b""
# Reset the ESP32
ser.setDTR(False)
ser.setRTS(True)
time.sleep(0.1)
ser.setRTS(False)
time.sleep(2)
for i in range(50):
    try:
        d = ser.read(200)
        if d:
            data += d
    except:
        pass
print(data.decode("utf-8", errors="replace"))
# Now send WIFI command
print("\n[SEND] WIFI:TEST:test123")
ser.write(b"WIFI:TEST:test123\n")
time.sleep(1)
print("\n[READ] Response...")
resp = b""
for i in range(20):
    try:
        d = ser.read(100)
        if d:
            resp += d
    except:
        pass
ser.close()
print(resp.decode("utf-8", errors="replace"))
if "OK" in resp.decode("utf-8", errors="replace"):
    print("\n*** SUCCESS: ESP32 received the WIFI command! ***")
else:
    print("\n*** FAIL: No OK response ***")
