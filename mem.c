/*
 * mem.c
 *
 *  Created on: Sep 14, 2023
 *      Author: sadhanpawar
 */

#include "mem.h"
#include "uart0.h"
#include "commandline.h"
#include "tm4c123gh6pm.h"
#include "rtos.h"

malloc_heap_t mem_CB = {0};

void mem_init(void)
{
    mem_CB.top_ptr = (uint8_t*) 0x20008000;
    mem_CB.low_ptr = (uint8_t*) 0x20001000; //+ 516; //stack down approach
    mem_CB.memUpdater.lowIdx = sizeof(block_t)-1;
}

void *malloc_from_heap (int size_in_bytes)
{
    uint8_t* ptr;
    int size = 0;

    size = getBlockSize(size_in_bytes);

    if(memErrorCheck(size)) {
        return NULL;
    }

    //allocate memory
    if(size == 1536) {
        if ( (mem_CB.memUpdater.alloc_bigblock.block_1536[0] != 1) && 
            !((mem_CB.memUpdater.alloc.whole_block[BLOCK_1536_1K_1] == 1) || (mem_CB.memUpdater.alloc.whole_block[BLOCK_1536_512_1] == 1 ))
          )  {
            ptr = (uint8_t*)((uint32_t)0x20008000 - (uint32_t)(0x400*7));
            mem_CB.memUpdater.alloc_bigblock.block_1536[0] = 1;
            mem_CB.memUpdater.alloc.block_1K_1[7] = 1;
            mem_CB.memUpdater.alloc.block_512_1[0] = 1;
            mem_CB.memUpdater.alloc_bigblock.idx += 1;
        } else if ( (mem_CB.memUpdater.alloc_bigblock.block_1536[1] != 1) && 
            !((mem_CB.memUpdater.alloc.whole_block[BLOCK_1536_1K_2] == 1) || (mem_CB.memUpdater.alloc.whole_block[BLOCK_1536_512_2] == 1 ))
          )  {
            ptr = (uint8_t*)((uint32_t)0x20005000 - (uint32_t)(0x200*7));
            mem_CB.memUpdater.alloc_bigblock.block_1536[1] = 1;
            mem_CB.memUpdater.alloc.block_1K_2[0] = 1;
            mem_CB.memUpdater.alloc.block_512_1[15] = 1;
            mem_CB.memUpdater.alloc_bigblock.idx += 1;
        } else if ( (mem_CB.memUpdater.alloc_bigblock.block_1536[2] != 1) && 
            !((mem_CB.memUpdater.alloc.whole_block[BLOCK_1536_1K_3] == 1) || (mem_CB.memUpdater.alloc.whole_block[BLOCK_1536_512_3] == 1 ))
          )  {
            ptr = (uint8_t*)((uint32_t)0x20004000 - (uint32_t)(0x400*7));
            mem_CB.memUpdater.alloc_bigblock.block_1536[2] = 1;
            mem_CB.memUpdater.alloc.block_1K_2[7] = 1;
            mem_CB.memUpdater.alloc.block_512_2[0] = 1;
            mem_CB.memUpdater.alloc_bigblock.idx += 1;
        } else {
            putsUart0("\n\r1536 blocks are completely full\n\r");
            return NULL;
        }
    }
    else if( size > 512 ) {
        
        int temp_size = size;

        while(temp_size > 0) {
            if(!mem_CB.memUpdater.alloc.whole_block[mem_CB.memUpdater.highIdx]) {
                //ptr = mem_CB.top_ptr;
                //ptr = mem_CB.top_ptr =  ((uint32_t)mem_CB.top_ptr - (uint32_t)1024);
                mem_CB.memUpdater.alloc.whole_block[mem_CB.memUpdater.highIdx] = 1;
                if(mem_CB.memUpdater.highIdx < 7) {
                    ptr = mem_CB.top_ptr =  ((uint32_t)mem_CB.top_ptr - (uint32_t)1024);
                    mem_CB.memUpdater.highIdx++;
                } else if (mem_CB.memUpdater.highIdx == 7) {
                    ptr = mem_CB.top_ptr =  ((uint32_t)mem_CB.top_ptr - (uint32_t)1024*9); //skip the block
                    mem_CB.memUpdater.highIdx = mem_CB.memUpdater.highIdx + 16 + 1;
                } else {
                    ptr = mem_CB.top_ptr =  ((uint32_t)mem_CB.top_ptr - (uint32_t)1024);
                    mem_CB.memUpdater.highIdx++;
                }
                temp_size -= 1024;
            }
            else {
                putsUart0("\n\rNo memory to allocate\n\r");
                return NULL;
            }
            
        }
    }
    else if(size <= 512) {

        if(!mem_CB.memUpdater.alloc.whole_block[mem_CB.memUpdater.lowIdx]) {
                //ptr = mem_CB.low_ptr;
                mem_CB.memUpdater.alloc.whole_block[mem_CB.memUpdater.lowIdx] = 1;
                if(mem_CB.memUpdater.lowIdx  > 32 ) {
                    mem_CB.memUpdater.lowIdx--;
                    ptr = mem_CB.low_ptr;
                    mem_CB.low_ptr = ((uint32_t)mem_CB.low_ptr + (uint32_t)512);
                } else if (mem_CB.memUpdater.lowIdx == 32) { //  block change
                    mem_CB.memUpdater.lowIdx = mem_CB.memUpdater.lowIdx - 8 - 1;
                    ptr = mem_CB.low_ptr = ((uint32_t)mem_CB.low_ptr + (uint32_t)1024*8 + (uint32_t)512 ); // skip 8k block
                }else {
                    //mem_CB.memUpdater.lowIdx = mem_CB.memUpdater.lowIdx - 8 - 1;
                    mem_CB.memUpdater.lowIdx--;
                    ptr = mem_CB.low_ptr;
                    mem_CB.low_ptr = ((uint32_t)mem_CB.low_ptr + (uint32_t)512);
                }
            } else {
                putsUart0("\n\rNo memory to allocate\n\r");
                return NULL;
            }
            
    } else {
        return NULL;
    }

    return ptr;
}

