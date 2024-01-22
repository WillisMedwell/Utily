@echo off
set VCPKG_PATH=C:/apps/vcpkg/vcpkg/

@REM NATIVE 
if not exist "build-native\" (
    mkdir build-native
)
cd build-native
call cmake .. -G "Ninja" -DCMAKE_BUILD_TYPE=Release
call cmake --build . --config Release
cd ..

@REM WEB
if not exist "build-web\" (
    mkdir build-web
)
cd build-web
call emcmake cmake .. -DEMSCRIPTEN=1 -DCMAKE_BUILD_TYPE=Release
call cmake --build . --config Release
cd ..

cd build-native
call UtilyTest.exe
call UtilyBenchmark.exe