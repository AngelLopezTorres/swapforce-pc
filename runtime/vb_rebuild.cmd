@echo off
call "%~dp0tools\vsenv.cmd" cmake --build "%~dp0build_verify" --parallel 12
