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

static char *heap_listp;  // 힙의 시작점을 가리킬 포인터

void *coalesce(void *ptr) {
    size_t size;
    size_t size_prev = GET_SIZE(FTRP(PREV_BLKP(ptr)));
    size_t size_now = GET_SIZE(HDRP(ptr));
    size_t size_next = GET_SIZE(HDRP(NEXT_BLKP(ptr)));

    size_t alloc_prev = GET_ALLOC(FTRP(PREV_BLKP(ptr)));
    size_t alloc_now = GET_ALLOC(HDRP(ptr));
    size_t alloc_next = GET_ALLOC(HDRP(NEXT_BLKP(ptr)));

    /*
    if (alloc_next == 1 && size_next == 0) {
        printf("FUCKING ERROR\n");
        return NULL;
    }
    */
   
   if(alloc_prev == 1 && alloc_next == 1) {   // 합칠 게 없음
        return ptr;
    }
    else if(alloc_prev == 1 && alloc_next == 0) {
        size = size_now + size_next;
        PUT(HDRP(ptr), PACK(size, 0)); //필요한 만큼 할당
        PUT(FTRP(ptr), PACK(size, 0)); //필요한 만큼 할당
        return ptr;
    }
    else if(alloc_prev == 0 && alloc_next == 1) {
        size = size_prev + size_now;
        PUT(HDRP(PREV_BLKP(ptr)), PACK(size, 0)); //필요한 만큼 할당
        PUT(FTRP(ptr), PACK(size, 0)); //필요한 만큼 할당

        /*
        char *heap_end = (char *)mem_heap_hi() - WSIZE;
        printf("ptr %p\n", ptr);
        printf("heap_end %p\n", heap_end);
        printf("incresment amount %p\n", heap_end - (char *)ptr);
        */

        return PREV_BLKP(ptr);
    }
    else  {
        size = size_prev + size_now + size_next;
        PUT(HDRP(PREV_BLKP(ptr)), PACK(size, 0)); //필요한 만큼 할당
        PUT(FTRP(NEXT_BLKP(ptr)), PACK(size, 0)); //필요한 만큼 할당
        return PREV_BLKP(ptr);
    }
    return ptr;
}


static void *extend_heap(size_t words) {
    char *bp;
    size_t size;

    if(words % 2 == 1) //홀수면 
    {
        words += 1; //짝수로 맞춰주기
    }
    if(words == 0) {
        return NULL;
    }

    // printf("words!!!! %ld\n", words);
    assert(words % 2 == 0);

    size = words * WSIZE;  //words의 4바이트 배수의 크기를 넣음
    if ((bp = mem_sbrk(size)) == (void *)-1) {
        return NULL;  // 힙 확장 실패 시 NULL 반환
    }
    PUT(HDRP(bp), PACK(size, 0)); //현재 header의 주소에 크기와 할당 여부를 넣음
    PUT(FTRP(bp), PACK(size, 0)); //현재 footer의 주소에 크기와 할당 여부를 넣음
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); //다음 블록의 헤더의 주소에 크기와할당 여부를 넣음

    return coalesce(bp);
}


// [ 1 ] [ V ] ?


/*
 * mm_init - initialize the malloc package.
 */
int mm_init(void) 
{
    /* 초기 빈 힙 생성 (16바이트 할당) */
    if ((heap_listp = mem_sbrk(4*WSIZE)) == (void *)-1) {
        return -1;  // 힙 확장 실패 시 -1 반환
    }
    /* 힙의 초기 구조 설정 */
    PUT(heap_listp, 0);                           // Padding: 8바이트 정렬을 위한 더미 워드
    PUT(heap_listp + (1*WSIZE), PACK(DSIZE, 1));  // Prologue Header: 8바이트, allocated
    PUT(heap_listp + (2*WSIZE), PACK(DSIZE, 1));  // Prologue Footer: 8바이트, allocated  
    PUT(heap_listp + (3*WSIZE), PACK(0, 1));      // Epilogue Header: 0바이트, allocated
    
    /* heap_listp를 첫 번째 가용 블록 위치로 이동 (Prologue 다음) */
    heap_listp += (2*WSIZE);

    if(extend_heap(CHUNKSIZE/WSIZE) == NULL) {
        return -1;
    }
    
    return 0;  // 초기화 성공
}


