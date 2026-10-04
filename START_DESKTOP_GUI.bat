@echo off
setlocal
cd /d "%~dp0"

where python >nul 2>&1
if %errorlevel% neq 0 (
    echo Python not found on PATH.
    pause
    exit /b 1
)

echo Starting VortecoreOS Desktop Environment (1024x768 TrueColor)...
python "%~dp0desktop_gui.py"
pause
