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

$process = Start-Process -FilePath $renode `
    -ArgumentList @("`"$script`"") `
    -WorkingDirectory $workspace `
    -PassThru

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
    if (-not $process.HasExited) {
        Stop-Process -Id $process.Id -Force
    }
    throw "Renode did not open GDB port 3333 within 10 seconds."
}

Write-Output "Renode GDB server is live on localhost:3333!"
Wait-Process -Id $process.Id