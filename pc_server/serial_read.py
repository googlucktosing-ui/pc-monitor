import serial, time, sys
ser = serial.Serial("/dev/ttyUSB0", 115200, timeout=2)
time.sleep(0.5)
ser.write(b"\r\n")
time.sleep(2)
data = b""
for _ in range(50):
    chunk = ser.read(128)
    if not chunk:
        break
    data += chunk
ser.close()
sys.stdout.buffer.write(data)