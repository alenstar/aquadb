// Str v0.31
// Simple C++ string type with an optional local buffer, by Omar Cornut
// https://github.com/ocornut/str

// LICENSE
//  This software is in the public domain. Where that dedication is not
//  recognized, you are granted a perpetual, irrevocable license to copy,
//  distribute, and modify this file as you see fit.

// USAGE
//  Include this file in whatever places need to refer to it.
//  In ONE .cpp file, write '#define STR_IMPLEMENTATION' before the #include of this file.
//  This expands out the actual implementation into that C/C++ file.


/*
- This isn't a fully featured string class.
- It is a simple, bearable replacement to std::string that isn't heap abusive nor bloated (can actually be debugged by humans).
- String are mutable. We don't maintain size so length() is not-constant time.
- Maximum string size currently limited to 2 MB (we allocate 21 bits to hold capacity).
- Local buffer size is currently limited to 1023 bytes (we allocate 10 bits to hold local buffer size).
- In "non-owned" mode for literals/reference we don't do any tracking/counting of references.
- Overhead is 8-bytes in 32-bits, 16-bytes in 64-bits (12 + alignment).
- This code hasn't been tested very much. it is probably incomplete or broken. Made it for my own use.

The idea is that you can provide an arbitrary sized local buffer if you expect string to fit
most of the time, and then you avoid using costly heap.

No local buffer, always use heap, sizeof()==8~16 (depends if your pointers are 32-bits or 64-bits)

   Str s = "hey";

With a local buffer of 16 bytes, sizeof() == 8~16 + 16 bytes.

   Str16 s = "filename.h"; // copy into local buffer
   Str16 s = "long_filename_not_very_long_but_longer_than_expected.h";   // use heap

With a local buffer of 256 bytes, sizeof() == 8~16 + 256 bytes.

   Str256 s = "long_filename_not_very_long_but_longer_than_expected.h";  // copy into local buffer

Common sizes are defined at the bottom of Str.h, you may define your own.

Functions:

   Str256 s;
   s.set("hello sailor");                   // set (copy)
   s.setf("%s/%s.tmp", folder, filename);   // set (w/format)
   s.append("hello");                       // append. cost a length() calculation!
   s.appendf("hello %d", 42);               // append (w/format). cost a length() calculation!
   s.set_ref("Hey!");                       // set (literal/reference, just copy pointer, no tracking)

Constructor helper for format string: add a trailing 'f' to the type. Underlying type is the same.

   Str256f filename("%s/%s.tmp", folder, filename);             // construct (w/format)
   fopen(Str256f("%s/%s.tmp, folder, filename).c_str(), "rb");  // construct (w/format), use as function param, destruct

Constructor helper for reference/literal:

   StrRef ref("literal");                   // copy pointer, no allocation, no string copy
   StrRef ref2(GetDebugName());             // copy pointer. no tracking of anything whatsoever, know what you are doing!

All StrXXX types derives from Str and instance hold the local buffer capacity. So you can pass e.g. Str256* to a function taking base type Str* and it will be functional.

   void MyFunc(Str& s) { s = "Hello"; }     // will use local buffer if available in Str instance

(Using a template e.g. Str<N> we could remove the LocalBufSize storage but it would make passing typed Str<> to functions tricky.
 Instead we don't use template so you can pass them around as the base type Str*. Also, templates are ugly.)
*/

/*
 CHANGELOG
  0.31 - fixed various warnings.
  0.30 - turned into a single header file, removed Str.cpp.
  0.29 - fixed bug when calling reserve on non-owned strings (ie. when using StrRef or set_ref), and fixed <string> include.
  0.28 - breaking change: replaced Str32 by Str30 to avoid collision with Str32 from MacTypes.h .
  0.27 - added STR_API and basic .natvis file.
  0.26 - fixed set(cont char* src, const char* src_end) writing null terminator to the wrong position.
  0.25 - allow set(const char* NULL) or operator= NULL to clear the string. note that set() from range or other types are not allowed.
  0.24 - allow set_ref(const char* NULL) to clear the string. include fixes for linux.
  0.23 - added append(char). added append_from(int idx, XXX) functions. fixed some compilers warnings.
  0.22 - documentation improvements, comments. fixes for some compilers.
  0.21 - added StrXXXf() constructor to construct directly from a format string.
*/

