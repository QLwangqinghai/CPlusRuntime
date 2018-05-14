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

static inline void * _Nullable CBaseAlloc(size_t size) {
    void * ptr = NULL;
    
    if (size == 0) {
        abort();
    }

    ptr = malloc(size);
    if (NULL != ptr) {
        __CPAdd(size);
    }
    printf("alloc %p\n", ptr);
    
    return ptr;
}

static inline void CBaseFree(void * _Nonnull obj, size_t size) {
    assert(obj);
    printf("cfree %p\n", obj);

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



