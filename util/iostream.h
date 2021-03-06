#pragma once
#include "iobuffer.h"
#define IOSTREAM_MAX_TEXT_LINE_LENGTH ( 2048 )
#define IOSTREAM_DEFAULT_STREAM_NAME "Stream"
namespace util {

/*!
 * \brief Base class for streams
 *
 * This class serves as the base class for all streams. When implementing a new steam, derive from this class.
 */
class IOStream {
  public:
    /*!
     * The available access modes for this stream. This allows an implementation to ask the stream for
     * it's access permissions.
     *
     * This is implemented as a anonymous enum within a class, since enum classes do not support bitflags
     * at this moment.
     */
    class AccessMode {
      public:
        enum {
            READ       = 1, /**< enum Read-Only */
            WRITE      = 2, /**< enum Write-Only */
            READ_WRITE = 3,  /**< enum Read-Write (Full access) */
            APPEND     = 4  /**< enum Append-Write > */
        };
    };

    /*!
     * The seek direction used in the Stream::Seek function. This determines the behaviour of the pos
     * parameter.
     */
    enum class SeekDirection {
        Begin,   /**< enum Seek forwards from the beginning */
        Current, /**< enum Seek forwards from wherever the read pointer currently is */
        End      /**< enum Seek backwards from the end */
    };

    /*!
     * \brief Basic constructor for Stream
     *
     * This constructor will give the stream the default name as configured in the
     * AEON_STREAMS_DEFAULT_STREAM_NAME macro.
     *
     * When no parameter is given, Read-only mode is assumed.
     *
     * \param access_mode The access mode for the stream.
     * \sa AccessMode
     */
    IOStream( int access_mode = AccessMode::READ );

    /*!
     * \brief Prefered constructor for Stream
     *
     * This constructor will create a named stream.
     *
     * When no access_mode parameter is given, Read-only mode is assumed.
     *
     * \param name The name of the stream. Does not need to be unique.
     * \param access_mode The access mode for the stream.
     * \sa AccessMode
     */
    IOStream( const std::string &name, int access_mode = AccessMode::READ );

    /*!
     * Destructor.
     *
     * This will call close() to let the implementation close the stream properly if needed.
     */
    virtual ~IOStream();

    /*!
     * Get the name of the stream. The format of this name may be dependant on the implementation.
     * For example, in case of a FileStream, this contains the opened file name and path.
     *
     * \return The name of the stream as given to the constructor.
     */
    const std::string &get_name() { return m_name; }

    /*!
     * Get the access mode for this stream.
     *
     * \return The access mode
     * \sa AccessMode
     */
    virtual int get_access_mode() { return m_access_mode; }

    /*!
     * Determine of the stream is readable (true if AccessMode::READ is set)
     *
     * \return True if the stream is readable.
     * \sa get_access_mode()
     */
    virtual bool is_readable() const { return ( m_access_mode & AccessMode::READ ) != 0; }

    /*!
     * Determine of the stream is writable (true if AccessMode::WRITE is set)
     *
     * \return True if the stream is writable.
     * \sa get_access_mode()
     */
    virtual bool is_writeable() const { return ( m_access_mode & AccessMode::WRITE ) != 0; }

    /*!
     * Read raw data from the stream.
     *
     * To be able to read from the stream, AccessMode::READ must have been set. This can also be checked with
     * is_readable(). This function will return 0 if reading is not possible or permitted.
     *
     * \param buffer The buffer to read to. This buffer must be large enough to hold the requested data.
     * \param count The amount of data to read in bytes.
     * \return The actual amount of bytes that were read. May be equal or less than count. Returns 0 on error.
     * \sa is_readable()
     */
    virtual size_t read( void *buffer, size_t count ) = 0;

    /*!
     * Write raw data into the stream.
     *
     * To be able to write to the stream, AccessMode::WRITE must have been set. This can also be checked with
     * is_writable(). This function will return 0 if writing is not possible or permitted.
     *
     * \param buffer The buffer to write into the stream. This buffer must be atleast count in size.
     * \param count The amount of data to write into the stream (read from buffer)
     * \return The actual amount of bytes that were written. May be equal or less than count. Returns 0 on error.
     * \sa is_writeable()
     */
    virtual size_t write( const void *buffer, size_t count ) = 0;

