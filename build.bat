SET "CurrentDir=%~dp0"
SET build_dir=%CurrentDir%build
SET msys2bash=c:\msys64\msys2_shell.cmd -ucrt64 -c 

cd /d "%CurrentDir%"

mkdir %build_dir%
cd %build_dir%

REM echo Installing dependencies with vcpkg
REM vcpkg install glfw3 glm freetype

echo Configuring and building with CMake
cmake .. -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake
cmake --build .
cd ..

REM echo Building in %build_dir% using msys2_shell: %bash%
REM %msys2bash% "cd ""%build_dir%"" ; cmake ""%CurrentDir%"" -G 'MinGW Makefiles'"
REM pause
REM %msys2bash% "cmake --build . ; sleep 3"



REM rmdir /s /q build