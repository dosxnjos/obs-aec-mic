@echo off
rem Removes the aec-mic OBS plugin. / Remove o plugin aec-mic do OBS.
tasklist /fi "imagename eq obs64.exe" | find /i "obs64.exe" >nul && (echo. & echo  Close OBS and run this again. / Feche o OBS e rode de novo. & echo. & pause & exit /b 1)
rmdir /s /q "%ProgramData%\obs-studio\plugins\aec-mic" 2>nul
echo.
echo  Removed. / Removido.
pause
