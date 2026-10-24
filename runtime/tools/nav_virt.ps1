param([string]$Run,[int]$Port=8767,[int]$Tail=100,[switch]$StateLog)
Add-Type -AssemblyName System.Drawing
$b="http://127.0.0.1:$Port"
function Pulse($k,$ms=250){ curl.exe -s --max-time 2 "$b/key?down=$k" -o NUL; Start-Sleep -Milliseconds $ms; curl.exe -s --max-time 2 "$b/key?down=" -o NUL }
function Kind($f){ try{ $bm=[System.Drawing.Bitmap]::FromFile($f); $r=0;$g=0;$bl=0;$n=0; foreach($x in 160,640,1120){ foreach($y in 100,360,600){ $c=$bm.GetPixel($x,$y); $r+=$c.R;$g+=$c.G;$bl+=$c.B;$n++ } }; $tl=$bm.GetPixel(60,60); $bm.Dispose(); $r/=$n;$g/=$n;$bl/=$n
  if($r -gt 190 -and $g -gt 160 -and $bl -lt 130){ 'yellow' } elseif($tl.R -lt 40 -and $tl.G -lt 40 -and $tl.B -lt 60 -and $r -gt 50 -and $r -lt 160){ 'coloca' } else { '' } }catch{ '' } }
New-Item -ItemType Directory -Force "$Run\cap" | Out-Null
$t0=Get-Date; $step=0; $nextAt=36; $lastY=-9; $cool=@{yellow=-99;coloca=-99}; $marks=@()
while($true){
  $s=((Get-Date)-$t0).TotalSeconds
  $f="$Run\cap\{0:D5}.jpg" -f [int]$s
  curl.exe -s --max-time 3 "$b/frame.jpg" -o $f
  if($StateLog -and $step -ge 4){ $k=Kind $f; if($k -and ($s-$cool[$k]) -gt 45){ curl.exe -s --max-time 2 "$b/statelog" -o NUL; $cool[$k]=$s; $marks+="$([int]$s) $k"; "statelog $k en $([int]$s) s" } }
  if($step -lt 4 -and $s -ge $nextAt){
    switch($step){ 0 {Pulse 'enter'; $nextAt=$s+6} 1 {Pulse 'left'; Start-Sleep -Milliseconds 500; Pulse 'z'; $nextAt=$s+9} 2 {Pulse 'z'; $nextAt=$s+7} 3 {Pulse 'z'; $nextAt=$s+12} }
    $step++
  } elseif($step -ge 4 -and $s -ge $nextAt -and ($s-$lastY) -ge 3){ Pulse 'y' 200; $lastY=$s }
  if($step -ge 4 -and $s -gt ($nextAt+$Tail)){ break }
  Start-Sleep -Milliseconds 700
}
$marks | Set-Content "$Run\statelog_marks.txt"
