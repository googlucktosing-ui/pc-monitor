#!/usr/bin/env python3



"""



PC Monitor Server -?WebSocket server pushing PC system data to ESP32.







Usage:



    python pc_server.py                    # default port 9090



    python pc_server.py --port 9000        # custom port



    python pc_server.py --no-gpu           # disable GPU collection







As a module:



    from pc_server import create_server, run_server



    server = create_server(port=18090)



    run_server(server)



"""







import sys



import json



import time



import psutil



import platform



import argparse



import asyncio



import logging



import socket







import os as _os
_version_file = _os.path.join(_os.path.dirname(_os.path.abspath(__file__)), "..", "VERSION")
try:
    with open(_version_file) as _f:
        __version__ = _f.read().strip()
except Exception:
    __version__ = "1.0.0"
del _os, _version_file, _f







_HAS_GPU = False



try:



    import GPUtil



    _HAS_GPU = True



except ImportError:



    pass











def get_cpu_temp():
    """Return CPU temperature in Celsius. Tries multiple methods, returns 0 if unavailable."""
    import subprocess, platform as _platform

    # --- Method 1: MSAcpi_ThermalZoneTemperature (laptops) ---
    try:
        if _platform.system() == "Windows":
            r = subprocess.run(
                ["wmic", "/namespace:\\root\\wmi", "PATH", "MSAcpi_ThermalZoneTemperature",
                 "get", "CurrentTemperature"],
                capture_output=True, text=True, timeout=3,
                creationflags=subprocess.CREATE_NO_WINDOW)
            for line in r.stdout.strip().splitlines():
                line = line.strip()
                if line.isdigit():
                    val = int(line) / 10.0 - 273.15
                    if val > 0:
                        return round(val, 1)
    except Exception:
        pass

    # --- Method 2: Win32_TemperatureProbe ---
    try:
        if _platform.system() == "Windows":
            r = subprocess.run(
                ["wmic", "PATH", "Win32_TemperatureProbe", "get", "CurrentReading"],
                capture_output=True, text=True, timeout=3,
                creationflags=subprocess.CREATE_NO_WINDOW)
            for line in r.stdout.strip().splitlines():
                line = line.strip()
                if line.isdigit():
                    val = int(line) / 10.0
                    if val > 0:
                        return round(val, 1)
    except Exception:
        pass

    # --- Method 3: LibreHardwareMonitor WMI ---
    try:
        import win32com.client
        wmi = win32com.client.GetObject("winmgmts:{impersonationLevel=impersonate}!\\root\\LibreHardwareMonitor")
        sensors = wmi.ExecQuery("SELECT * FROM Sensor WHERE SensorType = 'Temperature'")
        best = 0.0
        for s in sensors:
            val = float(s.Value)
            if val <= 0:
                continue
            name = str(s.Name)
            ident = str(s.Identifier)
            # Priority: Tctl/Tdie (AMD) or Package (Intel) = package temp
            if "Tctl" in name or "Tdie" in name or "Package" in name:
                return round(val, 1)
            # Any core / CPU path sensor as fallback
            if "Core" in name or "/amdcpu/" in ident or "/intelcpu/" in ident:
                if val > best:
                    best = val
        if best > 0:
            return round(best, 1)
        # Last resort: first valid temp
        for s in sensors:
            val = float(s.Value)
            if val > 0:
                return round(val, 1)
    except Exception:
        pass

    # --- Method 4: Linux sensors ---
    try:
        if _platform.system() == "Linux":
            import psutil
            temps = psutil.sensors_temperatures()
            for name, entries in temps.items():
                if entries:
                    return round(entries[0].current, 1)
    except Exception:
        pass

    return 0.0




