#ifndef __C8EMU__
#define __C8EMU__

#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define Byte uint8_t
#define Word uint16_t

#define FONTSET_ADDRESS 0x0
#define EMU_SCREENWIDTH  64
#define EMU_SCREENHEIGHT 32
#define PROGRAM_START_LOCATION 0x200
#define MAX_GAME_SIZE (0x1000 - 0x200)
#define STACK_SIZE 16
#define KEY_SIZE 16
#define DISPLAY_COLS 64
#define DISPLAY_ROWS 32

#ifdef DEBUG
    #define EmuLog(...) printf(__VA_ARGS__)
#else 
    #define EmuLog(...)
#endif
// typedef struct {
//     union
//     {
//         Byte memory[0x1000];
//         struct
//         {
//             union
//             {
//                 Byte system[0x1FF];
//                 struct
//                 {
//                     Byte c8Font[0x50];
//                 };
//             };
            
//             Byte programram[0xE9F - 0x200];
//             union {
//                 Byte internal[0xEFF - 0xEA0];
//                 struct {
//                     Word Stack[STACK_SIZE];
//                     Word PC, IC, SP;
//                     Byte V[0xF];
//                     Byte delaytimer, soundtimer;
//                     Byte Key[KEY_SIZE];
//                     Word opcode;
//                 };
//             };
//             // Byte Display[ 0xFFF - 0xF00];
//         };
//     };
//     Byte Display[DISPLAY_COLS][DISPLAY_ROWS];
//     bool redrawscreen;
// }CPU;

Word opcode;
Byte memory[0x1000];
Word V[0x10];
Word I;
Word PC;
Byte Display[EMU_SCREENHEIGHT][EMU_SCREENWIDTH];
Byte delaytimer;
Byte soundtimer;
Word Stack[STACK_SIZE];
Word SP;
Byte Key[KEY_SIZE];
bool redrawscreen;
bool waiting_for_key_press;
char* filename;

void InitCPU()
{
    memset(memory, 0x0, sizeof(memory));
    PC = 0x200;
    I = 0;
    opcode = 0x00;
    SP = 0;
    redrawscreen = false;
    delaytimer = 0;
    soundtimer = 0;

    unsigned char FONTSET[80] =
        {
            0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
            0x20, 0x60, 0x20, 0x20, 0x70, // 1
            0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
            0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
            0x90, 0x90, 0xF0, 0x10, 0x10, // 4
            0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
            0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
            0xF0, 0x10, 0x20, 0x40, 0x40, // 7
            0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
            0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
            0xF0, 0x90, 0xF0, 0x90, 0x90, // A
            0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
            0xF0, 0x80, 0x80, 0x80, 0xF0, // C
            0xE0, 0x90, 0x90, 0x90, 0xE0, // D
            0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
            0xF0, 0x80, 0xF0, 0x80, 0x80  // F
        };

    for (int i = 0; i < 0x50; i++)
        memory[ FONTSET_ADDRESS + i] = FONTSET[i];

    srand(time(NULL));
}

void DumpMemory()
{
  EmuLog("      C 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F");
//   for(unsigned int rows = 0; rows < 0x20/*256*/; rows++)
  for(unsigned int rows = 0; rows < 256; rows++)
  {
    EmuLog("\nR 0x%x:\t", rows);
    for(unsigned int columns = 0; columns < 16; columns++)
    {
        EmuLog("%02x ", memory[ (rows * 16) + columns ]);
    }
  }
  EmuLog("\n");
}

void drawsprite(Byte x, Byte y, Byte n)
{
    Byte row = y, col = x;
    V[0xF] = 0;
    for(int byte_index = 0; byte_index < n; byte_index++)
    {
        Byte byte = memory[ I + byte_index];
        for(int bit_index = 0; bit_index < 8; bit_index++)
        {
            Byte bit = (byte >> bit_index) & 0x1;
            Byte *pixelp = &Display[ (row + byte_index) % DISPLAY_ROWS ]
                                   [(col + ( 7 - bit_index) % DISPLAY_COLS)];
            if(bit == 1 && *pixelp == 1)
                V[0xF] = 1;
            *pixelp = *pixelp ^ bit;
        }
    }
}

