//
//  CPlusRuntimePublic.h
//  O
//
//  Created by wangqinghai on 2018/3/16.
//  Copyright © 2018年 wangqinghai. All rights reserved.
//

#ifndef CPlusRuntimePublic_h
#define CPlusRuntimePublic_h

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <assert.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>

#ifndef C_TARGET_DEFINE
#define C_TARGET_DEFINE

#if __APPLE__
#define C_TARGET_OS_DARWIN       1
#define C_TARGET_OS_LINUX        0
#define C_TARGET_OS_WINDOWS      0
#define C_TARGET_OS_BSD          0
#define C_TARGET_OS_ANDROID      0
#define C_TARGET_OS_CYGWIN       0
#elif __ANDROID__
#define C_TARGET_OS_DARWIN       0
#define C_TARGET_OS_LINUX        1
#define C_TARGET_OS_WINDOWS      0
#define C_TARGET_OS_BSD          0
#define C_TARGET_OS_ANDROID      1
#define C_TARGET_OS_CYGWIN       0
#elif __linux__
#define C_TARGET_OS_DARWIN       0
#define C_TARGET_OS_LINUX        1
#define C_TARGET_OS_WINDOWS      0
#define C_TARGET_OS_BSD          0
#define C_TARGET_OS_ANDROID      0
#define C_TARGET_OS_CYGWIN       0
#elif __CYGWIN__
#define C_TARGET_OS_DARWIN       0
#define C_TARGET_OS_LINUX        1
#define C_TARGET_OS_WINDOWS      0
#define C_TARGET_OS_BSD          0
#define C_TARGET_OS_ANDROID      0
#define C_TARGET_OS_CYGWIN       1
#elif _WIN32 || _WIN64
#define C_TARGET_OS_DARWIN       0
#define C_TARGET_OS_LINUX        0
#define C_TARGET_OS_WINDOWS      1
#define C_TARGET_OS_BSD          0
#define C_TARGET_OS_ANDROID      0
#elif __unix__
#define C_TARGET_OS_DARWIN       0
#define C_TARGET_OS_LINUX        0
#define C_TARGET_OS_WINDOWS      0
#define C_TARGET_OS_BSD          1
#define C_TARGET_OS_ANDROID      0
#else
#error unknown operating system
#endif

