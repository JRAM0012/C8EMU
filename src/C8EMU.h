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
#define IS_BIT_SET(byte, bit) (((byte) & (0x80 >> (bit))) != 0x0)

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

#define STATIKU 


STATIKU Word opcode;
STATIKU Byte memory[0x1000];
STATIKU Word V[0x10];
STATIKU Word I;
STATIKU Word PC;
STATIKU Byte Display[EMU_SCREENHEIGHT][EMU_SCREENWIDTH];
STATIKU Byte delaytimer;
STATIKU Byte soundtimer;
STATIKU Word Stack[STACK_SIZE];
STATIKU Word SP;
STATIKU Byte Key[KEY_SIZE];
STATIKU bool redrawscreen;


void InitCPU()
{
    memset(memory, 0x0, sizeof(memory));
    PC = 0x200;
    I = 0;
    opcode = 0x00;
    SP = 0;
    redrawscreen = true;
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
  printf("      C 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F");
//   for(unsigned int rows = 0; rows < 0x20/*256*/; rows++)
  for(unsigned int rows = 0; rows < 256; rows++)
  {
    printf("\nR 0x%x:\t", rows);
    for(unsigned int columns = 0; columns < 16; columns++)
    {
        printf("%02x ", memory[ (rows * 16) + columns ]);
    }
  }
  printf("\n");
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
            if(bit == 1 & *pixelp == 1)
                V[0xF] = 1;
            *pixelp = *pixelp ^ bit;
        }
    }
}



void LoadProgram(char* gamefile)
{
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
        printf("Reading error\n");
        exit(3);
    }
    fclose(fprogram);
    PC = 0x200;
    printf("program %s loaded..\n", gamefile);
}

static inline Byte randbyte() { return (rand() % 256); }

