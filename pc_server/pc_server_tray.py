#!/usr/bin/env python3



"""



PC Monitor Tray - System tray companion using pystray + Pillow.



Log: tray_debug.log (beside exe)



UDP Discovery: broadcasts on 255.255.255.255:54789 so ESP32 auto-finds PC.



"""







import sys, os, time, threading, asyncio, logging, socket, struct







# -- File logging -------------------------------------------------------



if getattr(sys, "frozen", False):



    _base_dir = os.path.dirname(os.path.abspath(sys.executable))



else:



    _base_dir = os.path.dirname(os.path.abspath(__file__))



_log_path = os.path.join(_base_dir, "tray_debug.log")



# -- Rotate log if too large ----------------------------------

try:

    if os.path.exists(_log_path) and os.path.getsize(_log_path) > 5 * 1024 * 1024:

        os.rename(_log_path, _log_path + ".old")

except: pass









_root_logger = logging.getLogger()



_root_logger.setLevel(logging.DEBUG)



_fmt = logging.Formatter("%(asctime)s [%(levelname)s] %(message)s",



                         datefmt="%H:%M:%S")



_fh = logging.FileHandler(_log_path, mode="a", encoding="utf-8")



_fh.setFormatter(_fmt)



_root_logger.addHandler(_fh)



_ch = logging.StreamHandler()



_ch.setFormatter(_fmt)



_root_logger.addHandler(_ch)







log = logging.getLogger("PcMonTray")



log.info(f"Log file: {_log_path}")







try:



    from pc_server import create_server, __version__, get_cpu_temp, _auto_start_lhm



except ImportError:



    sys.path.insert(0, _base_dir)



    from pc_server import create_server, __version__, get_cpu_temp, _auto_start_lhm







import pystray



from PIL import Image, ImageDraw, ImageFont



# -- mDNS discovery -------------------------------------------------

try:

    from zeroconf import Zeroconf, ServiceInfo

    _HAS_MDNS = True

except ImportError:

    _HAS_MDNS = False



# -- UDP query port (ESP32 sends query, PC responds) ---------------

UDP_QUERY_PORT = 54789  # same port, different packet type

UDP_QUERY_MAGIC = "PC_MONITOR_QUERY"

UDP_QUERY_RESP = "PC_MONITOR_HERE|{}|{}"





# WiFi config

try:

    from wifi_config import show_wifi_config

except ImportError:

    def show_wifi_config(parent=None): pass



# WiFi config

try:

    from wifi_config import show_wifi_config

except ImportError:

    def show_wifi_config(parent=None): pass







# -- Constants ----------------------------------------------------------



UDP_DISC_PORT = 54789          # ESP32 listens on this port for discovery



DEFAULT_PORT = 18090            # Stable PC server port for WebSocket server



UDP_DISC_MAGIC = "PC_MONITOR"











# -- Icon ---------------------------------------------------------------







def _icon_img(cc=0, running=False):



    sz = 64



    img = Image.new("RGBA", (sz, sz), (0, 0, 0, 0))



    draw = ImageDraw.Draw(img)



    color = (34, 197, 94, 230) if running else (120, 120, 120, 180)



    draw.ellipse([4, 4, 60, 60], fill=color)



    try:



        font = ImageFont.truetype("segoeui.ttf", 28)



    except Exception:



        font = ImageFont.load_default()



    text = str(min(cc, 99))



    try:



        bb = draw.textbbox((0, 0), text, font=font)



        x = (sz - (bb[2] - bb[0])) / 2



        y = (sz - (bb[3] - bb[1])) / 2 - 1



        draw.text((x, y), text, fill=(255, 255, 255), font=font)



    except Exception:



        pass



    return img











# -- Auto-start helpers ------------------------------------------------







def _autostart_path():



    return os.path.join(os.environ.get("APPDATA", ""),



                        r"Microsoft\Windows\Start Menu\Programs\Startup",



                        "PcMonitor.lnk")







def _enable_autostart():



    """Create Startup shortcut via PowerShell (more reliable)."""



    import subprocess



    lnk = _autostart_path()



    if getattr(sys, "frozen", False):



        target = sys.executable



        args = ""



        wdir = os.path.dirname(sys.executable)



    else:



        target = sys.executable



        args = os.path.abspath(__file__)



        wdir = os.path.dirname(sys.executable)



    ps = (



        '$ws = New-Object -ComObject WScript.Shell; '



        '$s = $ws.CreateShortcut("' + lnk + '"); '



        '$s.TargetPath = "' + target + '"; '



        '$s.Arguments = "' + args + '"; '



        '$s.WorkingDirectory = "' + wdir + '"; '



        '$s.WindowStyle = 7; '



        '$s.Save()'



    )



    try:



        subprocess.run(["powershell", "-NoProfile", "-Command", ps],



                       capture_output=True, timeout=10, creationflags=subprocess.CREATE_NO_WINDOW)



        import logging



        logging.getLogger("PcMonTray").info("Autostart shortcut created: " + lnk)



    except Exception as e:



        import logging



        logging.getLogger("PcMonTray").warning("Autostart creation failed: " + str(e))







