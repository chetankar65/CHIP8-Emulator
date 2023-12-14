#include <iostream>
#include <cstdint>
#include <fstream>
#include <thread>
#include <chrono>
#include <bitset>

using namespace std;

class Chip8 {
    // initialise the emulator
    public:
        uint8_t registers[16];
        uint8_t memory[4096]; // 4k bytes of ram
        uint16_t index; // maximum address it can store is FFF
        uint16_t program_counter; // address of instruction
        uint16_t Stack[16];
        uint8_t stack_pointer;
        uint8_t delay_timer;
        uint8_t sound_timer;
        uint8_t keypad[16]{};
        uint32_t video[64 * 32]{};
        uint16_t opcode;

        const unsigned int START_ADDRESS = 0x200;

        static const unsigned int FONTSET_SIZE = 80;

        uint8_t fontset[FONTSET_SIZE] =
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

        const unsigned int START_FONT_ADDRESS = 0x50;

        //void (*Table[0XF])(); // will contain opcode mappings
        typedef void (Chip8::*OpcodeFunction)();
        OpcodeFunction Table[0xF];

        Chip8() {
            program_counter = START_ADDRESS;

            // initialise the characters as well which will be displayed
            for (int i = 0; i < FONTSET_SIZE; i++) {
                memory[START_FONT_ADDRESS + i] = fontset[i];
            }

            for (int i = 0; i < 0xF; i++) {
                registers[i] = 0x00;
            }

            // put the function's addresses in the table
            Table[0x0] = &Chip8::OP_00EE;
            Table[0x1] = &Chip8::OP_1nnn;
            Table[0x2] = &Chip8::OP_2nnn;
            Table[0x3] = &Chip8::OP_3xkk;
            Table[0x4] = &Chip8::OP_4xkk;
            Table[0x5] = &Chip8::OP_5xy0;
            Table[0x6] = &Chip8::OP_6xkk;
            Table[0x7] = &Chip8::OP_7xkk;
            Table[0x8] = &Chip8::OP_8xy0;
            Table[0x9] = &Chip8::OP_8xy1;
            Table[0xA] = &Chip8::OP_8xy2;
            Table[0xB] = &Chip8::OP_8xy3;
            Table[0xC] = &Chip8::OP_8xy4;
            Table[0xD] = &Chip8::OP_8xy5;
        }

        void loadRom(char const* filename) {
            ifstream file(filename, ios::binary | ios::ate);

            if (file.is_open()) {
                streampos size = file.tellg();
                char* buffer = new char[size];

                file.seekg(0, ios::beg);
                file.read(buffer, size);
                file.close();

                for (long i = 0; i < size; i++) {
                    memory[START_ADDRESS + i] = buffer[i];
                    cout << buffer[i] << endl;
                }

                delete[] buffer;
            }
        }

        void loadInstructions(uint8_t buffer[], int size) {
            for (long i = 0; i < size; i++) {
                memory[START_ADDRESS + i] = buffer[i];
            }
        }

        /// OPCODES
        // cls: clears the display
        // return function
        void OP_00EE() {
            --stack_pointer;
            program_counter = Stack[stack_pointer];
        }

        // 1nnn - JMP Instruction
        void OP_1nnn() {
            uint16_t jump_address = opcode & 0x0FFF; // will extract the lower 3 bytes
            program_counter = jump_address;
        }

        // 2nnn - CALL
        void OP_2nnn() {
            uint16_t jump_address = opcode & 0x0FFF; // will extract the lower 3 bytes
            Stack[stack_pointer] = program_counter;
            stack_pointer++;
            program_counter = jump_address;
        }

        // skip next instruction contents of Vx = kk
        void OP_3xkk() {
            uint16_t register_address = (opcode & 0x0F00) >> 8;
            uint16_t bytes = (opcode & 0x00FF);

            if (registers[register_address] == bytes) {
                program_counter += 2;
            }
        }

        // skip next instruction contents of Vx != kk
        void OP_4xkk() {
            uint16_t register_address = (opcode & 0x0F00) >> 8;
            uint16_t bytes = (opcode & 0x00FF);

            if (registers[register_address] != bytes) {
                program_counter += 2;
            }
        }