#if __x86_64__
#define CPTARGET_CPU_PPC          0
#define CPTARGET_CPU_PPC64        0
#define CPTARGET_CPU_X86          0
#define CPTARGET_CPU_X86_64       1
#define CPTARGET_CPU_ARM          0
#define CPTARGET_CPU_ARM64        0
#define CPTARGET_CPU_MIPS         0
#define CPTARGET_CPU_MIPS64       0
#define CPTARGET_CPU_S390X        0
#elif __arm64__ || __aarch64__
#define CPTARGET_CPU_PPC          0
#define CPTARGET_CPU_PPC64        0
#define CPTARGET_CPU_X86          0
#define CPTARGET_CPU_X86_64       0
#define CPTARGET_CPU_ARM          0
#define CPTARGET_CPU_ARM64        1
#define CPTARGET_CPU_MIPS         0
#define CPTARGET_CPU_MIPS64       0
#define CPTARGET_CPU_S390X        0
#elif __mips64__
#define CPTARGET_CPU_PPC          0
#define CPTARGET_CPU_PPC64        0
#define CPTARGET_CPU_X86          0
#define CPTARGET_CPU_X86_64       0
#define CPTARGET_CPU_ARM          0
#define CPTARGET_CPU_ARM64        0
#define CPTARGET_CPU_MIPS         0
#define CPTARGET_CPU_MIPS64       1
#define CPTARGET_CPU_S390X        0
#elif __powerpc64__
#define CPTARGET_CPU_PPC          0
#define CPTARGET_CPU_PPC64        1
#define CPTARGET_CPU_X86          0
#define CPTARGET_CPU_X86_64       0
#define CPTARGET_CPU_ARM          0
#define CPTARGET_CPU_ARM64        0
#define CPTARGET_CPU_MIPS         0
#define CPTARGET_CPU_MIPS64       0
#define CPTARGET_CPU_S390X        0
#elif __i386__
#define CPTARGET_CPU_PPC          0
#define CPTARGET_CPU_PPC64        0
#define CPTARGET_CPU_X86          1
#define CPTARGET_CPU_X86_64       0
#define CPTARGET_CPU_ARM          0
#define CPTARGET_CPU_ARM64        0
#define CPTARGET_CPU_MIPS         0
#define CPTARGET_CPU_MIPS64       0
#define CPTARGET_CPU_S390X        0
#elif __arm__
#define CPTARGET_CPU_PPC          0
#define CPTARGET_CPU_PPC64        0
#define CPTARGET_CPU_X86          0
#define CPTARGET_CPU_X86_64       0
#define CPTARGET_CPU_ARM          1
#define CPTARGET_CPU_ARM64        0
#define CPTARGET_CPU_MIPS         0
#define CPTARGET_CPU_MIPS64       0
#define CPTARGET_CPU_S390X        0
#elif __mips__
#define CPTARGET_CPU_PPC          0
#define CPTARGET_CPU_PPC64        0
#define CPTARGET_CPU_X86          0
#define CPTARGET_CPU_X86_64       0
#define CPTARGET_CPU_ARM          0
#define CPTARGET_CPU_ARM64        0
#define CPTARGET_CPU_MIPS         1
#define CPTARGET_CPU_MIPS64       0
#define CPTARGET_CPU_S390X        0
#elif __powerpc__
#define CPTARGET_CPU_PPC          1
#define CPTARGET_CPU_PPC64        0
#define CPTARGET_CPU_X86          0
#define CPTARGET_CPU_X86_64       0
#define CPTARGET_CPU_ARM          0
#define CPTARGET_CPU_ARM64        0
#define CPTARGET_CPU_MIPS         0
#define CPTARGET_CPU_MIPS64       0
#define CPTARGET_CPU_S390X        0
#elif __s390x__
#define CPTARGET_CPU_PPC          0
#define CPTARGET_CPU_PPC64        0
#define CPTARGET_CPU_X86          0
#define CPTARGET_CPU_X86_64       0
#define CPTARGET_CPU_ARM          0
#define CPTARGET_CPU_ARM64        0
#define CPTARGET_CPU_MIPS         0
#define CPTARGET_CPU_MIPS64       0
#define CPTARGET_CPU_S390X        1
#else
#error unknown architecture
#endif

#if __LITTLE_ENDIAN__
#define CPTARGET_RT_LITTLE_ENDIAN 1
#define CPTARGET_RT_BIG_ENDIAN    0
#elif __BIG_ENDIAN__
#define CPTARGET_RT_LITTLE_ENDIAN 0
#define CPTARGET_RT_BIG_ENDIAN    1
#else
#error unknown endian
#endif

#if __LP64__
#define CPTARGET_RT_64_BIT        1
#else
#define CPTARGET_RT_64_BIT        0
#endif

#if C_TARGET_OS_WINDOWS || C_TARGET_OS_CYGWIN
#error unsuport system
#endif

#endif


#if __GNUC__
#define CPMemoryBarrier()  __asm__ __volatile__("" ::: "memory");
#else
#define CPMemoryBarrier()  do { } while (0);
#endif



#ifndef CPMemoryHeaderAligent64
    #if CPTARGET_RT_64_BIT
        static size_t const CPMaxContentSize = (SIZE_T_MAX / 8 - 128) * 8;
        #define CPMemoryHeaderAligent64 1
    #else
    static size_t const CPMaxContentSize = (SIZE_T_MAX / 4 - 128) * 4;
        #define CPMemoryHeaderAligent64 0
    #endif
#endif

typedef float float32_t;
typedef double float64_t;


#pragma mark - base

/********************************* CPMemoryManage *********************************/

uint64_t CPMemoryManagerGetUsedMemorySize(void);
uint64_t CPMemoryManagerGetPtrCount(void);




#if CPMemoryHeaderAligent64

#pragma pack(push)
#pragma pack(1)
static size_t const CPMaxContentSizeInActiveInfo = 0xFFFF;

typedef struct {
    uint64_t contentSize: 16;
    uint64_t deallocing: 1;//on this time, deinit
    uint64_t prepareDealloc: 1;
    uint64_t autoDealloc: 1;
    uint64_t refrenceCount: 45;
} CPActiveInfo_s;
typedef struct {
    _Atomic(uintptr_t) type;
    _Atomic(uint_fast64_t) activeInfo;
} CPInfoStorage_s;
typedef struct {
    void const * _Nonnull type;
    CPActiveInfo_s activeInfo;
} CPInfo_s;

