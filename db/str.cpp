#define STR_IMPLEMENTATION
#include "str.h"


//-------------------------------------------------------------------------
// IMPLEMENTATION
//-------------------------------------------------------------------------

#ifdef STR_IMPLEMENTATION

#include <stdio.h> // for vsnprintf

// On some platform vsnprintf() takes va_list by reference and modifies it.
// va_copy is the 'correct' way to copy a va_list but Visual Studio prior to 2013 doesn't have it.
#ifndef va_copy
#define va_copy(dest, src) (dest = src)
#endif

// Static empty buffer we can point to for empty strings
// Pointing to a literal increases the likelihood of getting a crash if someone attempts to write in the empty string buffer.
char*   Str::EmptyBuffer = (char*)"\0NULL";

// Clear
void    Str::clear()
{
    if (Owned && !is_using_local_buf())
        STR_MEMFREE(Data);
    if (LocalBufSize)
    {
        Data = local_buf();
        Data[0] = '\0';
        Capacity = LocalBufSize;
        Owned = 1;
        Length = 0;
    }
    else
    {
        Data = EmptyBuffer;
        Capacity = 0;
        Owned = 0;
        Length = 0;
    }
}

// Reserve memory, preserving the current contents of the buffer
void    Str::reserve(int new_capacity)
{
    if (new_capacity <= Capacity)
        return;

    char* new_data;
    if (new_capacity < LocalBufSize)
    {
        // Disowned -> LocalBuf
        new_data = local_buf();
        new_capacity = LocalBufSize;
    }
    else
    {
        // Disowned or LocalBuf -> Heap
        new_data = (char*)STR_MEMALLOC((size_t)new_capacity * sizeof(char));
    }

    // string in Data might be longer than new_capacity if it wasn't owned, don't copy too much
#ifdef _MSC_VER
    strncpy_s(new_data, (size_t)new_capacity, Data, (size_t)new_capacity - 1);
#else
    strncpy(new_data, Data, (size_t)new_capacity - 1);
#endif
    new_data[new_capacity - 1] = 0;

    if (Owned && !is_using_local_buf())
        STR_MEMFREE(Data);

    Data = new_data;
    Capacity = new_capacity;
    Owned = 1;
}

// Reserve memory, discarding the current contents of the buffer (if we expect to be fully rewritten)
void    Str::reserve_discard(int new_capacity)
{
    if (new_capacity <= Capacity)
        return;

    if (Owned && !is_using_local_buf())
        STR_MEMFREE(Data);

    if (new_capacity < LocalBufSize)
    {
        // Disowned -> LocalBuf
        Data = local_buf();
        Capacity = LocalBufSize;
    }
    else
    {
        // Disowned or LocalBuf -> Heap
        Data = (char*)STR_MEMALLOC((size_t)new_capacity * sizeof(char));
        Capacity = new_capacity;
    }
    Owned = 1;
    //Length = 0; // FIXME ?
}

void    Str::shrink_to_fit()
{
    if (!Owned || is_using_local_buf())
        return;
    int new_capacity = length() + 1;
    if (Capacity <= new_capacity)
        return;

    char* new_data = (char*)STR_MEMALLOC((size_t)new_capacity * sizeof(char));
    memcpy(new_data, Data, (size_t)new_capacity);
    STR_MEMFREE(Data);
    Data = new_data;
    Capacity = new_capacity;
}

// FIXME: merge setfv() and appendfv()?
int     Str::setfv(const char* fmt, va_list args)
{
    // Needed for portability on platforms where va_list are passed by reference and modified by functions
    va_list args2;
    va_copy(args2, args);

    // MSVC returns -1 on overflow when writing, which forces us to do two passes
    // FIXME-OPT: Find a way around that.
#ifdef _MSC_VER
    int len = vsnprintf(NULL, 0, fmt, args);
    STR_ASSERT(len >= 0);

    if (Capacity < len + 1)
        reserve_discard(len + 1);
    len = vsnprintf(Data, len + 1, fmt, args2);
#else
    // First try
    int len = vsnprintf(Owned ? Data : NULL, Owned ? (size_t)Capacity : 0, fmt, args);
    STR_ASSERT(len >= 0);

    if (Capacity < len + 1)
    {
        reserve_discard(len + 1);
        len = vsnprintf(Data, (size_t)len + 1, fmt, args2);
    }
#endif
    Length = len;
    STR_ASSERT(Owned);
    return len;
}

int     Str::setf(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int len = setfv(fmt, args);
    va_end(args);
    Length = len;
    return len;
}

