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
 *  - split 구현?
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
 * 8. Segregated List 구현 
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
***/

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

    // debug("addFreeNode\n");    
    ST_FREELIST* currentNode = freeLists;
    while(1)
    {
    
        // debug("next Node? %p\n",currentNode->nextNode);
        if(currentNode->nextNode == NULL || currentNode->nextNode < initAddress)
        {   
            // debug("넥스트 널\n");
            freeNode->prevNode = currentNode;
            // debug("current %p\n",currentNode);
            // debug("prevNode %p\n",freeNode->prevNode);
            currentNode->nextNode = freeNode;
            freeNode->nextNode = NULL;
            // debug("freeNode %p\n",freeNode);
            return;
        }
        
        if(freeNode < currentNode)
        {   
            // debug("f (%p)\n\n",freeNode);
            // debug("작을 때\n");
            freeNode->nextNode = currentNode;
              
            freeNode->prevNode = currentNode->prevNode;
            
            currentNode->prevNode = freeNode;
            // debug("f next(%p)\n\n",freeNode->nextNode);
            // debug("f prev(%p)\n\n",freeNode->prevNode);
            // debug("c prev(%p)\n\n",currentNode->prevNode);
            
            if(freeNode->prevNode != NULL || initAddress < freeNode->prevNode){
                freeNode->prevNode->nextNode = freeNode;
                // debug("f p next(%p)\n\n",freeNode->prevNode->nextNode);
            }
            return;
        }

        currentNode = currentNode->nextNode;
            
            
    }
    
}

