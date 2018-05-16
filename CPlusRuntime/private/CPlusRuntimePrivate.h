//
//  CPlusRuntimePrivate.h
//  O
//
//  Created by wangqinghai on 2018/3/16.
//  Copyright © 2018年 wangqinghai. All rights reserved.
//

#ifndef CPlusRuntimePrivate_h
#define CPlusRuntimePrivate_h

#include <CPlusRuntime/CPlusRuntimePublic.h>
#include <stdlib.h>
#include <pthread/pthread.h>


typedef struct {
    uintptr_t ptrAddress;
    size_t size;
    CPType _Nonnull type;
    uint32_t source: 8;
    uint32_t code: 24;
} CPMemoryLogItem_s;


typedef struct __CPMemoryManagerLoggerContainer {
#if CPTARGET_RT_64_BIT
    _Atomic(uint_fast64_t) refrenceCount;
#else
    _Atomic(uint_fast32_t) refrenceCount;
#endif
    _Atomic(uintptr_t) logger;
} CPMemoryManagerLoggerContainer_s;

typedef struct __CPMemoryLoggerManager {
    _Atomic(uint_fast64_t) loggerFlags;
    CPMemoryManagerLoggerContainer_s loggerItems[64];
} CPMemoryLoggerManager_s;


typedef void (* CPMemoryManagerLogFunc)(CPMemoryLogItem_s item);

typedef struct {
    void * _Nullable context;
    void (* _Nullable contextRelease)(void * _Nonnull context);//context 非空时调用

    CPMemoryManagerLogFunc _Nonnull logFunc;
    uint32_t const contentSize;
} CPMemoryLogger_o;

typedef CPMemoryLogger_o const * CPMemoryLoggerRef;

void CPMemoryManagerLoggerDeinit(void const * _Nonnull object);

static CPTypeLayout_s const CPTypeStorage_CPMemoryManagerLogger = {
    .info = CPInfoDefaultTypeInfo,
    .type = {
        .base = {
            .isImmutable = 1,
            .domain = CCTypeDomain,
            .contentHasPadding = 0,
            .customInfoSize = 0,
            .contentBaseSize = sizeof(CPMemoryLogger_o),
            .name = "CPlus.MemoryLogger",
            .superType = NULL,
            .alloctor = NULL,
            .deinit = CPMemoryManagerLoggerDeinit,
        },
        .callbacks = NULL,
        .context = NULL,
    },
};



typedef struct __CPMemoryManager {
    _Atomic(uint_fast64_t) ptrCount;
    _Atomic(uint_fast64_t) usedMemory;
    void (* _Nullable zeroSizeErrorHandler)(struct __CPMemoryManager * _Nonnull manager, void * _Nonnull * _Nullable ptr);
    void (* _Nullable oomHandler)(struct __CPMemoryManager * _Nonnull manager, size_t size);
    _Atomic(uint_fast64_t) loggerFlags;
    CPMemoryManagerLoggerContainer_s loggerItems[64];
} CPMemoryManager_t;

static CPMemoryManager_t __CPMemoryManagerDefault = {};

CPMemoryManager_t * _Nonnull CPMemoryManagerDefault(void);

//return < 0 for error, 0 for failure, [1, 64] for key
int CPMemoryManagerAddLogger(CPAlloctor_s * _Nonnull alloctor, CPMemoryLoggerRef _Nonnull logger);


static inline uint64_t CPGetFast64(_Atomic(uint_fast64_t) * _Nonnull ptr) {
    assert(ptr);
    return atomic_load(ptr);
}
static inline _Bool CPCASSetFast64(_Atomic(uint_fast64_t) * _Nonnull ptr, uint64_t oldValue, uint64_t newValue) {
    assert(ptr);
    
    return atomic_compare_exchange_weak(ptr, &oldValue, newValue);
}

static inline void __CPAdd(size_t size) {
    uint64_t oldSize = 0;
    uint64_t newSize = 0;
    _Atomic(uint_fast64_t) * sizePtr = &(CPMemoryManagerDefault()->usedMemory);
    do {
        oldSize = CPGetFast64(sizePtr);
        newSize = oldSize + size;
    } while(!(CPCASSetFast64(sizePtr, oldSize, newSize)));
    
    uint64_t oldPtrCount = 0;
    uint64_t newPtrCount = 0;
    _Atomic(uint_fast64_t) * ptrCountPtr = &(CPMemoryManagerDefault()->ptrCount);
    do {
        oldPtrCount = CPGetFast64(ptrCountPtr);
        newPtrCount = oldPtrCount + 1;
    } while(!(CPCASSetFast64(ptrCountPtr, oldPtrCount, newPtrCount)));
}
static inline void __CPRemove(size_t size) {
    uint64_t oldSize = 0;
    uint64_t newSize = 0;
    _Atomic(uint_fast64_t) * sizePtr = &(CPMemoryManagerDefault()->usedMemory);
    do {
        oldSize = CPGetFast64(sizePtr);
        newSize = oldSize - size;
    } while(!(CPCASSetFast64(sizePtr, oldSize, newSize)));
    
    uint64_t oldPtrCount = 0;
    uint64_t newPtrCount = 0;
    _Atomic(uint_fast64_t) * ptrCountPtr = &(CPMemoryManagerDefault()->ptrCount);
    do {
        oldPtrCount = CPGetFast64(ptrCountPtr);
        newPtrCount = oldPtrCount - 1;
    } while(!(CPCASSetFast64(ptrCountPtr, oldPtrCount, newPtrCount)));
}



