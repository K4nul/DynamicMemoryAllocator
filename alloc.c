#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define BLOCKSIZE 8
#define LIMITSIZE 18
#define INITSIZE 32
#define DOUBLE(X) X<<=1 
#define SIZEINPUT(X,Y,Z) X=(Y<<1)+Z
#define MOVEPOINTER(P,X,Y) (P)=(X)+(Y) 
#define MOVENEXT(P,X,Y) (P)=(X)+(Y)-BLOCKSIZE
#define MOVEPREV(P,X,Y) (P)=(X)-(Y)-BLOCKSIZE
#define MOVEPREVFOOTER(P,X) (P)=(X)-(2*BLOCKSIZE)
#define MOVEFOOTER(P,X,Y) (P)=(X)+((Y)-(BLOCKSIZE))
#define OR(X,Y) (X)|=(X)>>(Y)
#define GETMSB(X) (X) = (X+1)>>1

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

ST_FREELIST *seglist[LIMITSIZE];
static int checkAddress = 0;
static void* initAddress;
static void* heapAddress;
extern void debug(const char *fmt, ...);
extern void *sbrk(intptr_t increment);

// unsigned int max_size;





void coalescing(void * p)
{
    
    ST_FREELIST * current = p-BLOCKSIZE;
    size_t currentSize = (current->size>>1);
    ST_FREELIST * prevFooter;
    ST_FREELIST * next;
    ST_FREELIST* prev;
    ST_FREELIST* curFooter; 
    
    MOVEFOOTER(curFooter,p-BLOCKSIZE,currentSize);
    SIZEINPUT(current->size,currentSize,0);
    SIZEINPUT(curFooter->size,currentSize,0);
    MOVEPREVFOOTER(prevFooter,p);
    MOVENEXT(next,p,currentSize);
    int nextAllocatedFlag = 0;
    int prevAllocatedFlag = 0;    
    size_t seglistSize;  
    

     
    if(heapAddress <= (void*)next)
    {
        nextAllocatedFlag = 1;
    }
    else
    {
        
        nextAllocatedFlag = next->size&1;
    }


    if((void*)prevFooter < initAddress)
    {

        prevAllocatedFlag = 1;
    }
    else
    {
     
        prevAllocatedFlag = prevFooter->size&1;       
        size_t prevSize = (prevFooter->size>>1); 
        MOVEPREV(prev,p,prevSize);


    }


    if( nextAllocatedFlag== 0)
    {
        
        
        size_t nextSize = (next->size>>1);


        if(next->nextNode != NULL)
        {

            next->nextNode->prevNode = next->prevNode;
            
        }
        
    

        if(next->prevNode != NULL)
        {
            next->prevNode->nextNode = next->nextNode; 
            
        }
        
        
        if(next->prevNode == NULL)
        {
            int i = 0; 
            size_t initSize = INITSIZE;
            for(i = 0; i <LIMITSIZE;i++)
            {
                if(initSize <= nextSize && nextSize < initSize<<1)
                {
                    break;
                }
                DOUBLE(initSize);
            }
            if(next->nextNode == NULL)
            {

                seglist[i] = NULL;
                
            }else
            {
                seglist[i] = seglist[i]->nextNode;
            }
            
            
        }

        
        currentSize = (current->size>>1)+(nextSize);
        
        void* vCurrent = current;      
        current->size = (currentSize<<1);
        SIZE* footer;
        MOVEFOOTER(footer,vCurrent,currentSize);
        footer->s = current->size;
        
        seglistSize = currentSize;
        
    }
    

    if( prevAllocatedFlag== 0)
    {   

        
        if(prev->nextNode != NULL)
        {

            prev->nextNode->prevNode = prev->prevNode;
            
        }

        if(prev->prevNode != NULL)
        {

            prev->prevNode->nextNode = prev->nextNode; 
            
        }
        
        if(prev->prevNode == NULL)
        {
            int i = 0; 
            size_t initSize = INITSIZE;
            for(i = 0; i <LIMITSIZE;i++)
            {
                if(initSize <= prev->size>>1 && prev->size>>1 < initSize<<1)
                {
                    break;
                }
                DOUBLE(initSize);
            }
            
            if(prev->nextNode == NULL)
            {
                seglist[i] = NULL;
            }else 
            {
                seglist[i] = seglist[i]->nextNode;
            }
        }
        if(prev->nextNode == current)
        {
            prev->nextNode = current->nextNode;
            if(current->nextNode != NULL)
                current->nextNode->prevNode = prev;
        }
        size_t prevSize = (current->size>>1) + (prev->size>>1); 
  ; 
        void* vPrev = prev;
        ST_FREELIST * prevFooter;
        MOVEFOOTER(prevFooter,vPrev,prevSize);
        SIZEINPUT(prev->size,prevSize,0);
        SIZEINPUT(prevFooter->size,prevSize,0);     
        seglistSize = prevSize;

        current = prev;
    }
    
    if(prevAllocatedFlag == 1 && nextAllocatedFlag == 1)
    {
        seglistSize = currentSize;
    }


    size_t initSize = INITSIZE;
    int i = 0;

    
    for(i = 0; i <LIMITSIZE; i++)
    {
        if(initSize <= seglistSize && seglistSize < initSize<<1)
        {
            break;
        }
            

        DOUBLE(initSize);
    } 
    
       
        


    

    if(seglist[i] == NULL)
    {
        seglist[i] = current;
        seglist[i]->nextNode = NULL;
        seglist[i]->prevNode = NULL;
        return;
    }
    ST_FREELIST* temp = seglist[i];        
    seglist[i] = current;
    seglist[i]->nextNode = temp;
    seglist[i]->prevNode = NULL;
    temp->prevNode = current;

    return;





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
            {
                DOUBLE(initSize);      
                continue;
            }

            break;  
            
        }
        DOUBLE(initSize);         
          
    }
    if(i == 18)
    {
        p = sbrk(blockSize);
        heapAddress = p+blockSize;
        SIZE* t = p;
        
        SIZEINPUT(t->s,blockSize,1);       
        MOVEFOOTER(t,p,blockSize);
        SIZEINPUT(t->s,blockSize,1);


        return p+BLOCKSIZE;
    }

    ST_FREELIST* current = seglist[i];

    int cSize = current->size>>1;
        
    if((int)blockSize < cSize-32)
    {
             
        
        ST_FREELIST* tempPrev = current->prevNode;
        ST_FREELIST* tempNext = current->nextNode;
        size_t freeNodeSize = current->size>>1;
        freeNodeSize = freeNodeSize - blockSize;
        size_t* temp = current;    
        *temp = (blockSize<<1)+1;             
        char * tempAdd = temp;
        tempAdd+= (blockSize)-8;
        temp = tempAdd;   
        *temp = (blockSize<<1)+1;
        void * tempType = temp;
        ST_FREELIST* freeNode = tempType+BLOCKSIZE;
 
        freeNode->size = (freeNodeSize<<1);

        void * returnAddress = current;


        
        if(tempPrev != NULL)
        {
            
            
            tempPrev->nextNode = tempNext;
        }
        

        if(tempNext != NULL)
        {
                
            tempNext->prevNode = tempPrev;
        }

        
        char *footerTemp = freeNode;
        
        ST_FREELIST* freeNodeFooter = footerTemp+(freeNodeSize-8);
        
        freeNodeFooter->size = (freeNodeSize<<1); 
        int x;
        size_t is = INITSIZE;
        size_t operTemp = freeNodeSize;

        for(x = 0; x <LIMITSIZE; x++)
        {

            if( is <= freeNodeSize && freeNodeSize < is<<1)
            {
                break;
            }
                        
            is <<=1;
        }

        seglist[i] = seglist[i]->nextNode;        
        
        if(seglist[x] == NULL)
        {
            seglist[x] = freeNode;
            freeNode->nextNode = NULL;
            freeNode->prevNode = NULL;
            
            return returnAddress+BLOCKSIZE;
        }
        
        

  
        freeNode->prevNode = NULL;
        freeNode->nextNode = seglist[x];
        seglist[x]->prevNode = freeNode;
        seglist[x] = freeNode;
        

        return returnAddress+BLOCKSIZE;
    
    }

    if(blockSize == cSize || blockSize < cSize) 
    {
        
        if(current->prevNode != NULL){
            current->prevNode->nextNode = current->nextNode;
        }
        if(current->nextNode != NULL){
            current->nextNode->prevNode = current->prevNode; 
        }

            
            
        ST_FREELIST* temp = current;

        temp->size += 1;
        char* tempTemp = temp;
        tempTemp+= ((temp->size>>1)-8);
            
        temp = tempTemp;
        temp->size += 1;
        size_t* returnAddr = current;
        returnAddr+= 1;
            
            
        seglist[i] = current->nextNode;
                
            
        if(current->nextNode == NULL && current->prevNode == NULL)
        {

            seglist[i] = NULL;
        }
            
        return returnAddr;
            
    }
    

}





void *myalloc(size_t size)
{   

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

    // int * memSize = p;
    // max_size += size;

    return p;
}

void *myrealloc(void *ptr, size_t size)
{
    
    void *p = NULL;
    myfree(ptr);
    size_t blockSize;
    if(size < 16){
        blockSize = 16;
    }else{
        blockSize = size;
    }    
    blockSize += BLOCKSIZE*2;    
    p = allocateFreeList(blockSize);
    return p;

}

void myfree(void *ptr)
{

    if(ptr == NULL ){
        return;
    }  

    coalescing(ptr);      


}