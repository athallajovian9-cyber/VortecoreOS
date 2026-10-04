@echo off
setlocal
cd /d "%~dp0"

where python >nul 2>&1
if %errorlevel% neq 0 (
    echo Python not found on PATH.
    pause
    exit /b 1
)

echo Building VortecoreOS boot image...
python "%~dp0build_boot.py"

echo.
echo Launching VortecoreOS VM Display...
python "%~dp0vm_display.py"
pause
