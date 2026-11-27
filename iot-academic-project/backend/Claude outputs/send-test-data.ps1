# Green Home - yeb3ath el JSON de test lkol lel API
# Lancement (fi dossier greenhome_api, serveur demarre) :
#   powershell -ExecutionPolicy Bypass -File .\send-test-data.ps1
#   powershell -ExecutionPolicy Bypass -File .\send-test-data.ps1 -BaseUrl http://192.168.1.15:8000

param(
    [string]$BaseUrl = "http://127.0.0.1:8000",
    [string]$DataFile = "$PSScriptRoot\greenhome-test-data.json"
)

$data = Get-Content $DataFile -Raw -Encoding UTF8 | ConvertFrom-Json

function Send-Json($path, $obj) {
    $json  = $obj | ConvertTo-Json -Depth 5 -Compress
    $bytes = [System.Text.Encoding]::UTF8.GetBytes($json)
    try {
        $res = Invoke-RestMethod -Method Post -Uri "$BaseUrl$path" `
               -ContentType "application/ld+json; charset=utf-8" -Body $bytes
        Write-Host "OK   POST $path -> id $($res.id)" -ForegroundColor Green
    } catch {
        $code = $_.Exception.Response.StatusCode.value__
        Write-Host "FAIL POST $path -> HTTP $code  $json" -ForegroundColor Red
    }
}

foreach ($endpoint in @("/api/devices", "/api/sensors", "/api/measurements", "/api/commands")) {
    Write-Host "`n=== $endpoint ===" -ForegroundColor Cyan
    foreach ($item in $data."POST $endpoint") { Send-Json $endpoint $item }
}

Write-Host "`n=== Tests d'erreur (lazem FAIL) ===" -ForegroundColor Yellow
$errs = $data."TESTS D'ERREUR (lazem yfechlou)"
foreach ($p in $errs.PSObject.Properties) {
    Write-Host "- $($p.Name)"
    Send-Json "/api/measurements" $p.Value
}

Write-Host "`n=== GET /api/measurements ===" -ForegroundColor Cyan
$list = Invoke-RestMethod -Uri "$BaseUrl/api/measurements" -Headers @{ Accept = "application/json" }
$list | Format-Table
