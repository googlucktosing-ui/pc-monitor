' PC Monitor Launcher - completely silent, no window
Dim shell
Set shell = CreateObject("WScript.Shell")
shell.Run """E:\CODEX-Pj\ESP32\pc_server\dist\pc_monitor.exe""", 0, False
Set shell = Nothing
