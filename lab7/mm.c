/*
 * mm.c - A malloc package applying explicit free list.
 *
 * In this approach, an allocated block has its header and footer, 
 * which contain its size and allocated information. However, a free
 * block has not only header and footer, but also pointers to the
 * previous free block and the next free block, which causes the minimum
 * size of a free block to be 16 bytes. The free list is maintained in a
 * LIFO order, which means a new free block to be inserted will be set 
 * at the beginning of the free list. (Actually I tried maintaining the
 * free list in an ascending order of the block size, and it would be 
 * better if we apply the first fit search in this list, but it caused
 * too many bugs and I gave up.
 *
 * As for searching method, I tried both best fit search and first 
 * fit search on my machine, and finally decided to use a combine of 
 * them. That means, the program decides which method to apply according
 * to the size that we need. If it's larger than BSIZE, apply best fit
 * serach; if not, apply first fit search.
 * 
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
    "515030910073",
    /* First member's full name */
    "Zhou Xin",
    /* First member's email address */
    "cgcg96@sjtu.edu.cn",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* Basic constants and macros */
#define WSIZE       4       /* Word and header/footer size(bytes) */
#define DSIZE       8       /* Double word size (bytes) */
#define CHUNKSIZE   4112    /* Extend heap by this amount (bytes) */

/* Decide whether we need to do a best fit search or not */
#define BSIZE       160     

#define MAX(x,y) ((x) > (y)? (x) : (y))

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc)  ((size) | (alloc))

/* Read and write a word at address p */
#define GET(p)       (*(unsigned int *)(p))
#define PUT(p, val)  (*(unsigned int *)(p) = (size_t)(val))

/* Read the size and alocated fields from address p */
#define GET_SIZE(p)  (GET(p) & ~0x7)
#define GET_ALLOC(p) (GET(p) & 0x1)

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp)       ((char *)(bp) - WSIZE)
#define FTRP(bp)       ((char *)(bp) + GET_SIZE(HDRP(bp)) - DSIZE)

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_BLKP(bp)  ((char *)(bp) + GET_SIZE(((char *)(bp) - WSIZE)))
#define PREV_BLKP(bp)  ((char *)(bp) - GET_SIZE(((char *)(bp) - DSIZE)))

/* Given free block ptr bp, compute address of next and previous
 * free blocks
 */
#define PREV_FREE(bp)  ((char *)GET(bp))
#define NEXT_FREE(bp)  ((char *)GET(bp + WSIZE))

/* Set previous and next free blocks */
#define SET_PREV_FREE(bp, np)  (PUT(bp, np))
#define SET_NEXT_FREE(bp, np)  (PUT(bp+WSIZE,np))


/* global variables */
static char *freep = NULL;       /* the beginning of the free list */
static char *heap_listp = NULL;

/* functions */
int mm_init(void);
void *mm_malloc(size_t size);
void mm_free(void* bp);
void *mm_realloc(void *ptr, size_t size);
static void *extend_heap(size_t words);
static void *coalesce(void *bp);
static void remove_freep(char *bp);
static void insert_freep(char *bp);
static void *best_fit(size_t asize);
static void *first_fit(size_t asize);
int mm_check(void);

/* 
 * extend_heap - a version nearly the same as the one in textbook 
 */
