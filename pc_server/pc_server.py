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
# cleanup: _os, _version_file, _f go out of scope







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
                capture_output=True, timeout=3, encoding="utf-8", errors="replace",
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
                capture_output=True, timeout=3, encoding="utf-8", errors="replace",
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
    import os, sys, subprocess, time, logging, zipfile, shutil, ctypes

    if getattr(sys, "frozen", False):
        base = os.path.dirname(sys.executable)
        bundle_dir = sys._MEIPASS
    else:
        base = os.path.dirname(os.path.abspath(__file__))
        bundle_dir = base

    lhm_dir = os.path.join(base, "LibreHardwareMonitor")
    exe_path = os.path.join(lhm_dir, "LibreHardwareMonitor.exe")

    # Extract if needed
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

    # Helper: check if LHM process is actually running
    def _lhm_running():
        try:
            r = subprocess.run(["tasklist","/FI","IMAGENAME eq LibreHardwareMonitor.exe","/FO","CSV"],
                capture_output=True,timeout=3,encoding="utf-8",errors="replace",
                creationflags=subprocess.CREATE_NO_WINDOW)
            return "LibreHardwareMonitor" in r.stdout
        except:
            return False

    # Helper: check if LHM WMI actually returns sensors
    def _lhm_wmi_works():
        try:
            import win32com.client
            wmi = win32com.client.GetObject(
                "winmgmts:{impersonationLevel=impersonate}!\\root\\LibreHardwareMonitor")
            sensors = wmi.ExecQuery("SELECT * FROM Sensor")
            for _ in sensors:
                return True  # at least one sensor
            return False
        except:
            return False

    # Check if LHM is truly running (process + WMI)
    if _lhm_running() and _lhm_wmi_works():
        log.info("LHM already running with valid sensors")
        return True

    # If namespace exists but no process, clean stale registration
    if not _lhm_running():
        # Try to remove stale WMI registration
        try:
            subprocess.run(["winmgmt","/verifyrepository"],
                capture_output=True,timeout=5,encoding="utf-8",errors="replace",
                creationflags=subprocess.CREATE_NO_WINDOW)
        except:
            pass

    # Start LHM - go straight to elevated since normal always needs admin
    log.info("Starting LibreHardwareMonitor (elevated)...")
    try:
        ctypes.windll.shell32.ShellExecuteW(
            None, "runas", exe_path, None, None, 0)
        # Wait up to 15s for LHM to start and register WMI
        for i in range(15):
            time.sleep(1)
            if _lhm_running() and _lhm_wmi_works():
                log.info(f"LHM started, CPU temp available (took {i+1}s)")
                return True
        log.info("LHM process started but WMI not ready yet")
        return True  # Still return True - process started, WMI will come
    except Exception as e:
        log.warning(f"Elevated start failed: {e}")

    log.warning("Failed to get LHM temperature sensors")
    return False

def _gpu_from_wmi():
    """Layer 1: Windows内置WMI (零依赖, Win10/11自带)"""
    try:
        import win32com.client
        from collections import defaultdict
        wmi = win32com.client.GetObject('winmgmts:{impersonationLevel=impersonate}!\\\\.\\root\\cimv2')
        # GPU利用率
        usage = 0.0
        engines = wmi.ExecQuery('SELECT * FROM Win32_PerfFormattedData_GPUPerformanceCounters_GPUEngine')
        phys_gpus = defaultdict(float)
        for e in engines:
            parts = e.Name.split('_')
            phys_idx = ''
            for i, p in enumerate(parts):
                if p == 'phys' and i+1 < len(parts):
                    phys_idx = parts[i+1]
            val = float(e.UtilizationPercentage) if e.UtilizationPercentage else 0.0
            key = f'GPU{phys_idx}'
            phys_gpus[key] = max(phys_gpus[key], val)
        if phys_gpus:
            usage = max(phys_gpus.values())
        # GPU型号
        name = ''
        for vc in wmi.ExecQuery('SELECT * FROM Win32_VideoController'):
            if vc.Name and 'virtual' not in vc.Name.lower():
                name = vc.Name
                break
        return (usage, 0.0, name)  # 温度需要在LHM中获取
    except Exception:
        return None

