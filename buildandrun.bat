@echo off
cd /d "%~dp0"

where ndk-build >nul 2>&1
if %errorlevel% neq 0 (
    echo ndk-build not found in PATH
    exit /b 1
)

call ndk-build
if %errorlevel% neq 0 exit /b %errorlevel%

if not exist ".\\adb\\adb.exe" (
    echo adb.exe not found
    exit /b 1
)

.\\adb\\adb.exe devices 2>&1 | findstr /r /c:"[0-9a-fA-F]" >nul
if %errorlevel% neq 0 (
    echo No device connected
    exit /b 1
)

if not exist ".\\libs\\arm64-v8a\\example.sh" (
    echo example.sh binary not found
    exit /b 1
)

.\\adb\\adb.exe push ".\\libs\\arm64-v8a\\example.sh" /data/local/tmp/example.sh
if %errorlevel% neq 0 (
    echo Failed to push file
    exit /b 1
)

.\\adb\\adb.exe shell "su -c 'chmod 777 /data/local/tmp/example.sh'"
if %errorlevel% neq 0 (
    echo Failed to set permissions
    exit /b 1
)

.\\adb\\adb.exe shell "su -c '/data/local/tmp/example.sh'"
