//
//  CPlusRuntime.c
//  O
//
//  Created by wangqinghai on 2018/3/16.
//  Copyright © 2018年 wangqinghai. All rights reserved.
//

#include "CPlusRuntimePrivate.h"

static CPMemoryManager_t _CPMemoryManagerDefault = {};

CPMemoryManager_t * _Nonnull CPMemoryManagerDefault(void) {
    return &_CPMemoryManagerDefault;
}


CPAllocedMemory_s CPBaseAlloc(struct __CPAlloctor const * _Nonnull alloctor, size_t size) {
    if (size == 0) {
        abort();
    }
    void * ptr = CBaseAlloc(size);
    if (NULL == ptr) {
        if(CPMemoryManagerDefault()->oomHandler == NULL) {
            printf("alloc oom error!!!\n");
            abort();
        } else {
            CPMemoryManagerDefault()->oomHandler(CPMemoryManagerDefault(), size);
            abort();
        }
    }
    CPAllocedMemory_s m = {ptr, size};
    return m;
}
void CPBaseDealloc(struct __CPAlloctor const * _Nonnull alloctor, void * _Nonnull ptr, size_t size) {
    assert(ptr);
    if (size == 0) {
        abort();
    }
    CBaseFree(ptr, size);
}

void * _Nonnull CPAllocInit(struct __CPAlloctor const * _Nonnull alloctor, CPType _Nonnull type, size_t contentPaddingSize, _Bool autoDealloc, _Bool isStatic) {
    
    assert(alloctor);
    assert(type);
    
    size_t customInfoSize = type->base.customInfoSize * CPContentAligentBlock;
    size_t contentSize = type->base.contentBaseSize;
    if (type->base.contentHasPadding) {
        contentSize += contentPaddingSize;
    }
    size_t realContentSize = CPAlignContentSize(contentSize);
    
    void * ptr = NULL;
    
    if (realContentSize > CPMaxContentSize) {
        printf("alloc a too large size memory!!!\n");
        abort();
    }
    CPObject t = CPRetain(type);
    assert(t);
    
    CPActiveInfo_s activeInfo = {};
    if (isStatic) {
        activeInfo.refrenceCount = CPRefrenceStaticFlag;
        activeInfo.autoDealloc = 0;
    } else {
        activeInfo.refrenceCount = 1;
        activeInfo.autoDealloc = autoDealloc;
    }
    
    if (type->base.contentHasPadding) {
        size_t contentBlockCountInHeader = realContentSize / CPContentAligentBlock;
        
        if (contentBlockCountInHeader >= CPMaxContentSizeInActiveInfo) {
            activeInfo.contentSize = CPMaxContentSizeInActiveInfo;
            
            size_t realSize = customInfoSize + CPContentSizeByteCount + sizeof(CPInfoStorage_s) + realContentSize;
            CPAllocedMemory_s m = alloctor->memoryAlloc(alloctor, realSize);
            ptr = m.ptr;
            
            __CPStoreContentSize((uint8_t *)ptr + customInfoSize, realContentSize);
            CPInfoStorage_s * tmp = (CPInfoStorage_s *)((uint8_t *)ptr + customInfoSize + CPContentSizeByteCount);
            CPInfoStorageStore(tmp, type, &activeInfo);
            return ((uint8_t *)ptr + customInfoSize + CPContentSizeByteCount + CPInfoStoreSize);
        } else {
            activeInfo.contentSize = contentBlockCountInHeader;
            
            size_t realSize = customInfoSize + CPInfoStoreSize + realContentSize;
            CPAllocedMemory_s m = alloctor->memoryAlloc(alloctor, realSize);
            ptr = m.ptr;
            
            CPInfoStorage_s * tmp = (CPInfoStorage_s *)((uint8_t *)ptr + customInfoSize);
            CPInfoStorageStore(tmp, type, &activeInfo);
            return ((uint8_t *)ptr + customInfoSize + CPInfoStoreSize);
        }
    } else {
        activeInfo.contentSize = 0;
        size_t realSize = customInfoSize + CPInfoStoreSize + realContentSize;
        CPAllocedMemory_s m = alloctor->memoryAlloc(alloctor, realSize);
        ptr = m.ptr;
        CPInfoStorage_s * tmp = (CPInfoStorage_s *)((uint8_t *)ptr + customInfoSize);
        CPInfoStorageStore(tmp, type, &activeInfo);
        return ((uint8_t *)ptr + customInfoSize + CPInfoStoreSize);
    }
}
void CPDealloc(struct __CPAlloctor const * _Nonnull alloctor, CPObject _Nonnull obj) {
    assert(obj);

    CPAllocedMemory_s m = __CPGetMemoryStoreInfo(obj);
    
    CPInfoStorage_s * info = CPGetInfo(obj);
    CPType_s * type = CPGetType(info);
    CPRelease(type);
    
    CBaseFree(m.ptr, m.size);
}

