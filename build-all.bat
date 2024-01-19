@echo off
set VCPKG_PATH=C:/apps/vcpkg/vcpkg/

@REM NATIVE 
if not exist "build-native\" (
    mkdir build-native
)
cd build-native
call cmake .. -G "Ninja" -DCMAKE_TOOLCHAIN_FILE=%VCPKG_PATH%/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-windows -DCMAKE_BUILD_TYPE=Release
call cmake --build . --config Release
call UtilyTest.exe
cd ..


@REM WEB
if not exist "build-web\" (
    mkdir build-web
)
cd build-web
call emcmake cmake .. -DCMAKE_TOOLCHAIN_FILE=%VCPKG_PATH%/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=wasm32-emscripten -DEMSCRIPTEN=1 -DVCPKG_CHAINLOAD_TOOLCHAIN_FILE=%EMSCRIPTEN%cmake/Modules/Platform/Emscripten.cmake -DCMAKE_BUILD_TYPE=Release
call cmake --build . --config Release
cd ..

