"""
PC Monitor Server — Console version (reliable, no tray dependency).
Shows live stats in the console window.
"""

import sys
import os
import time
import threading
import asyncio
import logging

try:
    from pc_server import create_server, __version__
except ImportError:
    sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
    from pc_server import create_server, __version__

log = logging.getLogger("PcMon")


class ConsoleApp:
    def __init__(self, host="0.0.0.0", port=9090, interval=1.0, use_gpu=True):
        self.host = host
        self.port = port
        self.interval = interval
        self.use_gpu = use_gpu
        self._server = None
        self._running = False
        self._stop_ev = threading.Event()

    @property
    def client_count(self):
        return self._server.client_count if self._server else 0

    def _run_async(self):
        loop = asyncio.new_event_loop()
        asyncio.set_event_loop(loop)
        self._server = create_server(
            host=self.host, port=self.port,
            interval=self.interval, use_gpu=self.use_gpu,
        )
        try:
            loop.run_until_complete(self._server.start())
        except Exception as e:
            log.error(f"Server error: {e}")
        finally:
            loop.close()
            self._running = False

    def _stats_printer(self):
        last = 0
        while not self._stop_ev.is_set():
            now = time.time()
            if now - last >= 5:
                last = now
                cc = self.client_count
                status = "RUNNING" if self._running else "STOPPED"
                print(f"  [{status}] Clients: {cc}  |  ws://{self.host}:{self.port}")
            time.sleep(1)

    def run(self):
        print()
        print("=" * 50)
        print("  PC Monitor Server  v%s" % __version__)
        print("=" * 50)
        print("  WebSocket:  ws://%s:%d" % (self.host, self.port))
        print("  Interval:   %ds" % self.interval)
        print("  GPU:        %s" % ("enabled" if self.use_gpu else "disabled"))
        print("  Press Ctrl+C to stop")
        print("=" * 50)
        print()

        self._running = True
        t = threading.Thread(target=self._run_async, daemon=True)
        t.start()
        time.sleep(2)

        s = threading.Thread(target=self._stats_printer, daemon=True)
        s.start()

        try:
            while True:
                time.sleep(1)
        except KeyboardInterrupt:
            print("\nShutting down...")
            if self._server:
                self._server.stop()
            self._stop_ev.set()
            self._running = False


def main():
    import argparse
    p = argparse.ArgumentParser(description="PC Monitor Server")
    p.add_argument("--port", type=int, default=9090)
    p.add_argument("--host", default="0.0.0.0")
    p.add_argument("--interval", type=float, default=1.0)
    p.add_argument("--no-gpu", action="store_true")
    p.add_argument("--debug", action="store_true")
    a = p.parse_args()
    logging.basicConfig(
        level=logging.DEBUG if a.debug else logging.INFO,
        format="%(asctime)s [%(levelname)s] %(message)s",
        datefmt="%H:%M:%S",
    )
    ConsoleApp(host=a.host, port=a.port, interval=a.interval,
               use_gpu=not a.no_gpu).run()


if __name__ == "__main__":
    main()