def _gpu_from_lhm():
    """Layer 2: LibreHardwareMonitor WMI (温度+详细信息)"""
    try:
        import win32com.client
        wmi = win32com.client.GetObject('winmgmts:{impersonationLevel=impersonate}!\\\\.\\root\\LibreHardwareMonitor')
        gt, gl, gn = 0.0, 0.0, ''
        for s in wmi.ExecQuery('SELECT * FROM Sensor WHERE SensorType="Temperature"'):
            if s.Name in ('GFX', 'GPU Core', 'GPU Temperature'):
                gt = float(s.Value)
                p = s.Parent.split('/')
                if len(p) >= 2: gn = p[1]
        for s in wmi.ExecQuery('SELECT * FROM Sensor WHERE SensorType="Load"'):
            if s.Name in ('GFX', 'GPU Core', 'GPU Core'):
                gl = float(s.Value)
        if gt > 0 or gl > 0:
            return (gl, gt, gn)
    except Exception:
        pass
    return None

def get_gpu():
    """三层回退: GPUtil(独显) -> LHM(温度+负载) -> Windows WMI(利用率+型号)"""
    lhm_data = None
    wmi_data = None
    # Layer 1: GPUtil (独显, 全数据)
    try:
        if _HAS_GPU:
            import GPUtil
            gpus = GPUtil.getGPUs()
            if gpus:
                g = gpus[0]
                return (g.load * 100, g.temperature, g.name)
    except Exception:
        pass
    # Layer 2: LibreHardwareMonitor WMI (温度+负载, 集显/独显都支持)
    try:
        lhm_data = _gpu_from_lhm()
    except Exception:
        pass
    # Layer 3: Windows内置WMI (利用率+型号, 零依赖Win10/11)
    try:
        wmi_data = _gpu_from_wmi()
    except Exception:
        pass
    # 合并各层数据
    usage = 0.0
    temp = 0.0
    name = ''
    if lhm_data:
        usage = lhm_data[0] if lhm_data[0] > 0 else (wmi_data[0] if wmi_data else 0.0)
        temp = lhm_data[1]
        name = lhm_data[2] if lhm_data[2] else (wmi_data[2] if wmi_data else '')
    elif wmi_data:
        usage = wmi_data[0]
        name = wmi_data[2]
    return (usage, temp, name)

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



                    capture_output=True, timeout=3, encoding="utf-8", errors="replace",
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



                    capture_output=True, timeout=3, encoding="utf-8", errors="replace",
                    creationflags=subprocess.CREATE_NO_WINDOW,



                )



                return r.stdout.strip()



        except Exception:



            return ""

    @staticmethod
    def get_gpu():
        lhm_data = None
        wmi_data = None
        try:
            if _HAS_GPU:
                import GPUtil
                gpus = GPUtil.getGPUs()
                if gpus:
                    g = gpus[0]
                    return (g.load * 100, g.temperature, g.name)
        except Exception:
            pass
        try:
            lhm_data = _gpu_from_lhm()
        except Exception:
            pass
        try:
            wmi_data = _gpu_from_wmi()
        except Exception:
            pass
        usage, temp, name = 0.0, 0.0, ''
        if lhm_data:
            usage = lhm_data[0] if lhm_data[0] > 0 else (wmi_data[0] if wmi_data else 0.0)
            temp = lhm_data[1]
            name = lhm_data[2] if lhm_data[2] else (wmi_data[2] if wmi_data else '')
        elif wmi_data:
            usage = wmi_data[0]
            name = wmi_data[2]
        return (usage, temp, name)



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







    

    async def send_cmd(self, cmd, value):
        """Send a command to all connected clients."""
        if not self.clients:
            return
        data = {"cmd": cmd, "value": value}
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



