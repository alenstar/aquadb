#pragma once
#include "stream.h"
namespace Streams {

class FileStream : public Stream {
  public:
    FileStream( const std::string &path, int access_mode = AccessMode::READ );
    virtual ~FileStream() override;

    virtual size_t read( void *buffer, size_t count ) override;
    virtual size_t write( const void *buffer, size_t count ) override;

    virtual size_t read_line( std::string &str ) override;

    virtual bool   seek( size_t pos, SeekDirection direction ) override;
    virtual size_t tell() const override;

    virtual bool eof() const override;

    void close();

    virtual void flush() override;

    virtual bool good() override;
    // WARN
    // seek or size invalid
    bool resize( size_t size );
    void swap( FileStream &other ) noexcept;

    template <typename T> bool read_array( T *data, size_t length, size_t *pReadBytes = nullptr ) {
        size_t read_bytes = 0;
        if ( !m_file || length != ( read_bytes = std::fread( data, sizeof( T ), length, m_file ) ) ) return false;

        if ( pReadBytes ) *pReadBytes = read_bytes;

        return true;
    }

    template <typename T> bool write_array( const T *data, size_t length ) {
        if ( !m_file || length != std::fwrite( data, sizeof( T ), length, m_file ) ) return false;

        return true;
    }

  protected:
    void __open_file();
    void __calculate_file_size();

    FILE *m_file;
};

typedef std::shared_ptr<FileStream> FileStreamPtr;

} /* namespace Streams */

