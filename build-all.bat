@REM This is for local building on windows. You will need emscripten SDK, cmake, and ninja.

@echo off
set VCPKG_PATH=C:/apps/vcpkg/vcpkg/

@REM NATIVE 
if not exist "build-native\" (
    mkdir build-native
)
cd build-native
call cmake .. -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DBUILD_UTILY_TESTS=1 -DBUILD_UTILY_BENCHMARKS=1
call cmake --build . --config Release
cd ..

@REM WEB
if not exist "build/emscripten-ninja" (
    mkdir build/emscripten-ninja
)
call emcmake cmake --preset emscripten-ninja -DCMAKE_SUPPRESS_DEVELOPER_WARNINGS=ON
call cmake --build build/emscripten-ninja --config Release
cd ..
cd build-native
call UtilyTest.exe
call UtilyBenchmark.exe