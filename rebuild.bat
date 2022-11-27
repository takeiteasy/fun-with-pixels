@echo off
if exist .\build\pplive.dll move .\build\pplive.dll .\build\pplive.%random%.dll
cl pp.c examples/live.c /I. /DPP_LIVE_LIBRARY /MD /link /DLL /SUBSYSTEM:CONSOLE /out:.\build\pplive.dll
del *.obj