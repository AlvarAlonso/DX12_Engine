@echo off

echo "building engine"

set include_paths= /I..\Engine\src

mkdir ..\bin

pushd ..\bin
cl /EHsc /WX /Zi %include_paths% /DDEBUG ..\Engine\src\*.cpp
lib /out:engine.lib *.obj
popd