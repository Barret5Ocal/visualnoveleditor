@echo off 

fxc /T vs_4_0 /E VShader /Fh vshader.h shader.shader

fxc /T ps_4_0 /E PShader /Fh pshader.h shader.shader