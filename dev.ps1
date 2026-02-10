$ErrorActionPreference = "Stop"

# Configuration
$FQBN = "m5stack:esp32:m5stack_atoms3:CDCOnBoot=cdc"
$BuildDir = "$PSScriptRoot\build"
$MonitorBaud = 115200
$DevicePort = "COM14"

# Locate Arduino CLI
$CliPath = "arduino-cli"
if (-not (Get-Command "arduino-cli" -ErrorAction SilentlyContinue)) {
    $DefaultPath = "C:\Program Files\Arduino CLI\arduino-cli.exe"
    if (Test-Path $DefaultPath) { $CliPath = $DefaultPath }
}

# Cleanup lingering monitors
Get-Process arduino-cli -ErrorAction SilentlyContinue | ForEach-Object {
    Write-Host "[DEV] Killing lingering arduino-cli process ($($_.Id))..." -ForegroundColor DarkYellow
    Stop-Process -Id $_.Id -Force
}

function Do-Flash {
    param($TargetPort)
    Write-Host "`n[DEV] Building & Flashing..." -ForegroundColor Magenta
    try {
        & $CliPath compile --fqbn $FQBN --build-path $BuildDir "$PSScriptRoot"
        if ($LASTEXITCODE -ne 0) { throw "Build Failed" }

        & $CliPath upload --fqbn $FQBN --port $TargetPort --input-dir $BuildDir
        if ($LASTEXITCODE -ne 0) { throw "Upload Failed" }
        
        Write-Host "[DEV] Success! Restarting monitor..." -ForegroundColor Green
        Start-Sleep -Seconds 1
    }
    catch {
        Write-Error $_
        Write-Host "[DEV] Press any key to resume monitor..."
        $null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
    }
}

Write-Host "[DEV] Starting Development Loop for $DevicePort."
Write-Host "      - Press 'F' at ANY TIME to build & upload."
Write-Host "      - Press 'Q' to quit." -ForegroundColor Cyan

$lastStatus = ""

while ($true) {
    # 1. Fast Port Check (Prevents spamming arduino-cli)
    if ($DevicePort -notin [System.IO.Ports.SerialPort]::GetPortNames()) {
        if ($lastStatus -ne "WAITING") {
            Write-Host "`n[DEV] Waiting for $DevicePort..." -NoNewline -ForegroundColor DarkGray
            $lastStatus = "WAITING"
        }
        else {
            # Just wait silently
        }
        Start-Sleep -Milliseconds 500
        
        # Check for Q to quit even while waiting
        if ([Console]::KeyAvailable) {
            $key = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
            if ($key.Character -eq 'q') { break }
            # Allow flashing even if port not seen (sometimes it appears briefly)
            if ($key.Character -eq 'f') { Do-Flash -TargetPort $DevicePort }
        }
        continue
    }

    # 2. Start Monitor
    if ($lastStatus -eq "WAITING") { Write-Host "" } # Newline
    Write-Host "[DEV] Connected. Listening..." -ForegroundColor Green
    $lastStatus = "CONNECTED"

    $pInfo = New-Object System.Diagnostics.ProcessStartInfo
    $pInfo.FileName = $CliPath
    $pInfo.Arguments = "monitor -p $DevicePort -c $MonitorBaud --quiet"
    $pInfo.RedirectStandardOutput = $true
    $pInfo.RedirectStandardError = $true
    $pInfo.UseShellExecute = $false
    $pInfo.CreateNoWindow = $true
    
    $proc = New-Object System.Diagnostics.Process
    $proc.StartInfo = $pInfo
    $proc.Start() | Out-Null
    
    $FLASH_TRIGGER = $false
    $FORCE_EXIT = $false
    $PORT_BUSY = $false

    # 3. Interactive Loop
    try {
        $buffer = New-Object char[] 4096
        while (-not $proc.HasExited) {
            # READ OUTPUT (Buffered)
            while ($proc.StandardOutput.Peek() -gt -1) { 
                $count = $proc.StandardOutput.Read($buffer, 0, $buffer.Length)
                if ($count -gt 0) {
                    $text = [string]::new($buffer, 0, $count)
                    [Console]::Write($text)
                    if ($text -match "Serial port busy") { $PORT_BUSY = $true }
                }
            }
            while ($proc.StandardError.Peek() -gt -1) { 
                $count = $proc.StandardError.Read($buffer, 0, $buffer.Length)
                if ($count -gt 0) {
                    $text = [string]::new($buffer, 0, $count)
                    [Console]::Write($text)
                    if ($text -match "Serial port busy") { $PORT_BUSY = $true }
                }
            }

            # READ INPUT
            if ([Console]::KeyAvailable) {
                $key = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
                if ($key.Character -eq 'f' -or $key.Character -eq 'F') {
                    $FLASH_TRIGGER = $true
                    $proc.Kill()
                    break
                }
                if ($key.Character -eq 'q' -or $key.Character -eq 'Q') {
                    $proc.Kill()
                    $FORCE_EXIT = $true
                    break
                }
            }
            Start-Sleep -Milliseconds 10
        }
    }
    catch {}

    if ($FORCE_EXIT) {
        Write-Host "`n[DEV] Exiting."
        exit
    }

    if ($FLASH_TRIGGER) {
        Do-Flash -TargetPort $DevicePort
    } 
    elseif ($PORT_BUSY) {
        Write-Host "[DEV] Port busy (OS hasn't released it yet). Waiting 2s..." -ForegroundColor Red
        Start-Sleep -Seconds 2
    }
    else {
        # Monitor died naturally (device slept/disconnected)
        Start-Sleep -Milliseconds 250
    }
}
