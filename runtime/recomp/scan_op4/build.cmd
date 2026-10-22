@echo off
call "%~dp0..\..\tools\vsenv.cmd" clang-cl /nologo /O2 /std:c++17 /EHsc -I%~dp0..\..\..\tools\XenonRecomp\XenonUtils -I%~dp0..\..\..\tools\XenonRecomp\thirdparty\disasm %~dp0scan_op4.cpp /Fe%~dp0scan_op4.exe /link %~dp0..\..\..\tools\XenonRecomp\out\XenonUtils\XenonUtils.lib %~dp0..\..\..\tools\XenonRecomp\out\thirdparty\disasm\disasm.lib
