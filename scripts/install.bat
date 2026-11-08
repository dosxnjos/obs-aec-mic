@echo off
rem Installs the aec-mic OBS plugin for all users (C:\ProgramData\obs-studio\plugins\aec-mic).
rem Instala o plugin aec-mic no OBS (C:\ProgramData\obs-studio\plugins\aec-mic).
setlocal
if not exist "%~dp0aec-mic\bin\64bit\aec-mic.dll" (echo  aec-mic folder not found next to this file / pasta aec-mic nao encontrada. & pause & exit /b 1)
tasklist /fi "imagename eq obs64.exe" | find /i "obs64.exe" >nul && (echo. & echo  Close OBS and run this again. / Feche o OBS e rode de novo. & echo. & pause & exit /b 1)
set "DEST=%ProgramData%\obs-studio\plugins\aec-mic"
if exist "%DEST%" rmdir /s /q "%DEST%"
xcopy /e /i /q /y "%~dp0aec-mic" "%DEST%" >nul || goto fail
echo.
echo  Installed / Instalado: %DEST%
echo  Open OBS and add the source "Microphone with echo cancellation (Windows)".
echo  Abra o OBS e adicione a fonte "Microfone com cancelamento de eco (Windows)".
echo.
pause
exit /b 0
:fail
echo.
echo  Copy failed. Try right-click, Run as administrator. / Falhou: tente Executar como administrador.
pause
exit /b 1
