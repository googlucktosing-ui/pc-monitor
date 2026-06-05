import serial, time
ser = serial.Serial("/dev/ttyACM0", 115200, timeout=2)
ser.setDTR(False)
ser.setRTS(True)
time.sleep(0.2)
ser.setRTS(False)
time.sleep(3)
data = ser.read(2000)
if data:
    print(repr(data[:500]))
else:
    print("NO DATA")
ser.close()
