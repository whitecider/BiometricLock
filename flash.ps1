$ErrorActionPreference = "Stop"

# Configuration
$FQBN = "m5stack:esp32:m5stack_atoms3"
$BuildDir = "$PSScriptRoot\build"

# Locate Arduino CLI
$CliPath = "arduino-cli"
if (-not (Get-Command "arduino-cli" -ErrorAction SilentlyContinue)) {
    $DefaultPath = "C:\Program Files\Arduino CLI\arduino-cli.exe"
    if (Test-Path $DefaultPath) {
        $CliPath = $DefaultPath
    }
    else {
        Write-Error "arduino-cli not found. Please install it or add to PATH."
    }
}

# Step 1: Build
Write-Host "Building Project..." -ForegroundColor Cyan
& $CliPath compile --fqbn $FQBN --build-path $BuildDir "$PSScriptRoot"
if ($LASTEXITCODE -ne 0) {
    Write-Error "Build Failed! Aborting flash."
}

# Step 2: Find Port
Write-Host "Searching for device..."
$jsonInfo = & $CliPath board list --format json | ConvertFrom-Json
$targetPort = $null

# Strategy: Look for USB Serial or known boards
if ($jsonInfo) {
    foreach ($item in $jsonInfo) {
        if ($item.matching_boards) {
            foreach ($board in $item.matching_boards) {
                # M5Stack devices often show up generically or with specific VID/PID
                if ($board.name -match "ESP32" -or $board.name -match "M5") {
                    $targetPort = $item.port.address
                    break
                }
            }
        }
        if ($targetPort) { break }
        
        # Fallback: Look for USB Serial ports (common for CP210x/CH9102)
        if ($item.port -and $item.port.protocol_label -like "*USB*" -or $item.port.label -like "*USB*") {
            $targetPort = $item.port.address
        }
    }
}

if ($targetPort) {
    Write-Host "Found device on port: $targetPort" -ForegroundColor Cyan
}
else {
    # Last Resort Fallback or Prompt
    Write-Host "Auto-detect failed." -ForegroundColor Yellow
    $ports = [System.IO.Ports.SerialPort]::GetPortNames()
    if ($ports) {
        Write-Host "Available ports: $($ports -join ', ')"
        $targetPort = $ports[-1] # Pick the last one as a guess?
        Write-Host "Guessing port: $targetPort" -ForegroundColor Yellow
    } else {
        Write-Error "No COM ports found!"
    }
}

# Step 3: Upload
Write-Host "Uploading to $targetPort..." -ForegroundColor Cyan
& $CliPath upload --fqbn $FQBN --port $targetPort --input-dir $BuildDir 

if ($LASTEXITCODE -eq 0) {
    Write-Host "Upload Successful!" -ForegroundColor Green
}
else {
    Write-Error "Upload Failed!"
}
