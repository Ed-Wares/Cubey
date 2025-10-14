# Cubey
Cubey is a basic OpenGL Application written in C++.  It simply displays a rotating cube (named Cubey), which can be rotated further using the keyboard arrow keys.  The example also renders a simple text overlay above the cube.  The application is using more modern OpenGL 3.3 with shaders for both the cube and the text.

### Demo

Here is a quick look at the application:

![Demo](https://github.com/Ed-Wares/Cubey/blob/main/DemoCubey.gif?raw=true)

## How It Works: 
This is a multi-platform OpenGL C++ application using GLFW (Graphics Library Framework) for windowing and input, and GLAD for loading OpenGL function pointers. This setup is highly standard for desktop OpenGL development.

It uses CMake for building and GLM for mathematical operations.
- GLFW: A lightweight, cross-platform library for creating OpenGL contexts, windows, and handling input. It's often preferred over SDL for pure OpenGL development due to its simplicity and focus.
- GLAD: Essential for loading OpenGL function pointers at runtime, allowing you to use modern OpenGL features (like shaders, VBOs, VAOs).
- GLM: Provides essential matrix and vector math operations for 3D graphics.

### The 2D Rendered Text Overlay (HUD)
- Separate Shaders: The text needs its own shaders because it's fundamentally different from the 3D cube. The cube uses a perspective projection (things farther away are smaller), while the text uses an orthographic projection (a flat, 2D view with no perspective).
- Orthographic Projection: We create a projection matrix with glm::ortho. This matrix maps screen pixel coordinates (e.g., from (0, 0) to (800, 600)) directly to OpenGL's normalized device coordinates. This ensures the text is always rendered flat on the screen, like a Heads-Up Display (HUD).
- Blending: glEnable(GL_BLEND) is crucial. The font texture we create only has one channel (representing the alpha, or transparency). Blending allows the GPU to draw the background, and then draw the text on top, using the font texture's alpha to make the non-character parts transparent.
- Rendering Order: In the main loop, we draw the 3D scene first, then we switch shaders, set up the 2D projection, and draw the 2D text last. This ensures the text always appears on top of the cube.

## Building

Build your own application binaries.

Prerequesites required for building source
- CMake: Download and install from [cmake.org](https://cmake.org/download/).  Add the installed path to the top of your Windows PATH environment variable.  For example: ```C:\Program Files\CMake\bin```
- vcpkg: If you don't have it already clone from: 

  ```git clone https://github.com/microsoft/vcpkg```

  ```./vcpkg/bootstrap-vcpkg.bat```

  ```./vcpkg/vcpkg install glfw3 glm freetype```
- C++ Compiler: msys2 - download the latest installer from the [MSYS2](https://github.com/msys2/msys2-installer/releases/download/2024-12-08/msys2-x86_64-20241208.exe)
  - Run the installer and follow the steps of the installation wizard. Note that MSYS2 requires 64 bit Windows 8.1 or newer.
  - Run Msys2 terminal and from this terminal, install the MinGW-w64 toolchain by running the following command:
  
    ```pacman -S --needed base-devel mingw-w64-ucrt-x86_64-toolchain```
  - Accept the default number of packages in the toolchain group by pressing Enter (default=all).
  - Enter Y when prompted whether to proceed with the installation.
  - Add the path of your MinGW-w64 bin folder (C:\msys64\ucrt64\bin) to the Windows PATH environment variable.
  - To check that your MinGW-w64 tools are correctly installed and available, open a new Command Prompt and type:

    ```g++ --version```
- Optionally, install Visual Studio Code IDE (with C++ extensions).  [VsCode](https://code.visualstudio.com/download)

- Optionally, to update GLAD Loader library.
  - Go to the GLAD Web Service: https://glad.dav1d.de/
    - Configure the following options:
    - Language: C/C++
    - Specification: OpenGL
    - API gl: Version 3.3 (or higher, e.g., 4.6 if you want newer features and your hardware supports it)
    - Profile: Core
    - Options: Make sure Generate a loader is checked.
  - Click Generate to download the .zip.
  - Unzip and copy the include and src folders into a ```vendor/glad``` directory in your project's root.
- Optionally, to update stb_truetype library.
  - Download stb_truetype.h: Go to the stb single-file libraries repository and download [stb_truetype.h](https://raw.githubusercontent.com/nothings/stb/refs/heads/master/stb_truetype.h). Place it in a new directory in your project, for example, ```vendor/stb/```
  - Download a Font: You need a .ttf font file. You can use a standard system font (like arial.ttf from your Windows Fonts directory)
  - Create an Implementation File: To avoid linker errors, it's best practice to create a separate C++ file that defines the implementation for stb_truetype. ```src/stb_impl.cpp```

Build binaries by running the ```build.bat``` script or from VsCode by running the Build and Debug Task.