void RunCPU()
{
    if(PC > 0x1000)
        return;
    opcode = memory[PC] << 8 | memory[PC + 1];
    printf("opcode: %04x  PC: %02x ", opcode, PC);

    // opcode ixyn

    Word x = ( opcode & 0x0F00 ) >>  8;
    Word y = ( opcode & 0x00F0 ) >>  4;
    Word n = ( opcode & 0x000F ) >>  0;

    switch (opcode & 0xF000)
    {
        case 0x0000:
        {
            switch (opcode & 0x00FF)
            {
                case 0x00E0: // clear screen
                {
                    printf("clear screen\n");
                    memset(Display, 0, sizeof(Display));
                    redrawscreen = true;
                    PC += 2;
                } break;

                case 0x00EE: // return
                {
                    printf("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaareturn\n");
                    PC = Stack[ --SP ];
                    Stack[ SP + 1 ] = 0;
                } break;

                default:
                    printf("unknown opcode: %x in case: 0x0000\n", opcode);
                    break;
            }
        }break;

        case 0x1000: // 0x1NNN
        {
            printf(" Jump instruction to: %x\n", opcode & 0x0FFF);
            PC = opcode & 0x0FFF;
        }

        case 0x2000: // 0x2NNN Calls subroutine at NNN.
        {
            printf("subroutine Call instruction: %x\n", opcode & 0x0FFF);
            Stack[SP++] = PC + 2;
            PC = opcode & 0x0FFF;
        }break;

        case 0x3000: // 3xkk: skip next instr if V[x] = kk
        {
            printf("Skip next instruction if 0x%x == 0x%x == ", V[x], opcode & 0x00FF);
            printf(( V[x] == ( opcode & 0x00FF ) ) ? "true\n" : "false\n");
            PC += ( V[x] == ( opcode & 0x00FF ) ) ? 4 : 2;
        }break;

        case 0x4000: // 4xkk: skip next instr if V[x] != kk
        {
            printf("Skip next instruction if 0x%x != 0x%02x", V[x], opcode & 0x00FF);
            printf(( V[x] != ( opcode & 0x00FF ) ) ? "true\n" : "false\n");
            PC += (V[x] != ( opcode & 0x00FF ) ) ? 4 : 2;
        }break;

        case 0x5000: // 5xy0: skip next instr if V[x] == V[y]
        {
            printf("Skip next instruction if 0x%x == 0x%x\n", V[x], V[y]);
            printf(( V[x] == V[y] ) ? "true\n" : "false\n");
            PC = ( V[x] == V[y] ) ? 4 : 2;
        }break;

        case 0x6000: // 6xkk: set V[x] = kk
        {
            printf("set V register instruction: %x, v[%d] = %d\n", opcode, x, opcode & 0x00FF);
            V[x] = opcode & 0x00FF;
            PC += 2;
        }break;

        case 0x7000: //  7xkk: set V[x] = V[x] + kk
        {
            printf("add to V register instruction: %x, v[%d] += %d\n", opcode, x, opcode & 0x00FF);
            V[x] += opcode & 0x00FF;
            PC += 2;
        }break;

        case 0x8000: // 8xyn: Arithmetic logic
        {
            printf("Arithmetic instruction: %x: ", opcode);
            switch (n)
            {
                case 0x0: // v[x] = v[y]
                {
                    printf("\t equal V[%d] = V[%d] = %d\n", x, y, V[y]);
                    V[x] = V[y];
                } break;

                case 0x1:
                {
                    printf("\t or V[%d] |= V[%d]\n", V[x], V[y]);
                    V[x] = V[x] | V[y];
                } break;

                case 0x2:
                {
                    printf("\t and V[%d] &= V[%d]\n", V[x], V[y]);
                    V[x] = V[x] & V[y];
                } break;

                case 0x3:
                {
                    printf("\t and V[%d] ^= V[%d]\n", V[x], V[y]);
                    V[x] = V[x] ^ V[y];
                } break;

                case 0x4:
                {
                    printf("\t add V[%d] += V[%d]\n", x, y);
                    V[0xF] = ( (int) V[x] + (int) V[y] ) > 255 ? 1 : 0;
                    V[x] = V[x] + V[y];
                } break;

                case 0x5:
                {
                    printf("\t sub V[%d] -= V[%d]\n", x, y);
                    V[0xF] = V[x] > V[y] ? 1 : 0;
                    V[x] = V[x] - V[y];
                } break;

                case 0x6:
                {
                    printf("V[%d] >>= 1 => %d >> 1\n", x, x, V[x]);
                    V[0xF] = V[x] & 0x1;
                    V[x] = ( V[x] >> 1);
                } break;

                case 0x7:
                {
                    printf("V[%d] = V[%d] - V[%d] => %d - %d\n", x, y, x, V[y], V[x]);
                    V[0xF] = ( V[y] > V[x] ) ? 1 : 0;
                    V[x] = V[y] - V[x];
                } break;

                case 0xE:
                {
                    printf("V[%d] <<== 1", x);
                    V[0xF] = ( V[x] >> 7 ) & 0x1;
                    V[x] = V[x] << 1;
                } break;

                default:
                    printf("[WARNING]: unknown arithmetic opcode");
                    break;
            }
            PC += 2;
        }break;

        case 0x9000:
        {
            printf("Skips the next instruction if VX (V[%d]) does not equal VY (V[%d]) instruction: %x\n", opcode, V[x], V[y]);
            PC += ( V[x] != V[y] ) ? 4 : 2;
        }break;

        case 0xA000: // set I address to 0x0FFF
        {
            printf(" set I instruction to %x\n", opcode & 0x0FFF);
            I = opcode & 0x0FFF;
            PC += 2;
        }break;

        case 0xB000: // jump to 0x0FFF + V[0]
        {
            printf("jump to 0x0FFF + V[%d]: instruction: %x\n", opcode);
            PC = ( opcode & 0x0FFF ) + V[0];
        }break;

        case 0xC000: // Cxkk: V[x] = random byte AND kk
        {
            printf("C000: instruction: %x\n", opcode);
            V[x] = randbyte() & ( opcode & 0x00FF );
            PC += 2;
        }break;

        case 0xD000: //
        {
            printf(" draw instruction: %x\n", opcode);
            drawsprite(V[x], V[y], n);
            PC += 2;
            redrawscreen = true;
        }break;

        case 0xE000: // key press events
        {
            printf("key press event instruction: %x\n", opcode);
            switch (opcode & 0x00FF)
            {
                case 0x9E:
                {
                    printf("skip next instruction if key[%d] is pressed\n", x);
                    PC += ( Key[V[x]] ) ? 4 : 2;
                } break;

                case 0xA1:
                {
                    printf("Skip next instruction if key[%d] is not pressed\n");
                    PC += ( ! Key[V[x]] ) ? 4 : 2;
                } break;
        
                default:
                    break;
            }
        }break;

        case 0xF000: // misc
        {
            switch (opcode & 0x00FF)
            {
                case 0x0007:
                {
                    printf("set delay timer to %x\n", x);
                    V[x] = delaytimer;
                    PC += 2;
                } break;

                case 0x000A:
                {
                    printf("Wait for a key press, store the value of the key in Vx. 0xF0xA\n");

                    int temp = -1;
                    for (int i = 0; i < KEY_SIZE; i++) {
                        if (Key[i]) {
                            temp = i;
                            break;
                        }
                    }
                    if (temp != -1)
                        V[x] = temp;
                    else
                        PC -= 2;
                    // int i = 0;
                    // while(true)
                    // {
                    //     for(i = 0; i < KEY_SIZE; i++)
                    //     {
                    //         if(Key[i])
                    //         {
                    //             V[x] = i;
                    //             goto got_key_press;
                    //         }
                    //     }
                    // }
                    // got_key_press:
                    // PC += 2;
                    for(int j = 0; j < KEY_SIZE; j++)
                        Key[j] = false;
                }

                case 0x15:
                {
                    delaytimer = V[x];
                    PC += 2;
                } break;

                case 0x18:
                {
                    soundtimer = V[x];
                    PC += 2;
                } break;

                case 0x1E:
                {
                    V[0xF] = ( ( I + V[x] ) > 0x0FFF) ? 1 : 0;
                    I = I + V[x];
                    PC += 2;
                } break;

                case 0x29:
                {
                    I = 5 * V[x];
                    PC += 2;
                } break;

                case 0x33:
                {
                    printf("Store BCD for %d starting at address 0x%x\n", V[x], I);
                    memory[I + 0] = ( V[x] % 1000 ) / 100;
                    memory[I + 1] = ( V[x] % 100  ) / 10 ;
                    memory[I + 2] = ( V[x] % 10   );
                    PC += 2;
                } break;

                case 0x55:
                {
                    printf("Copy sprite from registers 0 to 0x%x into memory at address 0x%x\n", x, I);
                    for(int i = 0; i <= x; i++)
                        memory[ I + i ] = V[i];
                    I += x + 1;
                    PC += 2;
                } break;

                case 0x65:
                {
                    printf("Copy sprite from memory at address 0x%x into registers 0 to 0x%x\n", x, I);
                    for(int i = 0; i <= x; i++)                    
                        V[i] = memory[ I + i ];
                    I += x + 1;
                    PC += 2;
                } break;
                default:
                    break;
            }
        }break;
        default:
            printf("unhandled instruction: %x\n", opcode);
            break;
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
    // printf("display: %d, ", *byte);
    if(*byte > 0x0)
        return 0xFF;
    return 0x00;
}


static void debug_draw() {
    int x, y;

    for (y = 0; y < DISPLAY_ROWS; y++) {
        for (x = 0; x < DISPLAY_COLS; x++) {
            if (Display[y][x] == 0) printf("0");
            else printf(" ");
        }
        printf("\n");
    }
    printf("\n");
}



// void SetPixel(int x, int y, Byte value)
// {
//     Display[x / 8][y] |= x % 8;
// }


#endif // __C8EMU__
