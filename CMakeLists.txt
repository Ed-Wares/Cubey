cmake_minimum_required(VERSION 3.10)
project(Cubey)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find GLFW and GLM
find_package(glfw3 CONFIG REQUIRED) # Use CONFIG for modern vcpkg GLFW
find_package(glm CONFIG REQUIRED)

# NEW: Add an include directory for GLAD's headers
include_directories(vendor/glad/include vendor/stb)

# Add GLAD's source file
add_executable(Cubey
    src/Cubey.cpp
    src/stb_impl.cpp
    vendor/glad/src/glad.c
)

# Link the executable against GLFW, OpenGL, and GLM
target_link_libraries(Cubey PRIVATE glfw)

# Ensure executable can find its DLLs on Windows if needed (useful for development)
if (WIN32)
    # This copies necessary DLLs (like glfw3.dll) from vcpkg to the build output directory
    file(GLOB DLL_FILES "$ENV{VCPKG_ROOT}/installed/${VCPKG_TARGET_TRIPLET}/bin/*.dll")
    install(FILES ${DLL_FILES} DESTINATION "$<TARGET_FILE_DIR:Cubey>")
endif()