#pragma pack(pop)
static uint64_t const CPRefrenceStaticFlag = 0x1FFFFFFFFFFFLLU;
static size_t const CPContentAligentBlock = sizeof(uint64_t);

#else

#pragma pack(push)
#pragma pack(1)
static size_t const CPMaxContentSizeInActiveInfo = 0x1;

typedef struct {
    uint32_t contentSize: 1;
    uint32_t deallocing: 1;
    uint32_t prepareDealloc: 1;
    uint32_t autoDealloc: 1;
    uint32_t refrenceCount: 28;
} CPActiveInfo_s;
typedef struct {
    _Atomic(uintptr_t) type;
    _Atomic(uint_fast32_t) activeInfo;
} CPInfoStorage_s;

typedef struct {
    void const * _Nonnull type;
    CPActiveInfo_s activeInfo;
} CPInfo_s;

#pragma pack(pop)
static size_t const CPContentAligentBlock = sizeof(uint32_t);
static uint64_t const CPRefrenceStaticFlag = 0xFFFFFFFLU;
#endif

static uint32_t const CPInfoStoreSize = sizeof(CPInfoStorage_s);
typedef void const * CPObject;

#pragma pack(push)
#pragma pack(1)

typedef struct __CPTypeBaseInfo {
    uint32_t isImmutable: 1;//a const type, must be 1
    uint32_t domain: 24;
    uint32_t contentHasPadding: 1;
    uint32_t customInfoSize: 6;// * CPContentAligentBlock
    size_t contentBaseSize;// * 1
    char const * _Nonnull name;
    void const * _Nullable superType;
    void const * _Nullable alloctor;//CPAlloctor_s *
    void (* _Nullable deinit)(void const * _Nonnull object);//有子类向父类递归调用
} CPTypeBaseInfo_s;

typedef struct __CPLifeCycleCallback_s {
    void (* _Nullable didRetain)(void const * _Nonnull typeInfo, CPObject _Nonnull obj);
    void (* _Nullable didRelease)(void const * _Nonnull typeInfo, CPObject _Nonnull obj);
    void (* _Nullable didPrepareDealloc)(void const * _Nonnull typeInfo, CPObject _Nonnull obj);
    void (* _Nullable willDealloc)(void const * _Nonnull typeInfo, CPObject _Nonnull obj);
    void (* _Nullable didResignActive)(void const * _Nonnull typeInfo, CPObject _Nonnull ptr);
} CPLifeCycleCallback_s;
typedef struct __CPType {
    CPTypeBaseInfo_s base;
    CPLifeCycleCallback_s * _Nullable callbacks;
    void * _Nullable context;
} CPType_s;

typedef struct __CPHeader {
    CPInfoStorage_s info;
    CPType_s type;
} CPHeader_s;

typedef CPType_s const * CPType;
static uint32_t const CCTypeDomain = 1;

typedef struct {
    CPInfo_s info;
    CPType_s type;
} CPTypeLayout_s;

#pragma pack(pop)

static char const * _Nonnull const CPTypeNameType = "Type";
static CPTypeLayout_s const CPTypeStorage_Type = {
#if CPMemoryHeaderAligent64
    .info = {
        .type = NULL,
        .activeInfo = {
            .contentSize = 0,
            .deallocing = 0,
            .prepareDealloc = 0,
            .autoDealloc = 0,
            .refrenceCount = CPRefrenceStaticFlag,
        }
    },
#else
    .info = {
        .type = NULL,
        .activeInfo = {
            .contentSize = 0,
            .deallocing = 0,
            .prepareDealloc = 0,
            .autoDealloc = 0,
            .refrenceCount = CPRefrenceStaticFlag,
        }
    },
#endif
    .type = {
        .base = {
            .isImmutable = 1,
            .domain = CCTypeDomain,
            .contentHasPadding = 0,
            .customInfoSize = 0,
            .contentBaseSize = sizeof(CPType_s),
            .name = CPTypeNameType,
            .superType = NULL,
            .alloctor = NULL,
            .deinit = NULL,
        },
        .callbacks = NULL,
        .context = NULL,
    },
};

