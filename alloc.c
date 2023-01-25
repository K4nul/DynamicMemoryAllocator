#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
/*** 요구사항 
 * 1. 구조체 만들기 (header와 footer 읽을 수 있게)
 *  - 구조체 리스트 header 구조체 (footer와 정보는 같음)
 *  - 몇개 필요한지 조사
 * 2. 구조체를 통해서 앞 뒤 메모리 연결 
 * 3. alloc 구현 
 *  - 가장 적합한 메모리 찾기 Best fit segregated list 구현시 first fit(메모리 효율 > 처리 속도) 
 *  - 메모리 사이즈 입력 
 *  - split 구현
 *  - free 연결 리스트 구조 변경 
 * 4. free 구현 
 *  - free 메모리 정보 입력
 *  - 메모리 정리 앞 뒤 free 영역 합치기  
 *  - free 연결 리스트 추가 
 * 5. realloc 
 *  - 음... 고민중 
 * 6. 상세 구현 alloc 
 *  - sbrk에 대해 알필요가 있음 현재 location? or size 더한 location? 
 *  - sbrk 0 할당? 
 *  - 받은 포인터를 구조체를 통해 정보 입력 앞뒤 size+ 헤더 푸터 
 *  - free 연결리스트 구조체 변경 함수 구현 
 *  - return p;
 * 7. 할당 실패  
 *  - 할당 실패에 대한 예외 처리
 * 8. Segregated List 구현 ( 실패 )
 *  - 각 크기별 free list 구현 
 *  TODO 
 *  - 구현 해야하는 부분 크게 나누기
 *      - split 함수 
 *      - 사이즈 별 free list  linked list (not double)
 *          - 노드 추가 함수 
 *          - 노드 제거 함수 
 *          - split시 남은 부분 이동 
 *          - 검색 함수 
 *  - 구현 해야하는 부분 작게 나누기 세분화 
 *  - 설계 작성
 * allocator 동작 흐름 
 *  - 메모리 사이즈를 받는다 
 *  - 메모리가 없다면 sbrk를 통해 힙영역 공간 추가 (최초 init 이후 공간 할당시 확인 이후 공간 확장 혹은 free list에서 확인)  
 *  - segregated List에서 크기를 비교해 적당한 크기의 공간을 할당 받는다 
 *  - 할당시 헤더에 사이즈 정보와 할당 여부를 넣는다 (explicit free 시에는 안넣어도 될듯)
 * 버퍼오버플로우 시도하는 듯 함
 *  - 아닐수도,,
 *  - 대처할 방법 찾기
 *  - 블록 사이즈 고쳐야함
 *  - 할당 하는 부분 이상함 고쳐야함
***/

#define BLOCKSIZE 4

typedef struct freeList
{
    u_int32_t size;
    struct freeList* prevNode;
    struct freeList* nextNode;
} ST_FREELIST;
typedef struct size
{
    u_int32_t s;
}SIZE;
static ST_FREELIST* freeLists;

static SIZE* debugPointer = NULL;
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
    

        if(currentNode->nextNode == NULL || currentNode->nextNode < initAddress)
        {   
            freeNode->size -= 1;
            u_int32_t tempSize = (freeNode->size>>1) - 4;
            char * tempAdd = freeNode;
            tempAdd += tempSize;
            SIZE * footer = tempAdd;

            footer->s = freeNode->size;
            freeNode->prevNode = currentNode;
            currentNode->nextNode = freeNode;
            freeNode->nextNode = NULL;

            return;
        }
        
        if(freeNode < currentNode)
        {   
            freeNode->size -= 1;
            u_int32_t tempSize = (freeNode->size<<1) - 4;
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

        currentNode = currentNode->nextNode;
            
            
    }
    
}

void moveRoot(ST_FREELIST * current){

    while(1)
    {
        if(current->nextNode == NULL || current->nextNode < initAddress || heapAddress < current->nextNode){
            current->nextNode = NULL;
        }
        if(current->prevNode == NULL || current->prevNode < initAddress || heapAddress < current->prevNode){

            current->prevNode = NULL;
            freeLists = current;
            break;
        }
        
        current = current->prevNode;
    }

}

