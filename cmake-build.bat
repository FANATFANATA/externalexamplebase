@echo off
set NDK_PATH=C:\android-ndk-r27d
set JNI_DIR=C:\Users\Dmode\OneDrive\Desktop\folders\games\standoff2\externalbase\jni
set BUILD_DIR=C:\Users\Dmode\OneDrive\Desktop\folders\games\standoff2\externalbase\build
"C:\Program Files\CMake\bin\cmake.EXE" -G "MinGW Makefiles" -S "%JNI_DIR%" -B "%BUILD_DIR%\arm64-v8a" ^
    -DCMAKE_MAKE_PROGRAM="%NDK_PATH%\prebuilt\windows-x86_64\bin\make.exe" ^
    -DCMAKE_TOOLCHAIN_FILE="%NDK_PATH%\build\cmake\android.toolchain.cmake" ^
    -DANDROID_ABI=arm64-v8a ^
    -DANDROID_PLATFORM=android-21 ^
    -DANDROID_STL=c++_static
"C:\Program Files\CMake\bin\cmake.EXE" --build "%BUILD_DIR%\arm64-v8a"
"C:\Program Files\CMake\bin\cmake.EXE" -G "MinGW Makefiles" -S "%JNI_DIR%" -B "%BUILD_DIR%\x86_64" ^
    -DCMAKE_MAKE_PROGRAM="%NDK_PATH%\prebuilt\windows-x86_64\bin\make.exe" ^
    -DCMAKE_TOOLCHAIN_FILE="%NDK_PATH%\build\cmake\android.toolchain.cmake" ^
    -DANDROID_ABI=x86_64 ^
    -DANDROID_PLATFORM=android-21 ^
    -DANDROID_STL=c++_static
"C:\Program Files\CMake\bin\cmake.EXE" --build "%BUILD_DIR%\x86_64"
pause
