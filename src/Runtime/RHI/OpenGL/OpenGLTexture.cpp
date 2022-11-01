// #include <iostream>
// #include "OpenGL/OpenGLTexture.h"
// #include "OpenGL/OpenGLDynamicRHI.h"

// namespace nilou {
// // RHITextureParams RHITextureParams::DefaultParams = RHITextureParams();
// // OpenGLTexture::OpenGLTexture(std::string TextureName, GLenum PixelFormat, GLenum Target, unsigned int Width, unsigned int Height/*,
// // 		unsigned int Mag_Filter, unsigned int Min_Filter,
// // 		unsigned int Wrap_S, unsigned int Wrap_T*/)
// // 		: TextureName(TextureName)
// // 		, m_Target(Target)
// // 		, Width(Width)
// // 		, Height(Height)
// // 		, m_MipmapGenerated(false)
// // 	{
// //   glGenTextures(1, &m_Texture);
// //   glBindTexture(m_Target, m_Texture);

// //   // glTexParameteri(m_Target, GL_TEXTURE_MAG_FILTER, ETextureFilters::TF_Linear);
// //   // glTexParameteri(m_Target, GL_TEXTURE_MIN_FILTER, ETextureFilters::TF_Linear);
// //   // glTexParameteri(m_Target, GL_TEXTURE_WRAP_S, ETextureWrapModes::TW_Repeat);
// //   // glTexParameteri(m_Target, GL_TEXTURE_WRAP_T, ETextureWrapModes::TW_Repeat);

// //   switch (PixelFormat) {
// //   case GL_RGBA8UI:
// //     this->PixelFormat = GL_RGBA8;
// //     m_Format = GL_RGBA;
// //     m_Type = GL_UNSIGNED_BYTE;
// //     BytesPerPixel = 4;
// //     break;
// //   case GL_RGB8UI:
// //     this->PixelFormat = GL_RGB8;
// //     m_Format = GL_RGB;
// //     m_Type = GL_UNSIGNED_BYTE;
// //     BytesPerPixel = 3;
// //     break;
// //   case GL_R16F:
// //     this->PixelFormat = PixelFormat;
// //     m_Format = GL_RED;
// //     m_Type = GL_FLOAT;
// //     BytesPerPixel = 2;
// //     break;
// //   case GL_R32F:
// //     this->PixelFormat = PixelFormat;
// //     m_Format = GL_RED;
// //     m_Type = GL_FLOAT;
// //     BytesPerPixel = 4;
// //     break;
// //   case GL_RG32F:
// //     this->PixelFormat = PixelFormat;
// //     m_Format = GL_RG;
// //     m_Type = GL_FLOAT;
// //     BytesPerPixel = 8;
// //     break;
// //   case GL_R16I:
// //     this->PixelFormat = GL_R16I;
// //     m_Format = GL_RED_INTEGER;
// //     m_Type = GL_SHORT;
// //     BytesPerPixel = 2;
// //     break;
// //   // case GL_RG16F:
// //   // case GL_RG32F:
// //   //	m_Format = GL_RG;
// //   //	m_Type = GL_FLOAT;
// //   //	break;
// //   // case GL_RGB16F:
// //   // case GL_RGB32F:
// //   //	m_Format = GL_RGB;
// //   //	m_Type = GL_FLOAT;
// //   //	break;
// //   case GL_RGBA16F:
// //     this->PixelFormat = PixelFormat;
// //     m_Format = GL_RGBA;
// //     m_Type = GL_HALF_FLOAT;
// //     BytesPerPixel = 8;
// //     break;
// //   case GL_RGBA32F:
// //     this->PixelFormat = PixelFormat;
// //     m_Format = GL_RGBA;
// //     m_Type = GL_FLOAT;
// //     BytesPerPixel = 16;
// //     break;
// //   case GL_DEPTH_COMPONENT:
// //     this->PixelFormat = PixelFormat;
// //     m_Format = GL_DEPTH_COMPONENT;
// //     m_Type = GL_FLOAT;
// //     BytesPerPixel = 3;
// //     break;

// //   default:
// //     std::cerr << "PixelFormat: " << PixelFormat << " not implemented"
// //               << std::endl;
// //     throw("");
// //     break;
// //   }
// // 	}
// // 	OpenGLTexture2D::OpenGLTexture2D(std::string TextureName, unsigned int PixelFormat, unsigned int width, unsigned int height)
// // 		: OpenGLTexture(TextureName, PixelFormat, GL_TEXTURE_2D, width, height) 
// // 	{
// // 	}
// // 	OpenGLTexture2D::OpenGLTexture2D(std::string TextureName, unsigned int PixelFormat, unsigned int width, unsigned int height, void *data)
// // 		: OpenGLTexture(TextureName, PixelFormat, GL_TEXTURE_2D, width, height)
// // 	{
// // 		glTexImage2D(m_Target, 0, this->PixelFormat, width, height, 0, m_Format, m_Type, data);
// // 		glBindTexture(m_Target, 0);
// // 	}

