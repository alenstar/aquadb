
#include <memory>
#include <string>

#include "iobuffer.h"
#include "iostream.h"

namespace util {

IOStream::IOStream( int access_mode /*= AccessMode::READ*/ )
    : m_name( IOSTREAM_DEFAULT_STREAM_NAME )
    , m_size( 0 )
    , m_access_mode( access_mode ) {}

IOStream::IOStream( const std::string &name, int access_mode /*= AccessMode::READ*/ )
    : m_name( name )
    , m_size( 0 )
    , m_access_mode( access_mode ) {}

IOStream::~IOStream() {
    // 由子类调用
    // close();
}

IOBufferPtr IOStream::get_as_buffer() {
    size_t s = size();

    auto buffer = std::make_shared<IOBuffer>( s );
    read( buffer->get(), s );

    return buffer;
}

size_t IOStream::write( const std::string &str ) { return write( str.data(), str.size() ); }

size_t IOStream::write( IOBufferPtr buffer ) {
    if ( !buffer ) {
#ifdef AEON_USE_AEON_CONSOLE_LIBRARY
        Console::error( "Stream: Tried writing an empty buffer to a stream." );
#endif

        return 0;
    }

    return write( buffer->get(), buffer->size() );
}

} /* namespace Streams */
