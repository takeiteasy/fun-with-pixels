@echo off
if exist .\build\ppLive.dll move .\build\ppLive.dll .\build\ppLive.%random%.dll
cl .\live\ppUtils.c .\examples\live.c /I.\src /I.\live /MD /link /DLL /SUBSYSTEM:CONSOLE /out:.\build\ppLive.dll
if exist .\build\pp.exe move .\build\pp.exe .\build\pp.%random%.exe
cl .\src\ppWindows.c .\live\*.c /I.\src /I.\live Psapi.lib /link /SUBSYSTEM:CONSOLE  /out:.\build\pp.exe
del *.obj