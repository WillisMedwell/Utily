@REM This is for local building on windows. You will need emscripten SDK, cmake, and ninja.

@echo off
set VCPKG_PATH=C:/apps/vcpkg/vcpkg/
set EMSDK=C:/apps/emscripten/emsdk/
set EMSCRIPTEN=C:/apps/emscripten/emsdk/upstream/emscripten/
set BUILD_TYPE=Release

@REM NATIVE 
if not exist "build-native\" (
    mkdir build-native
)
cd build-native
call cmake .. -G "Ninja" -DCMAKE_BUILD_TYPE=Release -DBUILD_UTILY_TESTS=1 -DBUILD_UTILY_BENCHMARKS=1 -DBUILD_UTILY_EXAMPLES=1 -DCMAKE_BUILD_TYPE=%BUILD_TYPE%
call cmake --build . --config %BUILD_TYPE%
cd ..

@REM if not exist "build-web\" (
@REM     mkdir build-web
@REM )
@REM cd build-web
@REM call emcmake cmake .. -DBUILD_UTILY_TESTS=1 -DBUILD_UTILY_BENCHMARKS=1 -DCMAKE_TOOLCHAIN_FILE=%VCPKG_PATH%/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=wasm32-emscripten -DEMSCRIPTEN=1 -DVCPKG_CHAINLOAD_TOOLCHAIN_FILE=%EMSCRIPTEN%cmake/Modules/Platform/Emscripten.cmake -DCMAKE_BUILD_TYPE=%BUILD_TYPE%
@REM call cmake --build . --config %BUILD_TYPE%
@REM cd ..

cd build-native
call UtilyTest.exe
call UtilyBenchmark.exe