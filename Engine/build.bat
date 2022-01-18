@echo off

echo "building engine"

mkdir ..\bin

pushd ..\bin
cl /EHsc /WX /Zi /DDEBUG ..\Engine\src\*.cpp
lib /out:engine.lib *.obj
popd