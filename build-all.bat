@REM This is for local building on windows. You will need emscripten SDK, cmake, and ninja.

@echo off
set VCPKG_PATH=C:/apps/vcpkg/vcpkg/
set EMSDK=C:/apps/emscripten/emsdk/
set EMSCRIPTEN=C:/apps/emscripten/emsdk/upstream/emscripten/
set BUILD_TYPE=Release

@REM NATIVE 
@REM if not exist "build-native\" (
@REM     mkdir build-native
@REM )
@REM cd build-native
@REM call cmake .. -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DBUILD_UTILY_TESTS=1 -DBUILD_UTILY_BENCHMARKS=1 -DCMAKE_BUILD_TYPE=%BUILD_TYPE%
@REM call cmake --build . --config %BUILD_TYPE%
@REM cd ..

if not exist "build-web\" (
    mkdir build-web
)
cd build-web
call emcmake cmake .. -DBUILD_UTILY_TESTS=1 -DBUILD_UTILY_BENCHMARKS=1 -DCMAKE_TOOLCHAIN_FILE=%VCPKG_PATH%/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=wasm32-emscripten -DEMSCRIPTEN=1 -DVCPKG_CHAINLOAD_TOOLCHAIN_FILE=%EMSCRIPTEN%cmake/Modules/Platform/Emscripten.cmake -DCMAKE_BUILD_TYPE=%BUILD_TYPE%
call cmake --build . --config %BUILD_TYPE%
cd ..

cd build-native
@REM call UtilyTest.exe
@REM call UtilyBenchmark.exe