@echo off

set VCD=E:\VS2008\VC
set SDK=E:\VSSDK\v6.0A

set APP=bci
set BIN=bin
set MCH=
set SUF=

set INCLUDE=%VCD%\include;%VCD%\atlmfc\include;%SDK%\include;D:\src\inc
set LIB=%VCD%\lib%SUF%;%VCD%\atlmfc\lib%SUF%;%SDK%\lib%SUF%

set DF1=/D_WIN32_WINNT=0x0502 /DUNICODE /D_UNICODE
set DF2=/D_CRT_SECURE_NO_WARNINGS /D_CRT_NONSTDC_NO_WARNINGS /D_CRT_NON_CONFORMING_SWPRINTFS
set COP1=/GF /Gy /GS- /GR- /Gr /Zp8 /c /J /nologo /W3 /LD /Oi
set COP2=/EHs-c-
set LOP1=%MCH% /nologo /release /MANIFEST:NO
set LOP2=/SUBSYSTEM:WINDOWS /nodefaultlib /entry:WinMainCRTStartup
set DEF=
set SRC=%APP%.cpp
set LN1=%APP%.obj
set LN2=gdi32.lib kernel32.lib user32.lib shell32.lib
set LN3=winmm.lib chkstk.obj
set OUT=/out:bin32\%APP%.exe

if exist resource.rc (
	"%SDK%\bin\rc.exe"	resource.rc
	set RES=resource.res
)

"%VCD%\%BIN%\cl.exe"	%COP1% %COP2% %DF1% %DF2% %SRC%
"%VCD%\%BIN%\link.exe"	%LOP1% %LOP2% %LN1% %LN2% %LN3% %DEF% %RES% %OUT%

del %APP%.obj
del %RES%
