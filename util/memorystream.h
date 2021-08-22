#pragma once
#include "stream.h"
namespace Streams {

class MemoryStream : public Stream {
  public:
    enum class DeleteMode { None, DeleteOnDestruct };

    MemoryStream( DeleteMode delete_mode = DeleteMode::DeleteOnDestruct, int access_mode = AccessMode::READ_WRITE );
    MemoryStream( BufferPtr buffer, int access_mode = AccessMode::READ_WRITE );
    MemoryStream( void *buffer, size_t size, DeleteMode delete_mode = DeleteMode::DeleteOnDestruct,
                  int access_mode = AccessMode::READ_WRITE );
    virtual ~MemoryStream() override;

    virtual size_t read( void *buffer, size_t count ) override;
    virtual size_t write( const void *buffer, size_t count ) override;

    virtual size_t read_line( std::string &str ) override;

    virtual bool   seek( size_t pos, SeekDirection direction ) override;
    virtual size_t tell() const override;

    virtual bool eof() const override;

    void close();

    virtual void flush() override;

    virtual bool good() override;

    virtual BufferPtr get_as_buffer() override;

  protected:
    BufferPtr  m_buffer;
    DeleteMode m_delete_mode;

    size_t m_buffer_offset;
};

typedef std::shared_ptr<MemoryStream> MemoryStreamPtr;
#define AeonEmptyMemoryStream MemoryStreamPtr;

} /* namespace Streams */

