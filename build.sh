#!/bin/bash
prj_name=Cubey
current_dir=$(dirname "$(realpath "$0")" )
build_dir="$current_dir/build"
distrib_dir="$current_dir/distrib/$prj_name"

echo removing old build
rm -fr "$build_dir"

echo creating build and distrib directories "$build_dir" and "$distrib_dir"
mkdir -p "$build_dir"
mkdir -p "$distrib_dir"
cd "$build_dir"

#echo Install dependencies with cmd: vcpkg install glfw3 glm freetype
echo dependencies can be installed in MinGW with: pacman -S mingw-w64-ucrt-x86_64-glfw mingw-w64-ucrt-x86_64-glm mingw-w64-ucrt-x86_64-freetype zip

echo Configuring and building with CMake
cmake ..
cmake --build . --config Release

echo copying binaries to the distrib folder...
cp -v *.exe "$distrib_dir"
cp -v *.dll "$distrib_dir"
cd ..

pushd "$distrib_dir/.." && zip -r $prj_name.zip $prj_name && popd
echo Created distribution file at distrib/$prj_name.zip
