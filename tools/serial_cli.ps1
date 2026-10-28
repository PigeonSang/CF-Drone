# 交互式串口终端：把串口输出实时打印，同时把你输入的命令发下去
# 用法：
#   powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\serial_cli.ps1
#   powershell -NoProfile -ExecutionPolicy Bypass -File .\tools\serial_cli.ps1 -Port COM4
# 注意：运行前关闭 Arduino IDE / VS Code / ESP-IDF 的串口监视器，否则端口被占用。

param(
    [string]$Port = "COM4",
    [int]$Baud = 115200
)

$ErrorActionPreference = "Stop"

$ports = [System.IO.Ports.SerialPort]::GetPortNames()
Write-Host "可用串口: $($ports -join ', ')" -ForegroundColor Cyan
if ($ports -notcontains $Port) {
    Write-Host "$Port 不存在。" -ForegroundColor Red
    exit 1
}

$sp = New-Object System.IO.Ports.SerialPort($Port, $Baud, [System.IO.Ports.Parity]::None, 8, [System.IO.Ports.StopBits]::One)
$sp.ReadTimeout = 100
$sp.WriteTimeout = 500
$sp.Encoding = [System.Text.Encoding]::UTF8
$sp.DtrEnable = $false   # 避免误触发自动复位
$sp.RtsEnable = $false

try {
    $sp.Open()
} catch {
    Write-Host "打开 $Port 失败：$($_.Exception.Message)" -ForegroundColor Red
    Write-Host "通常是串口被别的程序占用。" -ForegroundColor Yellow
    exit 1
}

Write-Host "已连接 $Port @ $Baud" -ForegroundColor Green
Write-Host "直接输入命令并回车发送；Ctrl+C 退出。" -ForegroundColor DarkGray
Write-Host ""

$line = ""
try {
    while ($true) {
        # 1) 把串口收到的内容打出来
        try {
            $chunk = $sp.ReadExisting()
            if ($chunk) { Write-Host -NoNewline $chunk }
        } catch { }

        # 2) 处理键盘输入
        $hasKey = $false
        try { $hasKey = [Console]::KeyAvailable } catch { $hasKey = $false }

        if ($hasKey) {
            $k = [Console]::ReadKey($true)
            switch ($k.Key) {
                'Enter' {
                    $sp.Write("$line`n")
                    $line = ""
                    Write-Host ""
                }
                'Backspace' {
                    if ($line.Length -gt 0) {
                        $line = $line.Substring(0, $line.Length - 1)
                        Write-Host -NoNewline "`b `b"
                    }
                }
                'Escape' {
                    $line = ""
                    Write-Host ""
                }
                default {
                    if ($k.KeyChar -and [int]$k.KeyChar -ge 32) {
                        $line += $k.KeyChar
                        Write-Host -NoNewline $k.KeyChar
                    }
                }
            }
        }

        Start-Sleep -Milliseconds 8
    }
} finally {
    $sp.Close()
    $sp.Dispose()
    Write-Host ""
    Write-Host "串口已关闭。" -ForegroundColor DarkGray
}
