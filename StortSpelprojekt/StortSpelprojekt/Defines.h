#pragma once

#define ALIGN16 __declspec(align(16))
#define RELEASE(cptr) if(cptr) { cptr->Release(); cptr = nullptr; }

#define FCAST(T) static_cast<float>(T)
#define DCAST(T) static_cast<double>(T)
#define ICAST(T) static_cast<int>(T)
#define UICAST(T) static_cast<unsigned int>(T)

#define MM_MALLOC_OPERATOR void* operator new(size_t i) { return _mm_malloc(i, 16); }
#define MM_FREE_OPERATOR void operator delete(void* p){ _mm_free(p); }
#define ALIGN16_ALLOC MM_MALLOC_OPERATOR MM_FREE_OPERATOR