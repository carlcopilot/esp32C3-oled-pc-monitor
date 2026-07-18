@echo off
setlocal enabledelayedexpansion

:: Check for Administrator privileges
net session >nul 2>&1
if %errorlevel% neq 0 (
    echo Requesting Administrator privileges...
    powershell -Command "Start-Process '%~f0' -Verb RunAs"
    exit /b
)

echo ========================================================
echo  ESP32 OLED PC Monitor - Startup Uninstaller (Windows)
echo ========================================================
echo.
echo Deleting task from Windows Task Scheduler...

schtasks /delete /tn "ESP32_OLED_PC_Monitor" /f

if %errorlevel% equ 0 (
    echo.
    echo [OK] Task deleted successfully. The script will no longer start automatically.
) else (
    echo.
    echo [WARNING] Could not delete the task.
)
echo.
pause
