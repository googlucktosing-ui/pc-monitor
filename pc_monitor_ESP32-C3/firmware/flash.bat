@echo off
REM ==============================================
REM ESP32-C3-MINI-1 PC Monitor 固件烧录脚本
REM ==============================================

echo.
echo ============================================
echo  PC Monitor - ESP32-C3 固件烧录工具
echo ============================================
echo.

REM 检查 esptool
pip show esptool >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo [INFO] 正在安装 esptool...
    pip install esptool -q
)

REM 自动检测端口
set PORT=
for /f "tokens=3" %%a in ('reg query HKLM\HARDWARE\DEVICEMAP\SERIALCOMM 2^>nul ^| findstr "VID_303A"') do set PORT=%%a
if "%PORT%"=="" (
    echo [提示] 未自动检测到 ESP32-C3 端口
    set /p PORT="请输入 COM 端口号 (如 COM3): "
)

echo [INFO] 使用端口: %PORT%
echo [INFO] 开始烧录...
echo.

python -m esptool --chip esp32c3 -b 460800 --port %PORT% --before default_reset --after hard_reset write_flash --flash_mode dio --flash_freq 80m --flash_size 4MB ^
  0x0 bootloader.bin ^
  0x8000 partition-table.bin ^
  0x10000 pc_monitor_c3.bin

if %ERRORLEVEL% EQU 0 (
    echo.
    echo [SUCCESS] 烧录成功!
    echo [INFO] 开发板会自动重启，请观察屏幕显示
) else (
    echo.
    echo [ERROR] 烧录失败，请检查:
    echo   1. USB线是否连接好（用数据线）
    echo   2. 端口号是否正确
    echo   3. 开发板是否已上电
)

pause
