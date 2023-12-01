// Serial Example
// Jason Losh

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target Platform: EK-TM4C123GXL Evaluation Board
// Target uC:       TM4C123GH6PM
// System Clock:    40 MHz

// Hardware configuration:
// Red LED:
//   PF1 drives an NPN transistor that powers the red LED
// Green LED:
//   PF3 drives an NPN transistor that powers the green LED
// UART Interface:
//   U0TX (PA1) and U0RX (PA0) are connected to the 2nd controller
//   The USB on the 2nd controller enumerates to an ICDI interface and a virtual COM port
//   Configured to 115,200 baud, 8N1

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#include <stdint.h>
#include <stdbool.h>

#include "clock.h"
#include "uart0.h"
#include "tm4c123gh6pm.h"
#include "commandline.h"
#include "rtos.h"
#include "mem.h"


//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

static void getsUart0(USER_DATA *info);
static void displayUserData(USER_DATA *info);
static bool processInput(char *cptr,uint8_t *countptr, USER_DATA *info);
static void parseFields(USER_DATA *info);
static char* getFieldString(USER_DATA* data, uint8_t fieldNumber);
static int32_t getFieldInteger(USER_DATA* data, uint8_t fieldNumber);
static int32_t converToInt(char *str);
static void clearUserData(USER_DATA *info, uint8_t *bufferIndx);
static int stringcmp(const char *str1, const char *str2);
static uint32_t reverseNum(uint32_t num);
static bool isCommand(USER_DATA* info,
                        const char strCommand[],
                        uint8_t minArguments);
                        
// Initialize Hardware
void initHw()
{
    // Initialize system clock to 40 MHz
    initSystemClockTo40Mhz();

    // Enable clocks
    SYSCTL_RCGCGPIO_R = SYSCTL_RCGCGPIO_R5;
    _delay_cycles(3);

    // Configure LED pins
    GPIO_PORTF_DIR_R |= GREEN_LED_MASK | RED_LED_MASK;  // bits 1 and 3 are outputs
    GPIO_PORTF_DR2R_R |= GREEN_LED_MASK | RED_LED_MASK; // set drive strength to 2mA (not needed since default configuration -- for clarity)
    GPIO_PORTF_DEN_R |= GREEN_LED_MASK | RED_LED_MASK;  // enable LEDs
}

//-----------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------

int main(void)
{
    USER_DATA user_data = {0};
    
	// Initialize hardware
	initHw();
	initUart0();
    initPbs_leds();
    rtosInit();

    // Setup UART0 baud rate
    setUart0BaudRate(115200, 40e6);

	// Display greeting
    putsUart0("\n\r-----Mini Project-------\n\r");
    putsUart0("Press enter an input \n\r");
    
    while(1)
    {
        putsUart0("\n\r> ");
        getsUart0(&user_data);
        parseFields(&user_data);
        //displayUserData(&user_data);
        processCommands(&user_data);
    }

}