def _disable_autostart():



    p = _autostart_path()



    if os.path.exists(p):



        os.remove(p)











# -- Firewall -----------------------------------------------------------

















def _add_firewall_rule():
    """Try add firewall rules in background thread. Never crashes."""
    import threading, subprocess, ctypes
    def _run():
        for name,proto,port in [("PC Monitor WS","TCP",18090),("PC Monitor UDP","UDP",54789)]:
            try:
                r = subprocess.run(["netsh","advfirewall","firewall","add","rule",
                    "name="+name,"dir=in","action=allow",
                    "protocol="+proto,"localport="+str(port),"profile=any"],
                    capture_output=True,timeout=10,encoding="utf-8",errors="replace",
                    creationflags=subprocess.CREATE_NO_WINDOW)
                if r and r.stdout and "ok" in r.stdout.strip().lower():
                    log.info("Firewall OK: "+name)
                    continue
            except: pass
            try:
                args = 'advfirewall firewall add rule name="%s" dir=in action=allow protocol=%s localport=%d profile=any' % (name,proto,port)
                ctypes.windll.shell32.ShellExecuteW(None,"runas","netsh",args,None,0)
                log.info("Elevated: "+name)
            except: pass
    threading.Thread(target=_run,daemon=True).start()
    return True

# Module-level mutex handle (keep alive to prevent GC from closing it)
_single_mutex_handle = None


# -- Status Window ------------------------------------------------------

class StatusWindow:
    """Shows status via a temporary Tk window. Safe, non-blocking."""
    def __init__(self, app):
        self._app = app
        self._root = None

    def show(self):
        """Show Tk status window in background thread so it never blocks."""
        import threading
        threading.Thread(target=self._show_impl, daemon=True, name="StatusWin").start()

    def _show_impl(self):
        try:
            import tkinter as tk
            r = self._app.is_running
            cc = self._app.client_count
            ip = getattr(self._app, "_local_ip", "?.?.?.?")
            port = self._app.port
            hostname = "N/A"
            try:
                if self._app._server and hasattr(self._app._server, "collector"):
                    hostname = self._app._server.collector.hostname
            except:
                pass
            root = tk.Tk()
            root.title("PC Monitor Status")
            root.resizable(False, False)
            root.geometry("420x260")
            root.attributes("-topmost", True)
            try: root.iconbitmap(default="")
            except: pass
            txt = tk.Text(root, width=50, height=12, font=("Consolas",10), padx=12, pady=12)
            txt.pack(fill="both", expand=True)
            status = "\u25cf Running" if r else "\u25cf Stopped"
            text = (
                f"Status:   {status}\n"
                f"Clients:  {cc}\n"
                f"Address:  ws://{ip}:{port}\n"
                f"Hostname: {hostname}\n\n"
                f"Right-click tray icon for options.\n"
                f"Port: {port}"
            )
            txt.insert("1.0", text)
            txt.configure(state="disabled")
            tk.Button(root, text="Close", command=root.destroy, width=10).pack(pady=(0,8))
            root.protocol("WM_DELETE_WINDOW", root.destroy)
            root.mainloop()
        except Exception as e:
            log.warning(f"StatusWindow: {e}")


