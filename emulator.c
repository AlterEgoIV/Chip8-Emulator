#include <stdio.h>
#include <stdlib.h>
#include <stdint.h> // Header file that includes cross platform fixed width integers
#include <SDL.h>

/*
    This is an array to represent the set of registers that exist in the Chip8 CPU.
    It has 16 elements because the Chip8 CPU contains 16 registers.

    Questions:
    Why is it of type integer?
    Why is every element 8 bits / 1 byte in size?
    Why is it unsigned?

    Answers:
    The type itself does not matter. What matters is the size of the type.

    The elements are 8 bits / 1 byte because the Chip8 registers are that size
    and so the elements must be to accurately emulate it.
*/
extern SDL_Surface* screenSurface;
uint8_t registers[16];
uint16_t indexRegister;

/*
    The instruction pointer / program counter. It is responsible for keeping track of
    which instruction in the program to execute.

    Questions:
    Why is it of type integer?
    Why is it 16 bits / 2 bytes in size?
    Why is it unsigned?

    Answers:
    The instruction pointer holds instructions. Chip8 instructions are 16 bits long,
    so the instruction pointer must be that size to be able to contain them fully.
*/
uint16_t instructionPointer; // Instruction pointer. Always points to next instruction.

/*
    This is an array to represent the overall memory of the Chip8.
    It has 4096 elements because the Chip8 has 4096 bytes of memory.
    Every element has 8 bits of memory in order to accurately represent each
    element as one byte. With 4096 elements, that adds up to 4096 bytes.

    Questions:
    Why is it of type integer?
    Why is it unsigned?
*/
uint8_t memory[4096]; // Will contain instructions and data

void decodeAndExecute(uint16_t instruction);
void printRegisters();

int emulator(char *filename)
{
    /*
        Assigning iPtr a value means assigning it a memory address
        to execute the next instruction at.
        In this case iPtr is assigned the value 512, so the instruction at
        address 512 in memory will be the next instruction to be executed.
    */
    /*FILE *fp=fopen(filename,"r");
    uint8_t byte;
    uint16_t address = 512;
    while(!feof(fp))
    {
        fscanf(fp,"%c",&byte);
        memory[address] = byte;
        address++;
    }
    fclose(fp);*/
    instructionPointer = 512;

    // Starting address we can work with is 0x200
    memory[0x200] = 0x60; // assign register 0
    memory[0x201] = 0x00; // the value 0

    memory[0x202] = 0x70; // add to register 0
    memory[0x203] = 0x01; // the value 1

    memory[0x204] = 0x30; // if register 0
    memory[0x205] = 0x0A; // equals 0A

    memory[0x206] = 0x12; // goto memory address
    memory[0x207] = 0x02; // 202

    memory[0x208] = 0x60; // assign register 0
    memory[0x209] = 0x00; // the value 0

    memory[0x20A] = 0x12; // goto memory address
    memory[0x20B] = 0x02; // 202

    uint8_t value = 1;

    //while(1)
    {
        uint16_t instruction = (memory[instructionPointer] << 8) + memory[instructionPointer + 1]; // fetch instruction

        decodeAndExecute(instruction);

        printRegisters();

        //scanf("%d", &value);
    }

    /*while(1) // Infinite loop
    {
        uint16_t instruction = (memory[iPtr] << 8) + memory[iPtr + 1];

        decodeAndExecute(instruction);

        printRegisters();
    }*/

    return 0;
}

