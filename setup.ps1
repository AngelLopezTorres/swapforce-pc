<#
.SYNOPSIS
  Reconstruye, a partir de la ISO propia del usuario, todo lo derivado del juego y compila SwapForceRT.exe.

.DESCRIPTION
  Este repositorio NO contiene ningun fichero de Activision. Este script genera EN LOCAL, a partir de la ISO
  aportada por el usuario, los ficheros derivados (iso_extract\, analysis\image.bin, ppc\, runtime\ppc_gen\).
  Esos ficheros estan en .gitignore y no deben subirse a ningun repositorio.

  Pasos: 1 comprobaciones; 2 extract-xiso (descarga con SHA-256 fijado); 3 extraccion de la ISO;
  4 descifrado/descompresion de default.xex -> analysis\image.bin; 5 clonado de hedge-dev/XenonRecomp en el
  commit fijado + patches\XenonRecomp-swapforce.patch + compilacion; 6 XenonAnalyse; 7 iterate_extents.py;
  8 tablas de salto adicionales; 9 prune_loop.py (genera runtime\ppc_gen con XenonRecomp);
  10 DXC (descarga de la version oficial de Microsoft con SHA-256 fijado); 11 compilacion del runtime.

  Uso (la politica de ejecucion de PowerShell puede exigir -ExecutionPolicy Bypass):
    powershell -NoProfile -ExecutionPolicy Bypass -File setup.ps1 -IsoPath "D:\ruta\Skylanders - Swap Force.iso"

.PARAMETER IsoPath      Ruta de la ISO propia del usuario (Xbox 360, XGD2).
.PARAMETER SkipExtract  No extrae la ISO si iso_extract\default.xex ya existe.
.PARAMETER SkipBuild    No compila el runtime (paso 11).
#>
param(
    [Parameter(Mandatory = $true)][string]$IsoPath,
    [switch]$SkipExtract,
    [switch]$SkipBuild
)
$ErrorActionPreference = 'Stop'
$Root = $PSScriptRoot

# ---- Versiones fijadas (verificadas el 2026-10-11) ----
$XenonRecompUrl    = 'https://github.com/hedge-dev/XenonRecomp'
$XenonRecompCommit = 'ddd128bcca99fe8bfbb99bea583c972351fa6ace'      # main, 2025-08-04 (upstream de los patches)
$XisoUrl           = 'https://github.com/XboxDev/extract-xiso/releases/download/build-202609111233/extract-xiso-Win64_Release.zip'
$XisoZipSha256     = 'fc891dfa72f08e3838a2f8b7ec34a8e4d61678da1cc8fee879677f00bf114ce3'
$DxcUrl            = 'https://github.com/microsoft/DirectXShaderCompiler/releases/download/v1.8.2407/dxc_2024_07_31.zip'
$DxcZipSha256      = 'e2627f004f0f9424d8c71ea1314d04f38c5a5096884ae9217f1f18bd320267b5'

function Step([string]$n, [string]$msg) { Write-Host ("`n=== [{0}] {1}" -f $n, $msg) -ForegroundColor Cyan }
function Fail([string]$msg) { Write-Host "ERROR: $msg" -ForegroundColor Red; exit 1 }

