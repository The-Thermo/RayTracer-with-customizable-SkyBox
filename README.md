# RayTracer-with-customizable-SkyBox
This is the fully built Ray Tracer project that is gone over by TheCherno (https://github.com/TheCherno) on You Tube.  This has the same features (sadly the built in multithreading tool is not working on my VS) however, this also has a new customizable SkyBox feature which allows the user to customize the sky which ever color they like. 
![RayTracer with Sky Color edit feature](https://github.com/user-attachments/assets/aab54807-2652-4def-8195-5700dfeac5d4)
_<center>Ray Tracer with optional Sky Edit mode made with Walnut</center>_

# Walnut:
Walnut is a simple application framework built with Dear ImGui and designed to be used with Vulkan - basically this means you can seemlessly blend real-time Vulkan rendering with a great UI library to build desktop applications. The plan is to expand Walnut to include common utilities to make immediate-mode desktop apps and simple Vulkan applications.

Currently supports Windows - with macOS and Linux support planned. Setup scripts support Visual Studio 2022 by default.
![image](https://github.com/user-attachments/assets/f989e16c-f3bd-411a-970d-b569db98ae0e)
_<center>Forest Launcher - an application made with Walnut</center>_

## Requirements
- [Visual Studio 2022](https://visualstudio.com) (not strictly required, however included setup scripts only support this)
- [Vulkan SDK](https://vulkan.lunarg.com/sdk/home#windows) (preferably a recent version)

## Getting Started
Once you've cloned, run `scripts/Setup.bat` to generate Visual Studio 2022 solution/project files. Once you've opened the solution, you can run the WalnutApp project to see a basic example (code in `WalnutApp.cpp`). I recommend modifying that WalnutApp project to create your own application, as everything should be setup and ready to go.

### 3rd party libaries
- [Dear ImGui](https://github.com/ocornut/imgui)
- [GLFW](https://github.com/glfw/glfw)
- [stb_image](https://github.com/nothings/stb)
- [GLM](https://github.com/g-truc/glm) (included for convenience)

### Additional
- Walnut uses the [Roboto](https://fonts.google.com/specimen/Roboto) font ([Apache License, Version 2.0](https://www.apache.org/licenses/LICENSE-2.0))

![Project photo 1](https://github.com/user-attachments/assets/4c814757-0daa-4a35-aa92-53a0f2d50972)
![Project photo 2](https://github.com/user-attachments/assets/06cc57ab-d116-445b-a4c4-324c4f4b5cc1)