void coalescing(void * p)
{
    
    debug("coalescing(%p)\n",p);

    if(init == 0){

        freeLists = p-BLOCKSIZE;

        SIZE* temp = p-BLOCKSIZE;
        u_int32_t blockSize = (temp->s>>1);        
        temp->s = blockSize;

        SIZE* footer = p+(blockSize)-4;
        footer->s = blockSize;
        

        freeLists->nextNode = NULL;
        freeLists->prevNode = NULL;

        init = 1;

        return;
    }

      
    ST_FREELIST* tempPtr = (ST_FREELIST*)p;
    
    ST_FREELIST * current = p-BLOCKSIZE;
    u_int32_t * currentA = current;
    u_int32_t currentSize = (current->size>>1);
    ST_FREELIST * prev = p-8; //HEADERLENGTH
    char * oneCurrent = current;
    ST_FREELIST * next = (p-4)+currentSize;

    int nextAllocatedFlag = 0;
    debug("currentSize %d\n",currentSize);
    // debug("heapAdd : %p\n",heapAddress);
    debug("next %p\n",next);
    
    
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
        debug("debug %p\n",current);   
        debug("debug %p\n",prev);   
        SIZE * prevSize = (SIZE*)prev;
        prevAllocatedFlag = prev->size&1;        
        u_int32_t prevS = (prevSize->s>>1); 
        prev = (void*)prev-(void*)(prevS-BLOCKSIZE);
        debug("prevSize %d\n\n",prevS);

    }
        debug("nextAllocatedFlag %p \n",nextAllocatedFlag);
        debug("prevAllocatedFlag %p \n",prevAllocatedFlag);

    if( nextAllocatedFlag== 0){
        u_int32_t nextSize = (next->size>>1);
        
        current->nextNode = next->nextNode;   
        current->prevNode = next->prevNode;
        debug("next size %d \n",next->size);
        if(next->nextNode != NULL || initAddress < next->nextNode){
            debug("debug next->nextNode %p\n",next->nextNode);
            next->nextNode->prevNode = current;
        }
      
        

        if(current->prevNode != NULL){
            debug("next %p\n",next);
            debug("current %p\n",current);
            debug("current->size %d\n",current->size>>1);
            debug("current->prevNode %p\n",current->prevNode);
            debug("current->prevNode->nextNode %p\n",current->prevNode->nextNode);
            current->prevNode->nextNode = current; 
            
        }
        
        
               
        current->size = (current->size>>1) + (nextSize);
        
        u_int32_t* newCurrent = current;      
        u_int32_t s = current->size;
        current->size = (current->size<<1);
        
        newCurrent += (s/4)-1;

        SIZE* footer = newCurrent;
        // debug("next s %d\n",next->size);
        // debug("cur s %d\n",currentSize);
        // debug("cur p %p\n",current);
        // debug("footer s %d\n",s);
        // debug("footer po %p\n",footer);
        
        footer->s = current->size;
        
        moveRoot(current);
        
    }
    

    if( prevAllocatedFlag== 0)
    {   
        debug("debug %p\n",prev);
   
        debug("heapAddr %p\n",heapAddress);
        prev->size =  (current->size>>1) + (prev->size>>1);
        debug("debug %p\n",prev->size);  
  
        u_int32_t* newCurrent = prev; 
        u_int32_t s = (prev->size>>1);
        newCurrent += (s/4)-1;     
        SIZE* footer = newCurrent;
        
  
  
        footer->s = (prev->size>>1);
 

        moveRoot(prev);
    }
    
    if(prevAllocatedFlag == 1 && nextAllocatedFlag == 1)
    {
    
        addFreeNode(current);
        moveRoot(current);
    }




}


