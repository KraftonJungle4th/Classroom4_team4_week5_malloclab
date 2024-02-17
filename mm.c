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
    "team4",
    /* First member's full name */
    "Harry Bovik",
    /* First member's email address */
    "bovik@cs.cmu.edu",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* Basic constants and macros */
#define WSIZE 4 // Word and header/footer size (bytes)
#define DSIZE 8 // Double word size (bytes)
#define CHUNKSIZE (1<<12) // Extend heap by this amount - 힙을 이만큼 늘려라 (bytes) 

#define MAX(x,y) ((x>y)? x : y) // If x is greater than y, select x. Otherwise (if y is greater), select y. (max function defined - 맥스함수 정의)

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc) ((size)|(alloc)) // 크기와 할당 비트 통합 - 헤더와 풋터에 저장할 수 있는 값 리턴 

/* Read and write a word at address p */
#define GET(p)          (*(unsigned int *)(p))        // get the address of 'p' - p가 참조하는 워드를 읽어서 리턴 
#define PUT(p, val)     (*(unsigned int *)(p)=(val))   // put 'val' into address 'p' - p가 가리키는 워드에 val을 저장

/* Read the size and allocated fields from address p */
#define GET_SIZE(p)     (GET(p) & ~0x7)
#define GET_ALLOC(p)    (GET(p) & 0x1)

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp)        ((char*)(bp) - WSIZE)
#define FTRP(bp)        ((char*)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp)   ((char*)(bp) + GET_SIZE(((char*)(bp)) - WSIZE))
#define PREV_BLKP(bp)   ((char*)(bp) - GET_SIZE(((char*)(bp)) - DSIZE))

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

static char *heap_listp;

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    //creating the initial empty heap - 초기 빈 힙공간 만들기 
    /* 
     * mem_sbrk - simple model of the sbrk function. Extends the heap 
     * by incr bytes and returns the start address of the new area. In
     * this model, the heap cannot be shrunk.
     * sbrk 함수의 단수화된 형태. 힙을 incr바이트만큼 늘리고 이 새 공간의
     * 시작 주소를 반환한다. 이 형태의 sbrk함수로는 힙의 크기를 줄일 수 없다.   
     */
    
    //4워드의 크기로 heap 초기화
    heap_listp = mem_sbrk(4*WSIZE);
    if (heap_listp == (void*)-1) 
    // mem_sbrk가 -1을  반환했다면 incr를 음수로 넣었거나 메모리가 꽉 찼다는 말이다.
    // 축소하라는 명령은 거부하며, 메모리를 더 못 늘리는 상황에도 거부한다. 
        return -1;  
    // initialization
    PUT(heap_listp, 0); // unused padding word - 미사용 패딩 워드
    // 연결과정 동안에 가장자리 조건을 없애주기 위한 속임수 - prologue & epilogue
    PUT(heap_listp + (WSIZE), PACK(DSIZE,1));  // prologue block header (8/1) does not returned
    PUT(heap_listp + (2*WSIZE), PACK(DSIZE,1)); // prologue block footer (8/1) does not get returned - blocks assigned via malloc come after this word 
    PUT(heap_listp + (3*WSIZE), PACK(0,1)); // epilogue block header (0/1) size 0. 
    return 0;
}

static void* extend_heap(size_t words) // 힙을 늘려주는 함수 (아래 두 가지에 시행됨 1. 힙이 초기화될 때 2. mm_malloc() 호출 시 알맞은 메모리 크기가 없을 때)
{
    char *bp;
    size_t size;

    size = (words % 2) ? (words + 1) * WSIZE | (words) * WSIZE; // 8바이트(더블  워드)씩 정렬 유지를 위한 코드
    if((long) (bp = mem_sbrk(size)) == -1){ // mem_sbrk 함수를 통해 힙 영역을 늘림(실제로 힙 영역을 늘리는 부분)
        return NULL; // mem_sbrk 실패 시 -1을 반환하기 때문에 예외 처리 코드
    }
    // 각 블록의 헤더와 풋터는 해당 블록의 크기와 할당 비트로 이루어져 있기 때문에 size와 할당 비트를 같이 할당해 주어야 한다. 할당 비트는 0으로 고정
    PUT(HDRP(bp), PACK(size,0)); // 블록의 헤더 값 할당 
    PUT(FTRP(bp), PACK(size,0)); // 블록의 풋터 값 할당
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0,1)); // 힙 자체를 늘려주다 보니 묵시적 가용 리스트의 끝을 나타내는 에필로그 블록 할당이 필수적

    return coalesce(bp); // 단지 늘리기만 했으므로 이전 블록과 연결시켜야함. 또한 단편화를 막기 위해 coalesce 함수 호출이 필요
}

static void *coalesce(void *bp){
    size_t size = GET_SIZE(HDRP(bp));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));

    if (next_alloc && prev_alloc){
        PUT(HDRP(bp),PACK(size,0));
        PUT(FTRP(bp),PACK(size,0));
    }else if(!next_alloc && prev_alloc){
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(bp),PACK(size,0));
        PUT(FTRP(NEXT_BLKP(bp)),PACK(size,0));
    }else if(next_alloc && !prev_alloc){
        size += GET_SIZE(FTRP(PREV_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)),PACK(size,0));
        PUT(FTRP(dp),PACK(size,0));
        bp = PREV_BLKP(bp);
    }else{
        size = size + GET_SIZE(HDRP(NEXT_BLKP(bp))) + GET_SIZE(FTRP(PREV_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)),PACK(size,0));
        PUT(FTRP(NEXT_BLKP(bp)),PACK(size,0));
        bp = PREV_BLKP(bp); 
    }
    return bp;
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
    else {
        *(size_t *)p = size;
        return (void *)((char *)p + SIZE_T_SIZE);
    }
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr) // 메모리 반환해주는 코드(= 가용 블록으로 변환) -> 할당비트를 0으로 만들고 coalesce() 호출
{
    size_t size = GET_SIZE(HDRP(ptr)); // 크기는 변하지 않고 할당 비트만 바꿔주어야 하니 반환되는 블록의 크기를 가져옴

    PUT(HDRP(ptr), PACK(size, 0)); // 헤더에 할당 비트만 0으로 변환됨
    PUT(FTRP(ptr), PACK(size, 0)); // 풋터에 할당 비트만 0으로 변환됨
    coalesce(ptr); // 가용 블록으로 변환이 되어 이전 블록의 연결과 단편화를 막기 위해 coalesce() 호출
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