int getBlockSize(int size_in_bytes) 
{
    int size = 512; //default size

    while(size_in_bytes > 0 ) {
        size_in_bytes -= 512;
        
        if(size_in_bytes > 0) {
            size += 512;
        }
    }

    return size;
}


bool memErrorCheck(int size)
{
    uint8_t i;
    bool status = false;

    /* check error conditions */
    //if(mem_CB.low_ptr >= mem_CB.top_ptr) {
    //    putsUart0("\n\rFull memory limit exceeded\n\r");
    //    return true;
    //}

    for(i = 0; i < 16; i++) {
        if(mem_CB.memUpdater.alloc.block_512_1[i] == 0) {
            status = true;
            break;
        }
    }

    if(status == false) {
        for(i = 0; i < 8; i++) {
            if(mem_CB.memUpdater.alloc.block_512_2[i] == 0) {
                status = true;
                break;
            }
        }
    }

    if((status == false) && (size <= 512)) {
        putsUart0("\n\r512 memory exceeded\n\r");
        return true;
    }

    status = false;

    for(i = 0; i < 8; i++) {
        if(mem_CB.memUpdater.alloc.block_1K_1[i] == 0) {
            status = true;
            break;
        }
    }

    if(status == false) {
        for(i = 0; i < 8; i++) {
            if(mem_CB.memUpdater.alloc.block_1K_2[i] == 0) {
                status = true;
                break;
            }
        }
    }

    if((status == false) && (size > 512)) {
        putsUart0("\n\r1024 memory exceeded\n\r");
        return true;
    }

    return false;
}