void processCommands(USER_DATA *info)
{
    bool valid = false;

    if(isCommand(info,"reboot",0))
        {
            NVIC_APINT_R = NVIC_APINT_VECTKEY | NVIC_APINT_SYSRESETREQ;
            valid = true;
        }

        if(isCommand(info,"ps",0))
        {
            ps();
            valid = true;
        }

        if(isCommand(info,"ipcs",0))
        {
            ipcs();
            valid = true;
        }

        if(isCommand(info,"kill",1))
        {
            uint32_t pid = getFieldInteger(info, 1);
            kill(pid);
            valid = true;
        }

        if(isCommand(info,"Pkill",1))
        {
            char *proc_name = getFieldString(info, 1);
            pkill(proc_name);
            valid = true;
        }

        if(isCommand(info,"preempt",1))
        {
            bool onOff;
            char *onOrOff = getFieldString(info, 1);
            
            if ( (0 == stringcmp(onOrOff,"ON")) || 
                 (0 == stringcmp(onOrOff,"on")) ) {
                onOff = true;
            } else if (
                (0 == stringcmp(onOrOff,"OFF")) || 
                 (0 == stringcmp(onOrOff,"off")) ) {
                onOff = false;
            } else {
                onOff = false;
            }

            preempt(onOff);
            valid = true;
        }

        if(isCommand(info,"sched",1))
        {
            bool rrOrPrio;
            char *schedule = getFieldString(info, 1);
            
            if ( (0 == stringcmp(schedule,"PRIO")) || 
                 (0 == stringcmp(schedule,"prio")) ) {
                rrOrPrio = true;
            } else if (
                (0 == stringcmp(schedule,"RR")) || 
                 (0 == stringcmp(schedule,"rr")) ) {
                rrOrPrio = false;
            } else {
                rrOrPrio = false;
            }

            sched(rrOrPrio);
            valid = true;
        }

        if(isCommand(info,"pidof",1))
        {
            char *proc_name = getFieldString(info, 1);

            pidof(proc_name);
            valid = true;
        }

        if(isCommand(info,"run",1))
        {
            char *proc_name = getFieldString(info, 1);

            run(proc_name);
            valid = true;
        }

        if(isCommand(info,"pbtest",0))
        {
            test_pb_leds();
            valid = true;
        }

        if(isCommand(info,"chkint",0))
        {
            check_interrupts();
            valid = true;
        }

        if(isCommand(info,"memchk",0))
        {
            mem_chk();
            valid = true;
        }

        if(isCommand(info,"mpuchk",0))
        {
            mpuchecks();
            valid = true;
        }

        if(!valid)
        {
            putsUart0("\n\rInvalid Command!\n");
        }

        clearUserData(info,&info->buffIndx);
}

void ps(void)
{
    putsUart0("\n\rcalled ps function\n");
}

void ipcs(void)
{
    putsUart0("\n\rcalled IPCS function\n");
}

void kill(const uint32_t pid)
{
    char str[20] = {0};
    convertIntToString(str, sizeof(str), pid);
    putsUart0("\n\r");
    putsUart0(str);
    putsUart0(" Killed");
}

void pkill(const char *proc_name)
{
    putsUart0("\n\rthread killed in ");
    putsUart0(proc_name);
    
}

void preempt(bool onOff)
{   
    char *onStr = "ON";
    char *offStr = "OFF";
    putsUart0("\n\rpreempt ");
    putsUart0((onOff == true)? onStr : offStr);
}

void sched(bool rrOrPrio)
{   
    char *rrStr = "RR";
    char *prioStr = "PRIO";
    putsUart0("\n\rsched ");
    putsUart0((rrOrPrio == true)? prioStr : rrStr);
}

void pidof(const char *proc_name)
{
    putsUart0("\n\r");
    putsUart0(proc_name);
    putsUart0(" launched\n");
}

void run(const char *proc_name)
{
    putsUart0("\n\r");
    putsUart0(proc_name);
    putsUart0(" ran\n");

    GREEN_LED_BB ^= 1;
}


static void getsUart0(USER_DATA *info)
{
    bool continueloop = true;
    char c = '\0';

    do
    {
        c = getcUart0();
        continueloop = processInput(&c,&info->buffIndx, info);

    }while(continueloop);

}

