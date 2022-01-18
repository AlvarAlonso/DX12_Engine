@echo off

echo "building sandbox"

set include_paths= /I..\Engine\src
set file_paths= ..\Sandbox\main.cpp

pushd ..\bin
cl /EHsc /WX /Zi %include_paths% /DDEBUG %file_paths% /link engine.lib user32.lib d3dcompiler.lib D3D12.lib dxgi.lib
popd