#!/bin/sh
# M8: medida estandar. Uso: measure.sh <etiqueta> [segundos=60] [opciones extra]
# Ejecuta desde runtime\: build\SwapForceRT.exe --perf perf_<etiqueta>.txt --timeout N <extra> > run_<etiqueta>.out
cd $(cd "$(dirname "$0")/../../.." && pwd)/runtime || exit 1
tag=$1; secs=${2:-60}; shift; shift
timeout $((secs + 60)) build/SwapForceRT.exe --perf m8/perf_$tag.txt --timeout $secs --headless "$@" > m8/run_$tag.out 2>&1
echo "EXIT=$?"
powershell -NoProfile -Command "Get-CimInstance Win32_Process -Filter \"Name='SwapForceRT.exe'\" | Select-Object ProcessId,ExecutablePath | Format-Table -HideTableHeaders | Out-String" > m8/procs_after_$tag.txt; grep -i "runtime\build\\" m8/procs_after_$tag.txt && echo "AVISO: proceso propio vivo"; grep -ic swapforce m8/procs_after_$tag.txt | sed "s/^/procesos SwapForceRT ajenos o propios tras la medida: /"
python -I tools/m8/fpsstat.py m8/run_$tag.out 30
python -I tools/perf_report.py m8/perf_$tag.txt build/SwapForceRT.map 40 > m8/perf_${tag}_sym.txt