#define CPType_Type (&(CPTypeStorage_Type.type))

#define CPInfoDefaultTypeInfo {\
    .type = CPType_Type,\
    .activeInfo = {\
        .contentSize = 0,\
        .deallocing = 0,\
        .prepareDealloc = 0,\
        .autoDealloc = 0,\
        .refrenceCount = CPRefrenceStaticFlag,\
    }\
}\

static inline void CPSuperDidRetain(CPType _Nonnull Self, CPObject _Nonnull obj) {
    assert(Self);
    assert(obj);
    
    CPType Super = Self;
    int hasRing = -1;//-1 未知， 0: 没有, 1: 有环
    CPType QuickSuper = Self;

    do {
        Super = (CPType)(Super->base.superType);
        if (Super == NULL) {
            return;
        }
        if (hasRing == -1) {
            QuickSuper = QuickSuper->base.superType;
            if (QuickSuper) {
                QuickSuper = QuickSuper->base.superType;
                if (Super == QuickSuper) {
                    hasRing = 1;
                    printf("class error, class list has ring! \n");
                    abort();
                }
            } else {
                hasRing = 0;
            }
        }
    } while (Super->callbacks == NULL || Super->callbacks->didRetain == NULL);
    Super->callbacks->didRetain(Super, obj);
}
static inline void CPSuperDidRelease(CPType _Nonnull Self, CPObject _Nonnull obj) {
    CPType Super = Self;
    int hasRing = -1;//-1 未知， 0: 没有, 1: 有环
    CPType QuickSuper = Self;
    
    do {
        Super = (CPType)(Super->base.superType);
        if (Super == NULL) {
            return;
        }
        if (hasRing == -1) {
            QuickSuper = QuickSuper->base.superType;
            if (QuickSuper) {
                QuickSuper = QuickSuper->base.superType;
                if (Super == QuickSuper) {
                    hasRing = 1;
                    printf("class error, class list has ring! \n");
                    abort();
                }
            } else {
                hasRing = 0;
            }
        }
    } while (Super->callbacks == NULL || Super->callbacks->didRelease == NULL);
    Super->callbacks->didRelease(Super, obj);
}
static inline void CPSuperDidPrepareDealloc(CPType _Nonnull Self, CPObject _Nonnull obj) {
    CPType Super = Self;
    int hasRing = -1;//-1 未知， 0: 没有, 1: 有环
    CPType QuickSuper = Self;
    
    do {
        Super = (CPType)(Super->base.superType);
        if (Super == NULL) {
            return;
        }
        if (hasRing == -1) {
            QuickSuper = QuickSuper->base.superType;
            if (QuickSuper) {
                QuickSuper = QuickSuper->base.superType;
                if (Super == QuickSuper) {
                    hasRing = 1;
                    printf("class error, class list has ring! \n");
                    abort();
                }
            } else {
                hasRing = 0;
            }
        }
    } while (Super->callbacks == NULL || Super->callbacks->didPrepareDealloc == NULL);
    Super->callbacks->didPrepareDealloc(Super, obj);
}
static inline void CPSuperWillDealloc(CPType _Nonnull Self, CPObject _Nonnull obj) {
    CPType Super = Self;
    int hasRing = -1;//-1 未知， 0: 没有, 1: 有环
    CPType QuickSuper = Self;
    
    do {
        Super = (CPType)(Super->base.superType);
        if (Super == NULL) {
            return;
        }
        if (hasRing == -1) {
            QuickSuper = QuickSuper->base.superType;
            if (QuickSuper) {
                QuickSuper = QuickSuper->base.superType;
                if (Super == QuickSuper) {
                    hasRing = 1;
                    printf("class error, class list has ring! \n");
                    abort();
                }
            } else {
                hasRing = 0;
            }
        }
    } while (Super->callbacks == NULL || Super->callbacks->willDealloc == NULL);
    Super->callbacks->willDealloc(Super, obj);
}
static inline void CPSuperDidResignActive(CPType _Nonnull Self, CPObject _Nonnull obj) {
    CPType Super = Self;
    int hasRing = -1;//-1 未知， 0: 没有, 1: 有环
    CPType QuickSuper = Self;
    
    do {
        Super = (CPType)(Super->base.superType);
        if (Super == NULL) {
            return;
        }
        if (hasRing == -1) {
            QuickSuper = QuickSuper->base.superType;
            if (QuickSuper) {
                QuickSuper = QuickSuper->base.superType;
                if (Super == QuickSuper) {
                    hasRing = 1;
                    printf("class error, class list has ring! \n");
                    abort();
                }
            } else {
                hasRing = 0;
            }
        }
    } while (Super->callbacks == NULL ||Super->callbacks->didResignActive == NULL);
    Super->callbacks->didResignActive(Super, obj);
}