size_t CPGetStoreSizeWithType(CPType _Nonnull type, size_t contentPaddingSize) {
    assert(type);
    
    size_t customInfoSize = type->base.customInfoSize * CPContentAligentBlock;
    size_t contentSize = type->base.contentBaseSize;
    if (type->base.contentHasPadding) {
        contentSize += contentPaddingSize;
    }
    size_t realContentSize = CPAlignContentSize(contentSize);
    
    size_t realSize = customInfoSize + sizeof(CPInfoStorage_s) + realContentSize;
    if (type->base.contentHasPadding) {
#if CPMemoryHeaderAligent64
        size_t contentBlockCountInHeader = realContentSize / CPContentAligentBlock;
        if (contentBlockCountInHeader >= CPMaxContentSizeInActiveInfo) {
            realSize += CPContentAligentBlock;
        }
#else
        realSize += CPContentAligentBlock;
#endif
        realSize += CPContentAligentBlock;
    }
    return realSize;
}

size_t CPGetStoreSize(void * _Nonnull obj) {
    CPAllocedMemory_s m = __CPGetMemoryStoreInfo(obj);
    return m.size;
}

static CPAlloctor_s const CPAlloctorDefault = {CPBaseAlloc, CPBaseDealloc, CPAllocInit, CPDealloc, NULL};
CPAlloctor_s const * _Nonnull CPAlloctorGetDefault(void) {
    return &CPAlloctorDefault;
}




CPInfoStorage_s * _Nullable CPReferenceCountAdd(CPInfoStorage_s * _Nonnull header) {
    assert(header);
    CPActiveInfo_s activeInfo = {};
    CPActiveInfo_s newValue = {};
    
    do {
        activeInfo = CPGetActiveInfo(header);
        if (activeInfo.deallocing == 1) {//dealloc obj
            printf("CPlus error, retain an dealloc obj.\n");
            return NULL;
        } else if (activeInfo.prepareDealloc == 1) {//prepare dealloc obj
            return NULL;
        } else if (activeInfo.refrenceCount == CPRefrenceStaticFlag) {//static obj
            return header;
        }
        newValue = activeInfo;
        newValue.refrenceCount += 1;
    } while (!CPCASSetActiveInfo(header, activeInfo, newValue));
    return header;
}

CPActiveInfo_s CPReferenceCountSubtract(CPInfoStorage_s * _Nonnull header) {
    assert(header);
    CPActiveInfo_s activeInfo = {};
    CPActiveInfo_s newValue = {};
    do {
        activeInfo = CPGetActiveInfo(header);
        if (activeInfo.deallocing == 1) {//dealloc obj
            printf("CPlus error, release an dealloc obj.\n");
            abort();
            return activeInfo;
        } else if (activeInfo.prepareDealloc == 1) {//prepare dealloc obj
            printf("CPlus error, release an prepare dealloc obj.\n");
            abort();
            return activeInfo;
        } else if (activeInfo.refrenceCount == CPRefrenceStaticFlag) {//static obj
            return activeInfo;
        }
        if (activeInfo.refrenceCount == 0) {
            printf("CPlus error, release an error obj.\n");
            abort();
            return activeInfo;
        } else {
            newValue = activeInfo;
            newValue.refrenceCount -= 1;

            if (activeInfo.autoDealloc == 1 && newValue.refrenceCount == 0) {
                newValue.prepareDealloc = 1;
            }
        }
    } while (!CPCASSetActiveInfo(header, activeInfo, newValue));
    return newValue;
}

int32_t CPDefaultSetDeallocingFlag(CPInfoStorage_s * _Nonnull header) {
    assert(header);
    
    CPActiveInfo_s activeInfo = {};
    CPActiveInfo_s newValue = {};
    do {
        activeInfo = CPGetActiveInfo(header);
        if (activeInfo.deallocing == 1) {//dealloc obj
            printf("CPlus error when prepare dealloc, reason: a error object.\n");
            abort();
            return 1;
        } else if (activeInfo.prepareDealloc == 1) {//prepare dealloc obj
#if DEBUG
            if (activeInfo.refrenceCount > 0) {
                abort();
            }
#endif
            newValue = activeInfo;
            newValue.deallocing = 1;
        } else {
            return 1;
        }
    } while (!CPCASSetActiveInfo(header, activeInfo, newValue));
    return 0;
}


typedef uint32_t CPAutoreleasePoolFlag_t;

CPAutoreleasePoolFlag_t CPAutoreleasePoolPush(void);
void CPAutoreleasePoolPop(CPAutoreleasePoolFlag_t flag);