// // 	OpenGLTexture3D::OpenGLTexture3D(std::string TextureName, unsigned int PixelFormat, unsigned int width, unsigned int height, unsigned int depth)
// // 		: OpenGLTexture(TextureName, PixelFormat, GL_TEXTURE_3D, width, height)
// // 		, Depth(depth)
// // 	{
// // 	}

// // 	OpenGLTexture3D::OpenGLTexture3D(std::string TextureName, unsigned int PixelFormat, unsigned int width, unsigned int height, unsigned int depth, void *data)
// // 		: OpenGLTexture(TextureName, PixelFormat, GL_TEXTURE_3D, width, height)
// // 		, Depth(depth)
// // 	{
// // 		glTexImage3D(m_Target, 0, this->PixelFormat, width, height, depth, 0, m_Format, m_Type, data);
// // 		glBindTexture(m_Target, 0);
// // 	}

// // 	OpenGLTextureCube::OpenGLTextureCube(std::string TextureName, unsigned int PixelFormat, unsigned int width, unsigned int height, void *data[6]/*, unsigned int Mag_Filter, unsigned int Min_Filter, unsigned int Wrap_S, unsigned int Wrap_T, unsigned int Wrap_R*/)
// // 		: OpenGLTexture(TextureName, PixelFormat, GL_TEXTURE_CUBE_MAP, width, height/*, Mag_Filter, Min_Filter, Wrap_S, Wrap_T*/)
// // 	{
// // 		//glTexParameteri(m_Target, GL_TEXTURE_WRAP_R, Wrap_R);
// // 		for (unsigned int i = 0; i < 6; i++)
// // 		{
// // 			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
// // 				0, this->PixelFormat, width, height, 0, m_Format, m_Type, data[i]
// // 			);
// // 		}
// // 	}
// // 	OpenGLTexture2DArray::OpenGLTexture2DArray(std::string TextureName, unsigned int PixelFormat, unsigned int width, unsigned int height, unsigned int layer_count, void *data/*, unsigned int Mag_Filter, unsigned int Min_Filter, unsigned int Wrap_S, unsigned int Wrap_T*/)
// // 		: OpenGLTexture(TextureName, PixelFormat, GL_TEXTURE_2D_ARRAY, width, height/*, Mag_Filter, Min_Filter, Wrap_S, Wrap_T*/)
// // 		, LayerCount(layer_count)
// // 	{
// // 		glTexImage3D(m_Target, 0, this->PixelFormat, width, height, layer_count, 0, m_Format, m_Type, data);
// // 		glBindTexture(m_Target, 0);
// // 	}
// // 	OpenGLTextureImage2D::OpenGLTextureImage2D(std::string TextureName, unsigned int PixelFormat, unsigned int width, unsigned int height, bool mipmap)
// // 		: OpenGLTexture2D(TextureName, PixelFormat, width, height)
// // 	{
// // 		int levels = std::min(log2(width), log2(height));
// // 		if (mipmap)
// // 			glTexStorage2D(m_Target, levels, this->PixelFormat, width, height);
// // 		else
// // 			glTexStorage2D(m_Target, 1, this->PixelFormat, width, height);
// // 		glBindTexture(m_Target, 0);
// // 	}
// // 	OpenGLTextureImage3D::OpenGLTextureImage3D(std::string TextureName, unsigned int PixelFormat, unsigned int width, unsigned int height, unsigned int depth, bool mipmap)
// // 		: OpenGLTexture3D(TextureName, PixelFormat, width, height, depth)
// // 	{
// // 		int levels = std::min(log2(width), std::min(log(height), log(depth)));
// // 		if (mipmap)
// // 			glTexStorage3D(m_Target, levels, this->PixelFormat, width, height, depth);
// // 		else
// // 			glTexStorage3D(m_Target, 1, this->PixelFormat, width, height, depth);
// // 		glBindTexture(m_Target, 0);
// // 	}
// 	//OpenGLTextureCube::OpenGLTextureCube(std::string TextureName, unsigned int internal_format[6], unsigned int width[6], unsigned int height[6], unsigned int format[6], unsigned int type[6], void *data[6], unsigned int Mag_Filter, unsigned int Min_Filter, unsigned int Wrap_S, unsigned int Wrap_T, unsigned int Wrap_R)
// 	//	: OpenGLTexture(TextureName, GL_TEXTURE_CUBE_MAP, Mag_Filter, Min_Filter, Wrap_S, Wrap_T)
// 	//{
// 	//	glTexParameteri(m_Target, GL_TEXTURE_WRAP_R, Wrap_R);
// 	//	for (unsigned int i = 0; i < 6; i++)
// 	//	{
// 	//		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
// 	//			0, internal_format[i], width[i], Height[i], 0, GL_RGB, GL_UNSIGNED_BYTE, image->data
// 	//		);
// 	//	}
// 	//}
// }
