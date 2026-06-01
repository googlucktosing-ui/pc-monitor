import serial, time, sys
try:
    ser = serial.Serial("/dev/ttyUSB0", 115200, timeout=2)
    ser.setDTR(False)
    time.sleep(0.1)
    ser.setDTR(True)
    time.sleep(3)
    data = b""
    for _ in range(100):
        chunk = ser.read(256)
        if not chunk:
            break
        data += chunk
    ser.close()
    sys.stdout.buffer.write(data)
except Exception as e:
    print(f"Error: {e}")