void free(void *memptr)
{
    uint8_t region = 0;
    uint32_t ptr = ((uint32_t)memptr & 0xFFFFFF00); //clearing last nibble

    if(ptr == NULL) {
        putsUart0("\n\rCannot Free, Invalid Pointer\n\r");
        return ;
    }

    //1536 blocks
    if (mem_CB.memUpdater.alloc_bigblock.idx > 0) {

        if ( (ptr >= (uint32_t*)0x20006000) && (ptr < (uint32_t*)0x20008000) //8k

        ) {
            mem_CB.memUpdater.alloc.block_1K_1[7] = 0;
            mem_CB.memUpdater.alloc.block_512_1[0] = 0;
        } else if ( (ptr >= (uint32_t*)0x20004000) && (ptr < (uint32_t*)0x20005000) // 3 4k region

        ) {
            mem_CB.memUpdater.alloc.block_1K_2[0] = 0;
            mem_CB.memUpdater.alloc.block_512_1[15] = 0;
        } else if ( (ptr >= (uint32_t*)0x20002000) && (ptr < (uint32_t*)0x20004000) // 3 4k region

        ){
            mem_CB.memUpdater.alloc.block_1K_2[7] = 0;
            mem_CB.memUpdater.alloc.block_512_2[0] = 0;
        } else {
            putsUart0("\n\r1536 cannot free\n\r");
        }

        #if 0
        switch (mem_CB.memUpdater.alloc_bigblock.idx)
        {
            case 1: //0
            {
                mem_CB.memUpdater.alloc.block_1K_1[7] = 0;
                mem_CB.memUpdater.alloc.block_512_1[0] = 0;
            }break;
            case 2: //1
            {
                mem_CB.memUpdater.alloc.block_1K_2[0] = 0;
                mem_CB.memUpdater.alloc.block_512_1[15] = 0;

            }break;
            case 3: //2
            {
                mem_CB.memUpdater.alloc.block_1K_2[7] = 0;
                mem_CB.memUpdater.alloc.block_512_2[0] = 0;

            }break;
            default: putsUart0("\n\r1536 cannot free\n\r");
                break;
        }
        #endif

    } else if( (ptr >= (uint32_t*)0x20001000) && (ptr < (uint32_t*)0x20002000) // 1 4K regions
        ) {
            region = (((uint32_t)ptr - (uint32_t)0x20001000) / 512);
            mem_CB.memUpdater.alloc.block_512_2[(7 - region)%8] = 0;

        } else if ( (ptr >= (uint32_t*)0x20004000) && (ptr < (uint32_t*)0x20005000) // 2 4k region

        ) {
            region = (((uint32_t)ptr - (uint32_t)0x20004000) / 512);
            mem_CB.memUpdater.alloc.block_512_1[(15 - region)%16] = 0;

        } else if ( (ptr >= (uint32_t*)0x20005000) && (ptr < (uint32_t*)0x20006000) // 3 4k region

        ) {
            region = (((uint32_t)ptr - (uint32_t)0x20005000) / 512);
            mem_CB.memUpdater.alloc.block_512_1[(15 - region)%16] = 0;

        } else if( (ptr >= (uint32_t*)0x20002000) && (ptr < (uint32_t*)0x20004000)  // 8k
        ) {
            region = ((((uint32_t)ptr - (uint32_t)0x20002000) / 1024));
            region = (7 - region)%8;
            mem_CB.memUpdater.alloc.block_1K_2[region] = 0;
        } else if ( (ptr >= (uint32_t*)0x20006000) && (ptr < (uint32_t*)0x20008000) //8k

        ) {
            region = (((uint32_t)ptr - (uint32_t)0x20006000) / 1024);
            region = (7 - region)%8;
            mem_CB.memUpdater.alloc.block_1K_1[region] = 0;
        }
        else {
            putsUart0("\n\rCannot Free, Invalid Pointer\n\r");
        }
}

void mpuOn(void)
{
    NVIC_MPU_CTRL_R |= NVIC_MPU_CTRL_ENABLE;
    __asm(" dsb");
    __asm(" isb");
    
}

void mpuOff(void)
{
    NVIC_MPU_CTRL_R &= ~NVIC_MPU_CTRL_ENABLE;
    __asm(" dsb");
    __asm(" isb");
}

