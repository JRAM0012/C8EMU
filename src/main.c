#include <stdio.h>
#include "raylib.h"
// #include "rlgl.h"
#define RAYGUI_IMPLEMENTATION
#include "raygui.h"
#define DEBUG
#include "C8EMU.h"

#define GUI_PADDINGRL 360
#define GUI_PADDINGTB 20

#define KEY_LOCATION_LEFTRIGHT 13
#define KEY_LOCATION_TOPDOWN 153
#define EMULATOR_BUTTON_WIDTH 50
#define EMULATOR_BUTTON_HEIGHT 0
// #define KEYPAD_BUTTON(x, y) (Rectangle) { KEY_LOCATION_LEFTRIGHT + 25 * x, KEY_LOCATION_TOPDOWN + (EMULATOR_BUTTON_HEIGHT + 10) * y, 50, 20 }
#define KEYPAD_BUTTON(x, y) (Rectangle) { KEY_LOCATION_LEFTRIGHT + (EMULATOR_BUTTON_WIDTH * x) + 25 * x, KEY_LOCATION_TOPDOWN + (EMULATOR_BUTTON_WIDTH + 10) * y, EMULATOR_BUTTON_WIDTH, 20 }

int main()
{
    InitWindow(500, 500, "C8EMU");
    InitCPU();
    LoadProgram("assets\\IBM Logo.ch8");
    // LoadProgram("assets\\testprogram.ch8");
    // LoadProgram("assets\\tank.ch8");
    // LoadProgram("assets\\invaders.ch8");
    // LoadProgram("assets\\pong.ch8");
    // LoadProgram("assets\\trip8.ch8");
    // LoadProgram("assets\\breakout.ch8");
    int run = 0;
    Texture2D screentarget;

    Image image = GenImageChecked(64, 32, 8, 4, RED, GREEN);
    screentarget = LoadTextureFromImage(image);
    UnloadImage(image);


    // SetTargetFPS(60);
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
                    int form = y * 64 + x;
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
        }


        ClearBackground(RAYWHITE);
        BeginDrawing();

        if(GuiButton((Rectangle) {GUI_PADDINGRL, GUI_PADDINGTB + 30 * 0, 90, 20}, "DUMP") || IsKeyPressed(KEY_D))
            DumpMemory();
        
        if(GuiButton((Rectangle) {GUI_PADDINGRL, GUI_PADDINGTB + 30 * 1, 90, 20}, "Step program") || IsKeyPressed(KEY_S))
            run = 1;

        if(GuiButton((Rectangle) {GUI_PADDINGRL, GUI_PADDINGTB + 30 * 2, 90, 20}, "run program") || IsKeyPressed(KEY_R))
            run = 2;

        if(GuiButton((Rectangle) {GUI_PADDINGRL, GUI_PADDINGTB + 30 * 3, 90, 20 }, "Redraw Screen") || IsKeyPressed(KEY_Q))
            setredrawscreen(true);

        if(GuiButton((Rectangle) {GUI_PADDINGRL, GUI_PADDINGTB + 30 * 4, 90, 20}, "set display 0x00"))
        {
            memset(Display, 0, sizeof(Display));
            setredrawscreen(true);
        }

        if(GuiButton((Rectangle) {GUI_PADDINGRL, GUI_PADDINGTB + 30 * 5, 90, 20}, "set display 0xFF"))
        {
            memset(Display, 0b11111111, sizeof(Display));
            setredrawscreen(true);
        }

        if(GuiButton((Rectangle) {GUI_PADDINGRL, GUI_PADDINGTB + 30 * 6, 90, 20}, "Reset Emulator") || IsKeyPressed(KEY_F))
            ResetEmulator();

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

        int mousex = GetMouseX();
        int mousey = GetMouseY();
        DrawText(FormatText("Mouse X: %d, Mouse Y: %d", mousex, mousey), 10, 450, 20, RED);

        if(GuiButton(KEYPAD_BUTTON(0, 0), "0")) Key[0x0] = true;
        if(GuiButton(KEYPAD_BUTTON(0, 1), "1")) Key[0x1] = true;
        if(GuiButton(KEYPAD_BUTTON(0, 2), "2")) Key[0x2] = true;
        if(GuiButton(KEYPAD_BUTTON(0, 3), "3")) Key[0x3] = true;
        if(GuiButton(KEYPAD_BUTTON(1, 0), "4")) Key[0x4] = true;
        if(GuiButton(KEYPAD_BUTTON(1, 1), "5")) Key[0x5] = true;
        if(GuiButton(KEYPAD_BUTTON(1, 2), "6")) Key[0x6] = true;
        if(GuiButton(KEYPAD_BUTTON(1, 3), "7")) Key[0x7] = true;
        if(GuiButton(KEYPAD_BUTTON(2, 0), "8")) Key[0x8] = true;
        if(GuiButton(KEYPAD_BUTTON(2, 1), "9")) Key[0x9] = true;
        if(GuiButton(KEYPAD_BUTTON(2, 2), "A")) Key[0xA] = true;
        if(GuiButton(KEYPAD_BUTTON(2, 3), "B")) Key[0xB] = true;
        if(GuiButton(KEYPAD_BUTTON(3, 0), "C")) Key[0xC] = true;
        if(GuiButton(KEYPAD_BUTTON(3, 1), "D")) Key[0xD] = true;
        if(GuiButton(KEYPAD_BUTTON(3, 2), "E")) Key[0xE] = true;
        if(GuiButton(KEYPAD_BUTTON(3, 3), "F")) Key[0xF] = true;

        DrawTextureEx(screentarget, (Vector2) { 12, 12 }, 0, 4.0, WHITE);

        EndDrawing();
    }
    UnloadTexture(screentarget);
    CloseWindow();
}
