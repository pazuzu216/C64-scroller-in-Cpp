// 1x1 Text Scroller Class for LLVM-MOS for the Commodore 64
// It's an experiment to see if LLVM-MOS can handle a simple scroller in C++. It does
// Gisle Kirkhaug, 18/8, 2024

// LLVM-MOS - https://github.com/llvm-mos/llvm-mos (Compile with: mos-c64-clang -O3 -o main.prg main.cc)
// VICE - https://vice-emu.sourceforge.io/ (Drag and drop main.prog into the VICE emulator)
// C64 Memory Map - https://sta.c64.org/cbm64mem.html

#include <cstdint>
#include <string.h>


// 1x1 Text Scroller Class for LLVM-MOS for the Commodore 64
class Scroller {
  public:
    Scroller(char *msg, int lLen) { // Constructor: pass a char array of uppercase scroll text and its length
      Message = &msg[0];            // Copy the address of the scroll text to a static variable in the Scroller class
      Len = lLen - 1;               // Copy the length of the scroll text to a static variable in the Scroll class, subtracting 1 since the first character is at index [0]
      clear_screen();               // Clears the screen of characters, a member of this class
      set_vb_inter(&Scroller::scroll);    // Set the raster interrupt to trigger our scroll; scroll and set_vb_inter are members of this class
    }

  private:
    static constexpr uint16_t DEFAULT_SCREEN = 0x0400;        // Location of character RAM
    #define InterCtrl1  (*((volatile unsigned char *)0xDC0D)) // Interrupt Control and Status Register, CIA1
    #define InterCtrl2  (*((volatile unsigned char *)0xDD0D)) // Interrupt Control and Status Register, CIA2
    #define InterCtrl   (*((volatile unsigned char *)0xD01A)) // Interrupt Control Register
    #define InterStat   (*((volatile unsigned char *)0xD019)) // Interrupt Status Register
    #define ScrCtrl1    (*((volatile unsigned char *)0xD011)) // Screen Control Register #1
    #define ScrCtrl2    (*((volatile unsigned char *)0xD016)) // Screen Control Register #2
    #define RasterLine1 (*((volatile unsigned char *)0xD012)) // Read: Current raster line. Write: Raster line to generate an interrupt at (bits #0–#7)
    static char *Message;                 // Pointer to scroll text. The scroll() member must be static for the raster interrupt, so this must also be static
    static int Len;                       // Length of scroll text

    void clear_screen() {   // Clears the screen of characters; used by the class constructor
      auto *screen_ptr = reinterpret_cast<uint8_t *>(DEFAULT_SCREEN);   // Pointer to character RAM at $400
      memset(screen_ptr,' ',40*25);                                     // Fill character screen memory with spaces: 25 rows, 40 columns
    }

    static void scroll() {  // Perform text scrolling; triggers on raster interrupt at line 0 and 241, set up by the set_vb_inter class member
      static int8_t offset;                                             // Offset for the bitwise scroll register, 0 to 7
      static uint8_t nextchar;                                          // Position in the scroll text (*Message)
      if (RasterLine1 < 10) {                                           // Check if the raster interrupt is triggered around line 0; if so, don't scroll this area ($D012)
        ScrCtrl2 &= 0xF0;                                               // Default to no scroll at the start of the screen, and set the screen to 38 columns so the leftmost character does not suddenly disappear ($DD0D)
        RasterLine1 = 241;                                              // Trigger the next raster interrupt on the last character row ($D012)
        if(--offset<0) {                                                // Decrement the bitwise scroll variable; if it's below 0, move the scroller one character and set the bitwise offset variable to 7
          auto *screen_ptr = reinterpret_cast<uint8_t *>(DEFAULT_SCREEN);   // Pointer to character RAM at $400
          offset = 7;                                                   // Scroll one bit 8 times, then reset to 7 (scrolling from 7 to 0)
          for(int i = (24*40); i < (24*40+39); i++) *(screen_ptr + i) = *(screen_ptr + i + 1); // Shift scroll text characters to the left on the bottom row. The screen is 40 characters wide and 25 characters high
          *(screen_ptr+39+24*40) = Message[nextchar++];                 // Add the next character in the scroll text to the text on the screen, and increment the position in the scroll text
          if(nextchar >= Len) nextchar = 0;                             // If the whole text has been displayed, start from the beginning again
        }
      } else { // Raster interrupt at line 241
        ScrCtrl2 += offset;                                             // This is the last character row; set the bitwise scroll register (7–0). ScrCtrl2 &= 248; is done at raster line 0, so it’s not necessary here ($DD0D)
        RasterLine1 = 0;                                                // Set the next raster interrupt to line 0 ($D012)
      }
      InterStat = 1;                                                    // Acknowledge raster interrupt
      asm("jmp $EA31");                                                 // Jump to default interrupt handler routine (assembler)
    }

    void set_vb_inter(void (* vector)(void)) {  // Initialize the raster interrupt to jump to the function passed (scroll class member), used by the class constructor
      asm("sei");                                         // Set interrupt disable (assembly); disable interrupts
      InterCtrl2 = InterCtrl1 = 0x7f;                     // Turn off the CIA1 and CIA2 interrupts ($DD0D and $DC0D)
      ScrCtrl1 &= 0x7f;                                   // Clear the high bit of the raster line ($D011)
      RasterLine1 = 0;                                    // Trigger the raster interrupt on the first scan line ($DD0D)
      (*((volatile unsigned char *)0x314)) = (unsigned int)vector;      // Execution address of the interrupt service routine, low byte ($0314)
      (*((volatile unsigned char *)0x315)) = (unsigned int)vector >> 8; // Execution address of the interrupt service routine, highbyte ($0315)
      InterCtrl = 1;                                      // Enable raster interrupts ($DC0D)
      asm("cli");                                         // Clear interrupt disable (assembly); enable interrupts
    }
};
char *Scroller::Message;    // These two are here to prevent the compiler from failing. It is fallout from the interrupt handler having to be static in the class
int Scroller::Len;

auto main() -> int {
  // Text in scroller: would have used a string, but #include <string> is not available at this time
  static char ScrollText[] =
      "1X1 TEXT SCROLLER CLASS FOR LLVM-MOS FOR THE COMMODORE 64. IT'S AN "
      "EXPERIMENT TO SEE IF LLVM-MOS CAN HANDLE A SIMPLE SCROLLER IN C++, IT "
      "DOES. GISLE KIRKHAUG, 18/8, 2024. ";

  Scroller MyScroll(ScrollText, sizeof(ScrollText));   // Start the 1x1 text scroller at the bottom row; pass the scroll text and its length
}