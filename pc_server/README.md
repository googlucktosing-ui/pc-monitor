# PC Monitor Server

向 ESP32 副屏推送 PC 系统数据的轻量 WebSocket 服务端。

## 功能

- **CPU** 使用率（%）
- **GPU** 使用率 + 温度（需安装 GPUtil / 有 NVIDIA 显卡）
- **内存** 使用量（GB）
- **磁盘** 使用量（GB）
- **网速** 上传/下载（KB/s）
- **系统信息** 主机名、CPU 型号、GPU 型号、OS 版本

## 系统要求

- Python 3.8+
- Windows / Linux / macOS

## 安装

```bash
pip install -r requirements.txt
```

可选（GPU 监控）：

```bash
pip install gputil
```

## 使用

```bash
# 默认端口 8765，1 秒推送一次
python pc_server.py

# 指定端口、推送间隔
python pc_server.py --port 9000 --interval 2

# 无独立显卡时禁用 GPU 采集
python pc_server.py --no-gpu
```

## ESP32 配置

编辑 pc_monitor/main/app_config.h，将 APP_PC_SERVER_URI 改为你的 PC IP：

```c
#define APP_PC_SERVER_URI "ws://192.168.1.100:9090"
```

## 参数

| 参数 | 默认值 | 说明 |
|------|--------|------|
| --port | 8765 | WebSocket 端口 |
| --host | 0.0.0.0 | 监听地址 |
| --interval | 1.0 | 采集/推送间隔（秒） |
| --no-gpu | - | 禁用 GPU 采集 |
| --debug | - | 开启调试日志 |

## 通信协议

服务端每 interval 秒向所有 WebSocket 客户端广播 JSON：

```json
{
  "cpu": 15.2,
  "gpu": 30.0,
  "gpu_temp": 65.0,
  "mem_used": 12.5,
  "mem_total": 32.0,
  "disk_used": 180.3,
  "disk_total": 500.0,
  "net_up": 5.2,
  "net_down": 125.8,
  "hostname": "DESKTOP-ABC",
  "cpu_name": "Intel Core i7-13700K",
  "gpu_name": "NVIDIA GeForce RTX 4070",
  "os_info": "Windows 11"
}
```