static void *find_fit(size_t asize) {
    char *bp = heap_listp;

    while(GET_SIZE(HDRP(bp)) > 0) { //다음 블록이 있는지 없는지 판별해야함
        if(GET_SIZE(HDRP(bp)) >= asize && GET_ALLOC(HDRP(bp)) == 0) { //블록이 사이즈가 충분하고 free일 경우 리턴
            return bp; 
        }
        bp = NEXT_BLKP(bp); //찾기 못했을 경우 다음 블록으로
    }
    return NULL;
}

static void place(char *bp, size_t asize) {

    size_t before_size = GET_SIZE(HDRP(bp));

    if(GET_SIZE(HDRP(bp)) == asize) { //Free 블록과 필요한 크기가 똑같을 경우
        PUT(HDRP(bp), PACK(asize, 1)); //전체 할당
        PUT(FTRP(bp), PACK(asize, 1)); //전체 할당
    }
    else if(GET_SIZE(HDRP(bp)) > asize) { //Free 블록이 필요한 크기보다 클 경우
        PUT(HDRP(bp), PACK(asize, 1)); //필요한 만큼 할당
        PUT(FTRP(bp), PACK(asize, 1)); //필요한 만큼 할당
        PUT(HDRP(NEXT_BLKP(bp)), PACK(before_size - asize, 0)); //나머지는 allocated
        PUT(FTRP(NEXT_BLKP(bp)), PACK(before_size - asize, 0)); //나머지는 allocated
    }
}


/*
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 * Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    // size 0 체크
    // 크기 계산
    // find_fit으로 찾기
    // 찾았으면? place
    // 못 찾았으면? extend_heap
    // 반환


    size_t asize;      // 조정된 블록 크기 (헤더/푸터/정렬 포함)
    size_t extendsize; // 힙 확장 시 요청할 크기
    char *bp;          // 블록 포인터 (페이로드 시작 주소)

    
    if (size == 0) { // 요청 크기가 0이면 NULL 반환
        return NULL;
    }

    
    if (size <= DSIZE) { // 요청 크기를 헤더/푸터 및 8바이트 정렬을 포함한 asize로 조정
        asize = 2 * DSIZE;  // 요청이 8B 이하: 최소 블록 크기인 16B (DSIZE*2)로 설정
    } else {
        asize = ALIGN(size + DSIZE); // (size + DSIZE)는 페이로드 + 헤더/푸터 오버헤드를 더한 크기, ALIGN(size + DSIZE)는 이 크기를 8의 배수로 올림 (8바이트 정렬 보장)
    }

    
    if ((bp = find_fit(asize)) != NULL) { // 가용 리스트에서 적합한 블록 찾기
        place(bp, asize); // 찾았으면 할당하고 분할 (place)
        return bp; // bp는 이미 payload 시작 주소를 가리킴 (HDRP(bp) + WSIZE)
    }

    
    extendsize = MAX(asize, CHUNKSIZE); // 적합한 블록을 찾지 못했으면 힙 확장 (extend_heap)
    
    
    if ((bp = extend_heap(extendsize / WSIZE)) == NULL) {  // extend_heap은 워드 단위 크기를 받으므로, 바이트 크기를 WSIZE로 나눔
        return NULL; // 확장 실패
    }

    place(bp, asize); // 확장된 새 블록에 할당하고 포인터 반환
    return bp;
}



/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    if(ptr == NULL) { //ptr이 NULL일 경우 Return
        return;
    }

    size_t size = GET_SIZE(HDRP(ptr)); // 현재 블록의 크기

    PUT(HDRP(ptr), PACK(size, 0)); //현재 header의 주소에 크기와 할당 여부를 넣음
    PUT(FTRP(ptr), PACK(size, 0)); //현재 footer의 주소에 크기와 할당 여부를 넣음

    coalesce(ptr);
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