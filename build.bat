@echo off

if not exist build mkdir build

pushd build

set CompilerFlags=/nologo /MTd /Z7 /Oi /WX /W4 /wd4100 /wd4189 /wd4201
set SourceFiles=..\src\*.c ..\src\games\*.c
set IncludeDirs=/I..\src
set LinkerFlags=/subsystem:windows /incremental:no /opt:ref
set LibFiles=user32.lib gdi32.lib

cl %CompilerFlags% %IncludeDirs% %SourceFiles% /Fe:RetroGames.exe /link %LinkerFlags% %LibFiles%

popd