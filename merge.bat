@echo off

set DIR=%~dp0
set MERGETOOL="D:\Program Files\mergetool\mergetool.exe"

%MERGETOOL% -i %DIR%\input -o %DIR%\output