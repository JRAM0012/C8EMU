#include <stdio.h>
#include "raylib.h"
#include "rlgl.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#include "C8EMU.h"

#define GUI_PADDINGRL 360
#define GUI_PADDINGTB 20

#define KEY_LOCATION_LEFTRIGHT 220
#define KEY_LOCATION_TOPDOWN 180

int main()
{
    InitWindow(500, 500, "C8EMU");
    InitCPU();
    // LoadProgram("assets\\IBM Logo.ch8");
    // LoadProgram("assets\\testprogram.ch8");
    // LoadProgram("assets\\tank.ch8");
    LoadProgram("assets\\invaders.ch8");
    // LoadProgram(&c8, "assets\\pong.ch8");
    // LoadProgram(&c8, "assets\\trip8.ch8");
    // LoadProgram("assets\\breakout.ch8");
    int run = 0;
    Texture2D screentarget;

    Image image = GenImageChecked(64, 32, 8, 4, RED, GREEN);
    screentarget = LoadTextureFromImage(image);
    UnloadImage(image);


    SetTargetFPS(60);
    while(!WindowShouldClose())
    {

        if(shouldredrawscreen())
        {
            Image screenimage;
            screenimage.height = EMU_SCREENHEIGHT;
            screenimage.width = EMU_SCREENWIDTH;
            screenimage.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;
            screenimage.mipmaps = 1;

            Color* pixles = RL_MALLOC(screenimage.width * screenimage.height * sizeof(Color));

            for(int x = 0; x < 64; x++)
            {
                for(int y = 0; y < 32; y++)
                {
                    Byte color = GetPixel( y, x );
                    // Byte color = GetPixel( x, y );
                    // printf("(%x, %x): %d\n", x, y, color);
                    int form;
                    // form = x * 32 + y;
                    form = y * 64 + x;
                    pixles[ form ].r = color;
                    pixles[ form ].g = color;
                    pixles[ form ].b = color;
                    pixles[ form ].a = 255;
                }
            }

            screenimage.data = pixles;
            UnloadTexture(screentarget);
            Texture2D screentarget = LoadTextureFromImage(screenimage);
            UnloadImage(screenimage);
            // debug_draw();
        }


        ClearBackground(RAYWHITE);
        BeginDrawing();

        if(GuiButton((Rectangle) {GUI_PADDINGRL, GUI_PADDINGTB + 30 * 0, 90, 20}, "DUMP"))
            DumpMemory();
        
        if(GuiButton((Rectangle) {GUI_PADDINGRL, GUI_PADDINGTB + 30 * 1, 90, 20}, "Step program"))
            run = 1;

        if(GuiButton((Rectangle) {GUI_PADDINGRL, GUI_PADDINGTB + 30 * 2, 90, 20}, "run program"))
            run = 2;

        if(GuiButton((Rectangle) {GUI_PADDINGRL, GUI_PADDINGTB + 30 * 3, 90, 20 }, "Redraw Screen"))
            setredrawscreen(true);

        if(GuiButton((Rectangle) {GUI_PADDINGRL, GUI_PADDINGTB + 30 * 4, 90, 20}, "set display 0x00"))
            memset(Display, 0, sizeof(Display));

        if(GuiButton((Rectangle) {GUI_PADDINGRL, GUI_PADDINGTB + 30 * 5, 90, 20}, "set display 0xaa"))
            memset(Display, 0b10101010, sizeof(Display));

        if(GuiButton((Rectangle) {GUI_PADDINGRL, GUI_PADDINGTB + 30 * 6, 90, 20}, "set display 0xFF"))
            memset(Display, 0xFF, sizeof(Display));


        if(run == 1 || run == 2)
        {
            RunCPU();
            if(run == 1) run = 0;
        }

        GuiLabel((Rectangle) {GUI_PADDINGRL, GUI_PADDINGTB + 30 * 7, 12, 34}, FormatText("Program counter: %x", PC));
        GuiLabel((Rectangle) {GUI_PADDINGRL, GUI_PADDINGTB + 30 * 8, 12, 34}, FormatText("Index Counter: %x", I));
        GuiLabel((Rectangle) {GUI_PADDINGRL, GUI_PADDINGTB + 30 * 9, 12, 34}, FormatText("Stack Pointer: %x", SP));

        for(int i = 0; i < 0x10; i++)
            GuiLabel((Rectangle) { GUI_PADDINGRL,      GUI_PADDINGTB + 30 * 10 + ( 10 * i ), 5, 6 }, FormatText("stack %x: %d", i, Stack[i]));

        for(int i = 0; i < 0x10; i++)
            GuiLabel((Rectangle) { GUI_PADDINGRL + 70, GUI_PADDINGTB + 30 * 10 + ( 10 * i ), 5, 6}, FormatText("V reg[%x]: %d", i, V[i]));


        if(IsKeyPressed(KEY_A))
        {
            UnloadTexture(screentarget);
            Image image = GenImageChecked(64, 32, 8, 4, RED, GREEN);
            screentarget = LoadTextureFromImage(image);
            UnloadImage(image);
        }

        if(GuiButton((Rectangle) { KEY_LOCATION_LEFTRIGHT + 25 * 0, KEY_LOCATION_TOPDOWN + 25 * 0, 20, 20 }, "0" ))
            Key[0x0] = true;
        if(GuiButton((Rectangle) { KEY_LOCATION_LEFTRIGHT + 25 * 0, KEY_LOCATION_TOPDOWN + 25 * 1, 20, 20 }, "1" ))
            Key[0x1] = true;
        if(GuiButton((Rectangle) { KEY_LOCATION_LEFTRIGHT + 25 * 0, KEY_LOCATION_TOPDOWN + 25 * 2, 20, 20 }, "2" ))
            Key[0x2] = true;
        if(GuiButton((Rectangle) { KEY_LOCATION_LEFTRIGHT + 25 * 0, KEY_LOCATION_TOPDOWN + 25 * 3, 20, 20 }, "3" ))
            Key[0x3] = true;
        if(GuiButton((Rectangle) { KEY_LOCATION_LEFTRIGHT + 25 * 1, KEY_LOCATION_TOPDOWN + 25 * 0, 20, 20 }, "4" ))
            Key[0x4] = true;
        if(GuiButton((Rectangle) { KEY_LOCATION_LEFTRIGHT + 25 * 1, KEY_LOCATION_TOPDOWN + 25 * 1, 20, 20 }, "5" ))
            Key[0x5] = true;
        if(GuiButton((Rectangle) { KEY_LOCATION_LEFTRIGHT + 25 * 1, KEY_LOCATION_TOPDOWN + 25 * 2, 20, 20 }, "6" ))
            Key[0x6] = true;
        if(GuiButton((Rectangle) { KEY_LOCATION_LEFTRIGHT + 25 * 1, KEY_LOCATION_TOPDOWN + 25 * 3, 20, 20 }, "7" ))
            Key[0x7] = true;
        if(GuiButton((Rectangle) { KEY_LOCATION_LEFTRIGHT + 25 * 2, KEY_LOCATION_TOPDOWN + 25 * 0, 20, 20 }, "8" ))
            Key[0x8] = true;
        if(GuiButton((Rectangle) { KEY_LOCATION_LEFTRIGHT + 25 * 2, KEY_LOCATION_TOPDOWN + 25 * 1, 20, 20 }, "9" ))
            Key[0x9] = true;
        if(GuiButton((Rectangle) { KEY_LOCATION_LEFTRIGHT + 25 * 2, KEY_LOCATION_TOPDOWN + 25 * 2, 20, 20 }, "A" ))
            Key[0xA] = true;
        if(GuiButton((Rectangle) { KEY_LOCATION_LEFTRIGHT + 25 * 2, KEY_LOCATION_TOPDOWN + 25 * 3, 20, 20 }, "B" ))
            Key[0xB] = true;
        if(GuiButton((Rectangle) { KEY_LOCATION_LEFTRIGHT + 25 * 3, KEY_LOCATION_TOPDOWN + 25 * 0, 20, 20 }, "C" ))
            Key[0xC] = true;
        if(GuiButton((Rectangle) { KEY_LOCATION_LEFTRIGHT + 25 * 3, KEY_LOCATION_TOPDOWN + 25 * 1, 20, 20 }, "D" ))
            Key[0xD] = true;
        if(GuiButton((Rectangle) { KEY_LOCATION_LEFTRIGHT + 25 * 3, KEY_LOCATION_TOPDOWN + 25 * 2, 20, 20 }, "E" ))
            Key[0xE] = true;
        if(GuiButton((Rectangle) { KEY_LOCATION_LEFTRIGHT + 25 * 3, KEY_LOCATION_TOPDOWN + 25 * 3, 20, 20 }, "F" ))
            Key[0xF] = true;
        DrawTextureEx(screentarget, (Vector2) { 12, 12 }, 0, 4.0, WHITE);

        EndDrawing();
    }
    UnloadTexture(screentarget);
    CloseWindow();
}
