@echo off
rem Uso: cc_check.cmd fichero.cpp ...  (compila sin enlazar para comprobar sintaxis; objetos en %TEMP%)
call "%~dp0vsenv.cmd" clang-cl /nologo /std:c++17 /EHsc /O1 /DNOMINMAX /DWIN32_LEAN_AND_MEAN /I"%~dp0..\src\gpu" /I"%~dp0..\src" /I"%~dp0..\src\memory" /I"%~dp0..\src\util" /I"%~dp0..\src\kernel" /I"%~dp0..\src\thread" /Fo"%TEMP%\\" /c %*
