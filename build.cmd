@echo off

@REM CMAKE
cd src
cmake -B ../bin -G "MinGW Makefiles" -S ../src

@REM COPY SHADERS AND OTHER CONTENTS
cd ..
xcopy shaders bin\shaders\ /Y > nul
echo Copied shaders
xcopy img bin\img\ /Y > nul
echo Copied images

@REM COMPILE
cd bin
mingw32-make

@echo on
