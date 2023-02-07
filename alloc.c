#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define BLOCKSIZE 8
#define LIMITSIZE 20
#define DOUBLE(X) X<<=1 
#define SIZEINPUT(X,Y,Z) X=(Y<<1)+Z
#define MOVEPOINTER(P,X,Y) (P)=(X)+(Y) 
#define MOVENEXT(P,X,Y) (P)=(X)+(Y)
#define MOVEPREV(P,X,Y) (P)=(X)-(Y)
#define MOVEPREVFOOTER(P,X) (P)=(X)-(2*BLOCKSIZE)
#define MOVEFOOTER(P,X,Y) (P)=(X)+(Y)-(BLOCKSIZE)

#define INITSIZE 32

typedef struct freeList
{
    size_t size;
    struct freeList* nextNode;
    struct freeList* prevNode;

} ST_FREELIST;

typedef struct size
{
    size_t s;

}SIZE;

void *seglist[LIMITSIZE];
static int checkAddress = 0;
static void* initAddress;
static void* heapAddress;
static ST_FREELIST* debugPointer = NULL;
extern void debug(const char *fmt, ...);
extern void *sbrk(intptr_t increment);



unsigned int max_size;

void tempFree(void * p)
{
    ST_FREELIST * realPoint = p-BLOCKSIZE;
    size_t blockSize = realPoint->size>>1;
    ST_FREELIST * prevPoint;
    ST_FREELIST * prevPointHeader;    
    ST_FREELIST * nextPoint; 
    MOVEPREVFOOTER(prevPointHeader,p);
    MOVEPREV(prevPoint,p,prevPointHeader->size>>1);
    MOVENEXT(nextPoint,p,realPoint->size>>1);
    if(nextPoint->size&1 == 0 && nextPoint < heapAddress)
    {
        blockSize += nextPoint->size>>1;
        if(nextPoint->nextNode != NULL)
        {
            realPoint->nextNode = nextPoint->nextNode;
            nextPoint->nextNode->prevNode = realPoint;
        }else
        {
            realPoint->nextNode = NULL;
        }
        if(nextPoint->prevNode != NULL)
        {
            realPoint->prevNode = nextPoint->prevNode;
            nextPoint->prevNode->nextNode = realPoint;
        }else
        {
            realPoint->prevNode = NULL;
        }
        
        SIZEINPUT(realPoint->size,blockSize,1);
        ST_FREELIST* footer;
        MOVEFOOTER(footer,(void*)realPoint,blockSize);
        SIZEINPUT(footer->size,blockSize,1);

    }
    if(prevPoint->size&1 == 0 && initAddress < prevPoint)
    {
        blockSize += prevPoint->size>>1;
        if(prevPoint->prevNode != NULL)
        {
            prevPoint->nextNode->prevNode = prevPoint->prevNode;
        }else
        {
            prevPoint->nextNode->prevNode = NULL; 
        }

        if(prevPoint->nextNode != NULL)
        {
            prevPoint->prevNode->nextNode = prevPoint->nextNode; 
        }else
        {
            prevPoint->prevNode->nextNode = NULL;
        }
        
        SIZEINPUT(prevPoint->size,blockSize,1);
        ST_FREELIST* footer;
        MOVEFOOTER(footer,(void*)prevPoint,blockSize);
        SIZEINPUT(footer->size,blockSize,1);

    }


    int i;
    size_t initSize=INITSIZE;
    realPoint->nextNode = NULL;
    
    
    for(i = 0; i <LIMITSIZE; i ++)
    {
        if(blockSize<=initSize<<1)
            break;
        DOUBLE(initSize);
    }
    
    if(seglist[i] == NULL)
    {
        
        seglist[i] = realPoint;
        realPoint->nextNode = NULL;
        realPoint->prevNode = NULL;
        return;
    }
    
    ST_FREELIST* temp = seglist[i];
    realPoint->prevNode = seglist[i];
    realPoint->nextNode = NULL;
    temp->nextNode = realPoint;
    
}

void* allocateFreeList(size_t blockSize)  
{
    size_t initSize = INITSIZE;
    int i=0;
    void * p; 
    for(i = 0; i < LIMITSIZE; i++)
    {
        if(blockSize<=initSize)
        {
            if(seglist[i] == NULL)
                break;
            //split 구현 
               
            ST_FREELIST* temp = seglist[i];
    
            p = seglist[i];
    
            if(temp->nextNode == NULL)
                seglist[i] = NULL;
            else{
                seglist[i] = temp->nextNode;
                temp->nextNode->prevNode = NULL;
            }
            return p+BLOCKSIZE;
            
        }
        DOUBLE(initSize);         
            
    }


    p = sbrk(initSize);
    heapAddress = p+initSize;
    SIZE* t = p;
    SIZEINPUT(t->s,initSize,1);
    MOVEPOINTER(t,p,initSize-BLOCKSIZE);
    SIZEINPUT(t->s,initSize,1);
        
    return p+BLOCKSIZE;




}


void *myalloc(size_t size)
{
    if(checkAddress == 0)
    {   
        
        initAddress = sbrk(0);
        // debugPointer = initAddress + 159405;
        // //debug("debugPointer(%p)\n",debugPointer);
        // //debug("initAdd(%p)\n",initAddress);  
        checkAddress = 1;
    } 
    
    // for(int i = 0; i < LIMITSIZE; i++)
    // {
    //     debug("seglist %d : %p\n",i,seglist[i]);       
    // }    
    debug("alloc start\n");
    if (size == 0)
        return 0;

    size_t  blockSize = 0;
    if(size < 16)
    {
        blockSize = 16;
    }else
    {
        blockSize = size;
    }

    blockSize +=  BLOCKSIZE*2;

    void *p = allocateFreeList(blockSize);

    debug("alloc (%p) size:(%d)\n",p,size);
    int * memSize = p;
    max_size += size;

    return p;
}

void *myrealloc(void *ptr, size_t size)
{
    // debug("realloc Start (%p)\n",ptr);
    void *p = NULL;
    myfree(ptr);
    size_t blockSize = size;
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