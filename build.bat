@echo off
if exist .\build\pplive.dll move .\build\pplive.dll .\build\pplive.%random%.dll
cl pp.c examples/live.c /I. /DPP_LIVE_LIBRARY /MD /link /DLL /SUBSYSTEM:CONSOLE /out:.\build\pplive.dll
if exist .\build\pp.exe move .\build\pp.exe .\build\pp.%random%.exe
cl pp.c win32\*.c /I.\ /I.\win32 Psapi.lib /DPP_LIVE /link /SUBSYSTEM:CONSOLE  /out:.\build\pp.exe
.\build\pp.exe -p .\build\pplive.dll
del *.obj