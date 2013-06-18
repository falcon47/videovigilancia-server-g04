#include "riff.h"


uint32_t char2int(const char * b)
{
    return *(reinterpret_cast<uint32_t *>(reinterpret_cast<void *>(const_cast<char*>(b))));
}

char * int2char4(uint32_t b)
{
    return reinterpret_cast<char *>(reinterpret_cast<void *>(b));
}

void* alignPointer(const void* pointer, int alignment)
{
    uintptr_t p = reinterpret_cast<uintptr_t>(pointer);
    uintptr_t alignmentMinusOne = alignment - 1;
    return reinterpret_cast<void*>((p + alignmentMinusOne) & ~alignmentMinusOne);
}

int impar(const void * ptr)
{
    uintptr_t p = reinterpret_cast<uintptr_t>(ptr);
    return p & 0x01;
}
