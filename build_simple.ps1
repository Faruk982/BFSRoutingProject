# Simple build script for BFSRoutingProject
Write-Host "========================================" -ForegroundColor Cyan
Write-Host "Building BFS Routing Project" -ForegroundColor Cyan  
Write-Host "========================================" -ForegroundColor Cyan

# Set environment
$env:PATH = "D:\downloads\omnetpp-6.2.0-windows-x86_64\omnetpp-6.2.0\bin;D:\downloads\omnetpp-6.2.0-windows-x86_64\omnetpp-6.2.0\tools\win64\mingw64\bin;D:\downloads\omnetpp-6.2.0-windows-x86_64\omnetpp-6.2.0\tools\win64\usr\bin;" + $env:PATH

Set-Location "D:\downloads\omnetpp-6.2.0-windows-x86_64\omnetpp-6.2.0\samples\BFSRoutingProject_New\src"

Write-Host "`n[1/2] Cleaning old files..." -ForegroundColor Yellow
if (Test-Path "*_m.cc") { Remove-Item "*_m.cc" }
if (Test-Path "*_m.h") { Remove-Item "*_m.h" }

Write-Host "[2/2] Building with make..." -ForegroundColor Yellow
& make MODE=release all

if ($LASTEXITCODE -eq 0) {
    Write-Host "`nBuild successful!" -ForegroundColor Green
} else {
    Write-Host "`nBuild failed!" -ForegroundColor Red
}
