#include <iostream>
#include <fstream>
//#include <vector>
#include "Z80.h"
//#include "gameboy.pro"
#include "screen.h"



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

unsigned char rows[2] = {0x0F, 0x0F};

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

    //std::cout << "Row 0: " << std::hex << (int)rows[0] << std::endl;
    //std::cout << "Row 1: " << std::hex << (int)rows[1] << std::endl;

    //std::cout << "Key value: " << std::hex << key_value << std::endl;
    std::cout << "Key value: " << std::hex << (int)keyboardColumn << std::endl;

    switch(keyboardColumn){
    case 0x10: return rows[1]; //buttons
    case 0x20: return rows[0]; //direction
    case 0x30: return result;
    default: return result;
    }
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
        //std::cout << "b = " << (int)b << std::endl;
        keyboardColumn = b & 0x30;
        //std::cout << "keyboardColumn = " << keyboardColumn << std::endl;
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
    case RIGHT:
        //keyboardColumn = 0x20;
        rows[0] &= 0xE; // Clear bit 0 of row 0
        break;
    case LEFT:
        //keyboardColumn = 0x20;
        rows[0] &= 0xD; // Clear bit 1 of row 0
        break;
    case UP:
      //  keyboardColumn = 0x20;
        rows[0] &= 0xB; // Clear bit 2 of row 0
        break;
    case DOWN:
     //   keyboardColumn = 0x20;
        rows[0] &= 0x7; // Clear bit 3 of row 0
        break;
    case A:
      //  keyboardColumn = 0x10;
        rows[1] &= 0xE; // Clear bit 0 of row 1
        break;
    case B:
     //   keyboardColumn = 0x10;
        rows[1] &= 0xD; // Clear bit 1 of row 1
        break;
    case ENTER:
      //  keyboardColumn = 0x10;
        rows[1] &= 0xB; // Clear bit 2 of row 1
        break;
    case SPACE:
     //   keyboardColumn = 0x10;
        rows[1] &= 0x7; // Clear bit 3 of row 1
        break;
    }
}

void keyup(int scanCode) {
    switch (scanCode) {
    case RIGHT:
        rows[0] |= 0x1; // Set bit 0 of row 0
        break;
    case LEFT:
        rows[0] |= 0x2; // Set bit 1 of row 0
        break;
    case UP:
        rows[0] |= 0x4; // Set bit 2 of row 0
        break;
    case DOWN:
        rows[0] |= 0x8; // Set bit 3 of row 0
        break;
    case A:
        rows[1] |= 0x1; // Set bit 0 of row 1
        break;
    case B:
        rows[1] |= 0x2; // Set bit 1 of row 1
        break;
    case ENTER:
        rows[1] |= 0x4; // Set bit 2 of row 1
        break;
    case SPACE:
        rows[1] |= 0x8; // Set bit 3 of row 1
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

        //std::cout << "Keyboard Column: " << std::hex << (int)keyboardColumn << std::endl;
        //0, 20, 10, 30

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
