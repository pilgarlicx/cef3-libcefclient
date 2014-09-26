@echo off
setlocal

set PWD=%~dp0

pushd %PWD% && ^
nmake run-cmake /f NMakefile && ^
popd
pause

endlocal
