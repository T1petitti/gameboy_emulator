#include <iostream>
#include <fstream>
//#include <vector>
#include "Z80.h"
//#include "gameboy.pro"
#include "screen.h"

#include <chrono>
#include <thread>


// Global ROM array - Will be loaded from a file
unsigned char* rom  = nullptr; // Use unsigned char* for binary data
int romSize = 0;
unsigned char graphicsRAM[8192];
int palette[4];
int tileset, tilemap, scrollx, scrolly;

int HBLANK=0, VBLANK=1, SPRITE=2, VRAM=3;
unsigned char workingRAM[0x2000];
unsigned char page0RAM[0x80];
int line=0, cmpline=0, videostate=0, keyboardColumn=0, horizontal=0;
int gpuMode=HBLANK;
int romOffset = 0x4000;
long totalInstructions=0;

bool keyAPressed = false;
bool keyBPressed = false;
bool keyLeftPressed = false;
bool keyUpPressed = false;
bool keyRightPressed = false;
bool keyDownPressed = false;
bool keyStartPressed = false;
bool keySelectPressed = false;

const int A = 65;
const int B = 66;
const int UP = 16777235; // Define UP as the scan code for the 'up' key
const int DOWN = 16777237; // Define DOWN as the scan code for the 'down' key
const int LEFT = 16777234; // Define LEFT as the scan code for the 'left' key
const int RIGHT = 16777236; // Define RIGHT as the scan code for the 'right' key
const int ENTER = 16777220; // Define ENTER as the scan code for the 'enter' key
const int SPACE = 32; // Define SPACE as the scan code for the 'space' key

unsigned char getKey() {
    unsigned char result = 0xF; // Initialize with all bits set (all keys released)

    //std::cout << "column = " << keyboardColumn << std::endl;
    //std::cout << "dir = " << direction << std::endl;
    //std::cout << "but = " << button << std::endl;

    if (keyboardColumn & 0x10) {
        //std::cout << "direction" << std::endl;
        if (keyRightPressed) {
            result &= ~(1 << 0); // Clear bit 0 (Right key pressed)
        }
        if (keyLeftPressed) {
            result &= ~(1 << 1); // Clear bit 1 (Left key pressed)
        }
        if (keyUpPressed) {
            result &= ~(1 << 2); // Clear bit 2 (Up key pressed)
        }
        if (keyDownPressed) {
            result &= ~(1 << 3); // Clear bit 3 (Down key pressed)
        }
    }
    if (keyboardColumn & 0x20) {
        //std::cout << "buttons" << std::endl;
        if (keyAPressed) {
            result &= ~(1 << 0); // Clear bit 4 (A key pressed)
        }
        if (keyBPressed) {
            result &= ~(1 << 1); // Clear bit 5 (B key pressed)
        }
        if (keyStartPressed) {
            result &= ~(1 << 2); // Clear bit 6 (Start key pressed)
        }
        if (keySelectPressed) {
            result &= ~(1 << 3); // Clear bit 7 (Select key pressed)
        }
    }
    //std::cout << "result = " << (int)result << std::endl;
    return result;

}


void setRomMode(int address, unsigned char b) { }

void setControlByte(unsigned char b) {
    tilemap=(b&8)!=0?1:0;

    tileset=(b&16)!=0?1:0;
}

void setPalette(unsigned char b) {
    palette[0]=b&3;
    palette[1]=(b>>2)&3;
    palette[2]=(b>>4)&3;
    palette[3]=(b>>6)&3;
}

unsigned char getVideoState() {
    int by=0;

    if(line==cmpline) by|=4;

    if(gpuMode==VBLANK) by|=1;

    if(gpuMode==SPRITE) by|=2;

    if(gpuMode==VRAM) by|=3;

    return (unsigned char)((by|(videostate&0xf8))&0xff);
}


// Memory access functions
unsigned char memoryRead(int address) {
    if (address >= 0 && address <=0x3FFF) {
        return rom[address];
    }
    if (address >= 0x4000 && address <= 0x7FFF) {
        return rom[romOffset+address%0x4000] ;
    }
    if (address >= 0x8000 && address <= 0x9FFF) {
        return graphicsRAM[address%0x2000];
    }
    if (address >= 0xC000 && address <= 0xDFFF) {
        return workingRAM[address%0x2000];
    }
    if (address >= 0xFF80 && address <= 0xFFFF) {
        return page0RAM[address % 0x80];
    }
    if (address == 0xFF00) {
        int keyState = getKey();
        //std::cout << "Key state: " << keyState << std::endl;
        return keyState;
    }
    if (address == 0xFF40) {
        return 0;
    }
    if (address == 0xFF41) {
        return getVideoState();
    }
    if (address == 0xFF42) {
        return scrolly;
    }
    if (address == 0xFF43) {
        return scrollx;
    }
    if (address == 0xFF44) {
        return line;
    }
    if (address == 0xFF45) {
        return cmpline;
    }
    if (address == 0xFF47) {
        return 0;
    }
    else {
        return 0;
    }
}

