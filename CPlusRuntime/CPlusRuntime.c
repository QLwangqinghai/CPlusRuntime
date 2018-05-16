//
//  CPlusRuntime.c
//  O
//
//  Created by wangqinghai on 2018/3/16.
//  Copyright © 2018年 wangqinghai. All rights reserved.
//

#include "CPlusRuntimePrivate.h"


CPMemoryManager_t * _Nonnull CPMemoryManagerDefault(void) {
    return &__CPMemoryManagerDefault;
}
void CPMemoryManagerLoggerDeinit(void const * _Nonnull object) {
    CPMemoryLogger_o * logger  = (CPMemoryLogger_o *)object;
    if (logger->context) {
        logger->contextRelease(logger->context);
    }
}
CPMemoryUsedInfo_s CPMemoryUsedInfo(void) {
    CPMemoryUsedInfo_s i = {};
    i.ptrCount = CPGetFast64(&(CPMemoryManagerDefault()->ptrCount));
    i.usedMemory = CPGetFast64(&(CPMemoryManagerDefault()->usedMemory));
    return i;
}

CPMemoryManager_t * _Nonnull CPMemoryManagerAllocInit() {
    return NULL;
}


int CPMemoryManagerAddLogger(CPAlloctor_s * _Nonnull alloctor, CPMemoryLoggerRef _Nonnull logger) {
// 0 NUll; 1 prepare; 2 active
    assert(alloctor);
    
    uintptr_t oldLoggerManager = 0;
    uintptr_t newLoggerManager = 0;
    
    CPMemoryManager_t * manager = NULL;
    CPMemoryManager_t * tmpManager = NULL;

    do {
        oldLoggerManager = atomic_load(&(alloctor->loggerManager));
        if (0 == oldLoggerManager) {
            if (NULL == tmpManager) {
                tmpManager = CPMemoryManagerAllocInit();
            }
            newLoggerManager = (uintptr_t)tmpManager;
        } else {
            manager = (CPMemoryManager_t *)oldLoggerManager;
            if (tmpManager) {
                CPRelease(tmpManager);
                tmpManager = NULL;
            }
            
            goto afterGetManager;
        }
    } while(!(atomic_compare_exchange_weak(&(alloctor->loggerManager), &oldLoggerManager, newLoggerManager)));
    manager = tmpManager;
    
afterGetManager:
    
    assert(manager);
    assert(logger);
    CPObject v = CPRetain(logger);
    
    int bitIndex = -1;
    if (v) {
        uint64_t flags = CPGetFast64(&(manager->loggerFlags));
        uint64_t activeMask = 0x1;
        for (int i=0; i<64; i++) {
            uint64_t offset = i;
            uint64_t flag = (activeMask << offset);
            if ((flags & flag) == 0) {
                CPMemoryManagerLoggerContainer_s * container = &(manager->loggerItems[i]);
                int setInitFlagResult = 1;
#if CPTARGET_RT_64_BIT
                uint64_t refrenceCount = 0;
                uint64_t newRefrenceCount = 1;
#else
                uint32_t refrenceCount = 0;
                uint32_t newRefrenceCount = 1;
#endif
                do {
                    refrenceCount = atomic_load(&(container->refrenceCount));
                    if (refrenceCount != 0) {
                        setInitFlagResult = 0;
                        break;
                    }
                } while(!CPCASSetFast64(&(container->refrenceCount), refrenceCount, newRefrenceCount));
                
                
                if (setInitFlagResult >= 1) {
                    bitIndex = i;
                    break;
                }
            }
        }
        if (bitIndex < 0) {
            CPRelease(logger);
            return 0;
        }
        uint64_t mask = activeMask << bitIndex;
        uint64_t newFlags = flags;

        do {
            flags = CPGetFast64(&(manager->loggerFlags));
            
#if DEBUG
            if ((flags & (~mask)) != flags) {
                abort();
            }
#endif
            
            newFlags = flags | mask;
        } while(!CPCASSetFast64(&(manager->loggerFlags), flags, newFlags));

        uintptr_t obj = (uintptr_t)logger;
        
        CPMemoryManagerLoggerContainer_s * container = &(manager->loggerItems[bitIndex]);
        atomic_store(&(container->logger), obj);
        CPMemoryBarrier();
        atomic_store(&(container->refrenceCount), 2);
        return bitIndex + 1;
    } else {
        return -1;
    }
}

