#include <memory>
#include <string.h>
#include <string>
#include "logdef.h"
#include "iobuffer.h"

namespace util {

IOBuffer::IOBuffer()
    : m_buffer( nullptr )
    , m_size( 0 )
    , m_reserved_size( 0 )
    , m_delete_mode( DeleteMode::DeleteOnDestruct ) {}

IOBuffer::IOBuffer( size_t size, DeleteMode delete_mode /*= DeleteMode::DeleteOnDestruct*/ )
    : m_buffer( nullptr )
    , m_size( 0 )
    , m_reserved_size( 0 )
    , m_delete_mode( delete_mode ) {
    reserve( size );
}

IOBuffer::IOBuffer( void *buffer, size_t size, DeleteMode delete_mode /*= DeleteMode::DeleteOnDestruct*/ )
    : m_buffer( buffer )
    , m_size( 0 )
    , m_reserved_size( size )
    , m_delete_mode( delete_mode ) {}

IOBuffer::~IOBuffer() {
    if ( m_delete_mode == DeleteMode::DeleteOnDestruct ) free();
}

bool IOBuffer::reserve( size_t n ) {
    // Do we already have this many bytes reserved?
    if ( n <= m_reserved_size ) return true;

    // Resize the array if we're requesting more bytes
    bool result = resize( n );

    //if ( result ) LOGD( "Buffer: Reserved %u bytes.", n );

    return result;
}

bool IOBuffer::resize( size_t n ) {
    // Reallocate the buffer to be the new size
    void *new_buffer = realloc( m_buffer, n );

    // Did we fail to reallocate our buffer?
    if ( new_buffer == nullptr ) {
        // Do we have data at all?
        if ( m_buffer != nullptr ) {
            LOGW( "Buffer: Failed to reallocate buffer from %lu to %lu. Trying copy.", m_reserved_size, n );

            // Try a fallback method...
            new_buffer = malloc( n );

            // Did we fail again?!
            if ( new_buffer == nullptr ) {
                LOGE( "Buffer: Failed to reallocate buffer from %lu to %lu in fallback mode. Aborting",
                                m_reserved_size, n );

                return false;
            }

            // How many bytes do we need to copy? Are we increasing or shrinking?
            size_t newsize = ( n < m_reserved_size ) ? n : m_reserved_size;

            // All ok! Lets copy!
            memcpy( new_buffer, m_buffer, newsize );

            // Free the old buffer
            ::free( m_buffer );

            m_buffer = new_buffer;

            return true;
        }

        LOGE( "Buffer: Could not allocate buffer for %lu bytes.", n );
        return false;
    }

    // All is ok! Let's keep our new data
    m_buffer        = new_buffer;
    m_reserved_size = n;

    return true;
}

bool IOBuffer::append( void *data, size_t len ) {
    // Does our appended data fit?
    if ( m_size + len > m_reserved_size ) {
        // Make sure our buffer is able to fit this data
        if ( !reserve( m_reserved_size + len ) ) return false;
    }

    // Copy our data into the new buffer
    char *buff = static_cast<char *>( m_buffer );
    memcpy( &buff[ m_size ], data, len );

    // Adjust the size
    m_size += len;

    return true;
}

void IOBuffer::free() {
    ::free( m_buffer );

    LOGD( "Buffer: Freed %lu bytes.", m_reserved_size );

    m_buffer        = nullptr;
    m_size          = 0;
    m_reserved_size = 0;
}

void IOBuffer::set_delete_mode( DeleteMode mode ) { m_delete_mode = mode; }

} /* namespace util */