void mem_chk(void)
{
    uint8_t *ptr;
    uint8_t val = 0;
    char temp_str[50]={0};

    ptr = malloc_from_heap(512);
    ptr = malloc_from_heap(512);
    ptr = malloc_from_heap(512);
    ptr = malloc_from_heap(512);
    //ptr = malloc_from_heap(512);
    //ptr = malloc_from_heap(512);
    free(ptr);

    //ptr = malloc_from_heap(512);
    //ptr = malloc_from_heap(512);
    //ptr = malloc_from_heap(512);
    //ptr = malloc_from_heap(512);
    //ptr = malloc_from_heap(512);
    //ptr = malloc_from_heap(512);
    //ptr = malloc_from_heap(512);
    //ptr = malloc_from_heap(512);
    //free(ptr);
    //ptr = malloc_from_heap(512);
    //ptr = malloc_from_heap(512);
    //ptr = malloc_from_heap(512);
    //ptr = malloc_from_heap(512);
    //ptr = malloc_from_heap(512);
    //ptr = malloc_from_heap(512);
    //ptr = malloc_from_heap(512);
    //ptr = malloc_from_heap(512);
    //ptr = malloc_from_heap(512);
    //ptr = malloc_from_heap(512);
    //ptr = malloc_from_heap(512);
    //ptr = malloc_from_heap(512);
    //ptr = malloc_from_heap(512);

    //if( NULL == ptr) {
    //    putsUart0("\n\rNo malloc memory allocated\nr\r");
    //    return;
    //}

    //*ptr = 1;
    //val = *ptr;
    //putsUart0("\n\rvalues written to heap\n\r");
    //itohex(temp_str,val);
    //putsUart0(temp_str);
    //putsUart0("\n\r");

    ptr = malloc_from_heap(1024);
    ptr = malloc_from_heap(1024);
    ptr = malloc_from_heap(1024);
    ptr = malloc_from_heap(1024);
    free(ptr);
    //ptr = malloc_from_heap(1024);
    //ptr = malloc_from_heap(1024);
    //ptr = malloc_from_heap(1024);
    //ptr = malloc_from_heap(1024);
    //ptr = malloc_from_heap(1024);
    //ptr = malloc_from_heap(1024);
    //ptr = malloc_from_heap(1024);
    //ptr = malloc_from_heap(1024);
    //ptr = malloc_from_heap(1024);
    //ptr = malloc_from_heap(1024);
    //free(ptr);
    //ptr = malloc_from_heap(1024);
    //ptr = malloc_from_heap(1024);
    //ptr = malloc_from_heap(1024);
    //ptr = malloc_from_heap(1024);
    //ptr = malloc_from_heap(1024);
    //ptr = malloc_from_heap(1024);
    //ptr = malloc_from_heap(1024);

    //if( NULL == ptr) {
    //    putsUart0("\n\rNo malloc memory allocated\n\r");
    //    return;
    //}

    //*ptr = 1;
    //val = *ptr;
    //putsUart0("\n\rvalues written to heap\n\r");
    //itohex(temp_str,val);
    //putsUart0(temp_str);
    //putsUart0("\n\r");

    ptr = malloc_from_heap(1536);
    ptr = malloc_from_heap(1536);
    free(ptr);
    ptr = malloc_from_heap(1536);
    //ptr = malloc_from_heap(1536);
    free(ptr);
    if( NULL == ptr) {
        putsUart0("\n\rNo malloc memory allocated\n\r");
        return;
    }

    *ptr = 1;
    val = *ptr;
    putsUart0("\n\rvalues written to heap\n\r");
    itohex(temp_str,val);
    putsUart0(temp_str);
    putsUart0("\n\r");

    ptr = malloc_from_heap(3);
    if( NULL == ptr) {
        putsUart0("\n\rNo malloc memory allocated\n\r");
        return;
    }

    *ptr = 1;
    val = *ptr;
    putsUart0("\n\rvalues written to heap\n\r");
    itohex(temp_str,val);
    putsUart0(temp_str);
    putsUart0("\n\r");

    ptr = malloc_from_heap(660);
    if( NULL == ptr) {
        putsUart0("\n\rNo malloc memory allocated\n\r");
        return;
    }

    *ptr = 1;
    val = *ptr;
    putsUart0("\n\rvalues written to heap\n\r");
    itohex(temp_str,val);
    putsUart0(temp_str);
    putsUart0("\n\r");

    ptr = malloc_from_heap(1100);
    if( NULL == ptr) {
        putsUart0("\n\rNo malloc memory allocated\n\r");
        return;
    }

    *ptr = 1;
    val = *ptr;
    putsUart0("\n\rvalues written to heap\n\r");
    itohex(temp_str,val);
    putsUart0(temp_str);
    putsUart0("\n\r");


}