void CPAutoreleasePool(void(^_Nonnull block)(void)) {
    assert(block);
    CPAutoreleasePoolFlag_t flag = CPAutoreleasePoolPush();
    block();
    CPAutoreleasePoolPop(flag);
}

static inline CPInfoStorage_s * _Nonnull __CPGetInfo(void const * _Nonnull obj) {
    assert(obj);

    uint8_t const * mem = (uint8_t const *)obj;
    mem = mem - CPInfoStoreSize;
    return (CPInfoStorage_s *)mem;
}
CPInfoStorage_s * _Nonnull CPGetInfo(void const * _Nonnull obj) {
    return __CPGetInfo(obj);
}


CPObject _Nullable CPRetain(void const * _Nullable obj) {
    if (NULL == obj) {
        return NULL;
    }
    
    CPInfoStorage_s * info = CPGetInfo(obj);
    CPActiveInfo_s activeInfo = CPGetActiveInfo(info);
    CPInfoStorage_s * newHeader = CPReferenceCountAdd(info);
    CPType_s * type = CPGetType(info);
    if (newHeader) {
        if (activeInfo.refrenceCount > 0) {
            CPDidRetainDispatch(type, obj);
        }
        return obj;
    } else {
        return NULL;
    }
}
void CPRelease(void const * _Nullable obj) {
    if (NULL == obj) {
        return;
    }
    CPInfoStorage_s * info = CPGetInfo(obj);
    CPType_s * type = CPGetType(info);

    CPActiveInfo_s activeInfo = CPReferenceCountSubtract(info);

    CPDidReleaseDispatch(type, obj);
    
    if (activeInfo.prepareDealloc) {
        CPDidPrepareDeallocDispatch(type, obj);
        int32_t result = CPDefaultSetDeallocingFlag(info);
        if (result == 0) {
            CPWillDeallocDispatch(type, obj);
            CPDeinitDispatch(type, obj);
            CPAlloctor_s const * alloctor = (CPAlloctor_s *)(type->base.alloctor);
            if (NULL == alloctor) {
                alloctor = CPAlloctorGetDefault();
            }
            alloctor->dealloc(alloctor, obj);
        }
    } else {
        if (activeInfo.refrenceCount == 0) {
            CPDidResignActiveDispatch(type, obj);
        }
    }
}

void CPAutoreleasePoolAppend(CPObject _Nullable obj);
void CPAutorelease(void const * _Nullable obj) {
    if (NULL == obj) {
        return;
    }
    
    CPAutoreleasePoolAppend(obj);
}


/********************************* COAutoreleasePool *********************************/

#pragma mark - COAutoreleasePool

static const uint32_t CPAutoreleasePoolPageCapacity = 2048;
static inline uint32_t CPAutoreleasePoolPageIndex(uint32_t location) {
    return location >> 11;
}
static inline uint32_t CPAutoreleasePoolItemIndexInPage(uint32_t location) {
    return location & 0x7FF;
}
typedef struct {
    CPObject _Nullable objects[CPAutoreleasePoolPageCapacity];
} CPAutoreleasePoolPage_t;
typedef struct {
    uint32_t count;
    uint32_t poolPageSize;
    CPAutoreleasePoolPage_t * _Nullable * _Nonnull poolPages;
} CPAutoreleasePool_t;
CPAutoreleasePool_t * _Nonnull CPAutoreleasePoolGet(void);

void __CPAutoreleasePoolPop(CPAutoreleasePool_t * _Nonnull pool, CPAutoreleasePoolFlag_t flag) {
    assert(pool);
    assert(flag < pool->count);
    uint32_t count = pool->count;
    pool->count = flag;
    
    for (uint32_t i=flag; i<count; i ++) {
        uint32_t pageIndex = CPAutoreleasePoolPageIndex(i);
        uint32_t itemIndex = CPAutoreleasePoolItemIndexInPage(i);
        
        if (pageIndex >= pool->poolPageSize) {
            abort();
        }
        CPAutoreleasePoolPage_t * page = pool->poolPages[pageIndex];
        if (NULL == page) {
            abort();
        }
        CPObject obj = page->objects[itemIndex];
        CPRelease(obj);
    }
}

void CPAutoreleasePoolDealloc(CPAutoreleasePool_t * _Nonnull pool) {
    __CPAutoreleasePoolPop(pool, 0);
    
    for (uint32_t index=0; index<pool->count; index ++) {
        CPAutoreleasePoolPage_t * page = pool->poolPages[index];
        free(page);
    }
    free(pool);
}

