@echo off
echo updating submodules...
git submodule update --init --recursive

echo.
echo generating solution...
premake5 %* vs2019