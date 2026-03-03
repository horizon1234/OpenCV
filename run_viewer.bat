@echo off
setlocal
set "APP_DIR=%~dp0"
set "PATH=%APP_DIR%dll;%PATH%"
"%APP_DIR%QtOpenCVWebpViewer.exe"