enum CPMemoryManagerLoggerReferenceCountAddResult {
    CPMemoryManagerLoggerReferenceCountAddResultSuccess,
    CPMemoryManagerLoggerReferenceCountAddResultRefrenceCountZero,
    CPMemoryManagerLoggerReferenceCountAddResultRefrenceCountOne,
    CPMemoryManagerLoggerReferenceCountAddResultRefrenceCountTooLarge,
};
int CPMemoryManagerLoggerReferenceCountAdd(CPMemoryManagerLoggerContainer_s * _Nonnull container, uint64_t * _Nonnull oldRefrenceCountRef, uint64_t * _Nonnull newRefrenceCountRef) {
    assert(container);
    assert(oldRefrenceCountRef);
    assert(newRefrenceCountRef);

#if CPTARGET_RT_64_BIT
    uint64_t refrenceCount = 0;
    uint64_t newRefrenceCount = 0;
#else
    uint32_t refrenceCount = 0;
    uint32_t newRefrenceCount = 0;
#endif
    do {
        refrenceCount = atomic_load(&(container->refrenceCount));
#if CPTARGET_RT_64_BIT
        if (refrenceCount == UINT64_MAX) {
            return CPMemoryManagerLoggerReferenceCountAddResultRefrenceCountTooLarge;
        } else if (refrenceCount == 0) {
            return CPMemoryManagerLoggerReferenceCountAddResultRefrenceCountZero;
        } else if (refrenceCount == 1) {
            return CPMemoryManagerLoggerReferenceCountAddResultRefrenceCountOne;
        }
#else
        if (refrenceCount == UINT32_MAX) {
            return CPMemoryManagerLoggerReferenceCountAddResultRefrenceCountTooLarge;
        } else if (refrenceCount == 0) {
            return CPMemoryManagerLoggerReferenceCountAddResultRefrenceCountZero;
        } else if (refrenceCount == 1) {
            return CPMemoryManagerLoggerReferenceCountAddResultRefrenceCountOne;
        }
#endif
        newRefrenceCount = refrenceCount + 1;
    } while(!CPCASSetFast64(&(container->refrenceCount), refrenceCount, newRefrenceCount));
    
    *oldRefrenceCountRef = (uint64_t)refrenceCount;
    *newRefrenceCountRef = (uint64_t)newRefrenceCount;
    return CPMemoryManagerLoggerReferenceCountAddResultSuccess;
}

// 最小值1

enum CPMemoryManagerLoggerReferenceCountSubtractResult {
    CPMemoryManagerLoggerReferenceCountSubtractResultSuccess,
    CPMemoryManagerLoggerReferenceCountSubtractResultRefrenceCountZero,
    CPMemoryManagerLoggerReferenceCountSubtractResultRefrenceCountOne,
};

int CPMemoryManagerLoggerReferenceCountSubtract(CPMemoryManagerLoggerContainer_s * _Nonnull container, uint64_t * _Nonnull oldRefrenceCountRef, uint64_t * _Nonnull newRefrenceCountRef) {
    assert(container);
    assert(oldRefrenceCountRef);
    assert(newRefrenceCountRef);
    
#if CPTARGET_RT_64_BIT
    uint64_t refrenceCount = 0;
    uint64_t newRefrenceCount = 0;
#else
    uint32_t refrenceCount = 0;
    uint32_t newRefrenceCount = 0;
#endif
    do {
        refrenceCount = atomic_load(&(container->refrenceCount));
        if (refrenceCount == 0) {
            return CPMemoryManagerLoggerReferenceCountSubtractResultRefrenceCountZero;
        } else if (refrenceCount == 1) {
            return CPMemoryManagerLoggerReferenceCountSubtractResultRefrenceCountOne;
        }
        newRefrenceCount = refrenceCount - 1;
    } while(!CPCASSetFast64(&(container->refrenceCount), refrenceCount, newRefrenceCount));
    *oldRefrenceCountRef = (uint64_t)refrenceCount;
    *newRefrenceCountRef = (uint64_t)newRefrenceCount;

    return CPMemoryManagerLoggerReferenceCountSubtractResultSuccess;
}

