import sys, serial, time
sys.stdout = open(sys.stdout.fileno(), mode="w", buffering=1)
print("START", flush=True)
ser = serial.Serial("/dev/ttyACM0", 115200, timeout=2)
print("OPENED", flush=True)
ser.setDTR(False)
ser.setRTS(True)
time.sleep(0.2)
ser.setRTS(False)
print("RESET", flush=True)
time.sleep(3)
data = ser.read(2000)
print(f"READ {len(data)} bytes", flush=True)
if data:
    print(repr(data[:300]), flush=True)
else:
    print("NO DATA", flush=True)
ser.close()
print("DONE", flush=True)