void CPAutoreleasePoolAppend(CPObject _Nullable obj) {
    CPAutoreleasePool_t * pool = CPAutoreleasePoolGet();
    
    uint32_t pageIndex = CPAutoreleasePoolPageIndex(pool->count);
    uint32_t itemIndex = CPAutoreleasePoolItemIndexInPage(pool->count);
    if (pool->count == UINT32_MAX) {
        abort();
    }
    if (pageIndex >= pool->poolPageSize) {
        uint64_t newPoolPageSize = pool->poolPageSize;
        newPoolPageSize *= 2;
        if (newPoolPageSize > UINT32_MAX) {
            newPoolPageSize = UINT32_MAX;
        }
        if (newPoolPageSize > pool->poolPageSize) {
            CPAutoreleasePoolPage_t ** poolPages = (CPAutoreleasePoolPage_t **)malloc(sizeof(CPAutoreleasePoolPage_t *) * newPoolPageSize);
            memcpy(poolPages, pool->poolPages, sizeof(CPAutoreleasePoolPage_t *) * pool->poolPageSize);
            free(pool->poolPages);
            pool->poolPages = poolPages;
            pool->poolPageSize = (uint32_t)newPoolPageSize;
        } else {
            abort();
        }
    }
    
    CPAutoreleasePoolPage_t * page = pool->poolPages[pageIndex];
    if (NULL == page) {
        page = (CPAutoreleasePoolPage_t *)malloc(sizeof(CPAutoreleasePoolPage_t));
        memset(page, 0, sizeof(CPAutoreleasePoolPage_t));
        pool->poolPages[pageIndex] = page;
    }
    
    page->objects[itemIndex] = obj;
    pool->count += 1;
}

CPAutoreleasePoolFlag_t CPAutoreleasePoolPush(void) {
    CPAutoreleasePool_t * pool = CPAutoreleasePoolGet();
    uint32_t location = pool->count;
    CPAutoreleasePoolAppend(NULL);
    return location;
}
void CPAutoreleasePoolPop(CPAutoreleasePoolFlag_t flag) {
    CPAutoreleasePool_t * pool = CPAutoreleasePoolGet();
    __CPAutoreleasePoolPop(pool, flag);
}


void __CPAutoreleasePoolShareKeyDealloc(void * _Nullable value) {
    if (value) {
        CPAutoreleasePoolDealloc((CPAutoreleasePool_t *)value);
    }
}

static pthread_key_t __CPAutoreleasePoolShareKey = 0;
void __CPAutoreleasesPoolShareKeyLoad(void) {
    if (0 == __CPAutoreleasePoolShareKey) {
        int result = pthread_key_create(&__CPAutoreleasePoolShareKey, __CPAutoreleasePoolShareKeyDealloc);
        assert(result == 0);
    }
}
static inline pthread_key_t CPAutoreleasePoolShareKey() {
    return __CPAutoreleasePoolShareKey;
}

CPAutoreleasePool_t * _Nonnull CPAutoreleasePoolGet() {
    void * data = pthread_getspecific(CPAutoreleasePoolShareKey());
    CPAutoreleasePool_t * object = (CPAutoreleasePool_t *)data;
    if (NULL == object) {
        CPAutoreleasePool_t * pool = (CPAutoreleasePool_t *)malloc(sizeof(CPAutoreleasePool_t));
        CPAutoreleasePoolPage_t * * poolPages = (CPAutoreleasePoolPage_t * *)malloc(sizeof(CPAutoreleasePoolPage_t *) * 8);
        memset(poolPages, 0, sizeof(CPAutoreleasePoolPage_t *) * 8);
        CPAutoreleasePoolPage_t * page = (CPAutoreleasePoolPage_t *)malloc(sizeof(CPAutoreleasePoolPage_t));
        memset(page, 0, sizeof(CPAutoreleasePoolPage_t));
        
        poolPages[0] = page;
        pool->poolPageSize = 8;
        pool->poolPages = poolPages;
        pool->count = 1;
        page->objects[0] = NULL;

        object = pool;
        int result = pthread_setspecific(CPAutoreleasePoolShareKey(), object);
        assert(result == 0);
    }
    return object;
}


#pragma mark - init
void __CPlusRuntimeInit() {
    __CPAutoreleasesPoolShareKeyLoad();
}

__attribute__((constructor(201)))
void CPlusModuleInit(void) {
#if DEBUG
    printf("CPlusModuleInit\n");
#endif
    static pthread_once_t token = PTHREAD_ONCE_INIT;
    pthread_once(&token,&__CPlusRuntimeInit);
    
#if DEBUG
    printf("CPlusModuleInit end\n");
#endif
}

__attribute__((destructor))
static void CObjectModuleDeinit(void) {
#if DEBUG
    printf("CObjectModuleDeinit\n");
#endif
}

