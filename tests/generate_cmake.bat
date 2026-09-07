@echo off
cmake -B build
if errorlevel 1 goto error
goto ok

:error
pause

:ok