static inline void CPDidRetainDispatch(CPType _Nonnull Self, CPObject _Nonnull obj) {
    assert(Self);
    assert(obj);
    
    CPType Super = Self;
    int hasRing = -1;//-1 未知， 0: 没有, 1: 有环
    CPType QuickSuper = Self;
    
    while (Super->callbacks == NULL || Super->callbacks->didRetain == NULL) {
        Super = (CPType)(Super->base.superType);
        if (Super == NULL) {
            return;
        }
        if (hasRing == -1) {
            QuickSuper = QuickSuper->base.superType;
            if (QuickSuper) {
                QuickSuper = QuickSuper->base.superType;
                if (Super == QuickSuper) {
                    hasRing = 1;
                    printf("class error, class list has ring! \n");
                    abort();
                }
            } else {
                hasRing = 0;
            }
        }
    }
    Super->callbacks->didRetain(Super, obj);
}
static inline void CPDidReleaseDispatch(CPType _Nonnull Self, CPObject _Nonnull obj) {
    CPType Super = Self;
    int hasRing = -1;//-1 未知， 0: 没有, 1: 有环
    CPType QuickSuper = Self;
    
    while (Super->callbacks == NULL ||Super->callbacks->didRelease == NULL) {
        Super = (CPType)(Super->base.superType);
        if (Super == NULL) {
            return;
        }
        if (hasRing == -1) {
            QuickSuper = QuickSuper->base.superType;
            if (QuickSuper) {
                QuickSuper = QuickSuper->base.superType;
                if (Super == QuickSuper) {
                    hasRing = 1;
                    printf("class error, class list has ring! \n");
                    abort();
                }
            } else {
                hasRing = 0;
            }
        }
    };
    Super->callbacks->didRelease(Super, obj);
}
static inline void CPDidPrepareDeallocDispatch(CPType _Nonnull Self, CPObject _Nonnull obj) {
    CPType Super = Self;
    int hasRing = -1;//-1 未知， 0: 没有, 1: 有环
    CPType QuickSuper = Self;
    
    while (Super->callbacks == NULL ||Super->callbacks->didPrepareDealloc == NULL) {
        Super = (CPType)(Super->base.superType);
        if (Super == NULL) {
            return;
        }
        if (hasRing == -1) {
            QuickSuper = QuickSuper->base.superType;
            if (QuickSuper) {
                QuickSuper = QuickSuper->base.superType;
                if (Super == QuickSuper) {
                    hasRing = 1;
                    printf("class error, class list has ring! \n");
                    abort();
                }
            } else {
                hasRing = 0;
            }
        }
    };
    Super->callbacks->didPrepareDealloc(Super, obj);
}
static inline void CPWillDeallocDispatch(CPType _Nonnull Self, CPObject _Nonnull obj) {
    CPType Super = Self;
    int hasRing = -1;//-1 未知， 0: 没有, 1: 有环
    CPType QuickSuper = Self;
    
    while (Super->callbacks == NULL ||Super->callbacks->willDealloc == NULL) {
        Super = (CPType)(Super->base.superType);
        if (Super == NULL) {
            return;
        }
        if (hasRing == -1) {
            QuickSuper = QuickSuper->base.superType;
            if (QuickSuper) {
                QuickSuper = QuickSuper->base.superType;
                if (Super == QuickSuper) {
                    hasRing = 1;
                    printf("class error, class list has ring! \n");
                    abort();
                }
            } else {
                hasRing = 0;
            }
        }
    };
    Super->callbacks->willDealloc(Super, obj);
}
static inline void CPDidResignActiveDispatch(CPType _Nonnull Self, CPObject _Nonnull obj) {
    CPType Super = Self;
    int hasRing = -1;//-1 未知， 0: 没有, 1: 有环
    CPType QuickSuper = Self;
    
    while (Super->callbacks == NULL ||Super->callbacks->didResignActive == NULL) {
        Super = (CPType)(Super->base.superType);
        if (Super == NULL) {
            return;
        }
        if (hasRing == -1) {
            QuickSuper = QuickSuper->base.superType;
            if (QuickSuper) {
                QuickSuper = QuickSuper->base.superType;
                if (Super == QuickSuper) {
                    hasRing = 1;
                    printf("class error, class list has ring! \n");
                    abort();
                }
            } else {
                hasRing = 0;
            }
        }
    };
    Super->callbacks->didResignActive(Super, obj);
}