def _auto_start_lhm():
    """Auto-extract and start LibreHardwareMonitor if CPU temp unavailable."""
    import os, sys, subprocess, time, logging, zipfile, shutil

    if getattr(sys, "frozen", False):
        base = os.path.dirname(sys.executable)
        bundle_dir = sys._MEIPASS
    else:
        base = os.path.dirname(os.path.abspath(__file__))
        bundle_dir = base

    lhm_dir = os.path.join(base, "LibreHardwareMonitor")
    exe_path = os.path.join(lhm_dir, "LibreHardwareMonitor.exe")

    if not os.path.isfile(exe_path):
        zip_src = os.path.join(bundle_dir, "lhm.zip")
        if os.path.isfile(zip_src):
            log = logging.getLogger("PcMon")
            log.info("Extracting LibreHardwareMonitor...")
            try:
                if os.path.isdir(lhm_dir):
                    shutil.rmtree(lhm_dir, ignore_errors=True)
                os.makedirs(lhm_dir, exist_ok=True)
                with zipfile.ZipFile(zip_src, "r") as z:
                    z.extractall(lhm_dir)
            except Exception as e:
                log.warning(f"Extract LHM failed: {e}")
                return False
        else:
            return False

    log = logging.getLogger("PcMon")
    try:
        import win32com.client
        wmi = win32com.client.GetObject("winmgmts:{impersonationLevel=impersonate}!\\root\\LibreHardwareMonitor")
        _ = wmi.ExecQuery("SELECT * FROM Sensor")
        log.info("LHM already running")
        return True
    except Exception:
        pass

    log.info("Starting LibreHardwareMonitor...")
    try:
        subprocess.Popen(
            [exe_path],
            stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL,
            creationflags=subprocess.CREATE_NO_WINDOW,
        )
        for _ in range(15):
            time.sleep(1)
            try:
                wmi = win32com.client.GetObject("winmgmts:{impersonationLevel=impersonate}!\\root\\LibreHardwareMonitor")
                _ = wmi.ExecQuery("SELECT * FROM Sensor")
                log.info("LHM started, CPU temp available")
                return True
            except Exception:
                continue
    except Exception as e:
        log.warning(f"Failed to start LHM: {e}")
    return False

def get_gpu():
    try:
        if _HAS_GPU:
            import GPUtil
            gpus = GPUtil.getGPUs()
            if gpus:
                g = gpus[0]
                return (g.load * 100, g.temperature, g.name)
    except Exception:
        pass
    return (0.0, 0.0, "")

class Collector:
    def __init__(self):



        self.hostname = socket.gethostname()



        self.os_info = f"{platform.system()} {platform.release()}"



        self.cpu_name = self._cpu_name()



        self.gpu_name = ""



        self._pn = psutil.net_io_counters()



        self._pt = time.time()







    @staticmethod



    def _cpu_name():



        try:



            if platform.system() == "Windows":



                import subprocess



                r = subprocess.run(



                    ["wmic", "cpu", "get", "name"],



                    capture_output=True, text=True, timeout=3,
                    creationflags=subprocess.CREATE_NO_WINDOW,



                )



                lines = r.stdout.strip().splitlines()



                return lines[1].strip() if len(lines) > 1 else ""



            if platform.system() == "Linux":



                with open("/proc/cpuinfo") as f:



                    for l in f:



                        if l.startswith("model name"):



                            return l.split(":")[1].strip()



            if platform.system() == "Darwin":



                import subprocess



                r = subprocess.run(



                    ["sysctl", "-n", "machdep.cpu.brand_string"],



                    capture_output=True, text=True, timeout=3,
                    creationflags=subprocess.CREATE_NO_WINDOW,



                )



                return r.stdout.strip()



        except Exception:



            return ""

    @staticmethod
    def get_gpu():
        try:
            if _HAS_GPU:
                import GPUtil
                gpus = GPUtil.getGPUs()
                if gpus:
                    g = gpus[0]
                    return (g.load * 100, g.temperature, g.name)
        except Exception:
            pass
        return (0.0, 0.0, "")

    def collect(self, use_gpu=True):



        cpu = psutil.cpu_percent(interval=None)



        m = psutil.virtual_memory()



        mu, mt = m.used / 1073741824, m.total / 1073741824



        d = psutil.disk_usage("C:" if platform.system() == "Windows" else "/")



        du, dt = d.used / 1073741824, d.total / 1073741824







        now = time.time()



        n = psutil.net_io_counters()



        ds = now - self._pt



        nu = ((n.bytes_sent - self._pn.bytes_sent) / ds / 1024) if ds > 0 else 0.0



        nd = ((n.bytes_recv - self._pn.bytes_recv) / ds / 1024) if ds > 0 else 0.0



        self._pn, self._pt = n, now







        gu = gt = 0.0



        gn = self.gpu_name



        if use_gpu:



            gu, gt, gn2 = get_gpu()



            if gn2 and not self.gpu_name:



                self.gpu_name = gn2



                gn = gn2







        return {



            "cpu": round(cpu, 1),



            "gpu": round(gu, 1),



            "gpu_temp": round(gt, 1),



            "mem_used": round(mu, 2),



            "mem_total": round(mt, 2),



            "disk_used": round(du, 2),



            "disk_total": round(dt, 2),



            "net_up": round(nu, 1),



            "net_down": round(nd, 1),



            "hostname": self.hostname,



            "cpu_name": self.cpu_name,



            "gpu_name": gn,



            "cpu_temp": round(get_cpu_temp(), 1),
            "os_info": self.os_info,



        }