/*
TODO
- Since we lose 4-bytes of padding on 64-bits architecture, perhaps just spread the header to 8-bytes and lift size limits?
- More functions/helpers.
*/

#ifndef STR_INCLUDED
#define STR_INCLUDED

//-------------------------------------------------------------------------
// CONFIGURATION
//-------------------------------------------------------------------------

#ifndef STR_MEMALLOC
#define STR_MEMALLOC  malloc
#include <stdlib.h>
#endif
#ifndef STR_MEMFREE
#define STR_MEMFREE   free
#include <stdlib.h>
#endif
#ifndef STR_ASSERT
#define STR_ASSERT    assert
#include <assert.h>
#endif
#ifndef STR_API
#define STR_API
#endif
#include <stdarg.h>   // for va_list
#include <string.h>   // for strlen, strcmp, memcpy, etc.
#include <utility>    // for std::move

// Configuration: #define STR_SUPPORT_STD_STRING 0 to disable setters variants using const std::string& (on by default)
#ifndef STR_SUPPORT_STD_STRING
#define STR_SUPPORT_STD_STRING  1
#endif

// Configuration: #define STR_DEFINE_STR32 1 to keep defining Str32/Str32f, but be warned: on macOS/iOS, MacTypes.h also defines a type named Str32.
#ifndef STR_DEFINE_STR32
#define STR_DEFINE_STR32 0
#endif

#if STR_SUPPORT_STD_STRING
#include <string>
#include <exception>
#include <stdexcept>
#endif

//-------------------------------------------------------------------------
// HEADERS
//-------------------------------------------------------------------------

// This is the base class that you can pass around
// Footprint is 8-bytes (32-bits arch) or 16-bytes (64-bits arch)
class STR_API Str
{
    char*               Data;                   // Point to LocalBuf() or heap allocated
    //int                 Capacity : 21;          // Max 2 MB
    //int                 LocalBufSize : 10;      // Max 1023 bytes
    //unsigned int        Owned : 1;              // Set when we have ownership of the pointed data (most common, unless using set_ref() method or StrRef constructor)
    // for x64 做如下调整
    int64_t Capacity:26; // Max 64MB
    int64_t Length:26; // Max 64MB
    int64_t LocalBufSize:10; // drop, 固定栈上数据大小,使用Capacity来存储容量
    uint64_t Owned:1; // 
    uint64_t Padding:1; // or onstack 在栈上分配

    // 可做如下调整，调整后Str16栈上16+8个字节， (char*)this + sizeof(Str) - 8
    // uint32_t cap:24 ; Max 16MB
    // //int32_t len:23 ; Max 8MB; onstack=1 时通过strlen(buffer)计算; onstack=0时data前四个字节就是长度; 当前对象栈上最大 8 个字节， 通过继承可扩展到 N + 8
    // uint32_t owened:1; // 所有权
    // uint32_t onstack:1; // 栈上数据
    // uint32_t padding:8; 1bytes
    // char* data or buffer[sizeof(char*)];

public:
    inline char*        c_str()                                 { return Data; }
    inline const char*  c_str() const                           { return Data; }
    inline char*        data()                                  { return Data; }
    inline const char*  data() const                            { return Data; }
    //inline bool         empty() const                           { return Data[0] == 0; }
    inline bool         empty() const                           { return Length == 0; }
    //inline int          length() const                          { return (int)strlen(Data); }    // by design, allow user to write into the buffer at any time
    inline size_t          length() const                          { return Length; }    // by design, allow user to write into the buffer at any time
    inline size_t          size() const                            { return Length; }    // by design, allow user to write into the buffer at any time
    inline size_t          capacity() const                        { return Capacity; }

