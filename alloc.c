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
 * Prototype 3 : 구조체 split 및 이동 구현 V
 * Prototype 4 : coalescing 구현 
 *  
***/


// void coalescing(void * p) //coalescing 구현
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
// 32 0 
// 64 1
// 128 2
// 256 3
// 512 4
// 1024 5
// 2048 6
// 4096 7
// 8192 8
// 16384 9
// 32768 10
// 65536 11
// 131072 12
// 262144 13 
// 524288 14
// 1048576 15 
// 2097152 16 
// 4194304 17 

void* allocateFreeList(size_t blockSize)  
{
    size_t initSize = 32;
    int i;
    for(i = 0; i < LIMITSIZE; i++){
        if(initSize <= blockSize && blockSize < initSize<<1)
            break;    
        initSize <<= 1;            
            
    }
    blockSize&32;
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
            t->s = (blockSize<<1)+1;
            t = p+blockSize-BLOCKSIZE;
            t->s = (blockSize<<1)+1;
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
            t->s = (blockSize<<1)+1;
            
            t = p+blockSize-BLOCKSIZE;
            debug("heap point %p\n",heapAddress);
            debug("debug point %p\n",t);
            t->s = (blockSize<<1)+1;
            
            size_t freeNodeSize = (current->size>>1) - blockSize;
            
            ST_FREELIST* freeNode = p+blockSize;
            freeNode->size = (freeNodeSize<<1);
            freeNode = p+blockSize+freeNodeSize-BLOCKSIZE;
            freeNode->size = (freeNodeSize<<1);
            
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
            // debug("freeNode  %p \n",freeNode);
            // debug("current  %p \n",current);
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

}


void *myalloc(size_t size)
{
    // for(int i = 0; i < LIMITSIZE; i++){
    //     debug("seglist %d : %p\n",i,seglist[i]);       
    // }          
       if(debugPointer < heapAddress){
        debug("heap %p\n",heapAddress);
        debug("fuckyou 1 %p\n",debugPointer);
        debug("fuckyou 2 %d\n",debugPointer->size);
        debug("fuckyou 3 %p\n",debugPointer->nextNode);
    }
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
    // }   
   
    if(debugPointer < heapAddress){
        debug("heap %p\n",heapAddress);
        debug("fuckyou 1 %p\n",debugPointer);
        debug("fuckyou 2 %d\n",debugPointer->size);
        debug("fuckyou 3 %p\n",debugPointer->nextNode);
    }   

    if(ptr < initAddress ){
        return;
    }
    if(heapAddress <= ptr){
        return;
    }    

    tempFree(ptr);
    debug("free %p \n",ptr);    


}
