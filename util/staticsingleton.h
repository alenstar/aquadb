// Copyright 2014 eric schkufza
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once
#include <cassert>
namespace util {

#if 0
template <typename T> class StaticSingleton {
  private:
  public:
    typedef T &reference;

    // StaticSingleton()                           = delete;
    // StaticSingleton( const StaticSingleton &s ) = delete;
    // StaticSingleton( StaticSingleton &&s ) = delete;
    // StaticSingleton &operator=( const StaticSingleton &s ) = delete;
    // StaticSingleton &operator=( StaticSingleton &&s ) = delete;

    static reference getInstance() {
        static T instance;
        return instance;
    }
};
#endif

template <typename T>
class StaticSingleton {
protected:
    // TODO: Come up with something better than this!
    // TODO:
    // TODO: This super-nasty piece of nastiness was put in for continued
    // TODO: compatability with MSVC++ and MinGW - the latter apparently
    // TODO: needs this.
    static T *_singleton;

public:
    StaticSingleton(void)
    {
        assert(!_singleton);
        _singleton = static_cast<T *>(this);
    }
    ~StaticSingleton(void)
    {
        assert(_singleton);
        _singleton = nullptr;
    }
    static T &getInstance(void)
    {
        assert(_singleton);
        return (*_singleton);
    }
    static T *getInstancePtr(void) { return (_singleton); }

private:
    StaticSingleton &operator=(const StaticSingleton &) { return *this; }
    StaticSingleton(const StaticSingleton &) = delete;
    StaticSingleton &operator=(StaticSingleton &&) = delete;
    StaticSingleton(StaticSingleton &&) = delete;
};

// template <typename T> T *util::StaticSingleton<T>::_singleton = nullptr;

} // namespace util
