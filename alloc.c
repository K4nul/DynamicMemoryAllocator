#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define BLOCKSIZE 8
#define LIMITSIZE 20

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
 * Prototype 3 : 구조체 split 및 이동 구현 
 * Prototype 4 : coalescing 구현 
 *  
***/


// void coalescing(void * p)
// {
    
//     if(init == 0){

//         freeLists = p-BLOCKSIZE;

//         SIZE* temp = p-BLOCKSIZE;
//         size_t blockSize = (temp->s>>1);        
//         temp->s = blockSize;

//         SIZE* footer = p+(blockSize)-8;
//         footer->s = blockSize;
        

//         freeLists->nextNode = NULL;
//         freeLists->prevNode = NULL;

//         init = 1;

//         return;
//     }

      
//     ST_FREELIST* tempPtr = (ST_FREELIST*)p;
    
//     ST_FREELIST * current = p-BLOCKSIZE;
//     size_t currentSize = (current->size>>1);
//     ST_FREELIST * prev = p-16; //HEADERLENGTH
//     ST_FREELIST * next = (p-8)+currentSize;

//     ST_FREELIST * debugfooter = (p-16)+currentSize;

//     int nextAllocatedFlag = 0;
    
  
    
//     if(heapAddress <= (void*)next || (void*)next < initAddress)
//     {
//         nextAllocatedFlag = 1;
//     }
//     else
//     {

//         nextAllocatedFlag = next->size&1;
//     }


//     int prevAllocatedFlag = 0;
          

//     if((void*)prev < initAddress){

//         prevAllocatedFlag = 1;
//     }
//     else
//     {
        
//         SIZE * prevSize = (SIZE*)prev;
//         prevAllocatedFlag = prevSize->s&1;       
//         size_t prevS = (prevSize->s>>1); 
//         prev = (void*)prev-(void*)(prevS-BLOCKSIZE);


//     }
        

//     if( nextAllocatedFlag== 0){
        
//         size_t nextSize = (next->size>>1);

//         current->nextNode = next->nextNode;   
//         current->prevNode = next->prevNode;
//         if(next->nextNode != NULL || initAddress < next->nextNode){
//             next->nextNode->prevNode = current;
            
//         }
        
    

//         if(current->prevNode != NULL){
//             current->prevNode->nextNode = current; 
            
//         }
        
//         currentSize = (current->size>>1)+(nextSize);
               
//         current->size = currentSize;
        
//         char* newCurrent = current;      
//         size_t s = current->size;
//         current->size = (current->size<<1);
//         newCurrent += s-8;

//         SIZE* footer = newCurrent;


//         footer->s = current->size;
        
//         if(current < freeLists){
//             freeLists = current;
//         }
        
//     }
    

//     if( prevAllocatedFlag== 0)
//     {   

//         if(prev->nextNode == current){
//             prev->nextNode = current->nextNode;
//             if(current->nextNode != NULL)
//                 current->nextNode->prevNode = prev;
//         }
//         prev->size =  (current->size>>1) + (prev->size>>1);
  
//         size_t* newCurrent = prev; 
        
//         size_t s = (prev->size);
//         char * prevTemp = prev;
//         newCurrent += (s/BLOCKSIZE)-1;
//         prevTemp += (s-BLOCKSIZE);
        
//         SIZE* footer = prevTemp;     
//         footer->s = (prev->size<<1); 
//         prev->size = (prev->size<<1);
//         if(prev < freeLists){
//             freeLists = prev;
//         }
//     }
    
//     if(prevAllocatedFlag == 1 && nextAllocatedFlag == 1)
//     {
        
//         SIZE * t = current;
        
//         addFreeNode(current);
        
//         if(current < freeLists){
//             freeLists = current;
//         }
//     }

// }

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
        initSize <<=1;
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
        initSize <<= 1;            
            
    }
    void * p;
    ST_FREELIST* current = seglist[i];
    ST_FREELIST* prev = NULL;
    while(1){
        if(current == NULL)
        {
            
            p = sbrk(blockSize);

            heapAddress = p+blockSize;
            SIZE* t = p;
            t->s = (blockSize<<1)+1;
    
            t = p+blockSize-BLOCKSIZE;


            t->s = (blockSize<<1)+1;
        
            return p+BLOCKSIZE;        
        }
        if(blockSize == (current->size>>1))
        {
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
            return p+BLOCKSIZE;
            break;
        }
        if(blockSize < (current->size>>1)-32)
        {
            //split 구현 및 seglist에 저장 
            break;
        }

        prev = current; 
        current = current->nextNode; 
        
    }
 


}

void seglistInit(){
    for(int i = 0; i < LIMITSIZE; i++){
            seglist[i] = NULL;       
    }

}


void *myalloc(size_t size)
{
     
    debug("alloc start\n");
    if (size == 0)
        return 0;
    if(checkAddress == 0)
    {   
        initAddress = sbrk(0);
        debugPointer = initAddress + 568;
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

    if(ptr < initAddress ){
        return;
    }
    if(heapAddress <= ptr){
        return;
    }    

    tempFree(ptr);
    debug("free %p \n",ptr);    


}
