#!/bin/bash
# Regenera runtime/ppc_gen con XenonRecomp (config derivada de analysis/ + funciones adicionales).
set -e
R="$(cd "$(dirname "$0")/../.." && pwd)"
python $R/runtime/recomp/make_recomp_config.py
rm -rf $R/runtime/ppc_gen; mkdir -p $R/runtime/ppc_gen
$R/tools/XenonRecomp/out/XenonRecomp/XenonRecomp.exe $R/runtime/recomp/SwapForce.toml $R/tools/XenonRecomp/XenonUtils/ppc_context.h > $R/runtime/recomp/recomp_last.txt 2>&1
grep -a "outside\|Switch" $R/runtime/recomp/recomp_last.txt | head -5 || true
echo "ERROR-sitios: $(grep -c '// ERROR' $R/runtime/ppc_gen/ppc_recomp.*.cpp | grep -v ':0' | wc -l)"
python $R/runtime/tools/gen_ppc_sources.py