void* allocateFreeList(size_t blockSize)  
{
    debug("allocate\n");



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
        debug("debug cSize %d \n",cSize);
        if((int)blockSize < cSize-24)
        {

            
            ST_FREELIST* tempPrev = current->prevNode;

            ST_FREELIST* tempNext = current->nextNode;

            u_int32_t tempSize = current->size>>1;
            tempSize = tempSize - blockSize;

            u_int32_t* temp = current;
            debug("cur size %d\n",current->size);
            
            *temp = (blockSize<<1)+1;
                   
            char * tempAdd = temp;

            tempAdd+= (blockSize)-BLOCKSIZE;

            temp = tempAdd;   
                        debug("curent %p\n",current);
            debug("temp %p\n",temp);
            // debug("c a %p\n",current);     
            debug("c s %d\n",cSize);     
            debug("b s %d\n",blockSize);     
            // debug("temp add %p\n",tempAdd);     

            

            *temp = (blockSize<<1)+1;

            u_int32_t * tempType = temp;

            ST_FREELIST* freeNode = tempType+1;
            freeNode->prevNode = tempPrev;
            

            freeNode->nextNode = tempNext;

     
            
            freeNode->size = (tempSize<<1);
            
            void * returnAddress = current;

            if(tempPrev != NULL){
                
                tempPrev->nextNode = freeNode;

            }
            // debug("debug freeNode %p\n",freeNode);
            debug("debug block %d\n",blockSize);
            if(tempNext < heapAddress && initAddress <= tempNext){
                
                tempNext->prevNode = freeNode;

            }
       
            debug("freeNode %p\n",freeNode);
            u_int32_t *footerTemp = freeNode;
            SIZE* freeNodeFooter = footerTemp+((tempSize>>1)/4)-1;
            // debug("heapAddress %p\n",heapAddress);
            // debug("debug currentSize %d \n",current->size);
            // debug("debug b s %d \n",blockSize);
            // debug("debug freeNodeFooter %p \n",freeNodeFooter);
            // debug("debug tempSize %d \n",tempSize);
            freeNodeFooter->s = (tempSize>>1);
            
            debug("freeNodeFooter %p\n",freeNodeFooter);

            moveRoot(freeNode);
            
            return returnAddress+BLOCKSIZE;
        }

        if(blockSize == (current->size>>1)) 
        {
            
            // debug("Debug c p n %p\n",current->prevNode->nextNode);
            // debug("Debug c n p %p\n",current->nextNode);
            if(current->prevNode != NULL && initAddress < current->prevNode ){
                current->prevNode->nextNode = current->nextNode;
            }
            if(current->nextNode != NULL && current->prevNode < heapAddress){
                current->nextNode->prevNode = current->prevNode; 
            }

            
            
            u_int32_t* temp = current;

            *temp = (blockSize<<1)+1;
            temp+= ((blockSize<<1)-1)/BLOCKSIZE;

            *temp = (blockSize<<1)+1;

            temp += 1;
            
            u_int32_t* returnAddr = current;
            returnAddr+= 1;
            
            
            if(current->prevNode != NULL && initAddress < current->prevNode ){
                
                moveRoot(current->prevNode);
            }
            if(current->nextNode != NULL && current->prevNode < heapAddress ){
                
                moveRoot(current->nextNode);
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
            // debug("blockSize %d\n",t->s);
            t = p+blockSize-BLOCKSIZE;

            t->s = (blockSize<<1)+1;
            // debug("blockSize %d\n",t->s);
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
        debugPointer = initAddress + 18388;
        debug("debugPointer(%p)\n",debugPointer);
        debug("initAdd(%p)\n",initAddress);  
        checkAddress = 1;
    }    

    if(debugPointer < heapAddress){
        debug("heap %p\n",heapAddress);
        debug("fuckyou 1 %p\n",debugPointer);
        debug("fuckyou 2 %d\n",debugPointer->s);
    }
    size_t  blockSize = 0;
    if(size < 16){
        blockSize = 16;
    }else{
        blockSize = size;
    }

    blockSize +=  BLOCKSIZE*2;

    void *p = allocateFreeList(blockSize);


    int * memSize = p;
    debug("alloc(%u): %p\n", (unsigned int)size, p);
    max_size += size;
    debug("max: %u\n", max_size);
    debug("heap : %p\n",heapAddress);
    // if (freeLists != NULL)
    // {
    
    
    //     ST_FREELIST* currentB = freeLists;


    //     debug("debug\n");
    //     while(1){
    //         debug("currentB(%p)\n",currentB);    
    //         if(currentB->nextNode == NULL){
    //             debug("fuck :%p\n\n",currentB->nextNode);
    //             break;
    //         }        
    //         currentB = currentB->nextNode;

    //     }
    //     while(1){
    //         debug("currentP(%p)\n",currentB);
    //         if(currentB->prevNode == NULL){
    //             debug("fuckP :%p\n\n",currentB->prevNode);
    //             break;
    //         }        
    //         currentB = currentB->prevNode;
    //     }
    // }    

    return p;
}

void *myrealloc(void *ptr, size_t size)
{
    
    // void *p = NULL;

    // myfree(ptr);
    // size_t blockSize = size * BLOCKSIZE;
    // blockSize += 16;    
    // p = allocateFreeList(blockSize);
    // debug("realloc(%p, %u): %p\n", ptr, (unsigned int)size, p);
    
    // return p;
    void *p = NULL;
    if (size != 0)
    {
        p = sbrk(size);
        if (ptr)
            memcpy(p, ptr, size);
        max_size += size;
        debug("max: %u\n", max_size);
    }
    debug("realloc(%p, %u): %p\n", ptr, (unsigned int)size, p);

    return p;
}

void myfree(void *ptr)
{
    debug("free strat (%p)\n", ptr);      
       if(debugPointer < heapAddress){
        debug("heap %p\n",heapAddress);
        debug("fuckyou 1 %p\n",debugPointer);
        debug("fuckyou 2 %d\n",debugPointer->s);
    }  
    // if (freeLists != NULL)
    // {
    
    
    //     ST_FREELIST* currentB = freeLists;


    //     debug("debug\n");
    //     while(1){
    //         debug("currentB(%p)\n",currentB);    
    //         if(currentB->nextNode == NULL){
    //             debug("fuck :%p\n\n",currentB->nextNode);
    //             break;
    //         }        
    //         currentB = currentB->nextNode;

    //     }
    //     while(1){
    //         debug("currentP(%p)\n",currentB);
    //         if(currentB->prevNode == NULL){
    //             debug("fuckP :%p\n\n",currentB->prevNode);
    //             break;
    //         }        
    //         currentB = currentB->prevNode;
    //     }
    // }    
    
    if(ptr < initAddress ){
        return;
    }
    if(heapAddress <= ptr){
        return;
    }    
    
    coalescing(ptr);
    debug("free(%p)\n", ptr);      






}
