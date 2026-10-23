@echo off
rem rem portal3
set "RT=%~dp0"
call "%RT%tools\vsenv.cmd" cmake -S "%RT%." -B "%RT%build_portal3" -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=clang-cl -DCMAKE_CXX_COMPILER=clang-cl -DSWAPFORCE_OUT_DIR="%RT%build_portal3\out"
if errorlevel 1 exit /b 1
call "%RT%tools\vsenv.cmd" cmake --build "%RT%build_portal3" --parallel 8
