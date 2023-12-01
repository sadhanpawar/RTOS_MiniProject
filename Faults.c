/*
 * Faults.c
 *
 *  Created on: Sep 14, 2023
 *      Author: sadhanpawar
 */

#include <stdint.h>
#include "Faults.h"
#include "uart0.h"
#include "tm4c123gh6pm.h"
#include "rtos.h"
#include "commandline.h"


void MpuFaultIntHandler(void)
{
    uint32_t *psp=0;
    uint32_t *msp = 0;
    char temp_str[50] = {0};
    uint32_t mfaultflags = 0;
    uint32_t hfaultflags = 0;
    uint32_t *ptr;

    putsUart0("\n\rMPU fault occurred \n\r");


    hfaultflags = NVIC_HFAULT_STAT_R;

    mfaultflags = (uint8_t)(hfaultflags & 0xFF);
    putsUart0("\n\rMem Fault Flags: ");
    itohex(temp_str,mfaultflags);
    putsUart0(temp_str);
    putsUart0("\n\r");
    zeroString(temp_str,sizeof(temp_str));

    psp = getPsp();
    itohex(temp_str,(uint32_t)psp);
    putsUart0("psp: ");
    putsUart0(temp_str);
    putsUart0("\n\r");
    zeroString(temp_str,sizeof(temp_str));

    msp = getMsp();
    itohex(temp_str,(uint32_t)msp);
    putsUart0("msp: ");
    putsUart0(temp_str);
    putsUart0("\n\r");
    zeroString(temp_str,sizeof(temp_str));

    if(hfaultflags & NVIC_FAULT_STAT_MMARV) {
        itohex(temp_str,NVIC_MM_ADDR_R);
        putsUart0("mem addr: ");
        putsUart0(temp_str);
        putsUart0("\n\r");
        zeroString(temp_str,sizeof(temp_str));
    }

    putsUart0("\n\r----Stack Dump----\n\r");
    ptr = psp;
    itohex(temp_str,*ptr);
    putsUart0("R0: ");
    putsUart0(temp_str);
    putsUart0("\n\r");
    ++ptr;
    zeroString(temp_str,sizeof(temp_str));

    itohex(temp_str,*ptr);
    putsUart0("R1: ");
    putsUart0(temp_str);
    putsUart0("\n\r");
    ++ptr;
    zeroString(temp_str,sizeof(temp_str));

    itohex(temp_str,*ptr);
    putsUart0("R2: ");
    putsUart0(temp_str);
    putsUart0("\n\r");
    ++ptr;
    zeroString(temp_str,sizeof(temp_str));

    itohex(temp_str,*ptr);
    putsUart0("R3: ");
    putsUart0(temp_str);
    putsUart0("\n\r");
    ++ptr;
    zeroString(temp_str,sizeof(temp_str));

    itohex(temp_str,*ptr);
    putsUart0("R12: ");
    putsUart0(temp_str);
    putsUart0("\n\r");
    ++ptr;
    zeroString(temp_str,sizeof(temp_str));

    itohex(temp_str,*ptr);
    putsUart0("LR: ");
    putsUart0(temp_str);
    putsUart0("\n\r");
    ++ptr;
    zeroString(temp_str,sizeof(temp_str));

    itohex(temp_str,*ptr);
    putsUart0("PC: ");
    putsUart0(temp_str);
    putsUart0("\n\r");
    ++ptr;
    zeroString(temp_str,sizeof(temp_str));
    
    itohex(temp_str,*ptr);
    putsUart0("xPSR: ");
    putsUart0(temp_str);
    putsUart0("\n\r");
    ++ptr;
    zeroString(temp_str,sizeof(temp_str));

    //clear mem fault bit
    NVIC_SYS_HND_CTRL_R &= ~NVIC_SYS_HND_CTRL_MEMP;

    putsUart0("\n\rPendsv bit is set\n\r");
    NVIC_INT_CTRL_R = (1 << 28);
}

void BusFaultIntHandler(void)
{
    putsUart0("\n\rBus fault occurred\n\r");
    while(1);
    
}

void UsageFaultIntHandler(void)
{
    putsUart0("\n\rUsage fault occurred \n\r");
    while(1);
}

void PendSvIntHandler(void)
{
    uint32_t faultreg = NVIC_FAULT_STAT_R;
    putsUart0("\n\rPend Sv ISR occurred \n\r");

    if((faultreg & NVIC_FAULT_STAT_DERR) || 
        (faultreg & NVIC_FAULT_STAT_IERR)
    )  {
        NVIC_FAULT_STAT_R &= ~NVIC_FAULT_STAT_DERR;
        NVIC_FAULT_STAT_R &= ~NVIC_FAULT_STAT_IERR;
        putsUart0("\n\rMPU called \n\r");
    } 

    while(1);
}
