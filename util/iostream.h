#pragma once
#include "stream.h"
namespace Streams {

class IOStream : public Stream {
  public:
    IOStream( int access_mode = AccessMode::READ );
    IOStream( const std::string &name, int access_mode = AccessMode::READ );

    virtual size_t read( void *buffer, size_t count ) override;
    virtual size_t write( const void *buffer, size_t count ) override;

    virtual size_t read_line( std::string &str ) override;

    virtual bool   seek( size_t pos, SeekDirection direction ) override;
    virtual size_t tell() const override;

    virtual bool eof() const override;

    virtual void flush() override;

    virtual bool good() override { return true; }

    virtual BufferPtr get_as_buffer() override;
};

} /* namespace Streams */

