@echo off
IF "%~1"=="" (
    cl src\ppWindows.c /MD /I. /I.\src /link /DLL /out:.\build\pp.dll
) ELSE (
    cl %* src\ppWindows.c /I.\ /I.\src /link /SUBSYSTEM:WINDOWS /out:.\build\pp.exe
)
del *.obj