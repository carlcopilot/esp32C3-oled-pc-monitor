@echo off
setlocal enabledelayedexpansion

:: Check for Administrator privileges
net session >nul 2>&1
if %errorlevel% neq 0 (
    echo Requesting Administrator privileges...
    powershell -Command "Start-Process '%~f0' -Verb RunAs"
    exit /b
)

:: Get the absolute directory of the script
set SCRIPT_DIR=%~dp0
set SCRIPT_PATH=%SCRIPT_DIR%pc_monitor_serial.py

echo ========================================================
echo  ESP32 OLED PC Monitor - Startup Installer (Windows)
echo ========================================================
echo.
echo Script to register: "%SCRIPT_PATH%"
echo.

:: Detect absolute path to pythonw.exe
set "PYTHONW_PATH="

:: Try to find a pythonw.exe path that is NOT in WindowsApps
for /f "usebackq delims=" %%i in (`where pythonw 2^>nul`) do (
    echo %%i | findstr /i /v "WindowsApps" >nul
    if !errorlevel! equ 0 (
        if not defined PYTHONW_PATH (
            set "PYTHONW_PATH=%%i"
        )
    )
)

:: Fallback to the first pythonw.exe found
if not defined PYTHONW_PATH (
    for /f "usebackq delims=" %%i in (`where pythonw 2^>nul`) do (
        if not defined PYTHONW_PATH (
            set "PYTHONW_PATH=%%i"
        )
    )
)

if not defined PYTHONW_PATH (
    echo [ERROR] pythonw.exe could not be found in your PATH.
    echo Please make sure Python is installed and added to the environment variables.
    echo.
    pause
    exit /b 1
)

echo Found pythonw at: "%PYTHONW_PATH%"
echo.
echo Registering task in Windows Task Scheduler...

:: Create a Task Scheduler entry to run at logon using the absolute path to pythonw.exe
schtasks /create /tn "ESP32_OLED_PC_Monitor" /tr "\"%PYTHONW_PATH%\" \"%SCRIPT_PATH%\"" /sc onlogon /rl highest /f

if %errorlevel% equ 0 (
    echo.
    echo [OK] Task registered successfully! 
    echo The monitor will run silently in the background every time you log in.
    echo.
    echo Starting the task now...
    schtasks /run /tn "ESP32_OLED_PC_Monitor"
) else (
    echo.
    echo [ERROR] Failed to register task.
)
echo.
pause
