xcopy %3\%4.dll %1\dll\*.dll /F /Y
xcopy %3\%4.lib %1\lib\*.lib /F /Y
xcopy %3\%4.exp %1\lib\*.exp /F /Y
xcopy %3\%4.pdb %1\lib\*.pdb /F /Y
xcopy %4.h %1\inc /Y /F
xcopy msg_str.h %1\inc /Y /F
exit 0