class TrayApp:
    """System tray application with WebSocket server + mDNS + UDP discovery."""

    DISCOVERY_PORT = 54789
    DISCOVERY_INTERVAL = 2

    def __init__(self, host="0.0.0.0", port=18090, interval=1.0, use_gpu=True):
        self.host = host
        self.port = port
        self.interval = interval
        self.use_gpu = use_gpu
        self._running = False
        self._server = None
        self._thread = None
        self._stop_ev = threading.Event()
        self._icon = None
        self._status = StatusWindow(self)
        self._last_menu_state = (False, False)
        self._disc_sock = None
        self._disc_thread = None
        self._discovery_addr = None
        self._zeroconf = None
        self._mdns_service = None
        self._query_thread = None

        # Local IP
        self._local_ip = "0.0.0.0"
        try:
            s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            s.connect(("10.255.255.255", 1))
            self._local_ip = s.getsockname()[0]
            s.close()
        except Exception:
            pass

        log.info(f"PC Monitor Tray v{__version__}: ws://{host}:{port} | PC IP={self._local_ip} | "
                 f"GPU={'yes' if use_gpu else 'no'} | UDP disc port={self.DISCOVERY_PORT}")

    @property
    def is_running(self):
        return self._running

    @property
    def client_count(self):
        return self._server.client_count if self._server else 0

    @property
    def discovery_addr(self):
        return self._discovery_addr

    # -- UDP Discovery Broadcaster -------------------------------------

    def _start_discovery(self):
        try:
            self._disc_sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
            self._disc_sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
            self._disc_sock.settimeout(1)
            self._disc_thread = threading.Thread(target=self._discovery_loop, daemon=True, name="UdpDisc")
            self._disc_thread.start()
            log.info(f"UDP discovery started: {self._local_ip}:{self.port} on port {self.DISCOVERY_PORT}")
        except Exception as e:
            log.warning(f"UDP discovery init failed: {e}")

    def _discovery_loop(self):
        UDP_DISC_MAGIC = "PC_MONITOR"
        while not self._stop_ev.is_set():
            payload = f"{UDP_DISC_MAGIC}|{self._local_ip}|{self.port}"
            try:
                self._disc_sock.sendto(payload.encode("utf-8"), ("255.255.255.255", self.DISCOVERY_PORT))
                self._discovery_addr = f"{self._local_ip}:{self.port}"
                self._sync_icon()
                time.sleep(self.DISCOVERY_INTERVAL)
            except socket.timeout:
                continue
            except OSError as e:
                if not self._stop_ev.is_set():
                    log.warning(f"UDP broadcast error: {e}")
                    time.sleep(5)
            except Exception:
                time.sleep(5)

    def _stop_discovery(self):
        if self._disc_sock:
            try: self._disc_sock.close()
            except: pass
            self._disc_sock = None

    # -- mDNS Service --------------------------------------------------

    def _start_mdns(self):
        try:
            from zeroconf import Zeroconf, ServiceInfo
            self._mdns_service = ServiceInfo(
                "_http._tcp.local.",
                "pc-monitor._http._tcp.local.",
                addresses=[socket.inet_aton(self._local_ip)],
                port=self.port,
                properties={"path": "/ws"},
                server="pc-monitor.local.",
            )
            self._zeroconf = Zeroconf()
            self._zeroconf.register_service(self._mdns_service)
            log.info(f"mDNS: pc-monitor.local -> {self._local_ip}:{self.port}")
        except ImportError:
            log.warning("mDNS: zeroconf not available, skip")
        except Exception as e:
            log.warning(f"mDNS: {e}")
            self._zeroconf = None

    def _stop_mdns(self):
        if hasattr(self, "_zeroconf") and self._zeroconf:
            try:
                self._zeroconf.unregister_service(self._mdns_service)
                self._zeroconf.close()
            except: pass
            self._zeroconf = None

    # -- UDP Query Listener --------------------------------------------

    def _start_query_listener(self):
        UDP_QUERY_MAGIC = "PC_MONITOR_QUERY"
        UDP_QUERY_RESP = "PC_MONITOR_HERE|{}|{}"

        def query_listen():
            sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
            sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            sock.setsockopt(socket.SOL_SOCKET, socket.SO_BROADCAST, 1)
            sock.settimeout(1.0)
            try:
                sock.bind(("0.0.0.0", self.DISCOVERY_PORT))
            except Exception as e:
                log.warning(f"Query listener bind: {e}")
                sock.close()
                return
            log.info(f"UDP query listener on port {self.DISCOVERY_PORT}")
            while not self._stop_ev.is_set():
                try:
                    data, addr = sock.recvfrom(256)
                    msg = data.decode("utf-8", errors="replace").strip()
                    if msg.startswith(UDP_QUERY_MAGIC):
                        log.info(f"Query from {addr[0]}:{addr[1]}")
                        resp = UDP_QUERY_RESP.format(self._local_ip, self.port)
                        sock.sendto(resp.encode("utf-8"), (addr[0], self.DISCOVERY_PORT))
                        log.info(f"Responded: {self._local_ip}:{self.port}")
                except socket.timeout:
                    continue
                except Exception as e:
                    if not self._stop_ev.is_set():
                        log.warning(f"Query listener: {e}")
            sock.close()

        self._query_thread = threading.Thread(target=query_listen, daemon=True, name="UdpQuery")
        self._query_thread.start()

    # -- Firewall -------------------------------------------------------

    def add_firewall_rule(self):
        ok = _add_firewall_rule()
        if not ok:
            log.warning("Firewall manual rule may need admin")

    # -- Server ---------------------------------------------------------

    def _on_client_change(self, count):
        self._rebuild_menu()

    def _find_free_port(self, base, max_tries=20):
        import socket as _sock
        _s = _sock.socket(_sock.AF_INET, _sock.SOCK_STREAM)
        _s.setsockopt(_sock.SOL_SOCKET, _sock.SO_REUSEADDR, 1)
        try:
            _s.bind(("0.0.0.0", base))
            _s.close()
            return base
        except OSError:
            _s.close()
        for p in range(base, base + max_tries):
            _s = _sock.socket(_sock.AF_INET, _sock.SOCK_STREAM)
            _s.setsockopt(_sock.SOL_SOCKET, _sock.SO_REUSEADDR, 1)
            try:
                _s.bind(("0.0.0.0", p))
                _s.close()
                return p
            except OSError:
                _s.close()
                continue
        return None

    def _server_thread(self):
        actual = self._find_free_port(self.port)
        if actual is None:
            log.error(f"Ports {self.port}-{self.port+19} unavailable!")
            self._running = False
            return
        if actual != self.port:
            log.info(f"Port {self.port} busy, using {actual}")
            self.port = actual
        self._running = True
        loop = asyncio.new_event_loop()
        asyncio.set_event_loop(loop)
        try:
            from pc_server import create_server
            self._server = create_server(host=self.host, port=self.port,
                interval=self.interval, use_gpu=self.use_gpu,
                on_client_change=self._on_client_change)
            self._running = True
            loop.run_until_complete(self._server.start())
        except Exception as e:
            log.error(f"Server error: {e}")
        finally:
            loop.close()
            self._running = False

    def start_server(self):
        if self._running:
            log.info("start_server: already running")
            return
        self._stop_ev.clear()
        self._thread = threading.Thread(target=self._server_thread, daemon=True, name="PcMonServer")
        self._thread.start()
        for _ in range(25):
            time.sleep(0.2)
            if self._running:
                break
        else:
            log.warning("Server may not have started yet")
        self._sync_icon()
        self._rebuild_menu()
        log.info(f"WebSocket server: ws://{self._local_ip}:{self.port}")

    def stop_server(self):
        if not self._running:
            return
        if self._server:
            self._server.stop()
        self._running = False
        self._stop_ev.set()
        self._server = None
        self._sync_icon()
        self._rebuild_menu()
        log.info("Server stopped")

    def restart_server(self):
        self.stop_server()
        time.sleep(0.5)
        self.start_server()

    # -- LHM auto-start -------------------------------------------------

    @staticmethod
    def _auto_start_lhm_bg():
        import time
        time.sleep(3)
        # Periodic retry: try every 30s for 3 minutes
        for attempt in range(6):
            try:
                from pc_server import _auto_start_lhm
                if _auto_start_lhm():
                    return
            except Exception as e:
                pass
            time.sleep(30)


    # -- Tray menu ------------------------------------------------------

    def _build_menu(self):
        import pystray
        r = self._running
        cc = self.client_count
        if r and cc > 0:
            status = "\u25cf Running"
            status_en = True
        elif r:
            status = "\u25cb Waiting"
            status_en = False
        else:
            status = "\u25cb Stopped"
            status_en = False

        return pystray.Menu(
            pystray.MenuItem(status, None, enabled=status_en),
            pystray.Menu.SEPARATOR,
            pystray.MenuItem("Start Server", self.start_server, enabled=(not r)),
            pystray.MenuItem("Stop Server", self.stop_server, enabled=r),
            pystray.MenuItem("Restart", self.restart_server),
            pystray.Menu.SEPARATOR,
            pystray.MenuItem("Status Window", lambda: self._status.show()),
            pystray.Menu.SEPARATOR,
            pystray.MenuItem("Add Firewall Rule", self.add_firewall_rule),
            pystray.MenuItem("Auto-start", self._toggle_autostart,
                checked=lambda _: os.path.exists(_autostart_path())),
            pystray.Menu.SEPARATOR,
            pystray.MenuItem("WiFi Config for ESP32",
                lambda: self._show_wifi_config()),
            pystray.Menu.SEPARATOR,
            pystray.MenuItem(f"v{__version__}  |  {self._local_ip}", None, enabled=False),
            pystray.MenuItem("Quit", self._quit),
        )

    def _show_wifi_config(self):
        try:
            from wifi_config import show_wifi_config
            show_wifi_config(None)
        except: pass

    def _sync_icon(self):
        if not self._icon:
            return
        try:
            r = self._running
            cc = self.client_count
            self._icon.icon = self._icon_img(cc, r)
            self._icon.title = (
                f"PC Monitor\\nClients: {cc}\\nIP: {self._local_ip}"
                if r else "PC Monitor\\nStopped")
            self._rebuild_menu()
        except Exception as e:
            log.warning(f"_sync_icon: {e}")

    def _rebuild_menu(self):
        if self._icon:
            try:
                r = self._running
                cc = self.client_count
                self._last_menu_state = (r, cc > 0)
                self._icon.menu = self._build_menu()
            except Exception as e:
                log.warning(f"_rebuild_menu: {e}")

    @staticmethod
    def _icon_img(cc=0, running=False):
        from PIL import Image, ImageDraw, ImageFont
        sz = 64
        img = Image.new("RGBA", (sz, sz), (0,0,0,0))
        draw = ImageDraw.Draw(img)
        color = (34,197,94,230) if running else (120,120,120,180)
        draw.ellipse([4,4,60,60], fill=color)
        try:
            font = ImageFont.truetype("segoeui.ttf", 28)
        except:
            font = ImageFont.load_default()
        text = str(min(cc,99))
        try:
            bb = draw.textbbox((0,0), text, font=font)
            x = (sz - (bb[2]-bb[0])) / 2
            y = (sz - (bb[3]-bb[1])) / 2 - 1
            draw.text((x,y), text, fill=(255,255,255), font=font)
        except: pass
        return img

    # -- Autostart ------------------------------------------------------

    def _toggle_autostart(self):
        if os.path.exists(_autostart_path()):
            _disable_autostart()
            log.info("Autostart disabled")
        else:
            _enable_autostart()
            log.info("Autostart enabled")

    def _quit(self):
        log.info("Quit")
        self.stop_server()
        self._stop_discovery()
        self._stop_mdns()
        if self._icon:
            try: self._icon.stop()
            except: pass

    # -- Run ------------------------------------------------------------

    def run(self):
        log.info("Starting PC Monitor Tray")
        _add_firewall_rule()
        self._start_discovery()
        self._start_mdns()
        self._start_query_listener()
        self.start_server()
        threading.Thread(target=self._auto_start_lhm_bg, daemon=True, name="LhmBg").start()

        self._icon = pystray.Icon(
            "pc_monitor", self._icon_img(0, True), "PC Monitor",
            menu=self._build_menu(),
        )
        try:
            log.info("pystray icon.run() entering message loop...")
            self._icon.run()
        except Exception as e:
            log.error(f"icon.run() failed: {e}", exc_info=True)
            raise