void LoadProgram(char* gamefile)
{
    filename = gamefile;
    FILE* fprogram = NULL;
    long lSize = 0;
    size_t result = 0;
    fprogram = fopen(gamefile, "rb");
    if(fprogram == NULL)
    {
        fprintf(stderr, "unable to open game: %s", gamefile);
        exit(42);
    }
    fseek(fprogram, 0, SEEK_END);
    lSize = ftell(fprogram);
    rewind(fprogram);
    result = fread(&(memory[PROGRAM_START_LOCATION]), 1, MAX_GAME_SIZE, fprogram);
    if(result != lSize)
    {
        EmuLog("Reading error\n");
        exit(3);
    }
    fclose(fprogram);
    PC = 0x200;
    EmuLog("program %s loaded..\n", gamefile);
}

static inline Byte randbyte() { return (rand() % 256); }

void RunCPU()
{
    if(PC > 0x1000)
        return;
    opcode = memory[PC] << 8 | memory[PC + 1];
    EmuLog("opcode: %04x  PC: %02x ", opcode, PC);

    // opcode ixyn

    Word inst = ( opcode & 0xF000 ) >> 0;
    Word x    = ( opcode & 0x0F00 ) >> 8;
    Word y    = ( opcode & 0x00F0 ) >> 4;
    Word n    = ( opcode & 0x000F ) >> 0;
    Word kk   = ( opcode & 0x00FF ) >> 0;
    Word nnn  = ( opcode & 0x0FFF ) >> 0;

    switch (inst)
    {
        case 0x0000:  // 00E0 - CLS 00EE - RET 0nnn - SYS addr
        {
            switch (kk)
            {
                case 0x00E0:
                {
                    EmuLog("Clear the screen\n");
                    memset(Display, 0, sizeof(Display));
                    redrawscreen = true;
                    PC += 2;
                }break;

                case 0x00EE:
                {
                    EmuLog("return from subroutine\n");
                    PC = Stack[ SP - 1 ];
                    Stack[ SP ] = 0;
                    SP--;
                } break;
            
                default:
                {
                    EmuLog("Unknown opcode %04x\n", opcode);
                } break;
            }
        } break;
        case 0x1000: // 1nnn: jump to address
        {
            EmuLog("Jump to address 0x%x\n", nnn);
            PC = nnn;
        } break;
        case 0x2000: // 2nnn: call to address diff is this pushes the pc to the stack
        {
            EmuLog("Call to subroutine 0x%x\n", nnn);
            Stack[SP++] = PC + 2;
            PC = nnn;
        } break;
        case 0x3000: // 3xkk: skip next inst if V[x] == kk
        {
            EmuLog("Skip next instruction if 0x%x == 0x%x ==> %s\n", V[x], kk, ( V[x] == kk ) ? "true" : "false");
            PC += ( V[x] == kk ) ? 4 : 2;
        } break;
        case 0x4000: // 4xkk: this would be the same as the prev but with not equal to
        {
            EmuLog("Skip next instruction if 0x%x != 0x%x ==> %s\n", V[x], kk, ( V[x] != kk ) ? "true" : "false");
            PC += ( V[x] != kk ) ? 4 : 2;
        } break;
        case 0x5000: // 5xy0: this would compare two registers
        {
            EmuLog("Skip next instruction if 0x%x == 0x%x ==> %s\n", V[x], V[y], (V[x] == V[y]) ? "true" : "false");
            PC += (V[x] == V[y]) ? 4 : 2;
        } break;
        case 0x6000: // 6xkk: set a register to a value
        {
            EmuLog("Set V[0x%02x] = 0x%x\n", x, kk);
            V[x] = kk;
            PC += 2;
        } break;
        case 0x7000: // 7xkk adds a value to the register
        {
            EmuLog("Set V[0x%02x] += 0x%x\n", x, kk);
            V[x] += kk;
            PC += 2;
        } break;
        case 0x8000: // 8xyn: Arithmetics 
        {
            switch (n)
            {
                case 0x0: // set V[x] = V[y]
                {
                    EmuLog("Reg SET V[0x%02x] = V[0x%02x] = 0x%x\n", x, y, V[y]);
                    V[x] = V[y];
                } break;
                case 0x1: // or two reg
                {
                    EmuLog("Reg OR  V[0x%02x] = V[0x%02x] (%02x) | V[0x%02x] (%02x) ==> 0x%x\n", x, x, V[x], y, V[y], V[x] | V[y]);
                    V[x] = V[x] | V[y];
                } break;
                case 0x2: // and two reg
                {
                    EmuLog("Reg AND V[0x%02x] = V[0x%02x] (%02x) & V[0x%02x] (%02x) ==> 0x%x\n", x, x, V[x], y, V[y], V[x] & V[y]);
                    V[x] = V[x] & V[y];
                } break;
                case 0x3: // xor two reg
                {
                    EmuLog("Reg XOR V[0x%02x] = V[0x%02x] (%02x) ^ V[0x%02x] (%02x) ==> 0x%x\n", x, x, V[x], y, V[y], V[x] ^ V[y]);
                    V[x] = V[x] ^ V[y];
                } break;
                case 0x4: // add two reg and store v[0xf] if carry is set
                {
                    EmuLog("Reg ADD V[0x%02x] = V[0x%02x] (%02x) + V[0x%02x] (%02x) ==> 0x%x\n", x, x, V[x], y, V[y], V[x] + V[y]);
                    V[0xF] = ((int) V[x] + (int) V[y]) > 255 ? 1 : 0; // is carry present
                    V[x] = V[x] + V[y];
                } break;
                case 0x5: // sub two reg and store v[0xf] if carry is set
                {
                    EmuLog("Reg SUB V[0x%02x] = V[0x%02x] (%02x) - V[0x%02x] (%02x) ==> 0x%x\n", x, x, V[x], y, V[y], V[x] - V[y]);
                    V[0xF] = (V[x] > V[y]) ? 1 : 0; // is borrow present
                    V[x] = V[x] - V[y];
                } break;
                case 0x6: // bit wise ( --> ) operation store least significant bit in V[x] and shifts V[x] to right by 1 
                {
                    EmuLog("Reg SHR V[0x%02x] >>= 1 (%x) ==> (%x)\n", V[x], V[x] >> 1);
                    V[0xF] = V[x] & 0x1;
                    V[x] = V[x] >> 1;
                } break;
                case 0x7: // Sets VX to VY minus VX. VF is set to 0 when there's a borrow, and 1 when there is not.
                {
                    EmuLog("Reg*SUB V[0x%02x] = V[%x] - V[%x] ==> (%x)\n", x, y, x, V[y] - V[x]);
                    V[0xf] = (V[y] > V[x]) ? 1 :0;
                    V[x] = V[y] - V[x];
                } break;
                case 0xE: // bit wise ( <-- ) operation store most significant bit in V[x] and shifts V[x] to left by 1 
                {
                    EmuLog("Reg SHL V[0x%02x] <<= 1 (%x) ==> (%x)\n", V[x], V[x] << 1);
                    V[0xF] = (V[x] >> 7) & 0x1;
                    V[x] = (V[x] << 1);
                } break;
                default:
                {
                    EmuLog("Unknown opcode: %04x\n", opcode);
                } break;
            }
            PC += 2;
        } break;
        case 0x9000: // 9xy0: skip next inst if Vx != Vy
        {
            EmuLog("Skip next instruction id 0x%x != 0x%x\n", V[x], V[y]);
            PC += (V[x] != V[y]) ? 4 : 2;
        } break;
        case 0xA000: // Annn: set I to address
        {
            EmuLog("Set I to 0x%x\n", nnn);
            I = nnn;
            PC += 2;
        } break;
        case 0xB000: // Bnnn: jump to location nnn + V[0]
        {
            EmuLog("Jump to 0x%x + V[0] (%x)\n", nnn, V[0]);
            PC = V[0] + nnn;
        } break;
        case 0xC000: // Cxkk: set V[x] to a random number AND (&) kk
        {
            Byte randomnumber = randbyte();
            EmuLog("Set V[0x%02x] = %x & %x ==> %x\n", randomnumber, kk, randomnumber & kk);
            V[x] = randomnumber & kk;
            PC += 2;
        } break;
        case 0xD000: // Draw an n-byte sprite starting at memory location I at (Vx, Vy) on the screen and set V[0xF] if collition occurs
        {
            EmuLog("Draw sprite at (V[%x], V[%x]) ==> (0x%x, 0x%x) of height: %d\n", x, y, V[x], V[y], n);
            drawsprite(V[x], V[y], n);
            redrawscreen = true;
            PC += 2;
        } break;
        case 0xE000: // key press events
        {
            switch (kk)
            {
            case 0x9E:
            {
                EmuLog("Skip next instruction if key[%d] is pressed\n", x);
                PC += (Key[V[x]]) ? 4 : 2;
            } break;
            case 0xA1:
            {
                EmuLog("Skip next instruction if key[%d] is NOT pressed\n", x);
                PC += (!Key[V[x]]) ? 4 : 2;
            } break;
            default:
                EmuLog("Unknown opcode: %04x\n", opcode);
                break;
            }
        } break;
        case 0xF000: // miscellaneous functions 
        {
            switch (kk)
            {
            case 0x07:
            {
                EmuLog("Set Reg to delay timer V[%x] = %x\n", x, delaytimer);
                V[x] = delaytimer;
                PC += 2;
            } break;

            case 0x0A: // waiting for a key press, and then stored in VX every thing is halted
            {
                EmuLog("Waiting for a key press\r");
                for(int i = 0; i < KEY_SIZE; i++)
                {
                    if(Key[i])
                    {
                        EmuLog("\n");
                        V[x] = i;
                        PC += 2;
                        memset(Key, 0, sizeof(Key));
                        goto got_key_press;
                    }
                }
                int jx  = 0;
                got_key_press:
                jx  = 12;
            } break;

            case 0x15: // Sets the delay timer to VX
            {
                EmuLog("Set delay timer = V[0x%02x] = %x\n", x, V[x]);
                delaytimer = V[x];
                PC += 2;
            } break;

            case 0x18: // Sets the sound timer to VX
            {
                EmuLog("Set sound timer = V[0x%02x] = %x\n", x, V[x]);
                soundtimer = V[x];
                PC += 2;
            } break;

            case 0x1E:
            {
                EmuLog("Adds VX to I: I += V[0x%04x] ==> 0x%04x\n", x, I + V[x]);
                V[0xF] = ( I + V[x] > 0xFFF) ? 1 : 0;
                I += V[x];
                PC += 2;
            } break;

            case 0x29: // Sets I to the location of the sprite for the character in VX
            {
                EmuLog("Sets I (%x) to the location of the sprite for the character in V[0x%02x]\n", I, V[x]);
                I = 5 * V[x];
                PC += 2;
            } break;
            
            case 0x33:
            {
                EmuLog("Store BCD for %d starting at address 0x%x\n", V[x], I);
                memory[I + 0] = (V[x] % 1000) / 100;
                memory[I + 1] = (V[x] %  100) /  10;
                memory[I + 2] = (V[x] %   10) /   1;
                PC += 2;
            } break;

            case 0x55:
            {
                EmuLog("Copy sprite from registers 0 to 0x%x into memory at address 0x%x\n", x, I);
                for(int i = 0; i <= x; i++)
                    memory[ I + i] = V[i];
                I  += x + 1;
                PC += 2;
            } break;

            case 0x65:
            {
                EmuLog("Copy sprite from memory at address 0x%x into registers 0 to 0x%x\n", x, I);
                for(int i = 0; i <= x; i++)
                    V[i] = memory[ I + i];
                I += x + 1;
                PC += 2;
            } break;

            default:
                EmuLog("Unknown opcode %04x\n", opcode);
                break;
            }
        } break;
        
        default:
            EmuLog("Unknown opcode %04x\n", opcode);
            break;
    }

    if (delaytimer > 0) {
        --delaytimer;
    }
    if (soundtimer > 0) {
        --soundtimer;
        if (soundtimer == 0) {
            EmuLog("BEEP!\n");
        }
    }
}

void setredrawscreen(bool redraw)
{
    redrawscreen = redraw;
}

bool shouldredrawscreen()
{
    if(redrawscreen)
    {
        redrawscreen = false;
        return true;
    }
    return false;
}

Byte GetPixel( int x, int y )
{
    Byte* byte = &Display[x][y];
    // EmuLog("display: %d, ", *byte);
    if(*byte > 0x0)
        return 0xFF;
    return 0x00;
}

void ResetEmulator()
{
    InitCPU();
    LoadProgram(filename);
    memset(Stack, 0, sizeof(Stack));
    memset(V, 0, sizeof(V));
    memset(Display, 0, sizeof(Display));
}

static void debug_draw() {
    int x, y;

    for (y = 0; y < DISPLAY_ROWS; y++) {
        for (x = 0; x < DISPLAY_COLS; x++) {
            if (Display[y][x] == 0) EmuLog(" ");
            else EmuLog("0");
        }
        EmuLog("\n");
    }
    EmuLog("\n");
}



// void SetPixel(int x, int y, Byte value)
// {
//     Display[x / 8][y] |= x % 8;
// }


#endif // __C8EMU__
