@echo off
rem Builds, tests and deploys Soccer Bet Adviser on Windows.
rem Edit QT_DIR to match your Qt install (the folder that contains lib\cmake).
setlocal
set QT_DIR=C:\Qt\6.8.0\msvc2022_64
if not "%1"=="" set QT_DIR=%1

cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DCMAKE_PREFIX_PATH="%QT_DIR%" || goto :err
cmake --build build --config Release --parallel || goto :err
ctest --test-dir build -C Release --output-on-failure || goto :err
cmake --build build --config Release --target deploy || goto :err
echo.
echo Done. Run: build\Release\SoccerBetAdviser.exe
exit /b 0
:err
echo.
echo BUILD FAILED - see the messages above.
exit /b 1
