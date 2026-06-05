    def _do_send(self, port_device, ssid, password):
        try:
            self._log('Connecting to %s...' % port_device)
            ser = serial.Serial(port_device, 115200, timeout=3)
            self._log('Connected. Sending WiFi config...')
            
            cmd = 'WIFI:%s:%s\n' % (ssid, password)
            
            # 发送前先清空缓冲区，确保设备端是干净的
            ser.reset_input_buffer()
            ser.reset_output_buffer()
            
            # 发送命令
            ser.write(cmd.encode('utf-8'))
            self._log('Sent: WIFI:%s:*****' % ssid)
            
            # 等待设备响应
            time.sleep(0.3)
            
            response = b''
            # 读取响应，最多等5秒
            deadline = time.time() + 5
            got_ok = False
            while time.time() < deadline:
                ch = ser.read(1)
                if not ch:
                    continue
                if ch == b'\n':
                    break
                response += ch
            
            resp_text = response.decode('utf-8', errors='replace').strip()
            if resp_text:
                self._log('ESP32 response: ' + resp_text)
                if 'OK' in resp_text:
                    self._log('WiFi configured successfully! Rebooting ESP32...')
                    got_ok = True
            else:
                self._log('No response, retrying...')
                # 重试一次
                time.sleep(0.5)
                ser.reset_input_buffer()
                ser.write(cmd.encode('utf-8'))
                time.sleep(0.5)
                response = b''
                deadline = time.time() + 3
                while time.time() < deadline:
                    ch = ser.read(1)
                    if not ch:
                        continue
                    if ch == b'\n':
                        break
                    response += ch
                resp_text = response.decode('utf-8', errors='replace').strip()
                if resp_text:
                    self._log('ESP32 response (retry): ' + resp_text)
                    if 'OK' in resp_text:
                        self._log('WiFi configured successfully! Rebooting ESP32...')
                        got_ok = True
                else:
                    self._log('Still no response. Check that ESP32 is in normal operation mode.')
            
            ser.close()
            if got_ok:
                self._log('Done. ESP32 will reboot and connect to: ' + ssid)
            else:
                self._log('Done. (no confirmation from ESP32)')
        except Exception as e:
            self._log('Error: ' + str(e))
        finally:
            self.root.after(0, lambda: self._send_btn.configure(state='normal', text='Send WiFi to ESP32'))