    inline void         set_ref(const char* src);
    inline void         set_ref(const char* src, int size);
    int                 setf(const char* fmt, ...);
    int                 setfv(const char* fmt, va_list args);
    int                 setf_nogrow(const char* fmt, ...);
    int                 setfv_nogrow(const char* fmt, va_list args);
    int                 append(char c);
    int                 append(int num, char c);
    int                 append(const char* s, const char* s_end = NULL);
    int                 append(const char* s, int size);
    int                 appendf(const char* fmt, ...);
    int                 appendfv(const char* fmt, va_list args);
    int                 append_from(int idx, char c);
    int                 append_from(int idx, const char* s, const char* s_end = NULL);		// If you know the string length or want to append from a certain point
    int                 appendf_from(int idx, const char* fmt, ...);
    int                 appendfv_from(int idx, const char* fmt, va_list args);
    int                 replace(int idx, int src_size, const char* dest, int dest_size);

    void                clear();
    void                reserve(int cap);
    void                reserve_discard(int cap);
    void                shrink_to_fit();

    inline char&        operator[](size_t i)                    { return Data[i]; }
    inline char         operator[](size_t i) const              { return Data[i]; }
    //explicit operator const char*() const{ return Data; }

    inline Str();
    inline Str(const char* rhs);
    inline Str(const char* rhs, int size);
    inline void         set(const char* src);
    inline void         set(const char* src, const char* src_end);
    inline void         set(const char* src, int size);
    inline Str&         operator=(const char* rhs)              { set(rhs); return *this; }
    inline bool         operator==(const char* rhs) const       { return strcmp(c_str(), rhs) == 0; }

    inline Str(const Str& rhs);
    inline Str(Str&& rhs);
    inline void         set(const Str& src);
    inline void         set(Str&& src);
    inline Str&         operator=(const Str& rhs)               { set(rhs); return *this; }
    inline Str&         operator=(Str&& rhs)                    { set(std::move(rhs)); return *this; }
    inline bool         operator==(const Str& rhs) const        { return strcmp(c_str(), rhs.c_str()) == 0; }
    inline int          compare(const Str& rhs) const           { return strncmp(c_str(), rhs.c_str(), length() > rhs.length() ? length() : rhs.length()); }

#if STR_SUPPORT_STD_STRING
    inline Str(const std::string& rhs);
    inline void         set(const std::string& src);
    inline Str&         operator=(const std::string& rhs)       { set(rhs); return *this; }
    inline bool         operator==(const std::string& rhs)const { return strcmp(c_str(), rhs.c_str()) == 0; }
    inline int          compare(const std::string& rhs) const   { return strncmp(c_str(), rhs.c_str(), length() > rhs.length() ? length() : rhs.length()); }
#endif

    // Destructor for all variants
    inline ~Str()
    {
        if (Owned && !is_using_local_buf())
            STR_MEMFREE(Data);
    }

    static char*        EmptyBuffer;

protected:
    inline char*        local_buf()                             { return (char*)this + sizeof(Str); }
    inline const char*  local_buf() const                       { return (char*)this + sizeof(Str); }
    inline bool         is_using_local_buf() const              { return Data == local_buf() && LocalBufSize != 0; }

    // Constructor for StrXXX variants with local buffer
    Str(unsigned short local_buf_size)
    {
        STR_ASSERT(local_buf_size < 1024);
        Data = local_buf();
        Data[0] = '\0';
        Capacity = local_buf_size;
        LocalBufSize = local_buf_size;
        Owned = 1;
        Length = 0;
    }
};

void    Str::set(const char* src)
{
    // We allow set(NULL) or via = operator to clear the string.
    if (src == NULL)
    {
        clear();
        return;
    }
    int buf_len = (int)strlen(src)+1;
    if (Capacity < buf_len)
        reserve_discard(buf_len);
    memcpy(Data, src, (size_t)buf_len);
    Owned = 1;
    Length = buf_len - 1;
}

void    Str::set(const char* src, const char* src_end)
{
    STR_ASSERT(src != NULL && src_end >= src);
    int buf_len = (int)(src_end-src)+1;
    if ((int)Capacity < buf_len)
        reserve_discard(buf_len);
    memcpy(Data, src, (size_t)(buf_len - 1));
    Data[buf_len-1] = 0;
    Owned = 1;
    Length = buf_len - 1;
}

void    Str::set(const char* src, int size)
{
    STR_ASSERT(src != NULL && size>= 0);
    int buf_len = (int)(size) + 1;
    if ((int)Capacity < buf_len)
        reserve_discard(buf_len);
    memcpy(Data, src, (size_t)(buf_len - 1));
    Data[buf_len-1] = 0;
    Owned = 1;
    Length = buf_len - 1;
}

