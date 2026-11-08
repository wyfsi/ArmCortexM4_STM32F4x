$workspace = $PSScriptRoot
$renodeCommand = Get-Command renode -ErrorAction SilentlyContinue
$renode = if ($renodeCommand) {
    $renodeCommand.Source
} else {
    Join-Path ${env:ProgramFiles} "Renode\renode.exe"
}
$script = Join-Path $workspace "stm32f4.resc"

if (-not (Test-Path $renode)) {
    throw "Renode executable was not found. Add Renode to PATH or install it under Program Files."
}

$tempDirectory = Join-Path ([System.IO.Path]::GetTempPath()) "RenodeSession-$([guid]::NewGuid().ToString('N'))"
New-Item -ItemType Directory -Path $tempDirectory | Out-Null

$originalTemp = $env:TEMP
$originalTmp = $env:TMP
$process = $null

try {
    try {
        $env:TEMP = $tempDirectory
        $env:TMP = $tempDirectory
        $process = Start-Process -FilePath $renode `
            -ArgumentList @("`"$script`"") `
            -WorkingDirectory $workspace `
            -PassThru
    } finally {
        $env:TEMP = $originalTemp
        $env:TMP = $originalTmp
    }

    Write-Output "Waiting for Renode GDB stub on localhost:3333..."
    $ready = $false
    for ($attempt = 0; $attempt -lt 100; $attempt++) {
        if ($process.HasExited) {
            throw "Renode exited before opening GDB port 3333."
        }

        $client = [System.Net.Sockets.TcpClient]::new()
        try {
            $client.Connect("127.0.0.1", 3333)
            $ready = $true
            break
        } catch {
            Start-Sleep -Milliseconds 100
        } finally {
            $client.Dispose()
        }
    }

    if (-not $ready) {
        throw "Renode did not open GDB port 3333 within 10 seconds."
    }

    Write-Output "Renode GDB server is live on localhost:3333!"
    Wait-Process -Id $process.Id
} finally {
    if ($process -and -not $process.HasExited) {
        Stop-Process -Id $process.Id -Force
    }

    Remove-Item -LiteralPath $tempDirectory -Recurse -Force
}