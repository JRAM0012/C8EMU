local RAYLIB_INSTALL_DIR = "C:/raylib/raylib"

workspace "sol"
    architecture "x32"
    configurations {"Debug", "Release"}

project "C8EMU"
    targetdir "bin/"
    kind "ConsoleApp"
    objdir ".int/C8EMU"
    files { "src/main.c", "src/C8EMU.c", "src/C8EMU.h"}
    includedirs {
        RAYLIB_INSTALL_DIR .. "/src", 
        RAYLIB_INSTALL_DIR .. "/src/extras", 
    }
    links {"raylib-nolog", "gdi32", "winmm"}
    libdirs {"raylib-nolog"}

project "raylib-nolog"
    targetdir "lib\\"
    kind "StaticLib"
    objdir ".int\\raylib-nolog"
    files { RAYLIB_INSTALL_DIR .. "/src/*.c", }
    defines {"PLATFORM_DESKTOP"}
    includedirs { RAYLIB_INSTALL_DIR .. "/src/external/glfw/include" }