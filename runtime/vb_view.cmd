@echo off
call "C:/Users/Pastrones/SwapForce-PC/_wt_view/runtime\tools\vsenv.cmd" cmake -S "C:/Users/Pastrones/SwapForce-PC/_wt_view/runtime" -B "C:/Users/Pastrones/SwapForce-PC/_wt_view/runtime\build_view" -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=clang-cl -DCMAKE_CXX_COMPILER=clang-cl -DSWAPFORCE_OUT_DIR="C:/Users/Pastrones/SwapForce-PC/_wt_view/runtime\build_view\out"
if errorlevel 1 exit /b 1
call "C:/Users/Pastrones/SwapForce-PC/_wt_view/runtime\tools\vsenv.cmd" cmake --build "C:/Users/Pastrones/SwapForce-PC/_wt_view/runtime\build_view" --parallel 8