    /*!
     * Write raw data into the stream using a BufferPtr.
     *
     * To be able to write to the stream, AccessMode::WRITE must have been set. This can also be checked with
     * is_writable(). This function will return 0 if writing is not possible or permitted.
     *
     * \param buffer The buffer object to write into the stream. The whole contents of this BufferPtr will be written.
     * \return The actual amount of bytes that were written. May be equal or less than count. Returns 0 on error.
     * \sa is_writeable()
     * \sa Buffer
     */
    virtual size_t write( IOBufferPtr buffer );

    /*!
     * Read a line of text from the stream.
     *
     * This will read ascii characters until '\\n' is found with a maximum of AEON_STREAMS_MAX_TEXT_LINE_LENGTH
     * characters.
     *
     * Currently, other line ending types (for example Window's \\r\\n) are not supported.
     *
     * To be able to read from the stream, AccessMode::READ must have been set. This can also be checked with
     * is_readable(). This function will return 0 if reading is not possible or permitted.
     *
     * \param str The string to read the characters to.
     * \return The amount of characters that were read. If 0 is returned, the content of str is undefined.
     * \sa is_readable()
     */
    virtual size_t read_line( std::string &str ) = 0;

    /*!
     * Write a string into the stream.
     *
     * The std::string will be written directly into the stream without modifications.
     *
     * To be able to write to the stream, AccessMode::WRITE must have been set. This can also be checked with
     * is_writable(). This function will return 0 if writing is not possible or permitted.
     *
     * \param str The string to be written into the stream.
     * \return The amount of characters written into the stream. Returns 0 on error.
     * \sa is_writeable()
     */
    virtual size_t write( const std::string &str );

    /*!
     * Seek to a certain position within the stream based on the SeekDirection.
     *
     * \param pos The offset to be used for seeking
     * \param direction The direction of seeking.
     * \return True if succeeded, false on fail.
     * \sa SeekDirection
     */
    virtual bool seek( size_t pos, SeekDirection direction ) = 0;

    /*!
     * Get the read/write position within the stream.
     *
     * This position changes on successful reading, writing and seeking.
     *
     * \return The position of the read/write index.
     */
    virtual size_t tell() const = 0;

    /*!
     * Check if the stream is at it's end. This may be an actual end of file,
     * or when the read/write index is at the end of a buffer in a MemoryStream.
     *
     * What this actually means may vary between stream types.
     *
     * \return True if at the end of the stream. Returns true on error.
     */
    virtual bool eof() const = 0;

    /*!
     * Get the current size of the stream.
     *
     * What this actually means may vary between stream types. In some cases, a size
     * may not be applicable.
     *
     * \return The size of the stream, if applicable. Returns 0 on error.
     */
    size_t size() const { return m_size; }

    /*!
     * Close the stream.
     *
     * What this actually means may vary between stream types. In some cases, closing
     * may not be applicable, in which case calling this function has no effect.
     *
     * This function is automatically called by the destructor.
     */
    // virtual void close() {}

    /*!
     * Flush the stream. This ensures that cached data is processed.
     *
     * What this actually means may vary between stream types. In some cases, flushing
     * may not be applicable, in which case calling this function has no effect.
     */
    virtual void flush() {}

    /*!
     * Check if the stream is still good for use, and no errors were detected.
     *
     * \return False when an error was detected.
     */
    virtual bool good() { return false; }

    /*!
     * Read the entire stream into a BufferPtr.
     *
     * \return A BufferPtr with the contents of this stream. Depending on the stream type,
     * this may not be a copy.
     * \sa Buffer
     */
    virtual IOBufferPtr get_as_buffer();

  protected:
    std::string m_name;
    size_t      m_size;
    int         m_access_mode;
};

typedef std::shared_ptr<IOStream> IOStreamPtr;

} /* namespace util */

