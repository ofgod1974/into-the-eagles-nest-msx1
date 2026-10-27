@echo off
if exist "%~dp0tools\vscode\Code.exe" (
  start "" "%~dp0tools\vscode\Code.exe" "%~dp0"
) else (
  code "%~dp0"
)
