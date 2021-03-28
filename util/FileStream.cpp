#include "FileStream.h"
#include "Buffer.h"
#include "logdef.h"
#include "Stream.h"
#include <memory>
#include <string>

#undef SPDLOG_TAG
#define SPDLOG_TAG "Streams"
namespace Streams {

FileStream::FileStream( const std::string &path, int access_mode /*= AccessMode::READ*/ )
    : Stream( access_mode )
    , m_file( nullptr ) {
    m_name = path;

    if ( access_mode == ( AccessMode::READ | AccessMode::WRITE ) ) {
        LOGE( "FileStream: Invalid access mode: Read+Write on file %s.", m_name.c_str() );
        return;
    }

    __open_file();
}

FileStream::~FileStream() { close(); }

void FileStream::__open_file() {
    if ( m_access_mode == AccessMode::READ )
        m_file = fopen( m_name.c_str(), "rb" );
    else
        m_file = fopen( m_name.c_str(), "wb" );

    if ( !m_file ) {
#ifdef AEON_USE_AEON_CONSOLE_LIBRARY
        Console::error( "FileStream: Could not open file: %s", m_name.c_str() );
#endif
        return;
    }

    if ( m_access_mode == AccessMode::READ ) __calculate_file_size();
}

void FileStream::__calculate_file_size() {
    if ( !m_file ) {
#ifdef AEON_USE_AEON_CONSOLE_LIBRARY
        Console::error( "FileStream: Size requested on unopened file." );
#endif
        return;
    }

    if ( !seek( 0, SeekDirection::End ) ) {
#ifdef AEON_USE_AEON_CONSOLE_LIBRARY
        Console::error( "Could not determine file size for file: %s. Seek end failed.", m_name.c_str() );
#endif
        return;
    }

    m_size = static_cast<size_t>( ftell( m_file ) );

#ifdef AEON_USE_AEON_CONSOLE_LIBRARY
    if ( m_size == 0 ) Console::warning( "FileStream: File is empty: %s", m_name.c_str() );
#endif

    if ( !seek( 0, SeekDirection::Begin ) ) {
#ifdef AEON_USE_AEON_CONSOLE_LIBRARY
        Console::error( "Could not determine file size for file: %s. Seek begin failed.", m_name.c_str() );
#endif
    }
}

size_t FileStream::read( void *buffer, size_t count ) {
    if ( !m_file ) {
#ifdef AEON_USE_AEON_CONSOLE_LIBRARY
        Console::error( "FileStream: Read on unopened file." );
#endif

        return 0;
    }

    if ( m_access_mode != AccessMode::READ ) {
#ifdef AEON_USE_AEON_CONSOLE_LIBRARY
        Console::error( "FileStream: Can not read from file in write mode for file %s.", m_name.c_str() );
#endif

        return 0;
    }

    if ( !buffer ) {
#ifdef AEON_USE_AEON_CONSOLE_LIBRARY
        Console::error( "FileStream: Input buffer is NULL." );
#endif

        return 0;
    }

    if ( count == 0 ) {
#ifdef AEON_USE_AEON_CONSOLE_LIBRARY
        Console::warning( "FileStream: Tried writing 0 bytes." );
#endif

        return 0;
    }

    return fread( buffer, 1, count, m_file );
}

size_t FileStream::read_line( std::string &str ) {
    if ( !m_file ) {
#ifdef AEON_USE_AEON_CONSOLE_LIBRARY
        Console::error( "FileStream: Read on unopened file." );
#endif

        return 0;
    }

    if ( m_access_mode != AccessMode::READ ) {
#ifdef AEON_USE_AEON_CONSOLE_LIBRARY
        Console::error( "FileStream: Can not read from file in write mode for file %s.", m_name.c_str() );
#endif

        return 0;
    }

    std::string line;

    for ( int i = 0; i < AEON_STREAMS_MAX_TEXT_LINE_LENGTH; ++i ) {
        int c = fgetc( m_file );

        if ( c == EOF ) break;

        if ( c == '\n' ) break;

        line += static_cast<char>( c );
    }

    str = line;

    return line.length();
}

size_t FileStream::write( const void *buffer, size_t count ) {
    if ( !m_file ) {
#ifdef AEON_USE_AEON_CONSOLE_LIBRARY
        Console::error( "FileStream: Write on unopened file." );
#endif

        return 0;
    }

    if ( m_access_mode != AccessMode::WRITE ) {
#ifdef AEON_USE_AEON_CONSOLE_LIBRARY
        Console::error( "FileStream: Can not write to file in read mode for file %s.", m_name.c_str() );
#endif

        return 0;
    }

    if ( !buffer ) {
#ifdef AEON_USE_AEON_CONSOLE_LIBRARY
        Console::error( "FileStream: Input buffer is NULL." );
#endif

        return 0;
    }

    if ( count == 0 ) {
#ifdef AEON_USE_AEON_CONSOLE_LIBRARY
        Console::warning( "FileStream: Tried writing 0 bytes." );
#endif

        return 0;
    }

    return fwrite( buffer, 1, count, m_file );
}

bool FileStream::seek( size_t pos, SeekDirection direction ) {
    if ( !m_file ) {
#ifdef AEON_USE_AEON_CONSOLE_LIBRARY
        Console::error( "FileStream: Seek on unopened file." );
#endif

        return false;
    }

    switch ( direction ) {
    case SeekDirection::Begin: {
        return fseek( m_file, static_cast<long>( pos ), SEEK_SET ) == 0;
    }
    case SeekDirection::Current: {
        return fseek( m_file, static_cast<long>( pos ), SEEK_CUR ) == 0;
    }
    case SeekDirection::End: {
        return fseek( m_file, static_cast<long>( pos ), SEEK_END ) == 0;
    }
    };

    return false;
}

size_t FileStream::tell() const {
    if ( !m_file ) {
#ifdef AEON_USE_AEON_CONSOLE_LIBRARY
        Console::error( "FileStream: Tell on unopened file." );
#endif

        return 0;
    }

    return static_cast<size_t>( ftell( m_file ) );
}

bool FileStream::eof() const {
    if ( !m_file ) {
#ifdef AEON_USE_AEON_CONSOLE_LIBRARY
        Console::error( "FileStream: EOF on unopened file." );
#endif

        return true;
    }

    return ( feof( m_file ) != 0 );
}

void FileStream::close() {
    if ( !m_file ) {
#ifdef AEON_USE_AEON_CONSOLE_LIBRARY
        Console::error( "FileStream: Close on unopened file." );
#endif

        return;
    }

    fclose( m_file );
    m_file = nullptr;
}

void FileStream::flush() {
    if ( !m_file ) {
#ifdef AEON_USE_AEON_CONSOLE_LIBRARY
        Console::error( "FileStream: Close on unopened file." );
#endif

        return;
    }

    fflush( m_file );
}

bool FileStream::good() { return m_file != nullptr; }

bool FileStream::resize( size_t size ) {
    if ( m_file ) {
#ifdef WIN32
        return _chsize_s( fileno( m_file ), static_cast<long>( size ) ) == 0;
#else
        return ftruncate( fileno( m_file ), static_cast<long>( size ) ) == 0;
#endif
    }
    return false;
}

void FileStream::swap( FileStream &other ) noexcept {
    std::swap( m_file, other.m_file );
    std::swap( m_size, other.m_size );
    std::swap( m_access_mode, other.m_access_mode );
    std::swap( m_name, other.m_name );
}

} /* namespace Streams */
