// #include "OpenGLVertexAttribBuffer.h"

// namespace und {
// 	//OpenGLVertexAttribBuffer::OpenGLVertexAttribBuffer(
// 	//	int ElemNum, GLint ElemSize,
// 	//	GLenum DataType, bool Normalize, size_t Offset, GLenum Usage)
// 	//	: ElemSize(ElemSize)
// 	//	, DataType(DataType)
// 	//	, Normalize(Normalize)
// 	//	, Offset(Offset)
// 	//	, Usage(Usage)
// 	//{
// 	//	void *Data;
// 	//	int byte_size;
// 	//	switch (DataType)
// 	//	{
// 	//	case 5120:	// BYTE
// 	//		Data = new char[ElemNum * ElemSize];
// 	//		byte_size = 1; break;
// 	//	case 5121:	// UNSIGNED_BYTE
// 	//		Data = new unsigned char[ElemNum * ElemSize];
// 	//		byte_size = 1; break;
// 	//	case 5122:	// SHORT
// 	//		Data = new short[ElemNum * ElemSize];
// 	//		byte_size = 2; break;
// 	//	case 5123:	// UNSIGNED_SHORT
// 	//		Data = new unsigned short[ElemNum * ElemSize];
// 	//		byte_size = 2; break;
// 	//	case 5125:	// UNSIGNED_INT
// 	//		Data = new unsigned int[ElemNum * ElemSize];
// 	//		byte_size = 4; break;
// 	//	case 5126:	// FLOAT
// 	//		Data = new float[ElemNum * ElemSize];
// 	//		byte_size = 4; break;
// 	//	default:
// 	//		throw("Invalid DataType");
// 	//		return;
// 	//	}
// 	//	Stride = ElemSize * byte_size;
// 	//	DataByteSize = Stride * ElemNum;
// 	//	glGenBuffers(1, &m_Buffer);
// 	//	glBindBuffer(GL_ARRAY_BUFFER, m_Buffer);
// 	//	glBufferData(GL_ARRAY_BUFFER, DataByteSize, Data, Usage);
// 	//	glBindBuffer(GL_ARRAY_BUFFER, 0);
// 	//	delete[] Data;
// 	//}

// 	// OpenGLVertexAttribBuffer::OpenGLVertexAttribBuffer(
// 	// 	int DataByteSize, void *Data, GLint ElemSize,
// 	// 	GLenum DataType, GLsizei Stride, bool Normalize, size_t Offset, GLenum Usage)
// 	// 	: OpenGLBaseBuffer(GL_ARRAY_BUFFER, DataByteSize, Data, Usage)
// 	// 	, DataByteSize(DataByteSize)
// 	// 	//, Data(Data)
// 	// 	, ElemSize(ElemSize)
// 	// 	, DataType(DataType)
// 	// 	, Normalize(Normalize)
// 	// 	, Offset(Offset)
// 	// 	, Usage(Usage)
// 	// 	, Stride(Stride)
// 	// {
// 	// 	int byte_size;
// 	// 	switch (DataType)
// 	// 	{
// 	// 	case 5120:	// BYTE
// 	// 	case 5121:	// UNSIGNED_BYTE
// 	// 		byte_size = 1; break;
// 	// 	case 5122:	// SHORT
// 	// 	case 5123:	// UNSIGNED_SHORT
// 	// 		byte_size = 2; break;
// 	// 	case 5125:	// UNSIGNED_INT
// 	// 	case 5126:	// FLOAT
// 	// 		byte_size = 4; break;
// 	// 	default:
// 	// 		throw("Invalid DataType");
// 	// 		return;
// 	// 	}
// 	// 	if (Stride == 0)
// 	// 		this->Stride = ElemSize * byte_size;
// 	// 	//glGenBuffers(1, &m_Buffer);
// 	// 	//glBindBuffer(GL_ARRAY_BUFFER, m_Buffer);
// 	// 	//glBufferData(GL_ARRAY_BUFFER, DataByteSize, Data, Usage);
// 	// 	//glBindBuffer(GL_ARRAY_BUFFER, 0);
// 	// }

// 	//OpenGLVertexAttribBuffer::~OpenGLVertexAttribBuffer()
// 	//{
// 	//	glDeleteBuffers(1, &m_Buffer);
// 	//	//if (Data)
// 	//	//{
// 	//	//	delete[] Data;
// 	//	//	Data = nullptr;
// 	//	//}
// 	//}
// }