<#
Richtet unter Windows alles Nötige ein und startet den Benchmark.
Gegenstück zu setup.sh (Linux/macOS).

    powershell -ExecutionPolicy Bypass -File .\setup.ps1
    .\setup.ps1 -Yes          ohne Rückfragen
    .\setup.ps1 -Cpu          nur CPU messen, GPU ignorieren
    .\setup.ps1 -Rebuild      Image neu bauen

    $env:SIZES="512 1024"; $env:REPS=3; .\setup.ps1   (schneller Probelauf)

Ablauf:
  1. Docker Desktop prüfen, bei Bedarf mit winget installieren und starten
  2. NVIDIA-GPU erkennen (Docker Desktop mit WSL2-Backend reicht sie durch)
  3. Image mit LLVM + AdaptiveCpp bauen (einmalig, ca. 15-30 Minuten)
  4. Benchmark im Container ausführen; Ergebnisse in ergebnisse\<datum>_<cpu|cuda>\
#>
param(
    [switch]$Yes,
    [switch]$Cpu,
    [switch]$Rebuild
)

$ErrorActionPreference = 'Stop'
Set-Location $PSScriptRoot

$CudaBaseImage = if ($env:CUDA_BASE_IMAGE) { $env:CUDA_BASE_IMAGE } else { 'nvidia/cuda:12.9.1-devel-ubuntu24.04' }

function Info($msg) { Write-Host "==> $msg" -ForegroundColor Cyan }
function Frage($msg) {
    if ($Yes) { return $true }
    $antwort = Read-Host "$msg [j/N]"
    return $antwort -match '^[jJyY]'
}
function Docker-Laeuft {
    docker info *> $null
    return $LASTEXITCODE -eq 0
}

# ---------------------------------------------------------------------------
# 1. Docker Desktop
# ---------------------------------------------------------------------------
if (-not (Get-Command docker -ErrorAction SilentlyContinue)) {
    if (-not (Get-Command winget -ErrorAction SilentlyContinue)) {
        throw 'Docker fehlt und winget ist nicht verfügbar. Bitte Docker Desktop installieren: https://www.docker.com/products/docker-desktop/'
    }
    if (-not (Frage 'Docker Desktop ist nicht installiert. Jetzt mit winget installieren?')) { throw 'Abgebrochen.' }
    winget install -e --id Docker.DockerDesktop --accept-package-agreements --accept-source-agreements
    $env:Path = [Environment]::GetEnvironmentVariable('Path', 'Machine') + ';' +
                [Environment]::GetEnvironmentVariable('Path', 'User')
    Write-Host 'Docker Desktop wurde installiert. Falls WSL2 neu eingerichtet wurde, ist ein Neustart nötig.'
}

if (-not (Docker-Laeuft)) {
    $dockerDesktop = Join-Path $env:ProgramFiles 'Docker\Docker\Docker Desktop.exe'
    if (Test-Path $dockerDesktop) { Start-Process $dockerDesktop }
    Info 'Warte auf Docker ...'
    for ($i = 0; $i -lt 60 -and -not (Docker-Laeuft); $i++) { Start-Sleep -Seconds 5 }
    if (-not (Docker-Laeuft)) { throw 'Docker läuft nicht. Bitte Docker Desktop starten und das Skript erneut ausführen.' }
}
Info "Docker: $(docker version --format '{{.Server.Version}}')"

# ---------------------------------------------------------------------------
# 2. NVIDIA-GPU
# ---------------------------------------------------------------------------
$gpu = $false
if (-not $Cpu -and (Get-Command nvidia-smi -ErrorAction SilentlyContinue)) {
    nvidia-smi -L *> $null
    if ($LASTEXITCODE -eq 0) {
        Info "NVIDIA-GPU gefunden: $((nvidia-smi -L | Select-Object -First 1))"
        docker run --rm --gpus all ubuntu:24.04 true *> $null
        if ($LASTEXITCODE -eq 0) {
            $gpu = $true
        } else {
            Write-Warning 'GPU im Container nicht nutzbar (WSL2-Backend in Docker Desktop und aktuellen NVIDIA-Treiber prüfen) - es wird nur die CPU gemessen.'
        }
    }
}
if (-not $gpu) { Info 'Es wird nur die CPU gemessen.' }

# ---------------------------------------------------------------------------
# 3. Image bauen
# ---------------------------------------------------------------------------
if ($gpu) {
    $tag = 'sycl-matmul:cuda'
    $buildArgs = @('--build-arg', "BASE_IMAGE=$CudaBaseImage", '--build-arg', 'WITH_CUDA=ON')
} else {
    $tag = 'sycl-matmul:cpu'
    $buildArgs = @('--build-arg', 'WITH_CUDA=OFF')
}

docker image inspect $tag *> $null
if ($Rebuild -or $LASTEXITCODE -ne 0) {
    Info "Baue Image $tag (beim ersten Mal ca. 15-30 Minuten) ..."
    docker build @buildArgs -t $tag -f docker/Dockerfile docker/
    if ($LASTEXITCODE -ne 0) { throw 'Image-Build fehlgeschlagen.' }
}

# ---------------------------------------------------------------------------
# 4. Benchmark ausführen
# ---------------------------------------------------------------------------
$suffix = $tag.Split(':')[1]
$outDir = Join-Path $PSScriptRoot ("ergebnisse\{0}_{1}" -f (Get-Date -Format 'yyyyMMdd_HHmm'), $suffix)
New-Item -ItemType Directory -Force -Path $outDir | Out-Null

$runArgs = @('run', '--rm', '-v', "${PSScriptRoot}:/work/sycl-matmul:ro", '-v', "${outDir}:/ergebnisse")
foreach ($var in 'SIZES', 'REPS', 'TILES', 'SEQ_MAX') {
    if (Test-Path "env:$var") { $runArgs += @('-e', $var) }
}
if ($gpu) { $runArgs += @('--gpus', 'all') }
$runArgs += @($tag, 'bash', '/work/sycl-matmul/docker/run_in_container.sh')

Info "Starte Benchmark (Ergebnisse: $outDir)"
docker @runArgs
if ($LASTEXITCODE -ne 0) { throw 'Benchmark fehlgeschlagen.' }
Info "Fertig. Ergebnisse in $outDir"
