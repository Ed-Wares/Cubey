@echo off
SET prj_name=Cubey
SET "current_dir=%~dp0"
SET "build_dir=%current_dir%build\"
SET "distrib_dir=%current_dir%distrib\%prj_name%\"

cd /d "%current_dir%"

echo removing old build
rmdir /s /q "%build_dir%"

echo creating build directory "%build_dir%"
mkdir "%build_dir%"
mkdir "%distrib_dir%"
pushd "%build_dir%"

REM echo Install dependencies with cmd: vcpkg install glfw3 glm freetype
echo dependencies can be installed in MinGW with: pacman -S mingw-w64-ucrt-x86_64-glfw mingw-w64-ucrt-x86_64-glm mingw-w64-ucrt-x86_64-freetype zip

echo Configuring and building with CMake
cmake .. -G "MinGW Makefiles"
REM cmake .. -G "Visual Studio 17 2022"
cmake --build . --config Release

echo copying binaries to the distrib folder...
copy /Y *.exe "%distrib_dir%"
copy /Y *.dll "%distrib_dir%"
copy /Y ..\font.ttf "%distrib_dir%"
copy /Y ..\smiley.png "%distrib_dir%"
popd

pushd "%distrib_dir%.." && C:\msys64\usr\bin\zip.exe -r %prj_name%.zip %prj_name% && popd
echo Created distribution file at distrib\%prj_name%.zip