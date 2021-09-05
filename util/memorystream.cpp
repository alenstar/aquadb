
#include <memory>
#include <string.h>
#include <string>
#include "logdef.h"
#include "iobuffer.h"
#include "memorystream.h"

#undef SPDLOG_TAG 
#define SPDLOG_TAG "[util]"
namespace util {

MemoryStream::MemoryStream( DeleteMode delete_mode /*= DeleteMode::DeleteOnDestruct*/,
                            int        access_mode /*= AccessMode::READ_WRITE*/ )
    : IOStream( access_mode )
    , m_buffer( std::make_shared<IOBuffer>() )
    , m_buffer_offset( 0 ) {
    // Set the correct delete mode in our buffer
    m_buffer->set_delete_mode( delete_mode == DeleteMode::DeleteOnDestruct ? IOBuffer::DeleteMode::DeleteOnDestruct
                                                                           : IOBuffer::DeleteMode::None );
}

MemoryStream::MemoryStream( IOBufferPtr buffer, int access_mode /*= AccessMode::READ_WRITE*/ )
    : IOStream( access_mode )
    , m_buffer( buffer )
    , m_buffer_offset( 0 ) {}

MemoryStream::MemoryStream( void *buffer, size_t size, DeleteMode delete_mode /*= DeleteMode::DeleteOnDestruct*/,
                            int access_mode /*= AccessMode::READ_WRITE*/ )
    : IOStream( access_mode )
    , m_buffer( std::make_shared<IOBuffer>( buffer, size ) )
    , m_buffer_offset( 0 ) {
    // Set the correct delete mode in our buffer
    m_buffer->set_delete_mode( delete_mode == DeleteMode::DeleteOnDestruct ? IOBuffer::DeleteMode::DeleteOnDestruct
                                                                           : IOBuffer::DeleteMode::None );
}

MemoryStream::~MemoryStream() { close(); }

size_t MemoryStream::read( void *buffer, size_t count ) {
    if ( !( m_access_mode & AccessMode::READ ) ) {
        LOGE( "MemoryStream: Read on write-only stream." );
        return 0;
    }

    char *data = (char *) m_buffer->get();

    if ( !data ) {
        LOGE( "MemoryStream: Read on empty stream. Buffer was NULL." );
        return 0;
    }

    if ( !buffer ) {
        LOGE( "MemoryStream: Input buffer is NULL." );
        return 0;
    }

    if ( count == 0 ) {
        LOGW( "MemoryStream: Tried writing 0 bytes." );
        return 0;
    }

    // Only read what we have
    if ( m_buffer->size() < m_buffer_offset + count ) count = m_buffer->size() - m_buffer_offset;

    // Are we really out of bounds?
    if ( count <= 0 ) return 0;

    // Copy our data
    memcpy( buffer, &data[ m_buffer_offset ], count );

    return count;
}

size_t MemoryStream::write( const void *buffer, size_t count ) {
    if ( !( m_access_mode & AccessMode::WRITE ) ) {
        LOGE( "MemoryStream: WRITE on write-only stream." );
        return 0;
    }

    // Make sure we have enough space in the buffer
    if ( !m_buffer->reserve( m_buffer_offset + count ) ) {
        LOGE( "MemoryStream: WRITE on stream failed. Could not reserve memory." );
        return 0;
    }

    // Get our data pointer
    char *data = (char *) m_buffer->get();

    // Copy our data
    memcpy( &data[ m_buffer_offset ], buffer, count );

    return count;
}

size_t MemoryStream::read_line( std::string &str ) {
    if ( !( m_access_mode & AccessMode::READ ) ) {
        LOGE( "MemoryStream: Read on write-only stream." );
        return 0;
    }

    char *data = (char *) m_buffer->get();

    if ( data == NULL ) {
        LOGE( "MemoryStream: Read on empty stream. Buffer was NULL." );
        return 0;
    }

    std::string line;
    size_t      character_offset = 0;
    for ( int i = 0; i < IOSTREAM_MAX_TEXT_LINE_LENGTH; ++i ) {
        // Can we still read a character?
        if ( m_buffer_offset + character_offset >= m_buffer->size() ) break;

        char c = data[ m_buffer_offset + character_offset ];

        if ( c == '\n' ) break;

        line += c;

        character_offset++;
    }

    str = line;

    return line.length();
}

bool MemoryStream::seek( size_t pos, SeekDirection direction ) {
    size_t new_pos = 0;
    switch ( direction ) {
    case SeekDirection::Begin: {
        new_pos = pos;
    } break;
    case SeekDirection::Current: {
        new_pos = m_buffer_offset + pos;
    } break;
    case SeekDirection::End: {
        new_pos = m_buffer->size() - pos;
    } break;
    };

    // Can't go higher then the size of our buffer...
    if ( new_pos >= m_buffer->size() ) return false;

    // Set the new offset if all is good
    m_buffer_offset = new_pos;
    return true;
}

size_t MemoryStream::tell() const { return m_buffer_offset; }

bool MemoryStream::eof() const { return m_buffer_offset >= m_buffer->size(); }

void MemoryStream::close() {
    // Create a new buffer, and remove all references to the old one.
    // This may leak memory if DeleteOnDestruct was not set.
    m_buffer = std::make_shared<IOBuffer>();
}

void MemoryStream::flush() {
    // Nothing to do here.
}

bool MemoryStream::good() {
    // Do we have a buffer?
    if ( !( m_buffer->get() != NULL ) ) return false;

    // Are we still within bounds?
    if ( m_buffer_offset >= m_buffer->size() ) return false;

    // All ok!
    return true;
}

IOBufferPtr MemoryStream::get_as_buffer() { return m_buffer; }

} /* namespace util */

#undef SPDLOG_TAG 
