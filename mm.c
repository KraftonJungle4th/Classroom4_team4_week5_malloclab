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
    void* heap_listp = mem_sbrk(4*WSIZE);
    if (heap_listp == (void*)-1) 
    // mem_sbrk가 -1을  반환했다면 incr를 음수로 넣었거나 메모리가 꽉 찼다는 말이다.
    // 축소하라는 명령은 거부하며, 메모리를 더 못 늘리는 상황에도 거부한다. 
        return -1;  
    PUT(heap_listp, 0);
    PUT(heap_listp + (WSIZE), PACK(DSIZE,1));    
    PUT(heap_listp + (2*WSIZE), PACK(DSIZE,1));    
    PUT(heap_listp + (3*WSIZE), PACK(0,1));    
    return 0;
}

static void* extend_heap(size_t words)
{

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