void allowFlashAccess(void)
{
    // Region 0, for Flash +x, +rw, +rw (X, Kernel, Usr)
    NVIC_MPU_NUMBER_R = 0;
    NVIC_MPU_BASE_R = (0 |  1 << 4 | 0);
    NVIC_MPU_ATTR_R = (0 << 28 | 3 << 24 | 0 << 19 | 0 << 18 | 1 << 17 | 0 << 16 | /*0xFF << 8 |*/ 17 << 1 | 1);
    __asm(" dsb");
    __asm(" isb");
}

void allowPeripheralAccess(void)
{
    //  Region 1, -x, +rw, +rw
    NVIC_MPU_NUMBER_R = 1;
    NVIC_MPU_BASE_R = (0x40000000 | 1 << 4 | 1);
    NVIC_MPU_ATTR_R = (3 << 24 |  28 << 1 | 1);
    __asm(" isb");
    __asm(" dsb");
}

void setupSramAccess(void)
{

    // Region 2, for SRAM Os (bottom 4K), -x, +rw, -rw
    NVIC_MPU_NUMBER_R = 2;
    NVIC_MPU_BASE_R = (0x20000000 |  1 << 4 | 2);
    NVIC_MPU_ATTR_R = (1 << 28 | 1 << 24 | 1 << 18 | 1 << 17 | 0 << 16 | 0xB << 1 | 1);
    __asm(" dsb");
    __asm(" isb");

    // Region 3, for SRAM, -x, +rw, -rw 4K
    NVIC_MPU_NUMBER_R = 3;
    NVIC_MPU_BASE_R = (0x20001000  |  1 << 4 | 3);
    NVIC_MPU_ATTR_R = (1 << 28 |1 << 24 | 1 << 18 | 1 << 17 | 0 << 16 |  0xFF << 8 | 0xB << 1 | 1);
    __asm(" dsb");
    __asm(" isb");


    // Region 4, for SRAM, -x, +rw, -rw 8K
    NVIC_MPU_NUMBER_R = 4;
    NVIC_MPU_BASE_R = (0x20002000 | 1 << 4 | 4);
    NVIC_MPU_ATTR_R = (1 << 28 |1 << 24 | 1 << 18 | 1 << 17 | 0 << 16 |  0xFF << 8 | 0xC << 1 | 1);
    __asm(" dsb");
    __asm(" isb");

    // Region 5, for SRAM, -x, +rw, -rw 4K
    NVIC_MPU_NUMBER_R = 5;
    NVIC_MPU_BASE_R = (0x20004000 | 1 << 4 | 5);
    NVIC_MPU_ATTR_R = (1 << 28 |1 << 24 | 1 << 18 | 1 << 17 | 0 << 16 |  0xFF << 8 | 0xB << 1 | 1);
    __asm(" dsb");
    __asm(" isb");

    // Region 6, for SRAM, -x, +rw, -rw 4K
    NVIC_MPU_NUMBER_R = 6;
    NVIC_MPU_BASE_R = (0x20005000 | 1 << 4 | 6);
    NVIC_MPU_ATTR_R = (1 << 28 |1 << 24 | 1 << 18 | 1 << 17 | 0 << 16 |  0xFF << 8 | 0xB << 1 | 1);
    __asm(" dsb");
    __asm(" isb");

    // Region 7, for SRAM, -x, +rw, -rw 8K
    NVIC_MPU_NUMBER_R = 7;
    NVIC_MPU_BASE_R = (0x20006000 | 1 << 4 | 7);
    NVIC_MPU_ATTR_R = (1 << 28 |1 << 24 | 1 << 18 | 1 << 17 | 0 << 16 |  0xFF << 8 | 0xC << 1 | 1);
    __asm(" dsb");
    __asm(" isb");

}