void    Str::set(const Str& src)
{
    int buf_len = (int)strlen(src.c_str())+1;
    if ((int)Capacity < buf_len)
        reserve_discard(buf_len);
    memcpy(Data, src.c_str(), (size_t)buf_len);
    Owned = 1;
    Length = buf_len - 1;
}

void    Str::set(Str&& src)
{
    if (!src.Owned)
        set_ref(src.c_str());
    else if (src.is_using_local_buf())
        set(src.local_buf());
    else
    {
        // Take over the other string's heap allocation
        if (Owned && !is_using_local_buf())
            STR_MEMFREE(Data);
        Data = src.Data;
        Capacity = src.Capacity;
        Owned = 1;
        Length = src.Length;
        src.Owned = 0;  // prevent src.clear() from freeing the memory
        src.clear();
    }
}

#if STR_SUPPORT_STD_STRING
void    Str::set(const std::string& src)
{
    int buf_len = (int)src.length()+1;
    if ((int)Capacity < buf_len)
        reserve_discard(buf_len);
    memcpy(Data, src.c_str(), (size_t)buf_len);
    Owned = 1;
    Length = buf_len - 1;
}
#endif

inline void Str::set_ref(const char* src)
{
    if (Owned && !is_using_local_buf())
        STR_MEMFREE(Data);
    Data = src ? (char*)src : EmptyBuffer;
    Capacity = 0;
    Owned = 0;
    Length = strlen(src);
}

inline void Str::set_ref(const char* src, int size)
{
    if (Owned && !is_using_local_buf())
        STR_MEMFREE(Data);
    Data = src ? (char*)src : EmptyBuffer;
    Capacity = 0;
    Owned = 0;
    Length = size;
}

Str::Str()
{
    Data = EmptyBuffer;      // Shared READ-ONLY initial buffer for 0 capacity
    Capacity = 0;
    LocalBufSize = 0;
    Owned = 0;
    Length = 0;
}

Str::Str(const Str& rhs) : Str()
{
    set(rhs);
}

Str::Str(Str&& rhs) : Str()
{
    set(std::move(rhs));
}

Str::Str(const char* rhs) : Str()
{
    set(rhs);
}

Str::Str(const char* rhs, int size) : Str()
{
    set(rhs, size);
}

#if STR_SUPPORT_STD_STRING
Str::Str(const std::string& rhs) : Str()
{
    set(rhs);
}
#endif

// Literal/reference string
class StrRef : public Str
{
public:
    StrRef(const char* s) : Str() { set_ref(s); }
    StrRef(const char* s, int size) : Str() { set_ref(s, size); }
};

// Types embedding a local buffer
// NB: we need to override the constructor and = operator for both Str& and TYPENAME (without the later compiler will call a default copy operator)
#if STR_SUPPORT_STD_STRING

#define STR_DEFINETYPE(TYPENAME, LOCALBUFSIZE)                                      \
class TYPENAME : public Str                                                         \
{                                                                                   \
    char local_buf[LOCALBUFSIZE];                                                   \
public:                                                                             \
    TYPENAME() : Str(LOCALBUFSIZE) {}                                               \
    TYPENAME(const Str& rhs) : Str(LOCALBUFSIZE) { set(rhs); }                      \
    TYPENAME(Str&& rhs) : Str(LOCALBUFSIZE) { set(std::move(rhs)); }                \
    TYPENAME(const char* rhs) : Str(LOCALBUFSIZE) { set(rhs); }                     \
    TYPENAME(const TYPENAME& rhs) : Str(LOCALBUFSIZE) { set(rhs); }                 \
    TYPENAME(TYPENAME&& rhs) : Str(LOCALBUFSIZE) { set(std::move(rhs)); }           \
    TYPENAME(const std::string& rhs) : Str(LOCALBUFSIZE) { set(rhs); }              \
    TYPENAME&   operator=(const char* rhs)          { set(rhs); return *this; }     \
    TYPENAME&   operator=(const Str& rhs)           { set(rhs); return *this; }     \
    TYPENAME&   operator=(Str&& rhs)                { set(std::move(rhs)); return *this; } \
    TYPENAME&   operator=(const TYPENAME& rhs)      { set(rhs); return *this; }     \
    TYPENAME&   operator=(TYPENAME&& rhs)           { set(std::move(rhs)); return *this; } \
    TYPENAME&   operator=(const std::string& rhs)   { set(rhs); return *this; }     \
};