void memoryWrite(int address, unsigned char b) {
    if (address >= 0 && address <=0x3FFF) {
        setRomMode(address,b);
    }
    if (address >= 0x4000 && address <= 0x7FFF) {
    }
    if (address >= 0x8000 && address <= 0x9FFF) {
        graphicsRAM[address%0x2000] = b;
    }
    if (address >= 0xC000 && address <= 0xDFFF) {
        workingRAM[address%0x2000] = b;
    }
    if (address >= 0xFF80 && address <= 0xFFFF) {
        page0RAM[address % 0x80] = b;
    }
    if (address == 0xFF00) {
        keyboardColumn = b;
        std::cout << "keyboardColumn = " << keyboardColumn << std::endl;
    }
    if (address == 0xFF40) {
        setControlByte(b);
    }
    if (address == 0xFF41) {
        videostate = b;
    }
    if (address == 0xFF42) {
        scrolly = b;
    }
    if (address == 0xFF43) {
        scrollx = b;
    }
    if (address == 0xFF44) {
        line = b;
    }
    if (address == 0xFF45) {
        cmpline = b;
    }
    if (address == 0xFF47) {
        setPalette(b);
    }
    else {
    }
}

void keydown(int scanCode) {
    switch (scanCode) {
    case A:
        keyAPressed = true;
        keyboardColumn = 32;
        std::cout << "A key pressed" << std::endl;
        break;
    case B:
        keyBPressed = true;
        keyboardColumn = 32;
        std::cout << "B key pressed" << std::endl;
        break;
    case LEFT:
        keyLeftPressed = true;
        keyboardColumn = 16;
        std::cout << "Left key pressed" << std::endl;
        break;
    case UP:
        keyUpPressed = true;
        keyboardColumn = 16;
        std::cout << "Up key pressed" << std::endl;
        break;
    case RIGHT:
        keyRightPressed = true;
        keyboardColumn = 16;
        std::cout << "Right key pressed" << std::endl;
        break;
    case DOWN:
        keyDownPressed = true;
        keyboardColumn = 16;
        std::cout << "Down key pressed" << std::endl;
        break;
    case ENTER:
        keyStartPressed = true;
        keyboardColumn = 32;
        std::cout << "Start key pressed" << std::endl;
        break;
    case SPACE:
        keySelectPressed = true;
        keyboardColumn = 32;
        std::cout << "Select key pressed" << std::endl;
        break;
    }
}

void keyup(int scanCode) {
    switch (scanCode) {
    case A:
        keyAPressed = false;
        break;
    case B:
        keyBPressed = false;
        break;
    case LEFT:
        keyLeftPressed = false;
        break;
    case UP:
        keyUpPressed = false;
        break;
    case RIGHT:
        keyRightPressed = false;
        break;
    case DOWN:
        keyDownPressed = false;
        break;
    case ENTER:
        keyStartPressed = false;
        break;
    case SPACE:
        keySelectPressed = false;
        break;
    }
}


void renderScreen() {
    for (int y = 0; y < 144; y++) {
        for (int x = 0; x < 160; x++) {
            // Apply scroll
            int scrolledX = (x + scrollx) & 255;
            int scrolledY = (y + scrolly) & 255;

            // Determine tile coordinates
            int tileX = scrolledX / 8;
            int tileY = scrolledY / 8;

            // Find tile's position in the tile map
            int tilePosition = tileY * 32 + tileX;

            // Get the tile index from the appropriate tile map
            int tileIndex = (tilemap == 0) ? graphicsRAM[0x1800 + tilePosition] : graphicsRAM[0x1c00 + tilePosition];

            // Address calculation differs for tileset 0
            int tileAddress = (tileset == 1) ? tileIndex * 16 : (tileIndex >= 128 ? tileIndex - 256 : tileIndex) * 16 + 0x1000;

            // Get the pixel within the tile
            int xoffset = scrolledX % 8;
            int yoffset = scrolledY % 8;

            // Retrieve the color bytes for the tile row
            unsigned char row0 = graphicsRAM[tileAddress + (yoffset * 2)];
            unsigned char row1 = graphicsRAM[tileAddress + (yoffset * 2) + 1];

            // Extract the color bits for the specific pixel
            int colorBit0 = (row0 >> (7 - xoffset)) & 1;
            int colorBit1 = (row1 >> (7 - xoffset)) & 1;

            // Combine the color bits to get the color index
            int colorIndex = (colorBit1 << 1) | colorBit0;

            // Map the color index to the actual color using the palette
            int color = palette[colorIndex];

            // Update the screen with the calculated color
            updateSquare(x, y, color);
        }
    }

    // Refresh the screen after all updates are done
    onFrame();
}

