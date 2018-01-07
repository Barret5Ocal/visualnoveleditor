@echo off 

if not exist ..\..\build mkdir ..\..\build

fxc /T vs_4_0 /E VShader /Fh vshader.h shader.shader

fxc /T ps_4_0 /E PShader /Fh pshader.h shader.shader

pushd ..\..\build

cl -nologo -Z7 ..\project\code\win32_vnedit.cpp User32.lib Gdi32.lib Winmm.lib d3d11.lib 


popd 

