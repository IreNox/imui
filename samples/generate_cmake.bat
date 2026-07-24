@echo off
cmake -B build/cmake
if errorlevel 1 goto error
goto ok

:error
pause

:ok