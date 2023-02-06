#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define BLOCKSIZE 4
#define LIMITSIZE 16
#define DOUBLE(X) X<<=1 
#define SIZEINPUT(X,Y,Z) X=(Y<<1)+Z
#define MOVEPOINTER(P,X,Y) (P)=(X)+(Y) 
#define INITSIZE 16

typedef struct freeList
{
    u_int32_t size;
    struct freeList* nextNode;

} ST_FREELIST;
typedef struct size
{
    u_int32_t s;

}SIZE;

int initFlag = 0;
void *seglist[LIMITSIZE];

static ST_FREELIST* debugPointer = NULL;
extern void debug(const char *fmt, ...);
extern void *sbrk(intptr_t increment);



unsigned int max_size;

void tempFree(void * p)
{
    ST_FREELIST * realPoint = p-BLOCKSIZE;
    int i;
    u_int32_t initSize=INITSIZE;
    realPoint->nextNode = NULL;
    u_int32_t blockSize = realPoint->size>>1;
    
    for(i = 0; i <LIMITSIZE; i ++)
    {
        if(blockSize<=initSize)
            break;
        DOUBLE(initSize);
    }
    
    if(seglist[i] == NULL)
    {
        
        seglist[i] = realPoint;
        return;
    }
    
    ST_FREELIST* temp = seglist[i];
    debug("debug %p\n",temp);
    debug("debug %p\n",realPoint);
    temp->nextNode = realPoint;
    
}

void* allocateFreeList(u_int32_t blockSize)  
{
    u_int32_t initSize = INITSIZE;
    int i=0;

    for(i = 0; i < LIMITSIZE; i++)
    {
        if(blockSize<=initSize)
            break;    
        DOUBLE(initSize);         
            
    }

    void * p;
    
    if(seglist[i] == NULL)
    {
        p = sbrk(initSize);

        SIZE* t = p;
        SIZEINPUT(t->s,initSize,1);
        MOVEPOINTER(t,p,initSize-BLOCKSIZE);
        SIZEINPUT(t->s,initSize,1);
        
        return p+BLOCKSIZE;

    };
    
    ST_FREELIST* temp = seglist[i];
    
    p = seglist[i];
    
    if(temp->nextNode == NULL)
        seglist[i] = NULL;
    else
        seglist[i] = temp->nextNode;
        
    return p+BLOCKSIZE;
    //할당


}


void *myalloc(u_int32_t size)
{
    
    
    // for(int i = 0; i < LIMITSIZE; i++)
    // {
    //     debug("seglist %d : %p\n",i,seglist[i]);       
    // }    
    debug("alloc start\n");
    if (size == 0)
        return 0;

    u_int32_t  blockSize = 0;
    if(size < 8)
    {
        blockSize = 8;
    }else
    {
        blockSize = size;
    }

    blockSize +=  BLOCKSIZE;

    void *p = allocateFreeList(blockSize);

    debug("alloc (%p) size:(%d)\n",p,size);
    int * memSize = p;
    max_size += size;

    return p;
}

void *myrealloc(void *ptr, u_int32_t size)
{
    // debug("realloc Start (%p)\n",ptr);
    void *p = NULL;
    myfree(ptr);
    u_int32_t blockSize = size;
    blockSize += BLOCKSIZE;   
    p = allocateFreeList(blockSize);
    // debug("realloc size(%d) %p\n ",size,p);
    
    return p;

}

void myfree(void *ptr)
{


    debug("free start\n");
    // for(int i = 0; i < LIMITSIZE; i++)
    // {
    //     debug("seglist %d : %p\n",i,seglist[i]);       
    // } 
    if(ptr == NULL)
        return;
    tempFree(ptr);
    debug("free %p \n",ptr);
    // coalescing(ptr);      


}