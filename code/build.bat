@echo off 

if not exist ..\..\build mkdir ..\..\build

REM fxc /T vs_4_0 /E VShader /Fh vshader.h shader.shader

REM fxc /T ps_4_0 /E PShader /Fh pshader.h shader.shader

pushd ..\..\build

if exist ..\ctime\ctime.exe ..\ctime\ctime.exe -begin "ctime.ctm"

cl -nologo -Z7 ..\project\code\win32_vnedit.cpp User32.lib Gdi32.lib Winmm.lib d3d11.lib d3dcompiler.lib

if exist ..\ctime\ctime.exe ..\ctime\ctime.exe -end "ctime.ctm"

popd 