void decodeAndExecute(uint16_t instruction) // Pass in the next instruction to execute
{
    printf("Instruction: 0x%x, %d\n", instruction, instruction);

    /*
        The portion of the instruction that specifies the type of instruction we are executing
        is the first four bits of the value the instruction variable holds.
        As such, we are only interested in checking the first four bits.

        In order to remove the bits after the first four from instruction,
        we must perform a bit mask operation.

        In this case, we AND a mask of value 0xF000 to instruction.

        What this does is compare the bits that make up instruction and the bits that
        make up 0xF000 and bitwise ANDs them together.

        Since AND only returns 1 when both compared bits are 1 and 0xF000 equates to 1111 0000 0000 0000,
        when the two values are ANDed together only the first four bits in instruction will be left unchanged.
        If any of instructions first four bits is 1, they will remain 1. If any are 0, they will remain 0.

        Thus we are left with the portion of the instruction that tells us what type of instruction it is.

        Questions:
        Is this operation performed on instruction itself, leaving it permanently changed?
    */
    switch(instruction & 0xF000)
    {
        // Any instruction beginning with 1 is a go to instruction
        case 0x1000: // Why are there trailing zeros?
        {
            instructionPointer = instruction & 0x0FFF; // Go to specified address
            break;
        }

        case 0x3000:
        {
            uint8_t operand = instruction & 0x00FF;
            uint8_t targetRegister = (instruction >> 8) & 0x000F;

            if(registers[targetRegister] == operand)
            {
                instructionPointer += 4; // skip
            }
            else
            {
                instructionPointer += 2; // don't skip
            }

            break;
        }

        case 0x4000:
        {
            break;
        }

        case 0x5000:
        {
            break;
        }

        // Any instruction beginning with 6 is an assignment instruction
        case 0x6000:
        {
            uint8_t operand = instruction & 0x00FF;
            uint8_t targetRegister = (instruction >> 8) & 0x000F;

            registers[targetRegister] = operand;

            instructionPointer += 2;

            break;
        }

        // Any instruction beginning with 7 is an addition instruction
        case 0x7000:
        {
            uint8_t operand = instruction & 0x00FF;
            uint8_t targetRegister = (instruction >> 8) & 0x000F;

            registers[targetRegister] += operand;

            instructionPointer += 2;

            break;
        }

        case 0x8000:
        {
            uint8_t registerX = (instruction & 0x0F00) >> 8;
            uint8_t registerY = (instruction & 0x00F0) >> 4;

            switch(instruction & 0x000F)
            {
                // Assignment
                case 0x0000:
                {
                    registers[registerX] = registers[registerY];

                    instructionPointer += 2;

                    break;
                }

                // Bitwise OR
                case 0x0001:
                {
                    registers[registerX] = registers[registerX] | registers[registerY];
                    registers[0xF] = 0;

                    instructionPointer += 2;

                    break;
                }

                // Bitwise AND
                case 0x0002:
                {
                    registers[registerX] = registers[registerX] & registers[registerY];
                    registers[0xF] = 0;

                    instructionPointer += 2;

                    break;
                }

                // Bitwise XOR
                case 0x0003:
                {
                    registers[registerX] = registers[registerX] ^ registers[registerY];
                    registers[0xF] = 0;

                    instructionPointer += 2;

                    break;
                }

                case 0x0004:
                {
                    unsigned int x, y, result;
                    x = registers[registerX];
                    y = registers[registerY];
                    result = x + y;

                    registers[registerX] = result & 0xFF;
                    registers[0xF] = result >> 8;

                    instructionPointer += 2;

                    break;
                }

                case 0x0005:
                {
                    unsigned int x, y, result;
                    x = registers[registerX] + 0b100000000; // allowing for a borrow from X during subtraction
                    y = registers[registerY];
                    result = x - y;

                    registers[registerX] = result & 0xFF;
                    registers[0xF] = result >> 8;

                    instructionPointer += 2;

                    break;
                }

                // Bitwise Right Shift
                case 0x0006:
                {
                    registers[0xF] = registers[registerX] & 0b00000001;
                    registers[registerX] = registers[registerX] >> 1;

                    instructionPointer += 2;

                    break;
                }

                case 0x0007:
                {
                    unsigned int x, y, result;
                    x = registers[registerY] + 0b100000000; // allowing for a borrow from X during subtraction
                    y = registers[registerX];
                    result = x - y;

                    registers[registerX] = result & 0xFF;
                    registers[0xF] = result >> 8;

                    instructionPointer += 2;

                    break;
                }

                // Bitwise Left Shift
                case 0x000E:
                {
                    registers[0xF] = registers[registerX] & 0b10000000;

                    if(registers[0xF] != 0)
                    {
                        registers[0xF] = 1;
                    }

                    registers[registerX] = registers[registerX] << 1;

                    instructionPointer += 2;

                    break;
                }
            }

            break;
        }

        case 0xD000:
        {
            int XRegNum = ((instruction & 0x0F00) >> 8);
            int YRegNum = ((instruction & 0x00F0) >> 4);
            int length = (instruction & 0x000F);
            drawSprite(screenSurface, &memory[indexRegister], length, registers[XRegNum], registers[YRegNum]);

            break;
        }
    }
}

void printRegisters()
{
    int i;
    for(i = 0; i < 16; ++i)
    {
        printf("Register %d: 0x%x, %d\n", i, registers[i], registers[i]);
    }

    printf("Instruction pointer: 0x%x, %d\n", instructionPointer, instructionPointer);
}
