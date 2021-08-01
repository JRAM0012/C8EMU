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
#define GUI_BUTTON(x) (Rectangle) {GUI_PADDINGRL, GUI_PADDINGTB + 30 * x, 90, 20}

int main()
{
    InitWindow(500, 500, "C8EMU");
    InitCPU();
    LoadProgram("assets\\IBM Logo.ch8");
    // LoadProgram("assets\\testprogram.ch8");
    //  LoadProgram("assets\\myperson.ch8");
    // LoadProgram("assets\\tank.ch8");
    // LoadProgram("assets\\invaders.ch8");
    // LoadProgram("assets\\Keypad Test.ch8");
    // LoadProgram("assets\\pong.ch8");
    // LoadProgram("assets\\trip8.ch8");
    // LoadProgram("assets\\breakout.ch8");
    // LoadProgram("assets\\GAMES\\maze.ch8");

    int run = 0;
    Texture2D screentarget;

    Image image = GenImageChecked(64, 32, 8, 4, RED, GREEN);
    screentarget = LoadTextureFromImage(image);


    SetTargetFPS(60);
    while(!WindowShouldClose())
    {
        if(IsFileDropped())
        {
            int count = 0;
            char** droppedFiles = GetDroppedFiles(&count);
            if(count == 1)
            {
                if(IsFileExtension(droppedFiles[0], ".ch8"))
                {
                    InitCPU();
                    LoadProgram(droppedFiles[0]);
                    memset(Stack, 0, sizeof(Stack));
                    memset(V, 0, sizeof(V));
                    memset(Display, 0, sizeof(Display));
                }
            }
            ClearDroppedFiles();
            run = 0;
        }
        if(shouldredrawscreen())
        {
            for(int x = 0; x < 32; x++)
            {
                for(int y = 0; y < 64; y++)
                {
                    Color* pixel = image.data;
                    Byte color = Display[x][y] > 0x0 ? 0xFF : 0x00;
                    pixel[ x * 64 + y ] = (Color) { color, color, color, 255 };
                }
            }
            UnloadTexture(screentarget);
            Texture2D screentarget = LoadTextureFromImage(image);
        }


        ClearBackground(RAYWHITE);
        BeginDrawing();

        if(GuiButton(GUI_BUTTON(0), "DUMP"))
            DumpMemory();

        if(GuiButton(GUI_BUTTON(1), "Step program"))
            run = 1;

        if(GuiButton(GUI_BUTTON(2), "run program"))
            run = 2;

        if(GuiButton(GUI_BUTTON(3), "Redraw Screen"))
            setredrawscreen(true);

        if(GuiButton(GUI_BUTTON(4), "set display 0x00"))
        {
            memset(Display, 0, sizeof(Display));
            setredrawscreen(true);
        }

        if(GuiButton(GUI_BUTTON(5), "set display 0xFF"))
        {
            memset(Display, 0b11111111, sizeof(Display));
            setredrawscreen(true);
        }

        if(GuiButton(GUI_BUTTON(6), "Reset Emulator"))
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


        if( IsKeyPressed(KEY_TWO)   || GuiButton(KEYPAD_BUTTON(0, 0), "1")) Key[0x1] = true;
        if( IsKeyPressed(KEY_Q)     || GuiButton(KEYPAD_BUTTON(0, 1), "4")) Key[0x4] = true;
        if( IsKeyPressed(KEY_A)     || GuiButton(KEYPAD_BUTTON(0, 2), "7")) Key[0x7] = true;
        if( IsKeyPressed(KEY_Z)     || GuiButton(KEYPAD_BUTTON(0, 3), "A")) Key[0xA] = true;
        if( IsKeyPressed(KEY_THREE) || GuiButton(KEYPAD_BUTTON(1, 0), "2")) Key[0x2] = true;
        if( IsKeyPressed(KEY_W)     || GuiButton(KEYPAD_BUTTON(1, 1), "5")) Key[0x5] = true;
        if( IsKeyPressed(KEY_S)     || GuiButton(KEYPAD_BUTTON(1, 2), "8")) Key[0x8] = true;
        if( IsKeyPressed(KEY_X)     || GuiButton(KEYPAD_BUTTON(1, 3), "0")) Key[0x0] = true;
        if( IsKeyPressed(KEY_FOUR)  || GuiButton(KEYPAD_BUTTON(2, 0), "3")) Key[0x3] = true;
        if( IsKeyPressed(KEY_E)     || GuiButton(KEYPAD_BUTTON(2, 1), "6")) Key[0x6] = true;
        if( IsKeyPressed(KEY_D)     || GuiButton(KEYPAD_BUTTON(2, 2), "9")) Key[0x9] = true;
        if( IsKeyPressed(KEY_C)     || GuiButton(KEYPAD_BUTTON(2, 3), "B")) Key[0xB] = true;
        if( IsKeyPressed(KEY_FIVE)  || GuiButton(KEYPAD_BUTTON(3, 0), "C")) Key[0xC] = true;
        if( IsKeyPressed(KEY_R)     || GuiButton(KEYPAD_BUTTON(3, 1), "D")) Key[0xD] = true;
        if( IsKeyPressed(KEY_F)     || GuiButton(KEYPAD_BUTTON(3, 2), "E")) Key[0xE] = true;
        if( IsKeyPressed(KEY_V)     || GuiButton(KEYPAD_BUTTON(3, 3), "F")) Key[0xF] = true;

        DrawTextureEx(screentarget, (Vector2) { 12, 12 }, 0, 4.0, WHITE);

        EndDrawing();
    }
    UnloadImage(image);
    UnloadTexture(screentarget);
    CloseWindow();
}
