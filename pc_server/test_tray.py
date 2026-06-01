import pystray, threading, time, ctypes, ctypes.wintypes
from PIL import Image, ImageDraw

img = Image.new("RGBA", (64,64), (0,0,0,0))
d = ImageDraw.Draw(img)
d.ellipse([4,4,60,60], fill=(34,197,94,230))
d.text((16,14), "PC", fill=(255,255,255))

icon = pystray.Icon("pcmon", img, "PC Monitor")
icon.menu = pystray.Menu(pystray.MenuItem("Quit", lambda ic: ic.stop()))
t = threading.Thread(target=icon.run, daemon=True)
t.start()
time.sleep(3)

# Check for pystray window
EnumWindows = ctypes.windll.user32.EnumWindows
WNDENUMPROC = ctypes.WINFUNCTYPE(ctypes.c_bool, ctypes.c_int, ctypes.c_int)
found = []
def enum(hwnd, lparam):
    cls = ctypes.create_unicode_buffer(256)
    ctypes.windll.user32.GetClassNameW(hwnd, cls, 256)
    if "SystemTrayIcon" in cls.value:
        found.append(f"hwnd=0x{hwnd:x} class=[{cls.value}]")
    return True
EnumWindows(WNDENUMPROC(enum), 0)
if found:
    print("TRAY ICON WINDOW FOUND!")
    for f in found: print(f)
else:
    print("NO TRAY ICON WINDOW")
    print("Check if pystray created any windows...")
    def enum2(hwnd, lparam):
        cls = ctypes.create_unicode_buffer(256)
        ctypes.windll.user32.GetClassNameW(hwnd, cls, 256)
        txt = ctypes.create_unicode_buffer(256)
        ctypes.windll.user32.GetWindowTextW(hwnd, txt, 256)
        if "pcmon" in cls.value.lower() or "pystray" in cls.value.lower() or "PC Monitor" in txt.value:
            found.append(f"hwnd=0x{hwnd:x} class=[{cls.value}] title=[{txt.value}]")
        return True
    EnumWindows(WNDENUMPROC(enum2), 0)
    for f in found: print(f)
    if not found:
        print("(no matching windows)")

icon.stop()