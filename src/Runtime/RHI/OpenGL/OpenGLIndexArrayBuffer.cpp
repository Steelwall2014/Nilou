// #include "OpenGL/OpenGLIndexArrayBuffer.h"

// namespace nilou {
// 	//OpenGLIndexArrayBuffer::OpenGLIndexArrayBuffer(int Count, GLenum DataType, GLenum Usage)
// 	//	: Usage(Usage)
// 	//	, DataType(DataType)
// 	//	, Count(Count)
// 	//{
// 	//	void *Data;
// 	//	int byte_size;
// 	//	switch (DataType)
// 	//	{
// 	//	case 5121:	// UNSIGNED_BYTE
// 	//		Data = new unsigned char[Count];
// 	//		byte_size = 1; break;
// 	//	case 5123:	// UNSIGNED_SHORT
// 	//		Data = new unsigned short[Count];
// 	//		byte_size = 2; break;
// 	//	case 5125:	// UNSIGNED_INT
// 	//		Data = new unsigned int[Count];
// 	//		byte_size = 4; break;
// 	//	default:
// 	//		throw("Invalid DataType");
// 	//		return;
// 	//	}
// 	//	DataByteSize = Count * byte_size;
// 	//	glGenBuffers(1, &m_Buffer);
// 	//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Buffer);
// 	//	glBufferData(GL_ELEMENT_ARRAY_BUFFER, DataByteSize, Data, Usage);
// 	//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
// 	//	delete[] Data;
// 	//}
// 	OpenGLIndexArrayBuffer::OpenGLIndexArrayBuffer(int DataByteSize, void *Data, GLenum DataType, int Count, GLenum Usage)
// 		: OpenGLBaseBuffer(GL_ELEMENT_ARRAY_BUFFER, DataByteSize, Data, Usage)
// 		, DataByteSize(DataByteSize)
// 		//, Data(Data)
// 		, Usage(Usage)
// 		, DataType(DataType)
// 		, Count(Count)
// 	{
// 		int byte_size;
// 		switch (DataType)
// 		{
// 		case 5121:	// UNSIGNED_BYTE
// 			byte_size = 1; break;
// 		case 5123:	// UNSIGNED_SHORT
// 			byte_size = 2; break;
// 		case 5125:	// UNSIGNED_INT
// 			byte_size = 4; break;
// 		default:
// 			throw("Invalid DataType");
// 			return;
// 		}
// 		if (Count == 0)
// 			this->Count = DataByteSize / byte_size;
// 		//glGenBuffers(1, &m_Buffer);
// 		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_Buffer);
// 		//glBufferData(GL_ELEMENT_ARRAY_BUFFER, DataByteSize, Data, Usage);
// 		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
// 	}

// 	//OpenGLIndexArrayBuffer::~OpenGLIndexArrayBuffer()
// 	//{
// 	//	glDeleteBuffers(1, &m_Buffer);
// 	//}


// }
