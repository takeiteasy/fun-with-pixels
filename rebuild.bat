@echo off
if exist .\build\ppLive.dll move .\build\ppLive.dll .\build\ppLive.%random%.dll
cl .\live\ppUtils.c .\examples\live.c /I.\src /I.\live /MD /link /DLL /SUBSYSTEM:CONSOLE /out:.\build\ppLive.dll
del *.obj