def check_single_instance():
    """Ensure only one instance. Uses named mutex."""
    global _single_mutex_handle
    import ctypes, sys
    pid = os.getpid()
    try:
        kernel32 = ctypes.windll.kernel32
        _single_mutex_handle = kernel32.CreateMutexW(None, False, "Global\\PcMonitorTray")
        err = kernel32.GetLastError()
        log.info(f"Mutex PID={pid} err={err}")
        if err == 183:
            log.info("Another instance already running, exiting")
            sys.exit(0)
        # Lock file removed - mutex is sufficient
    except Exception as e:
        log.warning(f"Mutex error: {e}")





def main():

    check_single_instance()

    import argparse



    p = argparse.ArgumentParser(description="PC Monitor Tray App")



    p.add_argument("--port", type=int, default=18090)



    p.add_argument("--host", default="0.0.0.0")



    p.add_argument("--interval", type=float, default=1.0)



    p.add_argument("--no-gpu", action="store_true")



    p.add_argument("--debug", action="store_true")



    a = p.parse_args()



    if a.debug:



        _root_logger.setLevel(logging.DEBUG)



        for h in _root_logger.handlers:



            h.setLevel(logging.DEBUG)



        log.debug("Debug mode enabled")



    try:
        TrayApp(host=a.host, port=a.port, interval=a.interval,
                use_gpu=not a.no_gpu).run()
    except Exception as e:
        log.error(f"TrayApp failed: {e}", exc_info=True)
        sys.exit(1)







if __name__ == "__main__":
    try:
        import six  # noqa: F401
    except ImportError:
        log.warning("six not found")
    try:
        main()
    except Exception as e:
        log.error(f"main() failed: {e}", exc_info=True)































