<##
Starts the test web server and Unreal Editor dedicated server for local testing.
Run from PowerShell:
  powershell -ExecutionPolicy Bypass -File .\Start-TestServers.ps1
##>

$ErrorActionPreference = 'Stop'

$ProjectRoot = $PSScriptRoot
$ProjectFile = Join-Path $ProjectRoot 'L20260713_Day03.uproject'
$WebServerFile = Join-Path $ProjectRoot 'WebServer\server.js'
$LogDirectory = Join-Path $ProjectRoot 'Saved\TestServerLogs'
$WebLog = Join-Path $LogDirectory 'webserver.log'
$WebErrorLog = Join-Path $LogDirectory 'webserver-error.log'
$UnrealEditor = 'C:\Program Files\Epic Games\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe'
$NodeCandidates = @(
    @(
        (Get-Command node -ErrorAction SilentlyContinue | Select-Object -ExpandProperty Source -ErrorAction SilentlyContinue),
        (Join-Path $env:ProgramFiles 'nodejs\node.exe'),
        (Join-Path $env:USERPROFILE '.cache\codex-runtimes\codex-primary-runtime\dependencies\node\bin\node.exe')
    ) | Where-Object { $_ -and (Test-Path -LiteralPath $_) }
)

if (-not (Test-Path -LiteralPath $ProjectFile)) { throw "Project file not found: $ProjectFile" }
if (-not (Test-Path -LiteralPath $WebServerFile)) { throw "Web server file not found: $WebServerFile" }
if (-not (Test-Path -LiteralPath $UnrealEditor)) { throw "Unreal Editor not found: $UnrealEditor" }
if ($NodeCandidates.Count -eq 0) { throw 'Node.js was not found. Install Node.js LTS, then run this script again.' }

$Node = (Resolve-Path -LiteralPath $NodeCandidates[0]).Path
Write-Host "Using Node.js: $Node"
New-Item -ItemType Directory -Force -Path $LogDirectory | Out-Null

function Test-WebServer {
    try {
        $Health = Invoke-RestMethod -Uri 'http://127.0.0.1:8080/health' -TimeoutSec 1
        return $Health.ok -eq $true
    }
    catch {
        return $false
    }
}

if (Test-WebServer) {
    Write-Host 'Web server is already running on port 8080.' -ForegroundColor Yellow
}
else {
    $PortOwner = Get-NetTCPConnection -LocalPort 8080 -State Listen -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($PortOwner) {
        throw "Port 8080 is already in use by PID $($PortOwner.OwningProcess), but it is not this test web server."
    }

    $WebProcess = Start-Process -FilePath $Node -ArgumentList $WebServerFile -WorkingDirectory (Split-Path $WebServerFile) -WindowStyle Hidden -RedirectStandardOutput $WebLog -RedirectStandardError $WebErrorLog -PassThru
    Write-Host "Started web server (PID $($WebProcess.Id))." -ForegroundColor Green

    $Ready = $false
    foreach ($Attempt in 1..20) {
        Start-Sleep -Milliseconds 500
        if (Test-WebServer) { $Ready = $true; break }
    }
    if (-not $Ready) { throw "Web server did not start. Check: $WebErrorLog" }
}

$ServerPortOwner = Get-NetUDPEndpoint -LocalPort 7777 -ErrorAction SilentlyContinue | Select-Object -First 1
if ($ServerPortOwner) {
    Write-Host "A server is already listening on UDP port 7777 (PID $($ServerPortOwner.OwningProcess))." -ForegroundColor Yellow
}
else {
    $UnrealArguments = @($ProjectFile, '/Game/ThirdPerson/Lvl_ThirdPerson', '-server', '-log', '-port=7777')
    $GameServer = Start-Process -FilePath $UnrealEditor -ArgumentList $UnrealArguments -WorkingDirectory $ProjectRoot -WindowStyle Hidden -PassThru
    Write-Host "Started Dedicated Server (PID $($GameServer.Id)). Waiting for registration..." -ForegroundColor Green
}

$Registered = $false
foreach ($Attempt in 1..30) {
    Start-Sleep -Seconds 1
    try {
        $Health = Invoke-RestMethod -Uri 'http://127.0.0.1:8080/health' -TimeoutSec 1
        if ($Health.registered_server_count -gt 0) { $Registered = $true; break }
    }
    catch { }
}

if ($Registered) {
    Write-Host 'Ready: web server and Dedicated Server are running.' -ForegroundColor Green
    Write-Host 'Log in with ID testuser and password test1234.' -ForegroundColor Green
}
else {
    Write-Warning "Dedicated Server did not register within 30 seconds. Check Unreal logs under: $ProjectRoot\Saved\Logs"
}