static inline void * _Nullable CBaseAlloc(size_t size) {
    void * ptr = NULL;
    
    if (size == 0) {
        abort();
    }

    ptr = malloc(size);
    if (NULL != ptr) {
        __CPAdd(size);
    }
    return ptr;
}

static inline void CBaseFree(void * _Nonnull obj, size_t size) {
    assert(obj);
    free(obj);
    __CPRemove(size);
}


static size_t const CPContentSizeOffset = CPInfoStoreSize + CPContentAligentBlock;

static inline void _CPCheckContentSize(CPType _Nonnull type, CPActiveInfo_s * _Nonnull activeInfo) {
    assert(type);
    assert(activeInfo);
    
#if CPMemoryHeaderAligent64
    if ((type->base.contentHasPadding == 1 && activeInfo->contentSize == 0) || (type->base.contentHasPadding == 0 && activeInfo->contentSize != 0)) {
        abort();
    }
#else
    if (type->base.contentHasPadding != activeInfo->contentSize) {
        abort();
    }
#endif
}
static inline size_t __CPObjectGetContentSize(void * _Nonnull sizeAddress) {
    assert(sizeAddress);
    
#if CPMemoryHeaderAligent64
    uint64_t v = *(uint64_t *)sizeAddress;
    return (size_t)v;
#else
    uint32_t v = *(uint32_t *)sizeAddress;
    return (size_t)v;
#endif
}

static inline CPAllocedMemory_s __CPGetMemoryStoreInfo(CPObject _Nonnull obj) {
    assert(obj);
    
    CPInfoStorage_s * info = (CPInfoStorage_s *)((uint8_t *)obj - CPInfoStoreSize);
    CPType type = CPGetType(info);
    CPActiveInfo_s activeInfo = CPGetActiveInfo(info);
    size_t customInfoSize = (size_t)(type->base.customInfoSize * CPContentAligentBlock);
    _CPCheckContentSize(type, &activeInfo);
    CPAllocedMemory_s m = {};
    if (activeInfo.contentSize == 0) {
        size_t offset = CPInfoStoreSize - customInfoSize;
        size_t contentSize = CPAlignContentSize(type->base.contentBaseSize);
        m.ptr = ((uint8_t *)obj - offset);
        m.size = contentSize + offset;
        return m;
    } else if (activeInfo.contentSize == CPMaxContentSizeInActiveInfo) {//有长度信息
        size_t offset = CPContentSizeOffset + customInfoSize;
        size_t contentSize = __CPObjectGetContentSize((uint8_t *)obj - CPContentSizeOffset);
        m.ptr = ((uint8_t *)obj - offset);
        m.size = contentSize + offset;
        return m;
    } else {
        size_t offset = CPInfoStoreSize + customInfoSize;
        size_t contentSize = activeInfo.contentSize * CPContentAligentBlock;
        m.ptr = ((uint8_t *)obj - offset);
        m.size = contentSize + offset;
        return m;
    }
}


static inline void CPInfoStorageStoreType(CPInfoStorage_s * _Nonnull ptr, CPType _Nonnull type) {
    uintptr_t typeValue = (uintptr_t)type;
    atomic_store(&(ptr->type), typeValue);
}
static inline void CPInfoStorageStoreActiveInfo(CPInfoStorage_s * _Nonnull ptr, CPActiveInfo_s * _Nonnull activeInfo) {
#if CPMemoryHeaderAligent64
    uint64_t infoValue = *(uint64_t *)(activeInfo);
    atomic_store(&(ptr->activeInfo), infoValue);
#else
    uint32_t infoValue = *(uint32_t *)(activeInfo);
    atomic_store(&(ptr->activeInfo), infoValue);
#endif
}

#if CPMemoryHeaderAligent64

static size_t CPContentSizeByteCount = sizeof(uint64_t);
static inline void __CPStoreContentSize(void * _Nonnull ptr, size_t size) {
    uint64_t * p = (uint64_t *)ptr;
    uint64_t value = (uint64_t)size;
    *p = value;
    CPMemoryBarrier();
}

static inline void CPInfoStorageStore(CPInfoStorage_s * _Nonnull ptr, CPType _Nonnull type, CPActiveInfo_s * _Nonnull activeInfo) {
    uintptr_t typeValue = (uintptr_t)type;
    uint64_t infoValue = *(uint64_t *)(activeInfo);
    atomic_store(&(ptr->type), typeValue);
    atomic_store(&(ptr->activeInfo), infoValue);
}

#else

static size_t CPContentSizeByteCount = sizeof(uint32_t);
static inline void __CPStoreContentSize(void * _Nonnull ptr, size_t size) {
    uint32_t * p = (uint32_t *)ptr;
    uint32_t value = size;
    *p = value;
    CPMemoryBarrier();
}

static inline void CPInfoStorageStore(CPInfoStorage_s * _Nonnull ptr, CPType _Nonnull type, CPActiveInfo_s * _Nonnull activeInfo) {
    uintptr_t typeValue = (uintptr_t)type;
    uint32_t infoValue = *(uint32_t *)(activeInfo);
    atomic_store(&(ptr->type), typeValue);
    atomic_store(&(ptr->activeInfo), infoValue);
}

#endif





#endif



