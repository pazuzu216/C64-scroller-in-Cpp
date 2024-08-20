# C64-scroller-in-Cpp
1x1 Text Scroller Class for LLVM-MOS for the Commodore 64

This is an experiment to see if LLVM-MOS can handle a simple scroller in C++. It does.

The scroller uses the character set from the C64 ROM, and the scroll register moves the text pixel by pixel. The code is written to be readable, not efficient—optimization is left to the compiler. The routine attaches to the vertical blank interrupt.

To compile and modify this code, you will need the LLVM-MOS compiler, which is freely available on GitHub: LLVM-MOS on GitHub: https://github.com/llvm-mos/llvm-mos (Compile with: mos-c64-clang -O3 -o main.prg main.cc)


If you don't want to run this on original hardware, you can use the VICE emulator for testing: https://vice-emu.sourceforge.io/

You are free to modify and use this code as you see fit.

Gisle Kirkhaug
August 20, 2024

Video: https://youtu.be/0NNIaadEyFc