class Server:



    """WebSocket server broadcasting system data to all connected clients."""







    def __init__(self, host="0.0.0.0", port=18090, interval=1.0, use_gpu=True, on_client_change=None):



        self.host = host



        self.port = port



        self.interval = interval



        self.use_gpu = use_gpu



        self.collector = Collector()



        self.clients = set()



        self.log = logging.getLogger("PcMon")



        self._running = False



        self._on_client_change = on_client_change







    @property



    def client_count(self):



        return len(self.clients)







    async def handler(self, ws):



        addr = ws.remote_address



        self.log.info(f"Client connected: {addr}")



        self.clients.add(ws)



        if self._on_client_change:



            self._on_client_change(len(self.clients))



        try:



            async for _ in ws:



                pass



        except Exception:



            pass



        finally:



            self.clients.discard(ws)



            self.log.info(f"Client disconnected: {addr}")



            if self._on_client_change:



                self._on_client_change(len(self.clients))







    async def broadcast(self, data):



        if not self.clients:



            return



        msg = json.dumps(data)



        dead = set()



        for ws in self.clients:



            try:



                await ws.send(msg)



            except Exception:



                dead.add(ws)



        self.clients -= dead







    async def run(self):



        self._running = True



        self.log.info(f"Collection started (GPU: {'yes' if self.use_gpu else 'no'})")



        while self._running:



            try:



                data = self.collector.collect(use_gpu=self.use_gpu)



                await self.broadcast(data)



            except Exception as e:



                self.log.warning(f"Collection error: {e}")



            await asyncio.sleep(self.interval)



        self.log.info("Collection stopped")







    def stop(self):



        self._running = False







    async def start(self):



        import websockets



        gs = "enabled" if self.use_gpu else "disabled"



        self.log.info(f"PC Monitor Server v{__version__}")



        self.log.info(f"  ws://{self.host}:{self.port}")



        self.log.info(f"  Interval: {self.interval}s | GPU: {gs}")



        self.log.info(f"  Press Ctrl+C to stop")



        async with websockets.serve(self.handler, self.host, self.port) as s:



            for sock in s.sockets:



                actual = sock.getsockname()[1]



                if actual != self.port:



                    self.port = actual



                    self.log.info(f"  Actual port: {actual}")



                break



            await self.run()











# -- Convenience factory functions --







def create_server(host="0.0.0.0", port=18090, interval=1.0, use_gpu=True, debug=False, on_client_change=None):



    """Create a configured Server instance."""



    server = Server(host=host, port=port, interval=interval, use_gpu=use_gpu, on_client_change=on_client_change)



    logging.basicConfig(



        level=logging.DEBUG if debug else logging.INFO,



        format="%(asctime)s [%(levelname)s] %(message)s",



        datefmt="%H:%M:%S",



    )



    return server











def run_server(server):



    """Run a Server instance (blocking)."""



    try:



        asyncio.run(server.start())



    except KeyboardInterrupt:



        print()











# -- CLI entry point --







def main():



    p = argparse.ArgumentParser(description="PC Monitor Server v1.0")



    p.add_argument("--port", type=int, default=18090, help="Port (default: 18090)")



    p.add_argument("--host", default="0.0.0.0", help="Bind address")



    p.add_argument("--interval", type=float, default=1.0, help="Push interval (s)")



    p.add_argument("--no-gpu", action="store_true", help="Disable GPU collection")



    p.add_argument("--debug", action="store_true", help="Debug logging")



    a = p.parse_args()



    server = create_server(



        host=a.host, port=a.port, interval=a.interval,



        use_gpu=not a.no_gpu, debug=a.debug,



    )



    run_server(server)











if __name__ == "__main__":



    try:



        import websockets



    except ImportError:



        print("Missing dependencies. Run: pip install websockets psutil")



        sys.exit(1)



    main()



