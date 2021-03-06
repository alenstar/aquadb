#pragma once
#include <iostream>
#include <memory>
namespace util
{

/*!
 * \brief Wrapper class for memory buffers
 *
 * This class serves as a memory buffer wrapper. This makes working with buffers much easier,
 * helping to prevent memory leaks.
 */
class IOBuffer
{
public:
	/*!
	 * The delete mode for a Buffer. This determines if the internal buffer is automatically deleted
	 * in the destructor.
	 */
	enum class DeleteMode
	{
		None,				/**< enum Don't touch the internal buffer on destruct. */
		DeleteOnDestruct	/**< enum Free the internal buffer when the destructor is called. */
	};

	/*!
	 * Construct an empty buffer. This constructor is merely used to construct an empty container
	 * that can be used to allocate memory later.
	 */
	IOBuffer();

	/*!
	 * Create a buffer of size bytes. If delete_mode is set to DeleteMode::None, this internal buffer
	 * will not be freed. Be sure to get() this pointer and free it yourself to prevent memory leaks.
	 *
	 * The internal buffer is created using malloc, this means you must free this buffer. Do not use delete [].
	 *
	 * \param size The size of the buffer to be allocated
	 * \param delete_mode Determines if the internal buffer should be automatically freed or not.
	 * \sa DeleteMode
	 */
	IOBuffer(size_t size, DeleteMode delete_mode = DeleteMode::DeleteOnDestruct);

	/*!
	 * Wrap around a buffer of size bytes. If delete_mode is set to DeleteMode::None, this buffer will not
	 * be freed. Be sure to free it yourself to prevent memory leaks.
	 *
	 * The buffer pointer must have been allocated using malloc in order to use reserve, resize, append,
	 * DeleteMode::DeleteOnDestruct, etc.
	 *
	 * \param buffer The buffer to wrap around
	 * \param size The size of the buffer
	 * \param delete_mode Determines if the internal buffer should be automatically freed or not.
	 * \sa DeleteMode 
	 */
	IOBuffer(void *buffer, size_t size, DeleteMode delete_mode = DeleteMode::DeleteOnDestruct);

	/*!
	 * Destructor
	 *
	 * If the DeleteMode is set to DeleteMode::DeleteOnDestruct, the buffer will be automatically freed.
	 */
	~IOBuffer();

	/*!
	 * Prevent copying this class.
	 */
	IOBuffer(const IOBuffer&) = delete;

	/*!
	 * Prevent copying this class.
	 */
	IOBuffer & operator=(const IOBuffer&) = delete;

	//Allocate at least n bytes
	bool				reserve(size_t n);

	//Resize to the specified size.
	bool				resize(size_t n);

	bool				append(void *data, size_t len);

	void				free();

	void *				get() { return m_buffer; }

	size_t				size() { return m_size; }
	size_t				reserved_size() { return m_reserved_size; }

	void				set_delete_mode(DeleteMode mode);
	DeleteMode			get_delete_mode() { return m_delete_mode; }

	void				set_size(size_t size) { m_size = size; }

private:
	void *				m_buffer;

	size_t				m_size;
	size_t				m_reserved_size;

	DeleteMode			m_delete_mode;
};

typedef std::shared_ptr<IOBuffer> IOBufferPtr;

} /* namespace util */

