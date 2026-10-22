# M8: registra cada 2 s la carga total de CPU y los procesos de compilacion/ejecucion ajenos (para anotar concurrencia).
# Uso: powershell -NoProfile -File loadwatch.ps1 <salida> <segundos>
param([string]$Out, [int]$Secs = 130)
$end = (Get-Date).AddSeconds($Secs)
while ((Get-Date) -lt $end) {
    $load = (Get-CimInstance Win32_Processor).LoadPercentage
    $p = Get-CimInstance Win32_Process | Where-Object { $_.Name -match '^(clang-cl|clang|lld-link|ninja|cl|link|SwapForceRT)\.exe$' }
    $own = ($p | Where-Object { $_.Name -eq 'SwapForceRT.exe' -and $_.ExecutablePath -like '*\runtime\build\SwapForceRT.exe' -and $_.ExecutablePath -notlike '*_wt_*' } | Measure-Object).Count
    $foreign = ($p | Where-Object { -not ($_.Name -eq 'SwapForceRT.exe' -and $_.ExecutablePath -like '*\runtime\build\SwapForceRT.exe' -and $_.ExecutablePath -notlike '*_wt_*') } | Measure-Object).Count
    "{0:HH:mm:ss} carga={1}% propios={2} ajenos={3}" -f (Get-Date), $load, $own, $foreign | Out-File -Append -Encoding utf8 $Out
    Start-Sleep -Seconds 2
}
