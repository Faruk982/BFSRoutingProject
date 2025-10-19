@echo off
REM Quick Build and Run Script for Windows
REM This script builds and runs the BFS Routing simulation

echo ============================================
echo  BFS Routing Project - Quick Build & Run
echo ============================================
echo.

cd /d %~dp0

echo [1/3] Cleaning old files...
if exist out rmdir /s /q out
if exist src\*_m.cc del /q src\*_m.cc
if exist src\*_m.h del /q src\*_m.h

echo [2/3] Building project...
cd src
call make MODE=debug all
if %errorlevel% neq 0 (
    echo.
    echo ERROR: Build failed!
    pause
    exit /b 1
)

echo [3/3] Running simulation...
cd ..\simulations
..\src\BFSRoutingProject_New.exe

pause
