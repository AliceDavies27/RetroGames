@echo off

if not exist build mkdir build

pushd build

set CompilerFlags=/nologo /MTd /Z7 /Oi /WX /W4 /wd4100 /wd4189
set SourceFiles=..\src\*.c
set LinkerFlags=/subsystem:windows /incremental:no /opt:ref
set LibFiles=user32.lib

cl %CompilerFlags% %SourceFiles% /Fe:RetroGames.exe /link %LinkerFlags% %LibFiles%

popd