#else

#define STR_DEFINETYPE(TYPENAME, LOCALBUFSIZE)                                      \
class TYPENAME : public Str                                                         \
{                                                                                   \
    char local_buf[LOCALBUFSIZE];                                                   \
public:                                                                             \
    TYPENAME() : Str(LOCALBUFSIZE) {}                                               \
    TYPENAME(const Str& rhs) : Str(LOCALBUFSIZE) { set(rhs); }                      \
    TYPENAME(Str&& rhs) : Str(LOCALBUFSIZE) { set(std::move(rhs)); }                \
    TYPENAME(const char* rhs) : Str(LOCALBUFSIZE) { set(rhs); }                     \
    TYPENAME(const TYPENAME& rhs) : Str(LOCALBUFSIZE) { set(rhs); }                 \
    TYPENAME(TYPENAME&& rhs) : Str(LOCALBUFSIZE) { set(std::move(rhs)); }           \
    TYPENAME&   operator=(const char* rhs)          { set(rhs); return *this; }     \
    TYPENAME&   operator=(const Str& rhs)           { set(rhs); return *this; }     \
    TYPENAME&   operator=(Str&& rhs)                { set(std::move(rhs)); return *this; } \
    TYPENAME&   operator=(const TYPENAME& rhs)      { set(rhs); return *this; }     \
    TYPENAME&   operator=(TYPENAME&& rhs)           { set(std::move(rhs)); return *this; } \
};

#endif

// Disable PVS-Studio warning V730: Not all members of a class are initialized inside the constructor (local_buf is not initialized and that is fine)
// -V:STR_DEFINETYPE:730

// Helper to define StrXXXf constructors
#define STR_DEFINETYPE_F(TYPENAME, TYPENAME_F)                                      \
class TYPENAME_F : public TYPENAME                                                  \
{                                                                                   \
public:                                                                             \
    TYPENAME_F(const char* fmt, ...) : TYPENAME() { va_list args; va_start(args, fmt); setfv(fmt, args); va_end(args); } \
};

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-private-field"         // warning : private field 'local_buf' is not used
#endif

// Declaring types for common sizes here
STR_DEFINETYPE(Str16, 16)
STR_DEFINETYPE(Str30, 30)
STR_DEFINETYPE(Str64, 64)
STR_DEFINETYPE(Str128, 128)
STR_DEFINETYPE(Str256, 256)
STR_DEFINETYPE(Str512, 512)

// Declaring helper constructors to pass in format strings in one statement
STR_DEFINETYPE_F(Str16, Str16f)
STR_DEFINETYPE_F(Str30, Str30f)
STR_DEFINETYPE_F(Str64, Str64f)
STR_DEFINETYPE_F(Str128, Str128f)
STR_DEFINETYPE_F(Str256, Str256f)
STR_DEFINETYPE_F(Str512, Str512f)

#if STR_DEFINE_STR32
STR_DEFINETYPE(Str32, 32)
STR_DEFINETYPE_F(Str32, Str32f)
#endif

