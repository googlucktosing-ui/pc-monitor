import subprocess, os
os.chdir('/home/esp32/esp32/esp32-board/pc_monitor')
os.environ['IDF_PATH'] = '/home/esp32/esp32/esp-idf'
result = subprocess.run(
    ['/home/esp32/esp32/esp-idf/tools/idf.py', 'add-dependency', 'esp_websocket_client'],
    capture_output=True, text=True
)
print(result.stdout[-500:] if len(result.stdout) > 500 else result.stdout)
print(result.stderr[-500:] if len(result.stderr) > 500 else result.stderr)