static inline void CPDeinitDispatch(CPType _Nonnull Self, CPObject _Nonnull obj) {
    CPType Super = Self;
    int hasRing = -1;//-1 未知， 0: 没有, 1: 有环
    CPType QuickSuper = Self;
    
    while (Super != NULL) {
        if (Super->base.deinit != NULL) {
            Super->base.deinit(obj);
        }
        Super = (CPType)(Super->base.superType);
        if (Super == NULL) {
            return;
        }
        if (hasRing == -1) {
            QuickSuper = QuickSuper->base.superType;
            if (QuickSuper) {
                QuickSuper = QuickSuper->base.superType;
                if (Super == QuickSuper) {
                    hasRing = 1;
                    printf("class error, class list has ring! \n");
                    abort();
                }
            } else {
                hasRing = 0;
            }
        }
    };
}

size_t CPGetStoreSizeWithType(CPType _Nonnull obj, size_t paddingSize);
size_t CPGetStoreSize(void * _Nonnull obj);
CPInfoStorage_s * _Nonnull CPGetInfo(void const * _Nonnull obj);


typedef struct __CPAllocedMemory {
    void * _Nonnull ptr;
    size_t size;
} CPAllocedMemory_s;

typedef struct __CPAlloctor {
    CPAllocedMemory_s (* _Nonnull memoryAlloc)(struct __CPAlloctor const * _Nonnull alloctor, size_t size);
    void (* _Nonnull memoryDealloc)(struct __CPAlloctor const * _Nonnull alloctor, void * _Nonnull ptr, size_t size);

//    CPAllocedMemory_s (* _Nonnull alloc)(struct __CPAlloctor const * _Nonnull alloctor, CPType _Nonnull type, size_t contentPaddingSize);
//    CPObject _Nonnull (* _Nonnull init)(CPAllocedMemory_s memory, _Bool autoDealloc, _Bool isStatic);
    void * _Nonnull (* _Nonnull allocInit)(struct __CPAlloctor const * _Nonnull alloctor, CPType _Nonnull type, size_t contentMutableSize, _Bool autoDealloc, _Bool isStatic);
    void (* _Nonnull dealloc)(struct __CPAlloctor const * _Nonnull alloctor, CPObject _Nonnull obj);
    void * _Nullable context;
} CPAlloctor_s;

CPAlloctor_s const * _Nonnull CPAlloctorGetDefault(void);

static inline size_t CPAlignContentSize(size_t size) {
    if (size == 0) {
        return CPContentAligentBlock;
    }
    size_t result = size;
    size_t rsize = result % CPContentAligentBlock;
    if (rsize > 0) {
        result += CPContentAligentBlock - rsize;
    }
    return result;
}


typedef struct __CPMemoryManager {
    uint64_t ptrCount;
    uint64_t usedMemory;
    void (* _Nullable zeroSizeErrorHandler)(struct __CPMemoryManager * _Nonnull manager, void * _Nonnull * _Nullable ptr);
    void (* _Nullable oomHandler)(struct __CPMemoryManager * _Nonnull manager, size_t size);
} CPMemoryManager_t;
CPMemoryManager_t * _Nonnull CPMemoryManagerDefault(void);


static inline void __CPAdd(size_t size) {
    
}
static inline void __CPRemove(size_t size) {
    
}