static bool processInput(char *cptr,uint8_t *countptr, USER_DATA *info)
{
    bool retStatus = true;
    static bool chardetected = false;
    static bool intdetected = false;
    static bool errordetected = false;
    
    if(*countptr >= (MAX_CHARS-1))
    {
        retStatus = false;
        intdetected = false;
        chardetected = false;
        errordetected = false;
        putsUart0("\n\r max 80 characters reached \n\r");
    }
    else if( (*cptr == 0xA) || (*cptr == 0xD) ) /* LF or Carriage return(enter) */
    {
        retStatus = false;
        info->buffer[*countptr] = '\0';
        *countptr += 1;
        intdetected = false;
        chardetected = false;
        errordetected = false;
    }
    else if ((*cptr == 0x8) || (*cptr == 0x7F) ) /* backspace or delete */
    {
        if(*countptr == 0)
        {
             /* skip */
        }
        else
        {
            if (!errordetected)
            {
                *countptr -= 1;
                info->buffer[*countptr] = '\0';
            }
            putcUart0(*cptr);
        }
    }
    else if ( (*cptr == 0x20) || 
                (*cptr == ',')
            ) /* space */
    {
        info->buffer[*countptr] = *cptr;
        *countptr += 1;
        putcUart0(*cptr);
        intdetected = false;
        chardetected = false;
        errordetected = false;
    }
    else if ( (*cptr >= 'a' && *cptr <= 'z') ||
               (*cptr >= 'A' && *cptr <= 'Z')
            )
    {
        if(!intdetected)
        {
            info->buffer[*countptr] = *cptr;
            *countptr += 1;
            chardetected = true;
            errordetected = false;
            putcUart0(*cptr);
        }
        else
        {
            /* display error in form of '*' */
            errordetected = true;
            putcUart0('*');
        }
    }
    else if ( (*cptr >= '0' && *cptr <= '9') ||  
                (*cptr == '-') ||
                (*cptr == '.')
            )
    {
        if(!chardetected)
        {
            info->buffer[*countptr] = *cptr;
            *countptr += 1;
            intdetected = true;
            errordetected = false;
            putcUart0(*cptr);
        }
        else
        {
            /* display error in form of '*' */
            errordetected = true;
            putcUart0('*');
        }
    }
    else
    {
        /* skip the character */
        errordetected = true;
        putcUart0('^');
    }
    
    return retStatus;
}

static void parseFields(USER_DATA *info)
{
   uint8_t count = 0;
   uint8_t position = 0;
   bool charDetected = false;
   bool intDetected = false;
   bool transDetected = false;
   char c;
   
   for(count = 0; count < MAX_CHARS; count++ )
   {
       c = info->buffer[count];

       if( (c >= 'a' && c <= 'z') ||
             (c >= 'A' && c <= 'Z')
         )
       {
           charDetected = true ;
       }
       else if ( (c >= '0' && c <= '9') ||
                   (c == '-') ||
                   (c == '.')
               )
       {
           intDetected = true ;
       }
       else
       {
           info->buffer[count] = '\0';
           intDetected = false;
           charDetected = false;
           transDetected = false;
       }
       
       if((charDetected || intDetected) && (!transDetected) 
           )
       {
           info->fieldPosition[position] = count;
           transDetected = true;
           
           if(intDetected) {
               info->fieldType[position] = 'n';
               ++info->fieldCount;
           } else if (charDetected) {
               info->fieldType[position] = 'a';
               ++info->fieldCount;
           } else {
               /* skip all the others */
           }
           
           intDetected = false;
           charDetected = false;
           ++position;
           position %= MAX_FIELDS;
           
       }
       
   }
}

static char* getFieldString(USER_DATA *info, uint8_t fieldNumber)
{
    uint8_t index = 0;
    
    if(fieldNumber < MAX_FIELDS)
    {
        index = info->fieldPosition[fieldNumber];
        return &info->buffer[index];
    }
    else
    {
        return NULL;
    }
}    

static  int32_t getFieldInteger(USER_DATA *info, uint8_t fieldNumber)
{
    uint8_t index = 0;
    
    if( (fieldNumber < MAX_FIELDS) &&
        (info->fieldType[fieldNumber] == 'n')
    )
    {
        index = info->fieldPosition[fieldNumber];
        return converToInt(&info->buffer[index]);
    }
    else
    {
        return 0;
    }
}

static int32_t converToInt(char *str)
{
    int32_t sum = 0;
    uint8_t n = 0;
    bool negNumber = false;
    char *ptr = str;
    
    if(*ptr == '-')
    {
        negNumber = true;
        ++ptr;
    }

    while(*ptr != '\0')
    {
        n = *ptr - '0';
        sum = sum*10 + n;
        ++ptr;
    }
    
    return (negNumber) ? (sum* -1) : sum;
}

static bool isCommand(USER_DATA* info, 
                        const char strCommand[], 
                        uint8_t minArguments)
{
    if( (0 == stringcmp(&info->buffer[info->fieldPosition[0]],strCommand) ) &&
        (minArguments <= (info->fieldCount - 1))
        )
    {
        return true;
    }
    
    
    return false;
}
                        