# Ejecuta una orden dentro del entorno de compilacion (vsenv.cmd: vcvars64 + LLVM + CMake + Ninja).
function InEnv([string]$cmdline) {
    & cmd.exe /c "call `"$Root\runtime\tools\vsenv.cmd`" $cmdline"
    if ($LASTEXITCODE -ne 0) { Fail "fallo (exit $LASTEXITCODE): $cmdline" }
}
function Sha256([string]$path) { (Get-FileHash -Algorithm SHA256 -LiteralPath $path).Hash.ToLowerInvariant() }
function Download([string]$url, [string]$dest, [string]$sha) {
    if (-not (Test-Path $dest)) {
        Write-Host "Descargando $url"
        [Net.ServicePointManager]::SecurityProtocol = [Net.SecurityProtocolType]::Tls12
        Invoke-WebRequest -Uri $url -OutFile $dest -UseBasicParsing
    }
    $h = Sha256 $dest
    if ($h -ne $sha) { Fail "SHA-256 distinto en $dest`n  esperado $sha`n  obtenido $h" }
}

# ---- 1. Comprobaciones ----
Step 1 'Comprobaciones'
if (-not (Test-Path -LiteralPath $IsoPath)) { Fail "no existe la ISO: $IsoPath" }
$IsoPath = (Resolve-Path -LiteralPath $IsoPath).Path
foreach ($c in 'git', 'python') { if (-not (Get-Command $c -ErrorAction SilentlyContinue)) { Fail "falta '$c' en PATH" } }
& cmd.exe /c "python -c `"from Crypto.Cipher import AES`" 2>nul"
if ($LASTEXITCODE -ne 0) { Fail "falta pycryptodome: pip install pycryptodome" }
$vc = 'C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat'
if (-not (Test-Path $vc)) { Fail "falta VS Build Tools 2022 en '$vc' (ruta fijada en runtime\tools\vsenv.cmd; editarla si la instalacion es otra)" }
if (-not (Test-Path 'C:\Program Files\LLVM\bin\clang-cl.exe')) { Fail "falta LLVM/clang-cl en 'C:\Program Files\LLVM\bin' (ruta fijada en vsenv.cmd y runtime\CMakeLists.txt)" }
InEnv 'cmake --version'
InEnv 'ninja --version'
$free = (Get-PSDrive -Name ($Root.Substring(0, 1))).Free
if ($free -lt 20GB) { Fail ("espacio libre insuficiente: {0:N1} GB (se estiman >= 20 GB)" -f ($free / 1GB)) }

# ---- 2. extract-xiso ----
Step 2 'extract-xiso (XboxDev/extract-xiso build-202609111233)'
$toolsDir = Join-Path $Root 'tools'
New-Item -ItemType Directory -Force -Path $toolsDir | Out-Null
$xisoZip = Join-Path $toolsDir 'extract-xiso-Win64_Release.zip'
Download $XisoUrl $xisoZip $XisoZipSha256
$xisoDir = Join-Path $toolsDir 'extract-xiso'
if (-not (Test-Path "$xisoDir\artifacts\extract-xiso.exe")) { Expand-Archive -Path $xisoZip -DestinationPath $xisoDir -Force }
$xiso = Join-Path $xisoDir 'artifacts\extract-xiso.exe'
if (-not (Test-Path $xiso)) { Fail "no se encuentra $xiso tras descomprimir" }

# ---- 3. Extraer la ISO ----
Step 3 'Extraccion de la ISO a iso_extract\'
$iso = Join-Path $Root 'iso_extract'
$xex = Join-Path $iso 'default.xex'
if ($SkipExtract -and (Test-Path $xex)) {
    Write-Host 'iso_extract\default.xex ya existe: se omite la extraccion (-SkipExtract).'
} else {
    New-Item -ItemType Directory -Force -Path $iso | Out-Null
    & $xiso -d $iso -x $IsoPath
    if ($LASTEXITCODE -ne 0) { Fail "extract-xiso devolvio $LASTEXITCODE" }
}
if (-not (Test-Path $xex)) { Fail "no existe $xex tras la extraccion (ISO no valida o estructura distinta)" }
Write-Host ("default.xex: {0} B" -f (Get-Item $xex).Length)

# ---- 4. XEX -> image.bin ----
Step 4 'Descifrado y descompresion de default.xex -> analysis\image.bin'
& python "$Root\analysis\dump_image.py"
if ($LASTEXITCODE -ne 0) { Fail 'dump_image.py fallo' }

# ---- 5. XenonRecomp (upstream + patch) ----
Step 5 'hedge-dev/XenonRecomp @ commit fijado + patch + compilacion'
$xr = Join-Path $toolsDir 'XenonRecomp'
if (-not (Test-Path "$xr\.git")) {
    & git clone $XenonRecompUrl $xr
    if ($LASTEXITCODE -ne 0) { Fail 'git clone XenonRecomp fallo' }
    & git -C $xr checkout --detach $XenonRecompCommit
    if ($LASTEXITCODE -ne 0) { Fail 'git checkout del commit fijado fallo' }
    & git -C $xr submodule update --init --recursive
    if ($LASTEXITCODE -ne 0) { Fail 'git submodule update fallo' }
    & git -C $xr apply --whitespace=nowarn "$Root\patches\XenonRecomp-swapforce.patch"
    if ($LASTEXITCODE -ne 0) { Fail 'no se pudo aplicar patches\XenonRecomp-swapforce.patch' }
}
$head = (& git -C $xr rev-parse HEAD).Trim()
if ($head -ne $XenonRecompCommit) { Fail "XenonRecomp en $head, se esperaba $XenonRecompCommit" }
InEnv "cmake -S `"$xr`" -B `"$xr\out`" -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_C_COMPILER=clang-cl -DCMAKE_CXX_COMPILER=clang-cl"
InEnv "cmake --build `"$xr\out`" --target XenonRecomp XenonAnalyse"
$recompExe  = "$xr\out\XenonRecomp\XenonRecomp.exe"
$analyseExe = "$xr\out\XenonAnalyse\XenonAnalyse.exe"
foreach ($e in $recompExe, $analyseExe) { if (-not (Test-Path $e)) { Fail "no se genero $e" } }

# ---- 6. XenonAnalyse ----
Step 6 'XenonAnalyse -> analysis\SwapForce_switch_tables.toml'
& $analyseExe $xex "$Root\analysis\SwapForce_switch_tables.toml"
if ($LASTEXITCODE -ne 0) { Fail "XenonAnalyse devolvio $LASTEXITCODE" }

# ---- 7. Primera pasada de XenonRecomp (ppc\) y extensiones de funcion ----
Step 7 'iterate_extents.py -> analysis\SwapForce.toml (y primera pasada en ppc\)'
New-Item -ItemType Directory -Force -Path "$Root\ppc" | Out-Null
& python "$Root\analysis\iterate_extents.py"
if ($LASTEXITCODE -ne 0) { Fail 'iterate_extents.py fallo' }

# ---- 8. Tablas de salto adicionales ----
Step 8 'find_missing_switch.py y fusion de tablas'
$rc = "$Root\runtime\recomp"
Copy-Item "$Root\analysis\SwapForce_switch_tables.toml" "$rc\SwapForce_switch_tables.toml" -Force
Copy-Item "$rc\SwapForce_switch_tables.toml" "$rc\SwapForce_switch_tables.toml.pre_extra" -Force
if (Test-Path "$rc\extra_switch_tables.toml") { Remove-Item "$rc\extra_switch_tables.toml" }
& python "$rc\find_missing_switch.py"
if ($LASTEXITCODE -ne 0) { Fail 'find_missing_switch.py fallo' }
# SwapForce_switch_tables.toml (usado por XenonRecomp) = tablas de XenonAnalyse + tablas adicionales, concatenadas byte a byte.
$bytes = [IO.File]::ReadAllBytes("$rc\SwapForce_switch_tables.toml.pre_extra") + [IO.File]::ReadAllBytes("$rc\extra_switch_tables.toml")
[IO.File]::WriteAllBytes("$rc\SwapForce_switch_tables.toml", $bytes)

# ---- 9. Regeneracion final: runtime\ppc_gen ----
Step 9 'prune_loop.py -> runtime\ppc_gen'
New-Item -ItemType Directory -Force -Path "$Root\runtime\ppc_gen" | Out-Null
& python "$rc\prune_loop.py"
if ($LASTEXITCODE -ne 0) { Fail 'prune_loop.py fallo' }
# runtime\src\kernel\import_stubs.gen.cpp (no se distribuye): imports de kernel del XEX, generado desde ppc\ppc_recomp_shared.h.
& python "$Root\runtime\tools\gen_import_stubs.py"
if ($LASTEXITCODE -ne 0) { Fail 'gen_import_stubs.py fallo' }
$mf = Join-Path $Root 'setup_hashes.txt'
Get-ChildItem "$Root\runtime\ppc_gen" -File | Sort-Object Name | ForEach-Object { '{0}  {1}' -f (Sha256 $_.FullName), $_.Name } | Set-Content -Path $mf -Encoding ascii
Write-Host "SHA-256 de runtime\ppc_gen escritos en setup_hashes.txt"

# ---- 10. DXC ----
Step 10 'DirectX Shader Compiler v1.8.2407 (descarga oficial de Microsoft)'
$dxcZip = Join-Path $toolsDir 'dxc_2024_07_31.zip'
Download $DxcUrl $dxcZip $DxcZipSha256
$dxcDir = Join-Path $toolsDir 'dxc'
if (-not (Test-Path "$dxcDir\bin\x64\dxcompiler.dll")) { Expand-Archive -Path $dxcZip -DestinationPath $dxcDir -Force }
foreach ($f in 'bin\x64\dxcompiler.dll', 'bin\x64\dxil.dll') { if (-not (Test-Path "$dxcDir\$f")) { Fail "falta $dxcDir\$f" } }

# ---- 11. Runtime ----
if ($SkipBuild) { Write-Host 'Compilacion omitida (-SkipBuild).'; exit 0 }
Step 11 'Compilacion de runtime\ (SwapForceRT.exe)'
# build.cmd: gen_ppc_sources.py + cmake configure + build
InEnv "`"$Root\runtime\tools\build.cmd`""
$exe = "$Root\runtime\build\SwapForceRT.exe"
if (-not (Test-Path $exe)) { Fail "no se genero $exe" }
Write-Host ("`nOK: {0} ({1:N1} MB)" -f $exe, ((Get-Item $exe).Length / 1MB)) -ForegroundColor Green
Write-Host 'Ejecucion: cd runtime; build\SwapForceRT.exe   (ver README.md: estado actual, no jugable).'
