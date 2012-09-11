mc.exe -u -U %~n0.mc
::rc.exe -r %~n0.rc
windres -J rc -O coff -F pe-x86-64 -i event_msg.rc -o event_msg.res