template <int N> class TinyStr
{
  public:
    TinyStr() { _buffer[0] = '\0'; }
    TinyStr(const TinyStr &s) { set_str(s.data(), s.size()); }
    TinyStr(const std::string &s) { set_str(s.data(), s.size()); }
    TinyStr(const char *s) { set_str(s, static_cast<int>(::strnlen(s, N))); }
    TinyStr(const char *s, int size) { set_str(s, size); }
    ~TinyStr() {}

    inline TinyStr &operator=(const char *rhs)
    {
        set_str(rhs, ::strnlen(rhs, N));
        return *this;
    }
    inline bool operator==(const char *rhs) const { return strcmp(c_str(), rhs) == 0; }

    inline TinyStr &operator=(const TinyStr &rhs)
    {
        set_str(rhs.c_str(), rhs.size());
        return *this;
    }
    inline bool operator==(const TinyStr &rhs) const { return ::strcmp(c_str(), rhs.c_str()) == 0; }
    inline int compare(const TinyStr &rhs) const
    {
        return ::strncmp(c_str(), rhs.c_str(), length() > rhs.length() ? length() : rhs.length());
    }

    inline TinyStr &operator=(const std::string &rhs)
    {
        set_str(rhs.c_str(), rhs.size());
        return *this;
    }
    inline bool operator==(const std::string &rhs) const { return ::strcmp(c_str(), rhs.c_str()) == 0; }
    inline int compare(const std::string &rhs) const
    {
        return ::strncmp(c_str(), rhs.c_str(), length() > rhs.length() ? length() : rhs.length());
    }

    const char *strstr(const char *s) const { return ::strstr(_buffer, s); }
    const char *find_first_of(char c) const { return ::strchr(_buffer, c); }
    const char *find_last_of(char c) const { return ::strrchr(_buffer, c); }
    const char *data() const { return _buffer; }
    const char *c_str() const { return _buffer; }
    int capacity() const { return N; }
    int size() const { return ::strnlen(_buffer, N); }
    int length() const { return ::strnlen(_buffer, N); }
    void clear() { _buffer[0] = '\0'; }

    void set_str(const char *s, int size)
    {
        if (s == nullptr)
        {
            return;
        }
        if (size < N)
        {
            ::memcpy(_buffer, s, size);
            _buffer[size] = '\0';
        }
        else
        {
            throw std::length_error("size too big");
        }
    }

  private:
    char _buffer[N];
};


class StrView
{
  public:
    StrView() { _buffer = nullptr; }
    StrView(const StrView &s) { set_ref(s.data(), s.size()); }
    StrView(const std::string &s) { set_ref(s.data(), s.size()); }
    StrView(const char *s) { set_ref(s); }
    StrView(const char *s, int size) { set_ref(s, size); }
    ~StrView() {}

    inline StrView &operator=(const char *rhs)
    {
        if(rhs == nullptr)
        {
            set_ref(rhs, 0);
        }
        else {
            set_ref(rhs, ::strlen(rhs));
        }
        return *this;
    }
    inline bool operator==(const char *rhs) const { return strcmp(c_str(), rhs) == 0; }

    inline StrView &operator=(const StrView &rhs)
    {
        set_ref(rhs.c_str(), rhs.size());
        return *this;
    }
    inline bool operator==(const StrView &rhs) const { return ::strcmp(c_str(), rhs.c_str()) == 0; }
    inline int compare(const StrView &rhs) const
    {
        return ::strncmp(c_str(), rhs.c_str(), length() > rhs.length() ? length() : rhs.length());
    }

    inline StrView &operator=(const std::string &rhs)
    {
        set_ref(rhs.c_str(), rhs.size());
        return *this;
    }
    inline bool operator==(const std::string &rhs) const { return ::strcmp(c_str(), rhs.c_str()) == 0; }
    inline int compare(const std::string &rhs) const
    {
        return ::strncmp(c_str(), rhs.c_str(), length() > rhs.length() ? length() : rhs.length());
    }

    const char *strstr(const char *s) const { return ::strstr(_buffer, s); }
    const char *find_first_of(char c) const { return ::strchr(_buffer, c); }
    const char *find_last_of(char c) const { return ::strrchr(_buffer, c); }
    const char *data() const { return _buffer; }
    const char *c_str() const { return _buffer; }
    size_t size() const { return _size; }
    size_t length() const { return _size; }
    void clear() { _buffer = nullptr; }

    void set_ref(const char *s)
    {
        _buffer = const_cast<char*>(s);
        if(s == nullptr)
        {
            _size = 0;
        }
        else {
            _size = ::strlen(s);
        }
    }
    void set_ref(const char *s, int size)
    {
        _buffer = const_cast<char*>(s);
        _size = size;
    }

  private:
    char* _buffer{nullptr};
    int _size {0};
};


#ifdef __clang__
#pragma clang diagnostic pop
#endif

#endif // #ifndef STR_INCLUDED

//-------------------------------------------------------------------------