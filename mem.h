/*
 * mem.h
 *
 *  Created on: Sep 14, 2023
 *      Author: sadhanpawar
 */

#ifndef MEM_H_
#define MEM_H_

#include <stdint.h>
#include <stdbool.h>

#define HEAP_REGION_512_1       (0x2)
#define HEAP_REGION_512_2       (0)
#define HEAP_REGION_512_3       (0)
#define HEAP_REGION_512_4       (0)
#define HEAP_REGION_512_5       (0)
#define HEAP_REGION_512_6       (0)
#define HEAP_REGION_512_7       (0)
#define HEAP_REGION_512_8       (0)

#define HEAP_REGION_1024_1      (0)
#define HEAP_REGION_1024_2      (0)
#define HEAP_REGION_1024_3      (0)
#define HEAP_REGION_1024_4      (0)
#define HEAP_REGION_1024_5      (0)
#define HEAP_REGION_1024_6      (0)
#define HEAP_REGION_1024_7      (0)
#define HEAP_REGION_1024_8      (0)

#define NULL        ((void*)0)
#define BLOCK_1536_1K_1            (7u)
#define BLOCK_1536_512_1           (8u)
#define BLOCK_1536_1K_2            (23u)
#define BLOCK_1536_512_2           (24u)
#define BLOCK_1536_1K_3            (31u)
#define BLOCK_1536_512_3           (32u)


#define SET_STACK_ADDR(x,y)     (((uint32_t)x)+((uint32_t)y))

typedef union
{
    struct
    {
        uint8_t block_1K_1[8];              //20006000 - 20008000
        uint8_t block_512_1[16];            //20004000 - 20006000
        uint8_t block_1K_2[8];              //20002000 - 20004000
        uint8_t block_512_2[8];             //20001000 - 20002000
    };

    uint8_t whole_block[8+16+8+8];
}block_t;

typedef struct
{
    uint8_t block_1536[3];
    uint8_t idx;
}bigblock_t;


typedef struct 
{
    block_t     alloc;
    bigblock_t  alloc_bigblock;
    uint8_t lowIdx;
    uint8_t highIdx;
}memTrk_t;


typedef struct 
{
    uint8_t *top_ptr;
    uint8_t *low_ptr;
    memTrk_t memUpdater;
    //uint32_t proc_ID; can be placed in task block
}malloc_heap_t;

void mem_init(void);
void *malloc_from_heap (int size_in_bytes);
void free(void *ptr);
void allowFlashAccess(void);
void allowPeripheralAccess(void);
void setupSramAccess(void);
void setSramAccessWindow(uint32_t *baseAdd, uint32_t size_in_bytes);
bool memErrorCheck(int size);
int getBlockSize(int size_in_bytes);
void mem_chk(void);
uint8_t getSrdRegion(uint32_t *addr, uint32_t size, uint8_t *mpuNum, uint8_t *logSize, uint32_t *convBaseAddr);
void mpuchecks(void);

#endif /* MEM_H_ */
