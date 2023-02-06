#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define BLOCKSIZE 8
#define LIMITSIZE 20
#define DOUBLE(X) X<<=1 
#define SIZEINPUT(X,Y,Z) X=(Y<<1)+Z

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

static ST_FREELIST* freeLists;

void *seglist[LIMITSIZE];
void *initSpace;
static ST_FREELIST* debugPointer = NULL;
static int init=0;
static void* initAddress;
static int checkAddress = 0;
static void* heapAddress;
extern void debug(const char *fmt, ...);
extern void *sbrk(intptr_t increment);

unsigned int max_size;
/***
 * Prototype 1 : seglist 32 ~ 2^25구조체 구현 및 저장 
 * Prototype 2 : seglist 구조체 범위 저장 
 * Prototype 3 : 구조체 split 및 이동 구현 V
 * Prototype 4 : coalescing 구현 
 *  
***/


void tempFree(void * p)
{
    ST_FREELIST * realPoint = p-BLOCKSIZE;
    int i;
    size_t initSize=32;
    realPoint->nextNode = NULL;
    size_t blockSize = realPoint->size>>1;
    for(i = 0; i <LIMITSIZE; i ++)
    {
        if( initSize <= blockSize && blockSize < initSize<<1)
            break;
        DOUBLE(initSize);
    }

    if(seglist[i] == NULL)
    {
        seglist[i] = realPoint;
        return;
    }
    
    ST_FREELIST* temp = seglist[i];
    temp->nextNode = realPoint;
}

void* allocateFreeList(size_t blockSize)  
{
    size_t initSize = 32;
    int i;
    for(i = 0; i < LIMITSIZE; i++){
        if(initSize <= blockSize && blockSize < initSize<<1)
            break;    
        DOUBLE(initSize);            
            
    }
    
    void * p;
    ST_FREELIST* current = seglist[i];
    ST_FREELIST* prev = NULL;
    debug("init : %d\n",initSize);
    debug("bloc : %d\n",blockSize);
    while(1){
        if(current == NULL)
        {
            debug("1\n");
            p = sbrk(blockSize);

            heapAddress = p+blockSize;
            SIZE* t = p;
            SIZEINPUT(t->s,blockSize,1);
            t = p+blockSize-BLOCKSIZE;
            SIZEINPUT(t->s,blockSize,1);
            break;
        }
        if(blockSize == (current->size>>1))
        {
            debug("2\n");
            p = current;
            if(current->nextNode == NULL){
                if(prev == NULL){
                    seglist[i] = NULL;
                }
                else
                    prev->nextNode = NULL;
            }
            else{
                if(prev == NULL)
                    seglist[i] = current->nextNode;
                else
                    prev->nextNode = current->nextNode;
            }    
            
            break;
        }

        if((int)blockSize < (int)(current->size>>1)-32)
        {
            //current에 0이 들어가면 안됨 
            //split 구현 및 seglist에 저장 
            p = current;
            debug("debug curr %p\n",current);
            debug("debug size %d\n",current->size);
            if(current->nextNode == NULL){
                if(prev == NULL){
                    seglist[i] = NULL;
                }
                else
                    prev->nextNode = NULL;
            }
            else{
                if(prev == NULL)
                    seglist[i] = current->nextNode;
                else
                    prev->nextNode = current->nextNode;
            }      
                      
            SIZE* t = p;
            SIZEINPUT(t->s,blockSize,1);
            
            t = p+blockSize-BLOCKSIZE;
            debug("heap point %p\n",heapAddress);
            debug("debug point %p\n",t);
            SIZEINPUT(t->s,blockSize,1);
            
            size_t freeNodeSize = (current->size>>1) - blockSize;
        

            ST_FREELIST* freeNode = p+blockSize;
            debug("freeNode  %p \n",freeNode);            
            SIZEINPUT(freeNode->size,(freeNodeSize),0);
            freeNode = p+blockSize+freeNodeSize-BLOCKSIZE;
            SIZEINPUT(freeNode->size,(freeNodeSize),0);
            
            int x;
            size_t is = 32;
            for(x = 0; x <LIMITSIZE; x ++)
            {
                if( is <= blockSize && blockSize < is<<1)
                    break;
                is <<=1;
            }
            freeNode->nextNode = seglist[x];
            seglist[x] = freeNode;

                  
            // Error current 삭제로 이전노드 연결 
            // 새 프리노드 크기에 맞는 노드로 이전 
            
            // debug("2 current -> size %d\n",current->size>>1);
            // debug("2 blockSize %d\n",blockSize);
            debug("freeNode  %p \n",freeNode);
            debug("current  %p \n",current);
            break;

        }
        // debug("prev  %p \n",prev);
        // debug("current  %p \n",current);
        prev = current; 
        current = current->nextNode; 
        
    }
 
    return p+BLOCKSIZE;

}

void seglistInit(){
    for(int i = 0; i < LIMITSIZE; i++){
            seglist[i] = NULL;       
    }
    initSpace = sbrk(7000);
    SIZE* t = initSpace;
    SIZEINPUT(t->s,7000,1);
            
    t = initSpace+7000-BLOCKSIZE;
    SIZEINPUT(t->s,7000,1);    

}


void *myalloc(size_t size)
{
    // for(int i = 0; i < LIMITSIZE; i++){
    //     debug("seglist %d : %p\n",i,seglist[i]);       
    // }          
    //    if(debugPointer < heapAddress){
    //     debug("heap %p\n",heapAddress);
    //     debug("fuckyou 1 %p\n",debugPointer);
    //     debug("fuckyou 2 %d\n",debugPointer->size);
    //     debug("fuckyou 3 %p\n",debugPointer->nextNode);
    // }
    debug("alloc start\n");
    if (size == 0)
        return 0;
    if(checkAddress == 0)
    {   
        initAddress = sbrk(0);
        debugPointer = initAddress + 85135;
        debug("debugPointer(%p)\n",debugPointer);
        debug("initAdd(%p)\n",initAddress);  
        checkAddress = 1;
    }  

    if(init == 0){
        seglistInit();
        init = 1;
    } 

    size_t  blockSize = 0;
    if(size < 16){
        blockSize = 16;
    }else{
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
    debug("realloc Start (%p)\n",ptr);
    void *p = NULL;
    myfree(ptr);
    size_t  blockSize = 0;
    if(size < 16){
        blockSize = 16;
    }else{
        blockSize = size;
    }
    blockSize += 16;       
    p = allocateFreeList(blockSize);
    debug("realloc size(%d) %p\n ",size,p);
    
    return p;

}

void myfree(void *ptr)
{
    // for(int i = 0; i < LIMITSIZE; i++){
    //     debug("seglist %d : %p\n",i,seglist[i]);       
    // // }   
   
    // if(debugPointer < heapAddress){
    //     debug("heap %p\n",heapAddress);
    //     debug("fuckyou 1 %p\n",debugPointer);
    //     debug("fuckyou 2 %d\n",debugPointer->size);
    //     debug("fuckyou 3 %p\n",debugPointer->nextNode);
    // }   

    if(ptr < initAddress ){
        return;
    }
    if(heapAddress <= ptr){
        return;
    }    

    tempFree(ptr);
    debug("free %p \n",ptr);    


}
