
#include <string>
#include <memory>

#include "buffer.h"
#include "stream.h"
#include "iostream.h"

namespace Streams
{

IOStream::IOStream(int access_mode /*= AccessMode::READ*/)
:
Stream(access_mode)
{

}

IOStream::IOStream(const std::string &name, int access_mode /*= AccessMode::READ*/)
:
Stream(name, access_mode)
{

}

size_t IOStream::read(void *buffer, size_t count)
{
	if (!(m_access_mode & AccessMode::READ))
	{
#ifdef AEON_USE_AEON_CONSOLE_LIBRARY
		Console::error("IOStream: Read on write-only stream.");
#endif

		return 0;
	}

	if (!buffer)
	{
#ifdef AEON_USE_AEON_CONSOLE_LIBRARY
		Console::error("IOStream: Input buffer is NULL.");
#endif

		return 0;
	}

	if (count == 0)
	{
#ifdef AEON_USE_AEON_CONSOLE_LIBRARY
		Console::warning("IOStream: Tried writing 0 bytes.");
#endif

		return 0;
	}

	return fread(buffer, 1, count, stdin);
}

size_t IOStream::write(const void *buffer, size_t count)
{
	if (m_access_mode != AccessMode::WRITE)
	{
#ifdef AEON_USE_AEON_CONSOLE_LIBRARY
		Console::error("IOStream: Write on read-only stream.");
#endif

		return 0;
	}

	if (!buffer)
	{
#ifdef AEON_USE_AEON_CONSOLE_LIBRARY
		Console::error("IOStream: Input buffer is NULL.");
#endif

		return 0;
	}

	if (count == 0)
	{
#ifdef AEON_USE_AEON_CONSOLE_LIBRARY
		Console::warning("IOStream: Tried writing 0 bytes.");
#endif

		return 0;
	}

	return fwrite(buffer, 1, count, stdout);
}

size_t IOStream::read_line(std::string &str)
{
	if (m_access_mode != AccessMode::READ)
	{
#ifdef AEON_USE_AEON_CONSOLE_LIBRARY
		Console::error("IOStream: Read on write-only stream.");
#endif

		return 0;
	}

	std::string line;

	for (int i = 0; i < AEON_STREAMS_MAX_TEXT_LINE_LENGTH; ++i)
	{
		int c = fgetc(stdin);

		if (c == EOF)
			break;

		if (c == '\n')
			break;

		line += (char) c;
	}

	str = line;

	return line.length();
}

bool IOStream::seek(size_t pos, SeekDirection direction)
{
	(void)(pos);
	(void)(direction);

	//This won't work for STDIN...
	return false;
}

size_t IOStream::tell() const
{
	//This won't work for STDIN...
	return 0;
}

bool IOStream::eof() const
{
	return (feof(stdin) != 0);
}

void IOStream::flush()
{
	fflush(stdout);
}

BufferPtr IOStream::get_as_buffer()
{
	//This won't work for STDIN...
	return AeonEmptyBuffer;
}

} /* namespace Streams */
