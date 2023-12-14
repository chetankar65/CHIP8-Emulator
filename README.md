# CHIP8-Emulator

Built a small (incomplete) CHIP8 Emulator using C++ . I referred to the documentation of CHIP8 and implemented a few opcodes. <br>
I have not implemented all, as it would've taken too long.<br>

I have tested it only with two opcodes:
6XKK - Loads value (KK) into register X
7XKK - Adds value (KK) into register x

The emulator as of now just prints register values. Project was just to strengthen my understanding so will not implement GUI and other opcodes.

Screenshots of the two opcodes in action:

First loading into Vx register (Opcode: 0x6022):
![SC1](https://github.com/chetankar65/CHIP8-Emulator/assets/26086224/972bd6b8-f43f-40fc-aa75-3c9211ab5d47)

Adding to Vx register (Opcode: 0x7022):
![SC2](https://github.com/chetankar65/CHIP8-Emulator/assets/26086224/ca3a086d-83a1-41b2-9871-c1db8317f6ad)

Ideally, we will load in all the opcodes from a .ch8 file. We can use a CHIP8 assembler to convert instructions into a .ch8 file
and then load it into the program. I have not tested that and for now just tried with two opcodes.

The CHIP8 processor has the following components:
- 16 registers labelled V0-VF, of 8 bits size each.
- 4K memory; each location being 8 bits
- A 16 bit index to access a particular memory location. We need to use a 16 bit index as the maximum memory locations we can access is 0xFFF, but C++ does not support uint12_t.
- 16 bit program counter; the program instruction starts from address 0x200
- 16 level stack ( For CALL and RET functions)
- An 8 bit stack pointer
- 8 bit delay timer (not implemented)
- 8 bit sound timer (not implemented)
- Keypad (not implemented)
- 64 x 32 pixel dimension display (not implemented)
- Opcode is of 16 bits length.

Referred to CHIP8 documentation and some other articles online.