void moveRoot(ST_FREELIST * current){
    // debug("moveRoot");
    while(1)
    {
        // debug("current %p\n",current);
        if(current->prevNode == NULL || current->prevNode < initAddress || heapAddress < current->prevNode){
            // debug("root %p\n",current);
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
        // debug("first Lists debug(%p)\n",freeLists);
        SIZE* temp = p-BLOCKSIZE;
        temp->s -=1;
        size_t blockSize = temp->s;
        SIZE* footer = p+(blockSize)-8;
        footer->s -=1;
        
        // debug("init debug(%p)\n",temp);
        // debug("init debug(%p)\n",footer);
        freeLists->nextNode = NULL;
        freeLists->prevNode = NULL;
        // debug("temp->s size(%d)\n\n",temp->s);
        // debug("footer->s size(%d)\n\n",footer->s);
        // debug("init next(%p)\n",freeLists->nextNode);
        // debug("init prev(%p)\n\n",freeLists->prevNode);
        init = 1;

        return;
    }
    
    // debug("coalescing\n");  
    // debug("debug(%p)\n",p);      
    ST_FREELIST* tempPtr = (ST_FREELIST*)p;
    
    ST_FREELIST * current = p-BLOCKSIZE;
    size_t currentSize = current->size&-2;
    ST_FREELIST * prev = p-16; //HEADERLENGTH
    
    ST_FREELIST * next = (p-BLOCKSIZE+currentSize);
    // debug("next(%p)\n\n",next); 
    // debug("heap(%p)\n\n",heapAddress); 
    // debug("c size(%d)\n\n",currentSize); 
    // debug("c size(%d)\n\n",next->size); 

    
    // debug("coalescing debug(%d)\n",prev->size);
    // debug("current(%p)\n",current);
    // debug("cuSiz(%d)\n",current->size&-2);
    // debug("next(%p)\n",next);
    int nextAllocatedFlag = 0;
    debug("currentSize %d\n",currentSize);
    debug("nextSize %d\n",next->size);
    debug("heapAdd : %p\n",heapAddress);

    if(heapAddress <= (void*)next || (void*)next < initAddress)
    {
        nextAllocatedFlag = 1;
    }
    else
    {

        nextAllocatedFlag = next->size&1;
    }

    // debug("coalescing debug(%d)\n",current->size);
    // debug("coalescing debug(%p)\n",p);
    // debug("coalescing debug(%p)\n",current);
    // debug("coalescing debug(%p)\n",prev);
    // debug("coalescing debug(%p)\n",next);  
    int prevAllocatedFlag = 0;

    // debug("prev(%p)\n",prev);
    // debug("init(%p)\n",initAddress);
    
    if((void*)prev < initAddress){

        prevAllocatedFlag = 1;
    }
    else
    {
        
        // debug("fprev(%p)\n",prev);
        SIZE * prevSize = (SIZE*)prev;
        prevAllocatedFlag = prev->size&1;        
        size_t prevS = prevSize->s&-2;
        // debug("prevSize(%d)\n",prevS);
        prev = (void*)prev-(void*)(prevS-BLOCKSIZE);
        // debug("hprev(%p)\n",prev);
        // debug("p size(%d)\n\n",prev->size); 

    }
   
    
    // debug("current size(%d)\n",current->size);
    // debug("coalescing debug(%p)\n",current);    
    // debug("prev size(%d)\n",prev->size);
    // debug("next size(%d)\n",next->size);
    // debug("nextAllocatedFlag size(%d)\n",nextAllocatedFlag);
    // debug("prevAllocatoedFlag size(%d)\n\n",prev->size);    
    // debug("heap(%p)\n",sbrk(0));
    // debug("next(%p)\n",next);
    // debug("next next(%p)\n",next->nextNode);
    // debug("next prev(%p)\n",next->prevNode);
    // debug("nextAllocatoedFlag size(%d)\n\n",nextAllocatedFlag); 
    // debug("prevAllocatoedFlag size(%d)\n\n",prevAllocatedFlag); 
  
    if( nextAllocatedFlag== 0){
        size_t nextSize = next->size;
        current->nextNode = next->nextNode;   
        current->prevNode = next->prevNode;
        debug("next %p\n",next);
        // debug("current prev %p\n",current->prevNode);
        debug("current Next %p\n",current->nextNode);
        debug("current prev %p\n\n",current->prevNode);
        if(next->nextNode != NULL || initAddress < next->nextNode){
         
            next->nextNode->prevNode = current;
        }
      

        // debug("debug nextNode1 (%p)\n",next->nextNode);
        // debug("debug prevNode1 (%p)\n",current->prevNode);
        // debug("debug current (%p)\n",current);
        // debug("current (%p)\n\n",current); 
        // debug("k size(%d)\n\n",nextSize);      
        // if(prevAllocatedFlag == 0)
        if(current->prevNode != NULL)
            current->prevNode->nextNode = current; 
   
        // debug("NEXT NODE %p\n",next);
        // debug("heapAddr %p\n",heapAddress);
        // debug("current Size %d\n",current->size);
        // debug("NEXT Size %d\n",nextSize);
               
        current->size = (current->size&-2) + (nextSize&-2);
        
        char* newCurrent = current;      
        size_t s = current->size;
        
        
        newCurrent += s;
        SIZE* footer = newCurrent;
        // debug("footer(%p)\n",footer);
        
        footer->s = current->size;
        moveRoot(current);
    }
    

    if( prevAllocatedFlag== 0)
    {   
        // debug("next : %p\n",next);
        // debug("heap : %p\n",heapAddress);
        if(next <= heapAddress)
        {
            prev->nextNode = NULL;
        
        }else
        {
            prev->nextNode = next;
            next->prevNode = prev;
            // debug("debug prevNode2 (%p)\n",next->prevNode);            

        }
        size_t a = current->size&-2 ;
        size_t b = prev->size&-2;
        // debug("a : %d\n",a);
        // debug("b : %d\n",b);

        prev->size =  (current->size&-2) + (prev->size&-2);
    
  
        char* newCurrent = prev; //나중 수정
        size_t s = prev->size;
        // debug("newcu : %p\n",newCurrent);
        newCurrent += s;
        // debug("newcu : %p\n",newCurrent);        
        SIZE* footer = newCurrent;
        // debug("footer(%p)\n",footer);

        footer->s = prev->size;

        moveRoot(prev);
    }
    
    if(prevAllocatedFlag != 0 && nextAllocatedFlag != 0)
    {
        // debug("f next(%p)\n\n",current);
        addFreeNode(current);
        moveRoot(current);
    }




}


void* allocateFreeList(size_t blockSize)  
{
    // debug("allocateFreeList\n");
    //allocate할때 sbrk 혹은 freenode에서 추가 
    //
    if(init == 0){
        void* p = sbrk(blockSize);
        // debug("header debug(%p)\n",p);
        // debug("blockSize(%d)\n",blockSize);
        heapAddress = p+blockSize;
        SIZE* t = p;
        t->s = blockSize+1;
        // debug("debug(%p)\n",&t->s);
        t = p+blockSize-BLOCKSIZE;

        // debug("foot debug(%p)\n",t);
        t->s = blockSize+1;
        // debug("debug(%p)\n",&t->s);
        return p+BLOCKSIZE;

    }

    ST_FREELIST* current = freeLists;

    while(1)
    {

        // debug("currentPointer %p\n",current);
        size_t cSize = current->size;

        if(blockSize < cSize-24)
        {
 

            // debug("A 블록 사이즈가 작을 때\n"); //footer 크기 붙이기
            ST_FREELIST* tempPrev = current->prevNode;
            ST_FREELIST* tempNext = current->nextNode;
            size_t tempSize = current->size;
            tempSize = tempSize - blockSize;
            // debug("c Size : %d\n",current->size);
            // debug("temp Size : %d\n",tempSize);
            size_t* temp = current;
            *temp = blockSize+1;
            // debug("temp Size %d\n",*temp);
            size_t * tempAdd = temp;
            // debug("temp ADD %p\n",tempAdd);
            tempAdd+= (blockSize-1)/8;
            // debug("temp ADD %p\n",tempAdd);
            temp = tempAdd;
            *temp = blockSize+1;
            // debug("temp Size %d\n",*temp);
            size_t * tempType = temp;
            ST_FREELIST* freeNode = tempType+1;

            freeNode->prevNode = tempPrev;
            // debug("debug prevNode3 (%p)\n",current);
            // debug("debug prevNode3 (%p)\n",current->prevNode);
            // debug("debug freeNode size (%d)\n",debugNode->size);    
 
            freeNode->nextNode = tempNext;
            // debug("debug freeNode 1 (%p)\n",freeNode);
            // debug("debug tempNext (%p)\n",tempNext);    
            // debug("debug freeNode (%p)\n",debugNode);
            // debug("debug freeNode size (%d)\n",debugNode->size);             
            freeNode->size = tempSize&-2;
            void * returnAddress = current;
        
            // debug("debug tempNext (%p)\n",tempNext);           
            if(tempPrev != NULL){
                // debug("debug size %d\n",tempPrev->size);
                // debug("temp Prev(%p)\n", tempPrev);
                tempPrev->nextNode = freeNode;
                // debug("debug size %d\n",tempPrev->size);
            }
            
            if(tempNext != NULL){
                tempNext->prevNode = freeNode;
            }

            size_t *footerTemp = freeNode;
            SIZE* freeNodeFooter = footerTemp+((tempSize)/8)-1;

            // debug("block Size %d\n",blockSize);
            // debug("free Node %p\n",freeNode);            
            // debug("freenodeFooter (%p)\n",freeNodeFooter);   
            // debug("heapAddr (%p)\n",heapAddress);  
            // debug("temS (%d)\n",tempSize&-2);           
            freeNodeFooter->s = tempSize&-2;
            // debug("free Node %p\n",freeNode);
            // debug("free Node size %d \n",freeNodeFooter->s);
            
            // debug("free Node size %d\n",freeNode->size);
            // debug("free Node footer %p\n",freeNodeFooter);
            // debug("return %p\n",returnAddress+BLOCKSIZE);
            // debug("current 1 %p\n",current);
            // debug("curSize %d\n",current->size);
            moveRoot(freeNode);
            return returnAddress+BLOCKSIZE;
        }

        if(blockSize == current->size) //꽉 채워지면 다음 걸로 넘어가야함 근데 어케함?
        {
            // debug("A 딱맞을 때\n");
            // debug("prev = %p\n",current->prevNode);
            // debug("next = %p\n",current->nextNode);
            if(current->prevNode != NULL && initAddress < current->prevNode ){
                current->prevNode->nextNode = current->nextNode;
            }
            if(current->nextNode != NULL && current->prevNode < heapAddress ){
                current->nextNode->prevNode = current->prevNode; 
            }

            
            
            size_t* temp = current;
        
            *temp = blockSize+1;
            temp+= (blockSize-1)/BLOCKSIZE;
            // debug("temp %p\n",temp);
            *temp = blockSize+1;
            temp += 1;
            debugPointer = temp;
            size_t* returnAddr = current;
            returnAddr+= 1;
            if(current->prevNode != NULL && initAddress < current->prevNode ){
                moveRoot(current->prevNode);
            }
            if(current->nextNode != NULL && current->prevNode < heapAddress ){
                moveRoot(current->nextNode);
            }
            if(current->nextNode == NULL && current->prevNode == NULL){
                // debug("is Called?\n");
                freeLists = NULL;
                init = 0;
            }
            return returnAddr;
            
        }
        // debug("current nextNode : %p\n",current->nextNode);
        if(current->nextNode == NULL || current->nextNode < initAddress){

            // debug("A 맞는게 없을 때\n");
            void* p = sbrk(blockSize);
            // debug("header debug(%p)\n",p);
            // debug("blockSize(%d)\n",blockSize);
            heapAddress = p+blockSize;            
            SIZE* t = p;
            t->s = blockSize+1;
            debug("t size %d\n",t->s);
        // debug("debug(%p)\n",&t->s);
            t = p+blockSize-BLOCKSIZE;
        // debug("foot debug(%p)\n",t);
            t->s = blockSize+1;
            debug("t size %d\n",t->s);
        // debug("debug(%p)\n",&t->s);
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
        // size_t s= 0x11D698;
        // debugPointer = initAddress+s;
        // debug("debugPointer(%p)\n",debugPointer);
        // debug("initAdd(%p)\n",initAddress);  
        checkAddress = 1;
    }    


 
    size_t blockSize = size * BLOCKSIZE;
    blockSize += 16;

    //freeList에서 확인 이후 없으면 sbrk 확인은 어떻게?
    void *p = allocateFreeList(blockSize);


    int * memSize = p;
    debug("alloc(%u): %p\n", (unsigned int)size, p);
    max_size += size;
    debug("max: %u\n", max_size);

    // if(debugPointer < heapAddress){
    //     debug("fuckyou 1 %p\n",debugPointer);
    //     debug("fuckyou 2 %d\n",debugPointer->s);
    // }
    
    // if (freeLists != NULL)
    // {

    //     ST_FREELIST* currentB = freeLists;


        
    //     while(1){
    //         debug("currentB(%p)\n",currentB);    
    //         if(currentB->nextNode == NULL || currentB->nextNode < initAddress){
    //             debug("fuck3 :%p\n",currentB->nextNode);
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
    void *p = NULL;
    // if (size != 0)
    // {
    //     p = sbrk(size);
    //     if (ptr)
    //         memcpy(p, ptr, size);
    //     max_size += size;
    //     debug("max: %u\n", max_size);
    // }
    myfree(ptr);
    p = myalloc(size);
    debug("realloc(%p, %u): %p\n", ptr, (unsigned int)size, p);
    return p;
}

void myfree(void *ptr)
{
    // debug("headSize(%p)\n",heapAddress);
    debug("free(%p)\n", ptr);      
    if(ptr < initAddress ){
        return;
    }
    if(heapAddress <= ptr){
        return;
    }

    // coalescing(ptr);    
    // if(debugPointer < heapAddress){
    //     // debug("fuckyou 1 %p\n",debugPointer);
    //     debug("fuckyou 2 %d\n",debugPointer->s);
    // }
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
  

}