static void *extend_heap(size_t words)
{
    char *bp;
    size_t size;

    /* Allocate an even number of words to maintain alignment */
    size = (words % 2) ? (words+1) * WSIZE : words * WSIZE;
    if ((long)(bp = mem_sbrk(size)) == -1)
       return NULL;

    /* Initialize free block header/footer and the epilogue header */
    PUT(HDRP(bp), PACK(size, 0));         /* Free block header */
    PUT(FTRP(bp), PACK(size, 0));         /* Free block footer */
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); /* New epilogue header */

    /* Coalesce if the previous block was free */
    return coalesce(bp);
}

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    /* Create the initial empty heap */
    if ((heap_listp = mem_sbrk(4*WSIZE)) == (void *)-1)
       return -1;
    PUT(heap_listp, 0);                          /* Alignment padding */
    PUT(heap_listp + (1*WSIZE), PACK(DSIZE, 1)); /* Prologue header */
    PUT(heap_listp + (2*WSIZE), PACK(DSIZE, 1)); /* Prologue footer */
    PUT(heap_listp + (3*WSIZE), PACK(0, 1));     /* Epilogue header */
   
    /* set the beginning of the free list, and this address will always
       be the ending of the free list as well. */
    freep = heap_listp + 2*WSIZE;

    /*Extend the empty heap with a free block of CHUNKSIZE bytes */
    if (extend_heap(CHUNKSIZE/WSIZE) == NULL)
       return -1;
    return 0;
}

static void *best_fit(size_t asize)
{
    /* best fit search */
    void *bp;
    void *cur = NULL;
    for (bp = freep; bp != (heap_listp+DSIZE); bp = NEXT_FREE(bp)) {
        if (asize <= GET_SIZE(HDRP(bp))) {
            if (cur == NULL) cur = bp;
            else if (GET_SIZE(HDRP(bp)) < GET_SIZE(HDRP(cur)))
              cur = bp;
        }
    }
    return cur;
}


static void *first_fit(size_t asize)
{
    /* first fit search */
    void *bp;
    for (bp = freep; bp != (heap_listp+DSIZE); bp = NEXT_FREE(bp)) {
        if (asize <= GET_SIZE(HDRP(bp))) return bp;
    }
    return NULL;
}

/*
 * place - Set a free block allocated, and coalesce if necessary.
 */
static void place(void *bp, size_t asize)
{
    size_t csize = GET_SIZE(HDRP(bp));

    if ((csize - asize) >= (2*DSIZE)) {
        PUT(HDRP(bp), PACK(asize, 1));
        PUT(FTRP(bp), PACK(asize, 1));
        remove_freep(bp);
        bp = NEXT_BLKP(bp);
        PUT(HDRP(bp), PACK(csize-asize, 0));
        PUT(FTRP(bp), PACK(csize-asize, 0));
        coalesce(bp);
    }
    else {
        PUT(HDRP(bp), PACK(csize, 1));
        PUT(FTRP(bp), PACK(csize, 1));
        remove_freep(bp);
    }
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    size_t asize;       /* Adjusted block size */
    size_t extendsize;  /* Amount to extend heap if not fit */
    char *bp;

    /* Ignore spurious requests */
    if (size == 0)
       return NULL;

    /* Adjust block size to include overhead and alignment reqs. */
    if (size <= DSIZE)
      asize = 2*DSIZE;
    else
      asize = DSIZE * ((size + (DSIZE) + (DSIZE-1)) / DSIZE);

    /* Search the free list for a fit */
    if (asize > BSIZE) bp = best_fit(asize);
    else bp = first_fit(asize);
    if (bp != NULL) {
        place(bp, asize);
        return bp;
    }

    /* No fit found. Get more memory and place the block */
    extendsize = MAX(asize, CHUNKSIZE);
    if ((bp = extend_heap(extendsize/WSIZE)) == NULL)
       return NULL;
    place(bp, asize);
    return bp;
}

/*
 * mm_free - Freeing a block.
 */
void mm_free(void *bp)
{
    size_t size = GET_SIZE(HDRP(bp));

    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
    coalesce(bp);
}

