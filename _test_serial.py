import serial, time
ser = serial.Serial("/dev/ttyACM0", 115200, timeout=2)
time.sleep(3)
data = b""
for i in range(30):
    try:
        d = ser.read(200)
        if d:
            data += d
    except:
        pass
ser.close()
text = data.decode("utf-8", errors="replace")
for line in text.split("\n"):
    line = line.strip()
    if "serial_cfg" in line.lower() or "wifi" in line.lower() or "listening" in line.lower() or "send" in line:
        print(line)
