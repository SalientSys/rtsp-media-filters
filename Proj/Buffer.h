///
/// @class Buffer
///
/// Created: 08/18/2019
/// Authored by: M. Kinzer
///
#pragma once

#include <memory>
#include <stdexcept>
#include <cassert>

namespace CvRtsp
{
	class Buffer
	{
	public:
		/// Byte array definition for a data buffer.
		using DataBuffer = std::unique_ptr<BYTE[], std::default_delete<BYTE[]>>;

		/// 
		/// Buffer default constructor.
		explicit Buffer() :
			m_buffer(nullptr),
			m_size(0),
			m_prebufferSize(0),
			m_postbufferSize(0)
		{
		}

		///
		/// Buffer constructor that takes ownership of the passed in buffer.
		///
		/// @param[in] buffer The buffer that will be managed by this class.
		/// @param[in] size The size of the buffer to be managed.
		explicit Buffer(BYTE* buffer, size_t size) :
			m_buffer(std::make_unique<BYTE[]>(size)),
			m_size(size),
			m_prebufferSize(0),
			m_postbufferSize(0)
		{
			memcpy(m_buffer.get(), buffer, size);
		}

		///
		/// Buffer Constructor that takes ownership of the passed in buffer.
		/// The buffer contains prebuffer space to write data ahead of the data
		/// already written. This can be useful for prepending data after the
		/// original data has already been written.
		///
		/// @param[in] buffer The buffer that will be managed by this class
		/// @param[in] size The size of the buffer to be managed
		/// @param[in] prebufferSize The buffer that will be managed by this class
		/// @param[in] postbufferSize The buffer that will be managed by this class
		explicit Buffer(BYTE* buffer, size_t size, size_t prebufferSize, size_t postbufferSize) :
			m_buffer(std::make_unique<BYTE[]>(size + prebufferSize + postbufferSize)),
			m_size(size),
			m_prebufferSize(prebufferSize),
			m_postbufferSize(postbufferSize)
		{
			assert(prebufferSize >= 0 && postbufferSize >= 0);
			if (size < m_prebufferSize + m_postbufferSize)
			{
				throw std::runtime_error("Invalid parameters");
			}
			memcpy(m_buffer.get(), buffer, size);
		}

		/// 
		/// Buffer default destructor.
		virtual ~Buffer() = default;

		/// 
		/// Implementation of [] operator.
		///
		/// @param[in] index Element in array to access.
		///
		/// @return Reference as byte element.
		BYTE& operator[](const std::ptrdiff_t index) const
		{
			return m_buffer[index + m_prebufferSize];
		}

		/// 
		/// Size of the data contained in the buffer.
		///
		/// @return Size of the data buffer.
		size_t GetSize() const
		{
			return m_size - m_prebufferSize - m_postbufferSize;
		}

		/// 
		/// Total size of the data contained in the buffer.
		///
		/// @return Size of the total data buffer including pre and post buffer.
		size_t GetTotalSize() const
		{
			return m_size + m_prebufferSize + m_postbufferSize;
		}

		/// 
		/// Size of the data contained in the prebuffer.
		///
		/// @return Size of the prebuffer.
		size_t GetPrebufferSize() const
		{
			return m_prebufferSize;
		}

		/// 
		/// Size of the data contained in the postbuffer.
		///
		/// @return Size of the postbuffer.
		size_t GetPostbufferSize() const
		{
			return m_postbufferSize;
		}

		/// 
		/// Pointer to the data in the buffer.
		///
		/// @return Pointer to data.
		BYTE* Data() const
		{
			return m_buffer.get() + m_prebufferSize;
		}

		/// 
		/// Put data into the data buffer.
		///
		/// @param[in] buffer Data for the buffer.
		/// @param[in] size Size of the data to be put in buffer.
		void SetData(BYTE* buffer, size_t size)
		{
			if (m_buffer != nullptr)
			{
				m_buffer.reset();
			}
			m_size = size;
			m_prebufferSize = 0;
			m_postbufferSize = 0;

			m_buffer = std::make_unique<BYTE[]>(size);
			memcpy(m_buffer.get(), buffer, size);
		}

	private:
		/// Buffer containing the data.
		DataBuffer m_buffer;

		/// Size of the data in the buffer.
		size_t m_size;

		/// Size of the prebuffer.
		size_t m_prebufferSize;

		/// Size of the postbuffer.
		size_t m_postbufferSize;
	};
}