static int stringcmp(const char *str1, const char *str2)
{
    const char *ptr1 = str1;
    const char *ptr2 = str2;
    
    int32_t len1; 
    int32_t len2;
    
    len1 = stringlen(str1);
    len2 = stringlen(str2);
    
    if(len1 != len2 )
    {
        return -1;
    }
    
    while( *ptr1 != '\0' && *ptr2 != '\0' )
    {
        if(*ptr1 > *ptr2)
        {
            return 1;
        }
        else if ( *ptr1 < *ptr2)
        {
            return -1;
        }
        else
        {
            /* continue */
        }
        ++ptr1;
        ++ptr2;
    }
    return 0;
}

int stringlen(const char *str)
{
    const char *ptr = str;
    int length = 0;
    
    while(*ptr++ != '\0')
    {
        ++length;
    }
    
    return length;
}

static void displayUserData(USER_DATA *info)
{
    uint8_t count = 0;
    char    temp_str[10] = {0};

    putsUart0("\n\r");
    putsUart0("output buffer: \n\r");

    for(count = 0; count < MAX_CHARS; count++ )
    {
        if(info->buffer[count] != '\0')
        {
            putcUart0(info->buffer[count]);
        }
        else
        {
            putcUart0('*');
        }
    }

    putsUart0("\n\r");
    
    if(0 == convertIntToString(temp_str,10,info->fieldCount))
    {
        putsUart0(temp_str);
    }
    
    putsUart0("\n\r");
    
    for(count = 0; count < MAX_FIELDS; count++ )
    {
        if(0 == convertIntToString(temp_str,10,info->fieldPosition[count]))
        {
            putsUart0(temp_str);
            putcUart0('-');
        }
    }
    
    putsUart0("\n\r");
    
    for(count = 0; count < MAX_FIELDS; count++ )
    {
        if(info->fieldType[count] != '\0')
        {
            putcUart0(info->fieldType[count]);
        }
        else
        {
            putcUart0('*');
        }
    }

    putsUart0("\n\r");
}

int convertIntToString(char *str, uint32_t len, uint32_t num )
{
    uint8_t count = 0;
    uint32_t temp = 0;
    char *ptr = str;
    uint32_t temp_num = num;
    
    if(str != NULL)
    {   
        if (num == 0)
        {
            str[count++] = (num + '0');
            str[count] = '\0';
        }
        else
        {
            //rev_num = reverseNum(num);
            
            while(num && (count < (len-1)) )
            {
                temp = num % 10;
                num /= 10;
                ++ptr;
                count++;
            }
            num = temp_num;
            *ptr-- = '\0';
            
            while(num)
            {
                temp = num % 10;
                *ptr-- = (temp + '0');
                num /= 10;
            }
        }  
        return 0;
    }
    
    return -1;
}

static uint32_t reverseNum(uint32_t num)
{
    uint32_t sum = 0;
    uint32_t temp = 0;
    
    while(num)
    {
        temp = num % 10;
        sum = sum*10 + temp;
        num /= 10;
    }
    return sum;
}

void itohex(char *str, uint32_t val)
{
    uint32_t temp = 0;
    uint8_t i = 0;
    uint8_t j = 0;
    uint32_t num = val;

    while(num != 0) {
        temp = num%16;

        if(temp > 10) {
            temp += '7';
        } else {
            temp += '0';
        }
        str[i++] = temp;
        num /= 16;
    }
    
    uint8_t len = stringlen(str);

    if(len>0) {
        for (i = 0, j = len - 1; i < j; i++, j--) {
            char temp = str[i];
            str[i] = str[j];
            str[j] = temp;
        }
    }
}

void zeroString(char *str,int len)
{
    uint8_t i = 0;

    for(i = 0; i < len; i++) {
        str[i] = 0;
    }
}

static void clearUserData(USER_DATA *info, uint8_t *bufferIndx)
{
    uint8_t count = 0;
    
    for(count = 0; count < MAX_CHARS; count++ )
    {
       info->buffer[count] = '\0';
    }
    
    info->fieldCount = 0;
    
    for(count = 0; count < MAX_FIELDS; count++ )
    {
        info->fieldPosition[count] = 0;
        info->fieldType[count] = '\0';
    }
    *bufferIndx = 0;
}
