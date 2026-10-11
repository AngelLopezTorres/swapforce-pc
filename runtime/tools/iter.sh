#!/bin/bash
# Uso: iter.sh [max-imports] [timeout]  : compila y ejecuta; muestra stubs llamados, excepciones y resumen
cd "$(dirname "$0")/.."
cmd //c "$(cygpath -w "$PWD/tools/build.cmd")" > build_last.log 2>&1; if grep -aq "FAILED\|error:" build_last.log; then grep -a "error" build_last.log | head -20; echo BUILD_FAILED; exit 9; fi
cd build
timeout $(( ${2:-30} + 30 )) ./SwapForceRT.exe --max-imports ${1:-5000} --timeout ${2:-30} > run_stdout.txt 2>&1
echo "exit=$?"
grep -a "^#.* stub \|EXCEP\|  guest:\|TIMEOUT\|^start\|^end\|ExCreateThread ->\|^imports" runtime.log | head -${3:-40}
tasklist | grep -i swapforce
