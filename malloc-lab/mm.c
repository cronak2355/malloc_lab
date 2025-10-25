/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 *
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "Cronak",
    /* First member's full name */
    "Kimtaewon",
    /* First member's email address */
    "kronak2355@gmail.com",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~0x7)

#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

/* Basic constants and macros */
#define WSIZE 4        /* Word and header/footer size (bytes) */ 
                       /* 워드 크기 = 4바이트. Header와 Footer의 크기 */
                       
#define DSIZE 8        /* Double word size (bytes) */ 
                       /* 더블 워드 크기 = 8바이트. 정렬(alignment) 요구사항 */
                       
#define CHUNKSIZE (1<<12)  /* Extend heap by this amount (bytes) */
                           /* 힙을 확장할 때 요청할 크기 = 4096바이트 = 4KB */

#define MAX(x,y) ((x)>(y)?(x):(y))
/* 두 값 중 큰 값을 반환하는 매크로 */

/* Pack a size and allocated bit into a word */
#define PACK(size,alloc) ((size)|(alloc))
/* 크기(size)와 할당 여부(alloc)를 하나의 워드로 합침 
   예: PACK(16, 1) = 17 (크기 16, 할당됨) 
   size의 하위 3비트는 항상 0이므로 (8바이트 정렬), 
   alloc(0 또는 1)과 OR 연산 가능 */

/* Read and write a word at address p */
#define GET(p) (*(unsigned int*)(p))
/* 주소 p에 있는 값을 읽어옴 (4바이트를 unsigned int로 읽음) */

#define PUT(p,val) (*(unsigned int*)(p)=(val))
/* 주소 p에 값 val을 씀 (4바이트를 unsigned int로 씀) */

/* Read the size and allocated fields from address p */
#define GET_SIZE(p) (GET(p)&~0x7)
/* 주소 p에서 블록 크기를 읽어옴 
   &~0x7 = 하위 3비트를 0으로 만듦 (할당 비트 제거)
   예: 17(10001) & ~0x7(11000) = 16(10000) */

#define GET_ALLOC(p) (GET(p)&0x1)
/* 주소 p에서 할당 여부를 읽어옴 
   &0x1 = 최하위 1비트만 추출
   0 = free, 1 = allocated */

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp) ((char*)(bp)-WSIZE)
/* 블록 포인터 bp가 주어졌을 때, 해당 블록의 header 주소 계산
   bp는 payload의 시작 주소를 가리키므로, 
   header는 그 앞(WSIZE = 4바이트)에 위치 */

#define FTRP(bp) ((char*)(bp)+GET_SIZE(HDRP(bp))-DSIZE)
/* 블록 포인터 bp가 주어졌을 때, 해당 블록의 footer 주소 계산
   1. HDRP(bp)로 header 주소 구함
   2. GET_SIZE()로 블록 전체 크기 구함
   3. bp + 블록크기 - DSIZE(8) = footer 위치
   (블록크기는 header+payload+footer 포함, 
    DSIZE를 빼는 이유는 header(4) + footer(4) = 8바이트) */

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp) ((char*)(bp)+GET_SIZE(((char*)(bp)-WSIZE)))
/* 다음 블록의 bp(payload 시작 주소)를 계산
   1. (bp - WSIZE)로 현재 블록의 header 주소 구함
   2. GET_SIZE()로 현재 블록의 크기 구함
   3. bp + 현재블록크기 = 다음 블록의 bp */

#define PREV_BLKP(bp) ((char*)(bp)-GET_SIZE(((char*)(bp)-DSIZE)))
/* 이전 블록의 bp(payload 시작 주소)를 계산
   1. (bp - DSIZE)로 이전 블록의 footer 주소 구함
   2. GET_SIZE()로 이전 블록의 크기 구함
   3. bp - 이전블록크기 = 이전 블록의 bp */


/*
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    



    return 0;
}

/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    int newsize = ALIGN(size + SIZE_T_SIZE);
    void *p = mem_sbrk(newsize);
    if (p == (void *)-1)
        return NULL;
    else
    {
        *(size_t *)p = size;
        return (void *)((char *)p + SIZE_T_SIZE);
    }
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;

    newptr = mm_malloc(size);
    if (newptr == NULL)
        return NULL;
    copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
    if (size < copySize)
        copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}