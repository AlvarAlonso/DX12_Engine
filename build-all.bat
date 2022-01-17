@echo off

echo "building all projects..."

mkdir bin

pushd bin
del /Q *
popd

pushd Engine
call build.bat
popd

pushd Sandbox
call build.bat
popd