void setSramAccessWindow(uint32_t *baseAdd, uint32_t size_in_bytes)
{
    uint32_t addr = (uint32_t)baseAdd;
    uint8_t srd = 0;
    uint8_t mpuNum = 0;
    uint8_t logSize = 0;
    uint32_t convBaseAddr = 0;

    if( size_in_bytes != 1536) {
        srd = getSrdRegion(baseAdd,size_in_bytes, &mpuNum, &logSize, &convBaseAddr);

        // ap: +rw for K, usr
        // srd bit enable
        NVIC_MPU_NUMBER_R = mpuNum;
        NVIC_MPU_BASE_R = (convBaseAddr | 1 << 4 | mpuNum);
        NVIC_MPU_ATTR_R = (1 << 28 | 3 << 24  | 1 << 18 | 1 << 17 | 0 << 16 | srd << 8 | logSize << 1 | 1);
        __asm(" dsb");
        __asm(" isb");
    } else {
        // special handling for 1536 blocks
        (void)getSrdRegion(baseAdd,size_in_bytes, &mpuNum, &logSize, &convBaseAddr);

    }
}

uint8_t getSrdRegion(uint32_t *addr, uint32_t size, uint8_t *mpuNum, uint8_t *logSize, uint32_t *convBaseAddr)
{
    uint8_t region = 0;
    uint8_t srdfield = 0xFF;

    if(size == 512) {
            *logSize = 0xB;

            if( (addr >= (uint32_t*)0x20001000) && (addr < (uint32_t*)0x20002000) // 1 4K regions
            ) {
                region = (((uint32_t)addr - (uint32_t)0x20001000) / 512);
                *mpuNum = 3;
                *convBaseAddr = 0x20001000;
            } else if ( (addr >= (uint32_t*)0x20004000) && (addr < (uint32_t*)0x20005000) // 2 4k region

            ) {
                region = (((uint32_t)addr - (uint32_t)0x20004000) / 512);
                *mpuNum = 5;
                *convBaseAddr = 0x20004000;
            } else if ( (addr >= (uint32_t*)0x20005000) && (addr < (uint32_t*)0x20006000) // 3 4k region

            ) {
                region = (((uint32_t)addr - (uint32_t)0x20005000) / 512);
                *mpuNum = 6;
                *convBaseAddr = 0x20005000;
            }
            else {
                putsUart0("\n\rError in SRD \n\r");
                return 0xFF;
            }

    } else if (size == 1024) {

            *logSize = 0xC;

            if( (addr >= (uint32_t*)0x20002000) && (addr < (uint32_t*)0x20004000) 
            ) {
                region = (((uint32_t)addr - (uint32_t)0x20002000) / 1024);
                *mpuNum = 4;
                *convBaseAddr = 0x20002000;
            } else if ( (addr >= (uint32_t*)0x20006000) && (addr < (uint32_t*)0x20008000) 

            ) {
                region = (((uint32_t)addr - (uint32_t)0x20006000) / 1024);
                *mpuNum = 7;
                *convBaseAddr = 0x20006000;
            }
            else {
                putsUart0("\n\rError in SRD \n\r");
                return 0xFF;
            }

    } else if (size == 1536) {
        //putsUart0("\n\rDidn't handle SRD bits for this memory region\n\r");
        switch (mem_CB.memUpdater.alloc_bigblock.idx)
        {
            case 1: //0
            {
                NVIC_MPU_NUMBER_R = 7;
                NVIC_MPU_BASE_R = (0x20006000 | 1 << 4 | 7);
                NVIC_MPU_ATTR_R = (1 << 28 | 3 << 24  | 1 << 18 | 1 << 17 | 0 << 16 | 0xFE << 8 | 0xC << 1 | 1);
                __asm(" dsb");
                __asm(" isb");


                NVIC_MPU_NUMBER_R = 6;
                NVIC_MPU_BASE_R = (0x20005000 | 1 << 4 | 6);
                NVIC_MPU_ATTR_R = (1 << 28 | 3 << 24  | 1 << 18 | 1 << 17 | 0 << 16 | 0x7F << 8 | 0xB << 1 | 1);
                __asm(" dsb");
                __asm(" isb");

            }break;
            case 2: //1
            {
                NVIC_MPU_NUMBER_R = 5;
                NVIC_MPU_BASE_R = (0x20004000 | 1 << 4 | 5);
                NVIC_MPU_ATTR_R = (1 << 28 | 3 << 24  | 1 << 18 | 1 << 17 | 0 << 16 | 0xFE << 8 | 0xC << 1 | 1);
                __asm(" dsb");
                __asm(" isb");


                NVIC_MPU_NUMBER_R = 4;
                NVIC_MPU_BASE_R = (0x20002000 | 1 << 4 | 6);
                NVIC_MPU_ATTR_R = (1 << 28 | 3 << 24  | 1 << 18 | 1 << 17 | 0 << 16 | 0x7F << 8 | 0xB << 1 | 1);
                __asm(" dsb");
                __asm(" isb");

            }break;
            case 3: //2
            {
                NVIC_MPU_NUMBER_R = 4;
                NVIC_MPU_BASE_R = (0x20002000 | 1 << 4 | 4);
                NVIC_MPU_ATTR_R = (1 << 28 | 3 << 24  | 1 << 18 | 1 << 17 | 0 << 16 | 0xFE << 8 | 0xC << 1 | 1);
                __asm(" dsb");
                __asm(" isb");


                NVIC_MPU_NUMBER_R = 3;
                NVIC_MPU_BASE_R = (0x20001000 | 1 << 4 | 3);
                NVIC_MPU_ATTR_R = (1 << 28 | 3 << 24  | 1 << 18 | 1 << 17 | 0 << 16 | 0x7F << 8 | 0xB << 1 | 1);
                __asm(" dsb");
                __asm(" isb");

            }break;
            default: putsUart0("\n\r1536 no SRD bits set\n\r");
                break;
        }

    } else {
        putsUart0("\n\rDidn't handle SRD bits for this memory region\n\r");
        return 0xFF;
    }

    srdfield &= ~ (1 << region);
    return srdfield;

}