CPMemoryManagerLoggerContainer_s * _Nullable CPMemoryManagerLoggerRetain(CPMemoryManager_t * _Nonnull manager, uint32_t index) {
    assert(manager);
    if (index >= 64) {
        return NULL;
    }
    CPMemoryManagerLoggerContainer_s * container = &(manager->loggerItems[index]);
    uint64_t oldRefrenceCount;
    uint64_t newRefrenceCount;
    int addResult = CPMemoryManagerLoggerReferenceCountAdd(container, &oldRefrenceCount, &newRefrenceCount);
    if (addResult == CPMemoryManagerLoggerReferenceCountAddResultSuccess) {
        return container;
    }
    return NULL;
}
int CPMemoryManagerLoggerRelease(CPMemoryManager_t * _Nonnull manager, uint32_t index) {
    assert(manager);
    if (index >= 64) {
        abort();
    }
    CPMemoryManagerLoggerContainer_s * container = &(manager->loggerItems[index]);
    uint64_t oldRefrenceCount;
    uint64_t newRefrenceCount;
    int subResult = CPMemoryManagerLoggerReferenceCountSubtract(container, &oldRefrenceCount, &newRefrenceCount);
    if (subResult == CPMemoryManagerLoggerReferenceCountSubtractResultSuccess) {
        if (newRefrenceCount == 1) {
            uintptr_t loggerAddress = atomic_load(&(container->logger));
            CPMemoryLoggerRef logger = (CPMemoryLoggerRef)loggerAddress;
            atomic_store(&(container->logger), 0);
            CPMemoryBarrier();
            atomic_store(&(container->refrenceCount), 0);
            CPRelease(logger);
            return 0;
        } else {
            if (newRefrenceCount > INT_MAX) {
                return INT_MAX;
            } else {
                return (int)newRefrenceCount;
            }
        }
    } else {
        return -1;
    }
}

int CPMemoryManagerRemoveLogger(CPMemoryManager_t * _Nonnull manager, int key) {
    assert(manager);
    
    if (key <= 0 || key > 64) {
        return -1;
    }
    uint32_t index = (uint32_t)(key - 1);
    uint64_t flags = CPGetFast64(&(manager->loggerFlags));
    uint64_t activeMask = 0x1;
    uint64_t mask = activeMask << index;
    uint64_t newFlags = flags & (~mask);

    do {
        flags = CPGetFast64(&(manager->loggerFlags));
        
        if ((flags & mask) != mask) {
            return -1;
        }
        
        newFlags = flags & (~mask);;
    } while(!CPCASSetFast64(&(manager->loggerFlags), flags, newFlags));
    
    return CPMemoryManagerLoggerRelease(manager, index);
}



CPAllocedMemory_s const CPBaseAlloc(struct __CPAlloctor const * _Nonnull alloctor, size_t size) {
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
    CPAllocedMemory_s m = {
        .flag = {
            .action = 1,
            .code = 0,
        },
        .ptr = ptr,
        .size = size
    };
    return m;
}
CPMemoryFlag_s const CPBaseDealloc(struct __CPAlloctor const * _Nonnull alloctor, void * _Nonnull ptr, size_t size) {
    assert(ptr);
    if (size == 0) {
        abort();
    }
    CBaseFree(ptr, size);
    
    CPMemoryFlag_s flag = {
        .action = 1,
        .code = 0,
    };
    return flag;
}