        // skip next instruction contents if register contents (vx = vy)
        void OP_5xy0() {
            uint16_t register_x = (opcode & 0x0F00) >> 8;
            uint16_t register_y = (opcode & 0x00F0) >> 4;

            if (registers[register_x] == registers[register_y]) {
                program_counter += 2;
            }
        }

        // 6xkk load vx register by byte
        void OP_6xkk() {
            uint16_t register_x = (opcode & 0x0F00) >> 8;
            uint16_t byte = (opcode & 0x00FF);

            registers[register_x] = byte;
        }

        // add vx, byte
        void OP_7xkk() {
            cout << "Addition" << endl;
            uint16_t register_x = (opcode & 0x0F00) >> 8;
            uint16_t byte = (opcode & 0x00FF);

            registers[register_x] += byte;
        }

        // load Vx, Vy
        void OP_8xy0() {
            uint8_t Vx = (opcode & 0x0F00u) >> 8u;
            uint8_t Vy = (opcode & 0x00F0u) >> 4u;

            registers[Vx] = registers[Vy];
        }

        // vx = vx or vy
        void OP_8xy1() {
            uint8_t Vx = (opcode & 0x0F00u) >> 8u;
            uint8_t Vy = (opcode & 0x00F0u) >> 4u;

            registers[Vx] |= registers[Vy];
        }

        // AND vx, Vy
        void OP_8xy2() {
            uint8_t Vx = (opcode & 0x0F00u) >> 8u;
            uint8_t Vy = (opcode & 0x00F0u) >> 4u;

            registers[Vx] &= registers[Vy];
        }

        // XOR vx, vy
        void OP_8xy3() {
            uint8_t Vx = (opcode & 0x0F00u) >> 8u;
            uint8_t Vy = (opcode & 0x00F0u) >> 4u;

            registers[Vx] ^= registers[Vy];
        }

        // ADD Vx, Vy
        void OP_8xy4() {
            uint8_t Vx = (opcode & 0x0F00u) >> 8u;
            uint8_t Vy = (opcode & 0x00F0u) >> 4u;

            uint16_t sum = registers[Vx] + registers[Vy];

            if (sum > 255U)
            {
                registers[0xF] = 1;
            }
            else
            {
                registers[0xF] = 0;
            }

            registers[Vx] = sum & 0xFFu;
        }

        // SUB Vx, Vy
        void OP_8xy5() {
            uint8_t Vx = (opcode & 0x0F00u) >> 8u;
            uint8_t Vy = (opcode & 0x00F0u) >> 4u;

            if (registers[Vx] > registers[Vy])
            {
                registers[0xF] = 1;
            }
            else
            {
                registers[0xF] = 0;
            }

            registers[Vx] -= registers[Vy];
        }

        void Decode() {
            uint16_t first_digit = (opcode & 0xF000) >> 12;
            
            if (first_digit >= 0 && first_digit <= 7) {
                (this->*Table[first_digit])();
            } else {
                uint16_t last_digit = (opcode & 0x000F);
                (this->*Table[8 + first_digit])();
            }
        }

        void viewRegisters() {
            cout << "Register contents: " << endl;
            for (int i = 0; i <= 15; i++) {
                if (i != 0xF) {
                    cout << "Register " << i << ": " << dec << static_cast<unsigned int>(registers[i]) << endl;
                } else {
                    cout << "Carry or borrow register: " << dec << static_cast<unsigned int>(registers[i]) << endl;
                }
            }
        }

        // view memory also
        // write opcodes for some memory operations

        // fetch and decode the instructions from memory
        void Cycle() {
            opcode = (memory[program_counter] << 8) | (memory[program_counter + 1]);
            program_counter += 2;

            Decode();
        }
};

int main() {
    Chip8 chip8;
    //chip8.loadRom("test_opcode.ch8");

    uint8_t buffer[4] = {
        0x60,
        0x22,
        0x70,
        0x22
    };

    // 6xkk
    // 0b0110000011111111

    chip8.loadInstructions(buffer, 4);

    while (true) {
        chip8.Cycle();
        chip8.viewRegisters();
        std::this_thread::sleep_for(std::chrono::microseconds(300000)); // Adjust as needed
    }

    return 0;
}
