#include <iostream>
#include <fstream>
//#include <vector>
#include "Z80.h"
#include "gameboy.pro"
#include "screen.h"
#include "screen.cpp"

// Global ROM array - Will be loaded from a file
unsigned char* rom; // Use unsigned char* for binary data
int romSize = 0;
unsigned char graphicsRAM[8192];
int palette[4];
int tileset, tilemap, scrollx, scrolly;


// Memory access functions
unsigned char memoryRead(int address) {
    if(address < romSize) {
        return rom[address];
    }
    return 0; // Return 0 for addresses beyond the ROM size
}

void memoryWrite(int address, unsigned char b) {
    // For this project, writing to ROM is not handled
}

extern QApplication* app;
extern Screen* screen;
int main(int argc, char** argv) {

    setup(argc,argv);
    //part 1 code here
    // Load ROM from file
    std::ifstream romfile("testrom.gb", std::ios::in | std::ios::binary | std::ios::ate);
    if (!romfile.is_open()) {
        std::cerr << "Failed to open ROM file." << std::endl;
        return 1;
    }
    std::streampos size = romfile.tellg();
    rom = new unsigned char[size]; // Allocate memory for the ROM
    romSize = size;

    romfile.seekg(0, std::ios::beg);
    romfile.read(reinterpret_cast<char*>(rom), size); // Cast to char* for read function
    romfile.close();

    // Initialize the Z80 CPU with memory access functions
    Z80* z80 = new Z80(memoryRead, memoryWrite);

    // Reset the CPU
    z80->reset();

    // Execute instructions until the CPU halts
    while(!z80->halted) {
        z80->doInstruction();
        std::cout << "PC: " << z80->PC << ", A: " << (int)z80->A << ", B: " << (int)z80->B << std::endl;
    }

    // Output the final value in register A (expected: 21)
    std::cout << "Final A: " << (int)z80->A << std::endl;

    //return 0;
    //part 2 code here
    // read the first 8192 integers from screendump into graphicsRAM.
    if (!vidfile.is_open()) {
        std::cerr << "Unable to open screendump.txt" << std::endl;
        return 1;
    }
    int n;
    ifstream vidfile("screendump.txt",ios::in);
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

    // Enter the Qt application loop
    return app.exec();
}
 