CPAllocResult_s const CPAlloc(struct __CPAlloctor const * _Nonnull alloctor, CPType _Nonnull type, size_t contentPaddingSize) {
    
    assert(alloctor);
    assert(type);
    
    CPAllocResult_s result = {};
    
    size_t customInfoSize = type->base.customInfoSize * CPContentAligentBlock;
    size_t contentSize = type->base.contentBaseSize;
    if (type->base.contentHasPadding) {
        contentSize += contentPaddingSize;
    }
    size_t realContentSize = CPAlignContentSize(contentSize);
    
    if (realContentSize > CPMaxContentSize) {
        printf("alloc memory error, size too large !!!\n");
        abort();
    }
    CPObject t = CPRetain(type);
    assert(t);
    CPMemoryFlag_s flag = {};
    
    if (type->base.contentHasPadding) {
        size_t contentBlockCountInHeader = realContentSize / CPContentAligentBlock;
        
        if (contentBlockCountInHeader >= CPMaxContentSizeInActiveInfo) {
            result.contentSizeInActiveInfo = CPMaxContentSizeInActiveInfo;
            size_t realSize = result.customInfoSize + CPContentSizeByteCount + sizeof(CPInfoStorage_s) + realContentSize;
            CPAllocedMemory_s m = alloctor->memoryAlloc(alloctor, realSize);
            flag = m.flag;
            result.ptr = m.ptr;
            result.size = m.size;
            uint8_t * contentSizePtr = (uint8_t *)result.ptr + customInfoSize;
            __CPStoreContentSize(contentSizePtr, realContentSize);
            result.infoStorage = (CPInfoStorage_s *)((uint8_t *)result.ptr + customInfoSize + CPContentSizeByteCount);
            result.obj = ((uint8_t *)result.ptr + customInfoSize + CPContentSizeByteCount + CPInfoStoreSize);
        } else {
            result.contentSizeInActiveInfo = contentBlockCountInHeader;
            size_t realSize = customInfoSize + CPInfoStoreSize + realContentSize;
            CPAllocedMemory_s m = alloctor->memoryAlloc(alloctor, realSize);
            flag = m.flag;
            result.ptr = m.ptr;
            result.size = m.size;
            result.infoStorage = (CPInfoStorage_s *)((uint8_t *)result.ptr + customInfoSize);
            result.obj = ((uint8_t *)result.ptr + customInfoSize + CPInfoStoreSize);
        }
    } else {
        result.contentSizeInActiveInfo = 0;
        size_t realSize = customInfoSize + CPInfoStoreSize + realContentSize;
        CPAllocedMemory_s m = alloctor->memoryAlloc(alloctor, realSize);
        flag = m.flag;
        result.ptr = m.ptr;
        result.size = m.size;
        result.infoStorage = (CPInfoStorage_s *)((uint8_t *)result.ptr + customInfoSize);
        result.obj = ((uint8_t *)result.ptr + customInfoSize + CPInfoStoreSize);
    }
    if (flag.action == 1) {
        //log malloc
        
        
    }
    
    if (customInfoSize > 0) {
        result.customInfoSize = customInfoSize;
        result.customInfo = result.ptr;
    }
    CPInfoStorageStoreType(result.infoStorage, type);
    return result;
}
void * _Nonnull CPInit(struct __CPAlloctor const * _Nonnull alloctor, CPAllocResult_s * _Nonnull allocedInfo, _Bool autoDealloc, _Bool isStatic) {
    
    assert(alloctor);
    assert(allocedInfo);
    
    CPActiveInfo_s activeInfo = {};
    if (isStatic) {
        activeInfo.refrenceCount = CPRefrenceStaticFlag;
        activeInfo.autoDealloc = 0;
    } else {
        activeInfo.refrenceCount = 1;
        activeInfo.autoDealloc = autoDealloc;
    }
    activeInfo.contentSize = allocedInfo->contentSizeInActiveInfo;
    CPInfoStorageStoreActiveInfo(allocedInfo->infoStorage, &activeInfo);
    return allocedInfo->obj;
}