void mpuchecks(void)
{
    uint8_t pb_vals = 0;

    pb_vals = getPbs();

    switch (pb_vals)
    {
        case 1:
        {   
            putsUart0("\n\rChecking Flash Access\n\r");
            allowFlashAccess();
            //allowPeripheralAccess();
            //setupSramAccess();
            //uint32_t* ptr = (uint32_t*)malloc_from_heap(1024);

            //setSramAccessWindow(ptr,1024);

            //privileged mode
            mpuOn();
            putsUart0("\n\rIn privileged mode: status 'OK' \n\r");

            //unprivileged mode
            //setPsp(SET_STACK_ADDR(ptr,1024));
            setPsp(0x20008000);
            setAspPsp();
            __asm(" dsb");
            __asm(" isb");
            setTmplUnP();
            
            random_fun();

            setPendSv();

            // back to privileged mode not working, can't access control reg in unprivileged mode
            //setAspMsp();
            //__asm(" isb" );
            //setTmplP();
            //__asm(" isb" );
            //putsUart0("\n\rIn privileged mode: status 'OK' \n\r");


        }break;
        
        case 2:
        {
            putsUart0("\n\rChecking Peripheral Access\n\r");
            allowFlashAccess();
            allowPeripheralAccess();
            setupSramAccess();
            uint32_t* ptr = (uint32_t*)malloc_from_heap(1024);

            setSramAccessWindow(ptr,1024);

            //privileged mode
            mpuOn();
            RED_LED_BB = 1;
            putsUart0("\n\rIn privileged mode: status 'OK' \n\r");

            //unprivileged mode
            setPsp(SET_STACK_ADDR(ptr,1024));
            setAspPsp();
            __asm(" isb" );
            setTmplUnP();
            
            random_fun();
            random_fun();

            setPendSv();
            /*
            // back to privileged mode not working, can't access control reg in unprivileged mode
            setAspMsp();
            __asm(" isb" );
            setTmplP();
            __asm(" isb" );
            putsUart0("\n\rIn privileged mode: status 'OK' \n\r");
            */


        }break;

        case 4:
        {
            uint32_t* ptr;

            putsUart0("\n\rChecking SRAM Access\n\r");
            allowFlashAccess();
            allowPeripheralAccess();
            setupSramAccess();

            ptr = (uint32_t*)malloc_from_heap(512);

            setSramAccessWindow(ptr,512);

            //privileged mode
            mpuOn();
            putsUart0("\n\rIn privileged mode: status 'OK' \n\r");
            int a = 1;
            int b = 2;
            int c = a+b;

            uint8_t *osptr = (uint8_t*)0x20000001;
            uint8_t val = *osptr;

            //unprivileged mode
            setPsp(SET_STACK_ADDR(ptr,512));
            setAspPsp();
            __asm(" isb" );
            setTmplUnP();
            __asm(" isb" );

            //*osptr = (uint8_t*)0x20002001;
            //*osptr = 0xAA;
            
            random_fun();
            random_fun();

            setPendSv();

        }break;

        case 8:
        {
            uint32_t* ptr;

            putsUart0("\n\rChecking SRAM 1536 Access\n\r");
            allowFlashAccess();
            allowPeripheralAccess();
            setupSramAccess();

            ptr = (uint32_t*)malloc_from_heap(1536);

            setSramAccessWindow(ptr,1536);

            //privileged mode
            mpuOn();
            putsUart0("\n\rIn privileged mode: status 'OK' \n\r");
            int a = 1;
            int b = 2;
            int c = a+b;

            uint8_t *osptr = (uint8_t*)0x20000001;
            uint8_t val = *osptr;

            //unprivileged mode
            setPsp(SET_STACK_ADDR(ptr,1024));
            setAspPsp();
            __asm(" isb" );
            setTmplUnP();
            __asm(" isb" );

            //*osptr = (uint8_t*)0x20002001;
            //*osptr = 0xAA;
            
            random1536_fun();

            setPendSv();

        }break;

        default:
            break;
    }
}

void random_fun(void)
{
    int a = 0xAA;
    int b = 0xCC;
    int sum = a + b;

    GREEN_LED_BB = 1;

    putsUart0("\n\rchecking unprivileged access 'OK' \n\r");

    uint8_t *osptr_1 = (uint8_t*)0x20002220;
    uint8_t val_1 = *osptr_1;
    *osptr_1 = 0xCC;

    osptr_1 = (uint8_t*)0x20000001;
    //*osptr_1 = 0xAA;
}

void random1536_fun(void)
{
    int a = 0xAA;
    int b = 0xCC;
    int sum = a + b;

    putsUart0("\n\rchecking unprivileged access 'OK' \n\r");

    uint8_t *osptr_1 = (uint8_t*)0x20005220; // unacceptable
    uint8_t val_1 = *osptr_1;
    *osptr_1 = 0xCC;

    *osptr_1 = (uint8_t*)0x20001220; // unacceptable
    val_1 = *osptr_1;
    *osptr_1 = 0xCC;

    osptr_1 = (uint8_t*)0x20000001; //kernel
    //*osptr_1 = 0xAA;

    GREEN_LED_BB = 1;


    
}
