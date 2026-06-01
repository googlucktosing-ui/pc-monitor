# PC Monitor - 电脑监控副屏

PC端采集系统数据（CPU/GPU/内存/网速），通过 WebSocket 发送给 ESP32 显示屏。

## 项目结构

| 目录 | 说明 |
|------|------|
| `pc_server/` | **PC端** - Python 服务（WebSocket + 系统监控 + 系统托盘） |
| `pc_monitor/` | **ESP32固件** - LVGL UI + 数据展示 + 自动发现连接 |

## 快速开始

### PC端

```bash
cd pc_server
pip install -r requirements.txt
python pc_server_tray.py
```

或直接运行 `dist/pc_monitor.exe`（已打包，无需 Python 环境）。

### ESP32

使用 ESP-IDF 编译 `pc_monitor/` 目录下的固件，烧录到 ESP32 开发板。

## 技术栈

- **PC端**: Python, websockets, psutil, pystray, PyInstaller
- **ESP32**: ESP-IDF, LVGL, WebSocket Client, mDNS
- **通信**: WebSocket (JSON) + UDP 自动发现
- **注意**: 第三方组件 lvgl 需通过 ESP 组件管理器下载

## Git 使用

```bash
# 克隆仓库
git clone <仓库地址>

# 如果你需要 lvgl（ESP32 编译需要）
cd pc_monitor
idf.py add-dependency "lvgl/lvgl^8.3"
```
