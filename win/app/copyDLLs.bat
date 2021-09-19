del ..\%3\*.dll /Q /F
xcopy %1\dll\*.dll ..\%3 /F /Y /E /R
if "%3"=="Debug" (
  xcopy %1\lib\*.pdb ..\%3 /F /Y /E /R
)
if "%3"=="Release" (
  %1\Install\installer.bat %1 %2 %3
)
exit 0