@echo off
call git submodule update --init --recursive
tools\windows\premake5.exe vs2022