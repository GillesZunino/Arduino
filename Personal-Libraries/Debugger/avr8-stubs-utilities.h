#if defined(DEBUG)
  // ATMega AVR architecture
  #if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) || defined(__AVR_ATmega1284__) || defined(__AVR_ATmega1284P__) || defined(__AVR_ATmega328__) || defined(__AVR_ATmega328P__) || defined(__AVR_ATmega4809__) 
    #include <avr/interrupt.h>
    
    #define AVR8_GDB_STUB

    // On ATMega 2560, use USART1 and leave USART0 free for Arduino Serial
    #if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
        #define AVR8_UART_NUMBER 1
    #endif

    // On ATMega 2560, We can use INT6 or INT7 since pins PE6 and PE7 are not exposed
    #if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
        #define AVR8_SWINT_SOURCE INT6
    #endif

    #include "avr8-stub.h"
    #include "app_api.h"

    #define BREAKPOINT breakpoint();
    #define DEBUG_MESSAGE(__msg) debug_message(__msg);
    #define INIT_DEBUGGER() { debug_init(); sei(); }
  #endif
#else
  #define BREAKPOINT
  #define DEBUG_MESSAGE(__msg)
  #define INIT_DEBUGGER()
#endif