extern QApplication* app;
int main(int argc, char** argv) {

    setup(argc,argv);
    //part 1 code here
    // Load ROM from file
    std::ifstream romfile("/Users/tylerpetitti/Desktop/gameboy/tetris.gb", std::ios::binary);

    if (!romfile.is_open()) {
        std::cerr << "Failed to open ROM file." << std::endl;
        return 1;
    }
    // Get the size of the ROM file
    romfile.seekg(0, std::ios::end);
    romSize = romfile.tellg();
    romfile.seekg(0, std::ios::beg);

    // Allocate memory for the ROM
    rom = new unsigned char[romSize];
    if (!rom) {
        std::cerr << "Failed to allocate memory for ROM." << std::endl;
        return 1;
    }

    // Read the ROM file into memory
    romfile.read(reinterpret_cast<char*>(rom), romSize);
    romfile.close();

    // Initialize the Z80 CPU with memory access functions
    Z80* z80 = new Z80(memoryRead, memoryWrite);

    // Reset the CPU
    z80->reset();

    // Execute instructions until the CPU halts
    while (true) {
        // If not halted, do an instruction
        if (z80->halted) break;
        z80->doInstruction();

        if (keyAPressed) {
            //z80->throwInterrupt(); // Assuming interrupt 3 is for the left key
        }
        if (keyBPressed) {
            //z80->throwInterrupt(); // Assuming interrupt 4 is for the up key
        }
        if (keyLeftPressed) {
            //z80->throwInterrupt(); // Assuming interrupt 3 is for the left key
        }
        if (keyUpPressed) {
            //z80->throwInterrupt(); // Assuming interrupt 4 is for the up key
        }
        if (keyRightPressed) {
            //z80->throwInterrupt(); // Assuming interrupt 5 is for the right key
        }
        if (keyDownPressed) {
            //z80->throwInterrupt(); // Assuming interrupt 6 is for the down key
        }
        if (keyStartPressed) {
            //z80->throwInterrupt(); // Assuming interrupt 6 is for the down key
        }
        if (keySelectPressed) {
            //z80->throwInterrupt(); // Assuming interrupt 6 is for the down key
        }

        // Check for and handle interrupts
        if (z80->interrupt_deferred > 0) {
            z80->interrupt_deferred--;
            if (z80->interrupt_deferred == 1) {
                z80->interrupt_deferred = 0;
                z80->FLAG_I = 1;
            }
        }
        z80->checkForInterrupts();


        // Figure out screen position and set the video mode
        horizontal = (int)((totalInstructions + 1) % 61);
        if (line >= 145) {
            gpuMode = VBLANK;
        }
        if (horizontal <= 30) {
            gpuMode = HBLANK;
            if (horizontal == 0) {
                line++;
                if (line == 144) {
                    z80->throwInterrupt(1);
                }
                if ((line % 153) == cmpline && (videostate & 0x40) != 0) {
                    z80->throwInterrupt(2);
                }
                if (line == 153) {
                    line = 0;
                    renderScreen();
                }
            }
        }
        else if (horizontal >= 31 && horizontal <= 40) {
            gpuMode = SPRITE;
        }
        else {
            gpuMode = VRAM;
        }
        totalInstructions++;

        //std::cout << "GPU Mode: " << gpuMode << ", Horizontal: " << horizontal << ", Line: " << line << std::endl;

    }

    // Output the final value in register A (expected: 21)
    //std::cout << "Final A: " << (int)z80->A << std::endl;

    //part 2


    std::ifstream vidfile("/Users/tylerpetitti/Desktop/gameboy/screendump.txt",std::ios::in);

    if (!vidfile.is_open()) {
        std::cerr << "Unable to open screendump.txt" << std::endl;
        return 1;
    }
    for(int i=0; i<8192; i++){
        int n;
        vidfile>>n;
        graphicsRAM[i]=(unsigned char)n;
    }

    // Then read the other variables:

    vidfile >> tileset;
    vidfile >> tilemap;
    vidfile >> scrollx;
    vidfile >> scrolly;
    vidfile >> palette[0];
    vidfile >> palette[1];
    vidfile >> palette[2];
    vidfile >> palette[3];

    // Now that graphicsRAM and other variables are initialized, call renderScreen
    renderScreen();
}
