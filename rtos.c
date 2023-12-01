

#include "rtos.h"
#include "gpio.h"
#include "uart0.h"
#include "commandline.h"
#include "tm4c123gh6pm.h"
#include "mem.h"

static void rec_fun(void);

void rtosInit()
{   
        //system control bits to enable faults
    NVIC_SYS_HND_CTRL_R = 
                  NVIC_SYS_HND_CTRL_USAGE 
                | NVIC_SYS_HND_CTRL_BUS   
                | NVIC_SYS_HND_CTRL_MEM;

    NVIC_CFG_CTRL_R = NVIC_CFG_CTRL_DIV0| NVIC_CFG_CTRL_UNALIGNED;

    // psp bit
    //asp bit
    //isb
    
    //full access with -x,+rw,-rw Region 0, 4GB
    //NVIC_MPU_NUMBER_R = 0 ;
    //NVIC_MPU_BASE_R = (0 | 1 << 4 | 0);
    //NVIC_MPU_ATTR_R = (1 << 28 | 1 << 24 /*| 1 << 18|  1 << 17 | 1 << 16  | 0xFF << 8 */ | 0x1F << 1 | 1);
    //__asm(" isb");
    //__asm(" dsb");
    NVIC_MPU_CTRL_R |= NVIC_MPU_CTRL_PRIVDEFEN;
    
    //mpuOn();
    //allowFlashAccess();
    //setupSramAccess();

    mem_init();
}


void initPbs_leds(void)
{
    enablePort(PORTB);
    enablePort(PORTC);
    enablePort(PORTD);
    enablePort(PORTE);


    selectPinDigitalInput(PUSH_BUTTON_1);
    selectPinDigitalInput(PUSH_BUTTON_2);
    selectPinDigitalInput(PUSH_BUTTON_3);
    selectPinDigitalInput(PUSH_BUTTON_4);
    selectPinDigitalInput(PUSH_BUTTON_5);
    selectPinDigitalInput(PUSH_BUTTON_6);

    enablePinPullup(PUSH_BUTTON_1);
    enablePinPullup(PUSH_BUTTON_2);
    enablePinPullup(PUSH_BUTTON_3);
    enablePinPullup(PUSH_BUTTON_4);
    enablePinPullup(PUSH_BUTTON_5);
    enablePinPullup(PUSH_BUTTON_6);

    selectPinPushPullOutput(GREEN_LED);
    selectPinPushPullOutput(RED_LED);
    selectPinPushPullOutput(ORANGE_LED);
    selectPinPushPullOutput(YELLOW_LED);
}

void test_pb_leds(void)
{
    char temp_str[20]={0};
    uint8_t pb_vals=0;

    putsUart0("\n\rtesting pbs and leds\n");
    do 
    {
        putsUart0("\n\rpress '#' if you want it to terminate\n\r");
        setPinValue(GREEN_LED,1);
        setPinValue(RED_LED,1);
        setPinValue(ORANGE_LED,1);
        setPinValue(YELLOW_LED,1);

        pb_vals = getPbs();

        putsUart0("\n\rpb values: ");
        convertIntToString(temp_str,20,pb_vals);
        putsUart0(temp_str);
        putsUart0("\n\r");
        pb_vals = 0;
    } while ( (getcUart0()) != '#');
}


uint8_t getPbs(void)
{
    uint8_t pb_vals=0;

    (getPinValue(PUSH_BUTTON_1) == 0) ? (pb_vals |= 1):  (pb_vals &= ~1);
    (getPinValue(PUSH_BUTTON_2) == 0) ? (pb_vals |= 2):  (pb_vals &= ~2);
    (getPinValue(PUSH_BUTTON_3) == 0) ? (pb_vals |= 4):  (pb_vals &= ~4);
    (getPinValue(PUSH_BUTTON_4) == 0) ? (pb_vals |= 8):  (pb_vals &= ~8);
    (getPinValue(PUSH_BUTTON_5) == 0) ? (pb_vals |= 16): (pb_vals &= ~16);
    (getPinValue(PUSH_BUTTON_6) == 0) ? (pb_vals |= 32): (pb_vals &= ~32);   

    return pb_vals;
}

void check_interrupts(void)
{
    uint8_t pb_vals=0;

    putsUart0("\n\rtesting interrupts\n");
    do {

        putsUart0("\n\rpress '#' if you want it to terminate\n\r");

        pb_vals = getPbs();

        switch (pb_vals)
        {
        case 1:
        {
            // this gives usage fault
            //volatile uint32_t *ptr = (uint32_t*)0xFFFFFFFF;
            //volatile uint32_t val = *ptr;

            //Usage fault
            uint16_t a = 1;
            uint16_t b = 0;
            uint16_t result = a/b;
            ++result;

        }break;

        case 2:
        {  
            //Bus fault
            WATCHDOG0_LOAD_R = 13;

        }break;

        case 4:
        {
            //Hard Fault
            NVIC_SYS_HND_CTRL_R = NVIC_SYS_HND_CTRL_MEM;

            setPsp(0x20008000);
            setAspPsp();
            __asm(" isb" );
            //setTmplUnP();

            // bus fault
            //rec_fun();
            volatile uint32_t *ptr = (volatile uint32_t*)0x40014000;
            volatile uint32_t val = *ptr;
            //*ptr = 1;

            //bus fault
            //__asm("hardfault: mov R0, #0");
            //__asm("           ldr R1, [R0, #0]");

            //volatile uint32_t *ptr = (uint32_t*)0xFFFFFFFF;
            //volatile uint32_t val = *ptr;

        }break;

        case 8:
        {
            //PendSv
            NVIC_INT_CTRL_R = (1 << 28);
            //setPendSv();
        }break;

        case 16:
        {
            //Mpu fault
            setPsp(0x20007FFC); // testing
            setAspPsp();
            //__asm( "isb" );
            rec_fun();
        }break;

        case 32:
        {

        }break;
        
        default:
            break;
        }

        pb_vals = 0;

    }while ( (getcUart0()) != '#');



}

static void rec_fun(void)
{

    void (*fun)(void) = (void (*) (void))0x40000001;
    fun();
}