int     Str::setfv_nogrow(const char* fmt, va_list args)
{
    STR_ASSERT(Owned);

    if (Capacity == 0)
        return 0;

    int w = vsnprintf(Data, (size_t)Capacity, fmt, args);
    Data[Capacity - 1] = 0;
    Owned = 1;
    Length = (w == -1) ? Capacity - 1 : w;
    return (w == -1) ? Capacity - 1 : w;
}

int     Str::setf_nogrow(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int len = setfv_nogrow(fmt, args);
    va_end(args);
    Length = len;
    return len;
}

int     Str::append_from(int idx, char c)
{
    int add_len = 1;
    if (Capacity < idx + add_len + 1)
        reserve(idx + add_len + 1);
    Data[idx] = c;
    Data[idx + add_len] = 0;
    STR_ASSERT(Owned);
    Length += add_len;
    return add_len;
}

int     Str::append_from(int idx, const char* s, const char* s_end)
{
    if (!s_end)
        s_end = s + strlen(s);
    int add_len = (int)(s_end - s);
    if (Capacity < idx + add_len + 1)
        reserve(idx + add_len + 1);
    memcpy(Data + idx, (const void*)s, (size_t)add_len);
    Data[idx + add_len] = 0; // Our source data isn't necessarily zero-terminated
    STR_ASSERT(Owned);
    Length += add_len;
    return add_len;
}

// FIXME: merge setfv() and appendfv()?
int     Str::appendfv_from(int idx, const char* fmt, va_list args)
{
    // Needed for portability on platforms where va_list are passed by reference and modified by functions
    va_list args2;
    va_copy(args2, args);

    // MSVC returns -1 on overflow when writing, which forces us to do two passes
    // FIXME-OPT: Find a way around that.
#ifdef _MSC_VER
    int add_len = vsnprintf(NULL, 0, fmt, args);
    STR_ASSERT(add_len >= 0);

    if (Capacity < idx + add_len + 1)
        reserve(idx + add_len + 1);
    add_len = vsnprintf(Data + idx, add_len + 1, fmt, args2);
#else
    // First try
    int add_len = vsnprintf(Owned ? Data + idx : NULL, Owned ? (size_t)(Capacity - idx) : 0, fmt, args);
    STR_ASSERT(add_len >= 0);

    if (Capacity < idx + add_len + 1)
    {
        reserve(idx + add_len + 1);
        add_len = vsnprintf(Data + idx, (size_t)add_len + 1, fmt, args2);
    }
#endif

    STR_ASSERT(Owned);
    Length += add_len;
    return add_len;
}

int     Str::appendf_from(int idx, const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int len = appendfv_from(idx, fmt, args);
    va_end(args);
    Length = idx + len;
    return len;
}

int     Str::append(char c)
{
    int cur_len = length();
    return append_from(cur_len, c);
}
int     Str::append(int num, char c)
{
    int len  = 0;
    int cur_len = length();
        if (Capacity < cur_len + num + 1)
        reserve(cur_len + num + 1);
        for (int i =0; i < num; ++i)
        {
            len = append_from(cur_len + i, c);
        }
        return len;
}
int     Str::append(const char* s, const char* s_end)
{
    int cur_len = length();
    return append_from(cur_len, s, s_end);
}

int     Str::append(const char* s, int size)
{
    int cur_len = length();
    return append_from(cur_len, s, s + size);
}

int     Str::appendfv(const char* fmt, va_list args)
{
    int cur_len = length();
    return appendfv_from(cur_len, fmt, args);
}

int     Str::appendf(const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int len = appendfv(fmt, args);
    va_end(args);
    return len;
}

int     Str::replace(int idx, int src_size, const char* dest, int dest_size)
{
    int add_len = (int)(dest_size - src_size);
    if (Capacity < idx + add_len + 1)
        reserve(idx + add_len + 1);
    if(add_len == 0) { // inplace
        memcpy(Data + idx, (const void*)dest, (size_t)dest_size);
    }
    else if(add_len > 0) { // dest_size greet than src_size
        memmove(Data + idx + dest_size, Data + idx + src_size, Length - idx - src_size);
        memcpy(Data + idx, (const void*)dest, (size_t)dest_size);
    }
    else if(add_len < 0) { // 
        memmove(Data + idx + dest_size, Data + idx + src_size, Length - idx - src_size);
        memcpy(Data + idx, (const void*)dest, (size_t)dest_size);
    }
    Data[idx + add_len] = 0; // Our source data isn't necessarily zero-terminated
    STR_ASSERT(Owned);
    Length += add_len;
    return add_len;
}


#endif // #define STR_IMPLEMENTATION

//-------------------------------------------------------------------------
