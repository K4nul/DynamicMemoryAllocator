#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define BLOCKSIZE 8


typedef struct freeList
{
    size_t size;
    struct freeList* prevNode;
    struct freeList* nextNode;
} ST_FREELIST;
typedef struct size
{
    size_t s;
}SIZE;
static ST_FREELIST* freeLists;

static ST_FREELIST* debugPointer = NULL;
static int init=0;
static void* initAddress;
static int checkAddress = 0;
static void* heapAddress;
extern void debug(const char *fmt, ...);
extern void *sbrk(intptr_t increment);

unsigned int max_size;



void addFreeNode(ST_FREELIST* freeNode)
{

    ST_FREELIST* currentNode = freeLists;
    while(1)
    {
        if(freeNode < currentNode)
        {   
            
            freeNode->size -= 1;
            size_t tempSize = (freeNode->size>>1) - 8;
            char * tempAdd = freeNode;
            tempAdd += tempSize;
            SIZE * footer = tempAdd;

            footer->s = freeNode->size;
            freeNode->nextNode = currentNode;
            freeNode->prevNode = currentNode->prevNode;
            currentNode->prevNode = freeNode;

            
            if(freeNode->prevNode != NULL || initAddress < freeNode->prevNode){
                freeNode->prevNode->nextNode = freeNode;

            }
            return;
        }    

        if(currentNode->nextNode == NULL || currentNode->nextNode < initAddress)
        {   
            
            freeNode->size -= 1;
            size_t tempSize = (freeNode->size>>1) - 8;
            char * tempAdd = freeNode;
            tempAdd += tempSize;
            SIZE * footer = tempAdd;

            footer->s = freeNode->size;
            
            freeNode->prevNode = currentNode;
            
            currentNode->nextNode = freeNode;
            freeNode->nextNode = NULL;

            return;
        }
        


        currentNode = currentNode->nextNode;
            
            
    }
    
}


void coalescing(void * p)
{
    
    if(init == 0){

        freeLists = p-BLOCKSIZE;

        SIZE* temp = p-BLOCKSIZE;
        size_t blockSize = (temp->s>>1);        
        temp->s = blockSize;

        SIZE* footer = p+(blockSize)-8;
        footer->s = blockSize;
        

        freeLists->nextNode = NULL;
        freeLists->prevNode = NULL;

        init = 1;

        return;
    }

      
    ST_FREELIST* tempPtr = (ST_FREELIST*)p;
    
    ST_FREELIST * current = p-BLOCKSIZE;
    size_t currentSize = (current->size>>1);
    ST_FREELIST * prev = p-16; //HEADERLENGTH
    ST_FREELIST * next = (p-8)+currentSize;

    ST_FREELIST * debugfooter = (p-16)+currentSize;

    int nextAllocatedFlag = 0;
    
  
    
    if(heapAddress <= (void*)next || (void*)next < initAddress)
    {
        nextAllocatedFlag = 1;
    }
    else
    {

        nextAllocatedFlag = next->size&1;
    }


    int prevAllocatedFlag = 0;
          

    if((void*)prev < initAddress){

        prevAllocatedFlag = 1;
    }
    else
    {
        
        SIZE * prevSize = (SIZE*)prev;
        prevAllocatedFlag = prevSize->s&1;       
        size_t prevS = (prevSize->s>>1); 
        prev = (void*)prev-(void*)(prevS-BLOCKSIZE);


    }
        

    if( nextAllocatedFlag== 0){
        
        size_t nextSize = (next->size>>1);

        current->nextNode = next->nextNode;   
        current->prevNode = next->prevNode;
        if(next->nextNode != NULL || initAddress < next->nextNode){
            next->nextNode->prevNode = current;
            
        }
        
    

        if(current->prevNode != NULL){
            current->prevNode->nextNode = current; 
            
        }
        
        currentSize = (current->size>>1)+(nextSize);
               
        current->size = currentSize;
        
        char* newCurrent = current;      
        size_t s = current->size;
        current->size = (current->size<<1);
        newCurrent += s-8;

        SIZE* footer = newCurrent;


        footer->s = current->size;
        
        if(current < freeLists){
            freeLists = current;
        }
        
    }
    

    if( prevAllocatedFlag== 0)
    {   

        if(prev->nextNode == current){
            prev->nextNode = current->nextNode;
            if(current->nextNode != NULL)
                current->nextNode->prevNode = prev;
        }
        prev->size =  (current->size>>1) + (prev->size>>1);
  
        size_t* newCurrent = prev; 
        
        size_t s = (prev->size);
        char * prevTemp = prev;
        newCurrent += (s/BLOCKSIZE)-1;
        prevTemp += (s-BLOCKSIZE);
        
        SIZE* footer = prevTemp;     
        footer->s = (prev->size<<1); 
        prev->size = (prev->size<<1);
        if(prev < freeLists){
            freeLists = prev;
        }
    }
    
    if(prevAllocatedFlag == 1 && nextAllocatedFlag == 1)
    {
        
        SIZE * t = current;
        
        addFreeNode(current);
        
        if(current < freeLists){
            freeLists = current;
        }
    }




}