CPAllocedMemory_s CPBaseAlloc(struct __CPAlloctor const * _Nonnull alloctor, size_t size);
void CPBaseDealloc(struct __CPAlloctor const * _Nonnull alloctor, void * _Nonnull ptr, size_t size);
void * _Nonnull CPAllocInit(struct __CPAlloctor const * _Nonnull alloctor, CPType _Nonnull type, size_t contentPaddingSize, _Bool autoDealloc, _Bool isStatic);
CPAllocedMemory_s CPAlloc(struct __CPAlloctor const * _Nonnull alloctor, CPType _Nonnull type, size_t contentPaddingSize);
void * _Nonnull CPInit(void * _Nonnull ptr, CPType _Nonnull type, size_t contentPaddingSize, _Bool autoDealloc, _Bool isStatic);


void CPDealloc(struct __CPAlloctor const * _Nonnull alloctor, CPObject _Nonnull obj);



//void * _Nonnull CPAlloc(uint32_t domain, uint32_t typeIdentifier, size_t size, uint32_t customInfo, _Bool autoDealloc, _Bool isStatic);


/********************************* object *********************************/
#pragma mark - object

static inline CPActiveInfo_s CPActiveInfoMake(_Bool autoDealloc, uint32_t refrenceCount) {
    CPActiveInfo_s info;
    if (autoDealloc) {
        info.prepareDealloc = 1;
    } else {
        info.prepareDealloc = 0;
    }
    info.refrenceCount = refrenceCount;
    return info;
}

#if CPMemoryHeaderAligent64
static inline uint64_t CPActiveInfoBridgeTo(CPActiveInfo_s info) {
    return *(uint64_t *)(&info);
}
static inline CPActiveInfo_s CPActiveInfoBridgeFrom(uint64_t value) {
    return *(CPActiveInfo_s *)(&value);
}
#else
static inline uint32_t CPActiveInfoBridgeTo(CPActiveInfo_s info) {
    return *(uint32_t *)(&info);
}
static inline CPActiveInfo_s CPActiveInfoBridgeFrom(uint32_t value) {
    return *(CPActiveInfo_s *)(&value);
}
#endif


static inline CPActiveInfo_s CPGetActiveInfo(CPInfoStorage_s * _Nonnull object) {
    assert(object);
    
#if CPMemoryHeaderAligent64
    uint64_t content = atomic_load(&(object->activeInfo));
    return CPActiveInfoBridgeFrom(content);
#else
    uint32_t content = atomic_load(&(object->activeInfo));
    return CPActiveInfoBridgeFrom(content);
#endif
}
static inline CPType_s * _Nonnull CPGetType(CPInfoStorage_s * _Nonnull object) {
    assert(object);
    uintptr_t content = atomic_load(&(object->type));
    assert(content != 0);
    return (CPType_s *)((void *)content);
}

static inline _Bool CPCASSetActiveInfo(CPInfoStorage_s * _Nonnull header, CPActiveInfo_s oldValue, CPActiveInfo_s newValue) {
    assert(header);
    
#if CPMemoryHeaderAligent64
    uint64_t oldContent = CPActiveInfoBridgeTo(oldValue);
    uint64_t newContent = CPActiveInfoBridgeTo(newValue);
    return atomic_compare_exchange_weak(&(header->activeInfo), &oldContent, newContent);
#else
    uint32_t oldContent = CPActiveInfoBridgeTo(oldValue);
    uint32_t newContent = CPActiveInfoBridgeTo(newValue);
    return atomic_compare_exchange_weak(&(header->activeInfo), &oldContent, newContent);
#endif
}

/********************************* MRC *********************************/
void CPAutoreleasePool(void(^_Nonnull block)(void));
CPObject _Nullable CPRetain(void const * _Nullable obj);
void CPRelease(void const * _Nullable obj);
void CPAutorelease(void const * _Nullable obj);


/********************************* init module *********************************/

#pragma mark - init module
__attribute__((constructor(201)))
void CPlusModuleInit(void);


/********************************* weak support *********************************/

//typedef struct _CCWeakContainerFlag {
//    uint32_t useFlag;
//} CCWeakContainerFlag_t;
//
//typedef struct __CPWeakContainer {
//
//    _Atomic(uintptr_t) content;
//
//    struct _CCWeakContainer * _Nullable prev;
//    struct _CCWeakContainer * _Nullable next;
//} CPWeakContainer_o;
//
//typedef struct _CPWeakContainerList {
//    uint8_t content[sizeof(CPWeakContainer_o)];
//} CPWeakContainerList_t;


#endif /* CObjectBase_h */
