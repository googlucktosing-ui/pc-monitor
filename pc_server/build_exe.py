import os, sys, shutil

DIST_DIR = os.path.join(os.path.dirname(__file__), "dist")
BUILD_DIR = os.path.join(os.path.dirname(__file__), "build")

def build(console=False):
    import PyInstaller.__main__
    args = [
        "--name=pc_monitor",
        "--onefile",
        "--noconsole", "--windowed",
        "--distpath=" + DIST_DIR,
        "--workpath=" + BUILD_DIR,
        "--add-data=pc_server.py;.",
        "--add-data=lhm.zip;.",
        "--add-data=wifi_config.py;.",
        "--hidden-import=pystray",
        "--hidden-import=pystray._win32",
        "--hidden-import=pystray._base",
        "--hidden-import=pystray._util",
        "--hidden-import=pystray._util.win32",
        "--hidden-import=six",
        "--hidden-import=six.moves",
        "--hidden-import=websockets",
        "--hidden-import=psutil",
        "--hidden-import=PIL",
        "--hidden-import=GPUtil",
        "--hidden-import=serial",
        "--hidden-import=serial.tools.list_ports",
        "--hidden-import=zeroconf",
        "--hidden-import=ifaddr",
    ]
    if console:
        args.remove("--noconsole")
        args.remove("--windowed")
        args.append("--console")
        print("Building console version (debug)...")
    else:
        print("Building silent tray version (no black box)...")
    args.append("pc_server_tray.py")
    PyInstaller.__main__.run(args)
    exe = os.path.join(DIST_DIR, "pc_monitor.exe")
    if os.path.exists(exe):
        mb = os.path.getsize(exe) / 1024 / 1024
        print("Done: %s  Size: %.1f MB" % (exe, mb))
    return exe

if __name__ == "__main__":
    os.chdir(os.path.dirname(os.path.abspath(__file__)))
    build()
