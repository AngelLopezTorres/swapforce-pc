@echo off
rem Compila cputest\work\cpu_selftest_standalone.exe (banco de CPU + main minimo). Requiere runtime\ppc_gen\ppc_context.h (o la copia de XenonUtils).
set R=%~dp0..\..
call "%R%\tools\vsenv.cmd" clang-cl /nologo /O2 /EHsc /std:c++17 /clang:-msse4.1 /clang:-mssse3 /clang:-mavx /clang:-mf16c /clang:-w /DNOMINMAX /I"%R%\ppc_gen" /I"%R%\..\tools\XenonRecomp\thirdparty\simde" /I"%R%\src\selftest" "%R%\src\selftest\cpu_selftest.cpp" "%R%\src\selftest\cpu_tests_gen.cpp" "%~dp0standalone_main.cpp" /Fo"%~dp0work\\" /Fe"%~dp0work\cpu_selftest_standalone.exe"