static void *coalesce(void *bp)
{
    size_t prev_alloc = (bp == heap_listp+2*DSIZE)?
        1 : GET_ALLOC(FTRP(PREV_BLKP(bp)));
    size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
    size_t size = GET_SIZE(HDRP(bp));

    if (prev_alloc && next_alloc){
        insert_freep(bp);
        return bp;
    }
    else if (prev_alloc && !next_alloc){
        size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
        remove_freep(NEXT_BLKP(bp));
        PUT(HDRP(bp), PACK(size, 0));
        PUT(FTRP(bp), PACK(size, 0));
    }
    else if (!prev_alloc && next_alloc){
        size += GET_SIZE(HDRP(PREV_BLKP(bp)));
        PUT(FTRP(bp), PACK(size, 0));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        remove_freep(PREV_BLKP(bp));
        bp = PREV_BLKP(bp);
    }
    else if (!prev_alloc && !next_alloc){
        size += GET_SIZE(HDRP(PREV_BLKP(bp))) +
            GET_SIZE(HDRP(NEXT_BLKP(bp)));
        PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
        PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
        remove_freep(PREV_BLKP(bp));
        remove_freep(NEXT_BLKP(bp)); 
        bp = PREV_BLKP(bp);
    }
    insert_freep(bp);
    return bp;
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
    copySize = GET_SIZE(HDRP(oldptr)) - DSIZE;
    if (size < copySize)
      copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}

static void remove_freep(char* bp)
{
    if (PREV_FREE(bp)) 
      SET_NEXT_FREE(PREV_FREE(bp), NEXT_FREE(bp));
    else
      freep = NEXT_FREE(bp);
    SET_PREV_FREE(NEXT_FREE(bp), PREV_FREE(bp));
}

static void insert_freep(char* bp)
{
    SET_PREV_FREE(freep, bp);
    SET_NEXT_FREE(bp, freep);
    SET_PREV_FREE(bp, 0);
    freep = bp;
}


static int find_p(char *bp)
{
    char *chp;
    for (chp = freep; chp != heap_listp+DSIZE; chp = NEXT_FREE(chp)) {
        if (chp == bp) return 1;
    }
    return 0;
}

/*
 * mm_check - a heap consistency checker.
 */
int mm_check(void)
{
    char *chp;

    for (chp = freep; chp != heap_listp+DSIZE; chp = NEXT_FREE(chp)) {

        /* Is every block in the free list marked as free? */
        if (GET_ALLOC(HDRP(chp))) {
            printf("Error: a block in the free list not marked as free.\n");
            printf("Address: %p", chp);
            exit(1);
        }

        /* Are there any contiguous free blocks that somehow escaped 
         * coalescing?
         */
        if ((FTRP(chp) + DSIZE) == NEXT_FREE(chp)) {
            printf("Error: a free block escaped coalescing.\n");
            printf("Address: %p", chp);
            exit(1);
        }

        /* Do the pointers in the free list point to valid free blocks? */
        if (GET_ALLOC(FTRP(chp)) || (GET_SIZE(FTRP(chp))!=GET_SIZE(HDRP(chp)))) {
            printf("Error: a pointer in the free list points to an invalid free block.\n");
            printf("Address: %p", chp);
            exit(1);
        }
    }
    for (chp = heap_listp+DSIZE; GET_SIZE(HDRP(chp)); chp=NEXT_BLKP(chp)) {
        
        /* Is every free block actually in the free list? */
        if (!GET_ALLOC(HDRP(chp)) && find_p(chp)) {
            printf("Error: a free block not in the free list.\n");
            printf("Address: %p", chp);
            exit(1);
        }

        /* Do the pointers in a heap block point to valid heap addresses? */
        if ((size_t)(HDRP(chp)) < (size_t)(mem_heap_lo()) ||
            (size_t)(HDRP(chp)) > (size_t)(mem_heap_hi()) ||
            (size_t)(FTRP(chp)) < (size_t)(mem_heap_lo()) ||
            (size_t)(FTRP(chp)) > (size_t)(mem_heap_hi()) ) {
            printf("Error: a pointer in a heap block points to invalid heap address.\n");
            printf("Address: %p", chp);
            exit(1);
        }
        
        /* Do the pointers in a heap block point to aligned addresses? */
        if ((size_t)(chp) % ALIGNMENT) {
            printf("Error: pointer address not aligned.\n");
            printf("Address: %p", chp);
            exit(1);
        }
    }
    return 0;
}