void * _Nonnull CPAllocInit(struct __CPAlloctor const * _Nonnull alloctor, CPType _Nonnull type, size_t contentPaddingSize, _Bool autoDealloc, _Bool isStatic) {
    
    assert(alloctor);
    assert(type);
    
    CPAllocResult_s result = alloctor->alloc(alloctor, type, contentPaddingSize);;
    return alloctor->init(alloctor, &result, autoDealloc, isStatic);
    
    
//    size_t customInfoSize = type->base.customInfoSize * CPContentAligentBlock;
//    size_t contentSize = type->base.contentBaseSize;
//    if (type->base.contentHasPadding) {
//        contentSize += contentPaddingSize;
//    }
//    size_t realContentSize = CPAlignContentSize(contentSize);
//
//    void * ptr = NULL;
//
//    if (realContentSize > CPMaxContentSize) {
//        printf("alloc a too large size memory!!!\n");
//        abort();
//    }
//    CPObject t = CPRetain(type);
//    assert(t);
//
//    CPActiveInfo_s activeInfo = {};
//    if (isStatic) {
//        activeInfo.refrenceCount = CPRefrenceStaticFlag;
//        activeInfo.autoDealloc = 0;
//    } else {
//        activeInfo.refrenceCount = 1;
//        activeInfo.autoDealloc = autoDealloc;
//    }
//
//    if (type->base.contentHasPadding) {
//        size_t contentBlockCountInHeader = realContentSize / CPContentAligentBlock;
//
//        if (contentBlockCountInHeader >= CPMaxContentSizeInActiveInfo) {
//            activeInfo.contentSize = CPMaxContentSizeInActiveInfo;
//
//            size_t realSize = customInfoSize + CPContentSizeByteCount + sizeof(CPInfoStorage_s) + realContentSize;
//            CPAllocedMemory_s m = alloctor->memoryAlloc(alloctor, realSize);
//            ptr = m.ptr;
//
//            __CPStoreContentSize((uint8_t *)ptr + customInfoSize, realContentSize);
//            CPInfoStorage_s * tmp = (CPInfoStorage_s *)((uint8_t *)ptr + customInfoSize + CPContentSizeByteCount);
//            CPInfoStorageStore(tmp, type, &activeInfo);
//            return ((uint8_t *)ptr + customInfoSize + CPContentSizeByteCount + CPInfoStoreSize);
//        } else {
//            activeInfo.contentSize = contentBlockCountInHeader;
//
//            size_t realSize = customInfoSize + CPInfoStoreSize + realContentSize;
//            CPAllocedMemory_s m = alloctor->memoryAlloc(alloctor, realSize);
//            ptr = m.ptr;
//
//            CPInfoStorage_s * tmp = (CPInfoStorage_s *)((uint8_t *)ptr + customInfoSize);
//            CPInfoStorageStore(tmp, type, &activeInfo);
//            return ((uint8_t *)ptr + customInfoSize + CPInfoStoreSize);
//        }
//    } else {
//        activeInfo.contentSize = 0;
//        size_t realSize = customInfoSize + CPInfoStoreSize + realContentSize;
//        CPAllocedMemory_s m = alloctor->memoryAlloc(alloctor, realSize);
//        ptr = m.ptr;
//        CPInfoStorage_s * tmp = (CPInfoStorage_s *)((uint8_t *)ptr + customInfoSize);
//        CPInfoStorageStore(tmp, type, &activeInfo);
//        return ((uint8_t *)ptr + customInfoSize + CPInfoStoreSize);
//    }
}
void CPDealloc(struct __CPAlloctor const * _Nonnull alloctor, CPObject _Nonnull obj) {
    assert(obj);

    CPAllocedMemory_s m = __CPGetMemoryStoreInfo(obj);
    
    CPInfoStorage_s * info = CPGetInfo(obj);
    CPType_s * type = CPGetType(info);
    CPMemoryFlag_s flag = alloctor->memoryDealloc(alloctor, m.ptr, m.size);
    
    if (flag.action == 1) {
        //log free
        
        
        
    }
    CPRelease(type);
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

/*
 typedef struct __CPAlloctor {
 //memory
 CPAllocedMemory_s (* _Nonnull memoryAlloc)(struct __CPAlloctor const * _Nonnull alloctor, size_t size);
 void (* _Nonnull memoryDealloc)(struct __CPAlloctor const * _Nonnull alloctor, void * _Nonnull ptr, size_t size);
 
 
 CPAllocResult_s const (* _Nonnull alloc)(struct __CPAlloctor const * _Nonnull alloctor, CPType _Nonnull type, size_t contentPaddingSize);
 void * _Nonnull (* _Nonnull init)(struct __CPAlloctor const * _Nonnull alloctor, CPAllocResult_s * _Nonnull allocedInfo, _Bool autoDealloc, _Bool isStatic);
 void * _Nonnull (* _Nonnull allocInit)(struct __CPAlloctor const * _Nonnull alloctor, CPType _Nonnull type, size_t contentMutableSize, _Bool autoDealloc, _Bool isStatic);
 void (* _Nonnull dealloc)(struct __CPAlloctor const * _Nonnull alloctor, CPObject _Nonnull obj);
 
 
 void * _Nullable context;
 } CPAlloctor_s;
 */
static CPAlloctor_s const CPAlloctorDefault = {
    .memoryAlloc = CPBaseAlloc,
    .memoryDealloc = CPBaseDealloc,
    .alloc = CPAlloc,
    .init = CPInit,
    .allocInit = CPAllocInit,
    .dealloc = CPDealloc,
    .context = NULL,
};
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

