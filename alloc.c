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

#define BLOCKSIZE 4

typedef struct freeList
{
    size_t size;
    struct freeList* prevNode;
    struct freeList* nextNode;
} ST_FREELIST;

static ST_FREELIST* freeLists;
// static ST_FREELIST* freeLists[13];
static int init=0;

extern void debug(const char *fmt, ...);
extern void *sbrk(intptr_t increment);

unsigned int max_size;

void split(ST_FREELIST freeList)
{
    //split 구현

}



void addFreeNode(ST_FREELIST* freeNode)
{

    debug("addFreeNode\n");    
    ST_FREELIST* currentNode = freeLists;
    while(1)
    {
    
        if(freeLists->nextNode == NULL)
        {   
            freeNode->prevNode = currentNode;
            freeLists->nextNode = freeNode;
            return;
        }
        if(freeNode < freeLists->nextNode)
        {
            freeNode->nextNode = freeLists->nextNode;
            freeNode->prevNode = freeLists;
            ST_FREELIST* temp = freeLists->nextNode;
            freeLists->nextNode->prevNode = freeNode;
            freeLists->nextNode = freeNode;
            return;
        }

        currentNode = currentNode->nextNode;
            
            
    }
    
    return;
}

void coalescing(void * p)
{
    if(init == 0){
        freeLists = p-4;
        debug("first Lists debug(%p)\n",freeLists);
        int*temp = p-4;
        *temp -=1;
        debug("init debug(%d)\n",*temp);
        freeLists = (ST_FREELIST*)p;
        freeLists->nextNode = NULL;
        freeLists->prevNode = NULL;
        init = 1;
        return;
    }
    debug("coalescing\n");  
    debug("debug(%p)\n",p);      
    ST_FREELIST* tempPtr = (ST_FREELIST*)p;
    
    ST_FREELIST * current = p-4;
    ST_FREELIST * prev = p-8; //HEADERLENGTH
    ST_FREELIST * next = current+(current->size&-2);
    size_t nextAllocatedFlag = 0;
    if(sbrk(0)<next)
    {
        nextAllocatedFlag = 1;
    }
    else
    {
        nextAllocatedFlag = next->size&=1;
    }
    debug("coalescing debug(%d)\n",current->size);
    debug("coalescing debug(%p)\n",p);
    debug("coalescing debug(%p)\n",current);
    debug("coalescing debug(%p)\n",prev);
    debug("coalescing debug(%p)\n",next);  

    size_t prevAllocatedFlag = prev->size&=1;

    debug("current size(%d)\n",current->size);
    debug("current nextNode(%p)\n",current->nextNode);
    debug("current prevNode(%p)\n",current->prevNode);
    debug("prev size(%d)\n",prev->size);
    debug("next size(%d)\n",next->size);
    debug("nextAllocatedFlag size(%d)\n",nextAllocatedFlag);
    debug("prevAllocatoedFlag size(%d)\n",prevAllocatedFlag);    
    
    if( nextAllocatedFlag== 0){
        current->nextNode = next->nextNode;
        current->prevNode = next->prevNode;
        current->size = current->size&-2 + next->size&-2;

    }
    if( prevAllocatedFlag== 0)
    {   
        if(nextAllocatedFlag== 0)
        {
            prev->nextNode = current->nextNode;
            prev->nextNode = current->prevNode;
        }
        
        addFreeNode(prev);
    }else
    {
        addFreeNode(current);
    }


}
typedef struct size
{
    size_t s;
}SIZE;

void* allocateFreeList(size_t blockSize)
{
    debug("allocateFreeList\n");
    //allocate할때 sbrk 혹은 freenode에서 추가 
    //
    if(init == 0){
        void* p = sbrk(blockSize);
        debug("header debug(%p)\n",p);
        debug("blockSize(%d)\n",blockSize);
        SIZE* t = p;
        t->s = blockSize+1;
        debug("debug(%p)\n",&t->s);
        t = p+blockSize-4;

        debug("foot debug(%p)\n",t);
        t->s = blockSize+1;
        debug("debug(%p)\n",&t->s);
        return p+4;

    }

    ST_FREELIST* current = freeLists;

    while(1)
    {
        if(blockSize < current->size)
        {
            
            ST_FREELIST* tempPrev = current->prevNode;
            ST_FREELIST* tempNext = current->nextNode;
            size_t tempSize = current->size - blockSize;
            size_t* temp = current;
            *temp = blockSize+1;
            temp+= blockSize-4;
            *temp = blockSize+1;

            ST_FREELIST* freeNode = temp+4;
            
            freeNode->prevNode = tempPrev;
            freeNode->nextNode = tempNext;
            
            return current+4;
        }
        if(blockSize = current->size)
        {
            current->nextNode->prevNode = current->prevNode;
            current->prevNode->nextNode = current->nextNode;

            size_t* temp = current;
            *temp = blockSize+1;
            temp+= blockSize-4;
            *temp = blockSize+1;
            return current+4;
        }

        current = current->nextNode;
    }


}



void *myalloc(size_t size)
{
    if (size == 0)
        return 0;

 
    size_t blockSize = size * BLOCKSIZE;
    blockSize += 8;

    //freeList에서 확인 이후 없으면 sbrk 확인은 어떻게?
    void *p = allocateFreeList(blockSize);


    int * memSize = p;
    debug("alloc(%u): %p\n", (unsigned int)size, p);
    max_size += size;
    debug("max: %u\n", max_size);

    return p;
}

void *myrealloc(void *ptr, size_t size)
{
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
    debug("free(%p)\n", ptr);    
    coalescing(ptr);    
}
