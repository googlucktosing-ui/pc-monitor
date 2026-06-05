import sys, os, threading, tkinter as tk
from tkinter import ttk, messagebox
import queue, time

try:
    import serial
    import serial.tools.list_ports
    HAS_SERIAL = True
except ImportError:
    HAS_SERIAL = False

import logging
log = logging.getLogger('PcMonWifi')

class WifiConfigDialog:
    
    def __init__(self, parent=None):
        self.root = tk.Tk() if parent is None else tk.Toplevel(parent)
        self.root.title('Configure ESP32 WiFi')
        self.root.resizable(False, False)
        self.root.geometry('480x320')
        try:
            self.root.iconbitmap(default='')
        except Exception:
            pass
        
        self._serial_port = None
        self._result = None
        self._refresh_q = queue.Queue()
        self._build_ui()
        self.root.after(100, self._poll_queue)
        self.root.after(200, self._refresh_ports_bg)
        self.root.protocol('WM_DELETE_WINDOW', self._on_close)
        self.root.grab_set()
    
    def _build_ui(self):
        f = ttk.Frame(self.root, padding=12)
        f.pack(fill='both', expand=True)
        
        ttk.Label(f, text='ESP32 Serial Port:').grid(row=0, column=0, sticky='w', pady=(0,2))
        self._port_var = tk.StringVar()
        self._port_cb = ttk.Combobox(f, textvariable=self._port_var, state='readonly', width=35)
        self._port_cb.grid(row=0, column=1, sticky='ew', padx=(6,4))
        self._refresh_btn = ttk.Button(f, text='Refresh', command=self._refresh_ports_bg, width=8)
        self._refresh_btn.grid(row=0, column=2)
        
        ttk.Label(f, text='WiFi SSID:').grid(row=1, column=0, sticky='w', pady=(10,2))
        self._ssid_var = tk.StringVar()
        self._ssid_entry = tk.Entry(f, textvariable=self._ssid_var, width=40)
        self._ssid_entry.grid(row=1, column=1, columnspan=2, sticky='ew', padx=(6,0))
        
        ttk.Label(f, text='WiFi Password:').grid(row=2, column=0, sticky='w', pady=(10,2))
        self._pass_var = tk.StringVar()
        self._pass_entry = tk.Entry(f, textvariable=self._pass_var, width=40, show='*')
        self._pass_entry.grid(row=2, column=1, columnspan=2, sticky='ew', padx=(6,0))
        
        self._show_pw = tk.BooleanVar()
        ttk.Checkbutton(f, text='Show', variable=self._show_pw, command=self._toggle_pw).grid(row=2, column=2, sticky='w', padx=(2,0))
        
        btn_frame = ttk.Frame(f)
        btn_frame.grid(row=3, column=0, columnspan=3, pady=(12,0))
        self._send_btn = ttk.Button(btn_frame, text='Send WiFi to ESP32', command=self._send_wifi, width=25)
        self._send_btn.pack(side='left', padx=(0,8))
        ttk.Button(btn_frame, text='Close', command=self._on_close, width=10).pack(side='left')
        
        ttk.Label(f, text='Status:').grid(row=4, column=0, sticky='nw', pady=(8,0))
        self._status_text = tk.Text(f, height=8, width=50, state='disabled', wrap='word', font=('Consolas', 9))
        self._status_text.grid(row=4, column=1, columnspan=2, sticky='nsew', pady=(8,0), padx=(6,0))
        
        f.columnconfigure(1, weight=1)
        f.rowconfigure(4, weight=1)
    
    def _toggle_pw(self):
        self._pass_entry.configure(show='' if self._show_pw.get() else '*')
    
    def _log(self, msg):
        self._status_text.configure(state='normal')
        self._status_text.insert('end', msg + '\n')
        self._status_text.see('end')
        self._status_text.configure(state='disabled')
        self.root.update_idletasks()
    
    def _poll_queue(self):
        """Process queue items from background threads on the main thread."""
        try:
            while True:
                msg = self._refresh_q.get_nowait()
                if msg[0] == 'ports':
                    ports = msg[1]
                    self._port_cb['values'] = ports
                    if ports:
                        self._port_cb.current(0)
                    self._refresh_btn.configure(state='normal', text='Refresh')
                    # Ensure window and entry get focus for keyboard input
                    self.root.lift()
                    self.root.focus_force()
                    self._ssid_entry.focus_set()
                elif msg[0] == 'log':
                    self._log(msg[1])
        except queue.Empty:
            pass
        self.root.after(100, self._poll_queue)
    
    def _refresh_ports_bg(self):
        """Refresh ports in background thread to prevent GUI freeze."""
        self._refresh_btn.configure(state='disabled', text='Scanning...')
        self._port_cb['values'] = ['Scanning...']
        self._port_var.set('Scanning...')
        threading.Thread(target=self._do_scan_ports, daemon=True).start()
    
    def _do_scan_ports(self):
        """Scan ports in background with timeout."""
        result = ['No serial port detected']
        if HAS_SERIAL:
            detected = []
            try:
                for p in serial.tools.list_ports.comports():
                    try:
                        desc = p.device + ' - ' + (p.description or '')
                        if 'bluetooth' not in desc.lower() and 'blue' not in desc.lower():
                            detected.append(desc)
                    except Exception:
                        pass
                    # Safety: limit to 20 ports to avoid hanging
                    if len(detected) >= 20:
                        break
                if detected:
                    result = detected
                self._refresh_q.put(('log', 'Found %d port(s)' % len(detected)))
            except Exception as e:
                self._refresh_q.put(('log', 'Port scan: %s' % str(e)))
        self._refresh_q.put(('ports', result))
    
    def _get_port_device(self):
        val = self._port_var.get()
        if not val or 'No serial' in val or 'Scanning' in val:
            return None
        return val.split(' - ')[0].strip()
    
    def _send_wifi(self):
        ssid = self._ssid_var.get().strip()
        password = self._pass_var.get().strip()
        port_device = self._get_port_device()
        
        if not ssid:
            messagebox.showwarning('Input Error', 'Please enter WiFi SSID')
            return
        if not port_device:
            messagebox.showwarning('Port Error', 'No ESP32 serial port selected')
            return
        
        self._send_btn.configure(state='disabled', text='Sending...')
        threading.Thread(target=self._do_send, args=(port_device, ssid, password), daemon=True).start()
    
    def _do_send(self, port_device, ssid, password):
        try:
            self._log('Connecting to %s...' % port_device)
            ser = serial.Serial(port_device, 115200, timeout=5)
            self._log('Connected. Sending WiFi config...')
            
            cmd = 'WIFI:%s:%s\n' % (ssid, password)
            ser.write(cmd.encode('utf-8'))
            self._log('Sent: WIFI:%s:*****' % ssid)
            
            time.sleep(0.5)
            response = b''
            while True:
                ch = ser.read(1)
                if not ch or ch == b'\n':
                    break
                response += ch
            
            resp_text = response.decode('utf-8', errors='replace').strip()
            if resp_text:
                self._log('ESP32 response: ' + resp_text)
                if 'OK' in resp_text:
                    self._log('WiFi configured successfully! Rebooting ESP32...')
            else:
                self._log('No response from ESP32 (check baud rate)')
            
            ser.close()
            self._log('Done.')
        except Exception as e:
            self._log('Error: ' + str(e))
        finally:
            self.root.after(0, lambda: self._send_btn.configure(state='normal', text='Send WiFi to ESP32'))
    
    def _on_close(self):
        """Close dialog safely."""
        try:
            self.root.quit()
            self.root.destroy()
        except Exception:
            pass

_dialog_ref = [None]

def show_wifi_config(parent=None):
    """Show WiFi config dialog in a background thread."""
    global _dialog_ref
    # If dialog already open, just focus it
    if _dialog_ref[0] is not None:
        try:
            _dialog_ref[0].root.lift()
            _dialog_ref[0].root.focus_force()
        except Exception:
            pass
        return
    def _run():
        global _dialog_ref
        try:
            d = WifiConfigDialog(parent)
            _dialog_ref[0] = d
            d.root.protocol("WM_DELETE_WINDOW", lambda: _close_dialog())
            d.root.mainloop()
        except Exception as e:
            log.error('WiFi dialog: ' + str(e))
        finally:
            _dialog_ref[0] = None
    t = threading.Thread(target=_run, daemon=True)
    t.start()

def _close_dialog():
    global _dialog_ref
    if _dialog_ref[0] is not None:
        try:
            _dialog_ref[0].root.quit()
        except Exception:
            pass
        try:
            _dialog_ref[0].root.destroy()
        except Exception:
            pass
        _dialog_ref[0] = None

if __name__ == '__main__':
    show_wifi_config()