void* allocateFreeList(size_t blockSize)  
{


    if(init == 0){
        void* p = sbrk(blockSize);

        heapAddress = p+blockSize;
        SIZE* t = p;
        t->s = (blockSize<<1)+1;
    
        t = p+blockSize-BLOCKSIZE;


        t->s = (blockSize<<1)+1;
        
        return p+BLOCKSIZE;

    }

    ST_FREELIST* current = freeLists;
    
    while(1)
    {

        int cSize = current->size>>1;
        // debug("blocSize %d\n",blockSize);
        // debug("cSize %d\n",cSize);
        // debug("freeLists %p\n",freeLists);
        if((int)blockSize < cSize-32)
        {
            
            
            ST_FREELIST* tempPrev = current->prevNode;

            ST_FREELIST* tempNext = current->nextNode;

            size_t tempSize = current->size>>1;
            tempSize = tempSize - blockSize;

            size_t* temp = current;
            
            
            *temp = (blockSize<<1)+1;             
            char * tempAdd = temp;

            tempAdd+= (blockSize)-8;

            temp = tempAdd;   


            

            *temp = (blockSize<<1)+1;

            size_t * tempType = temp;

            ST_FREELIST* freeNode = tempType+1;
            
            freeNode->prevNode = tempPrev;

            freeNode->nextNode = tempNext;

     
            
            freeNode->size = (tempSize<<1);
            
            void * returnAddress = current;

            if(tempPrev != NULL){
                
                tempPrev->nextNode = freeNode;

            }
            if(tempNext < heapAddress && initAddress <= tempNext){
                
                tempNext->prevNode = freeNode;
            }
            char *footerTemp = freeNode;
            ST_FREELIST* freeNodeFooter = footerTemp+(tempSize-8);
    
        

            freeNodeFooter->size = (tempSize<<1); 
            // debug("freeNode->prev %p\n",freeNode->prevNode);
            // debug("freeNode %p\n",freeNode);
            if(freeNode->prevNode < initAddress){
                freeLists = freeNode;
            }            
            return returnAddress+BLOCKSIZE;
        }

        if(blockSize == (current->size>>1)) 
        {

            if(current->prevNode != NULL && initAddress < current->prevNode ){
                current->prevNode->nextNode = current->nextNode;
            }
            if(current->nextNode != NULL && current->prevNode < heapAddress){
                current->nextNode->prevNode = current->prevNode; 
            }

            
            
            ST_FREELIST* temp = current;

            temp->size = (blockSize<<1)+1;
            char* tempTemp = temp;
            tempTemp+= ((blockSize)-8);
            
            temp = tempTemp;
            temp->size = (blockSize<<1)+1;
            size_t* returnAddr = current;
            returnAddr+= 1;
            
            
            if(current->prevNode != NULL && initAddress < current->prevNode ){
                
                if(current->prevNode->prevNode < initAddress){
                    freeLists = current->prevNode;
                }
                
            }
            
            if(current->nextNode != NULL && current->prevNode < heapAddress ){
                
                if(current->nextNode->prevNode < initAddress){
                    freeLists = current->nextNode;
                }
                
            }
            
            if(current->nextNode == NULL && current->prevNode == NULL){

                freeLists = NULL;
                init = 0;
            }
            
            return returnAddr;
            
        }

        if(current->nextNode == NULL || current->nextNode < initAddress){
            
            void* p = sbrk(blockSize);
            heapAddress = p+blockSize;   
            SIZE* t = p;
            t->s = (blockSize<<1)+1;
            t = p+blockSize-BLOCKSIZE;
            
            t->s = (blockSize<<1)+1;

            
            return p+BLOCKSIZE;
        }


        current = current->nextNode;
    }


}


void *myalloc(size_t size)
{
    if (size == 0)
        return 0;
    if(checkAddress == 0)
    {   
        
        initAddress = sbrk(0);
        checkAddress = 1;
    }    

    size_t  blockSize = 0;
    if(size < 16){
        blockSize = 16;
    }else{
        blockSize = size;
    }

    blockSize +=  BLOCKSIZE*2;

    void *p = allocateFreeList(blockSize);

    // debug("alloc (%p) size:(%x)\n",p,size);
    int * memSize = p;
    max_size += size;

    return p;
}

void *myrealloc(void *ptr, size_t size)
{
    
    void *p = NULL;
    myfree(ptr);
    size_t blockSize = size * BLOCKSIZE;
    blockSize += 16;    
    p = allocateFreeList(blockSize);
    // debug("realloc %p\n",p);
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

    coalescing(ptr);      


}
