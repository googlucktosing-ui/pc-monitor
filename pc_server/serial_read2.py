import serial, time, sys
try:
    ser = serial.Serial("/dev/ttyUSB0", 115200, timeout=1)
    ser.setDTR(False)
    time.sleep(0.1)
    ser.setDTR(True)
    time.sleep(0.5)
    data = ser.read(500)
    ser.close()
    sys.stdout.buffer.write(data)
except Exception as e:
    print(f"Error: {e}")