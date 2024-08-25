#include "PixelFormat.h"

namespace nilou {

//
//	Pixel format information.
//

FPixelFormatInfo::FPixelFormatInfo(
    EPixelFormat InFormat,
    const std::string& InName,
	int32 InBlockSizeX,
	int32 InBlockSizeY,
	int32 InBlockSizeZ,
	int32 InBlockBytes,
	int32 InNumComponents)
	: Name(InName)
	, Format(InFormat)
	, BlockSizeX(InBlockSizeX)
	, BlockSizeY(InBlockSizeY)
	, BlockSizeZ(InBlockSizeZ)
	, BlockBytes(InBlockBytes)
	, NumComponents(InNumComponents)
	, bIs24BitUnormDepthStencil(true)
{
}

FPixelFormatInfo    GPixelFormats[PF_MAX] =
{
	//               Format                 Name                  BlockSizeX  BlockSizeY  BlockSizeZ  BlockBytes  NumComponents
	FPixelFormatInfo(PF_Unknown,            "unknown",                     0,          0,          0,          0,             0),
    FPixelFormatInfo(PF_R8,                 "R8",                          1,          1,          1,          1,             1),
    FPixelFormatInfo(PF_R8UI,               "R8UI",               		   1,          1,          1,          1,             1),
    FPixelFormatInfo(PF_R8G8,               "R8G8",               		   1,          1,          1,          2,             2),
    FPixelFormatInfo(PF_R8G8B8,             "R8G8B8",             		   1,          1,          1,          3,             3),
    FPixelFormatInfo(PF_R8G8B8_sRGB,        "R8G8B8_sRGB",        		   1,          1,          1,          3,             3),
	FPixelFormatInfo(PF_B8G8R8,             "B8G8R8",             		   1,          1,          1,          3,             3),
	FPixelFormatInfo(PF_B8G8R8_sRGB,        "B8G8R8_sRGB",        		   1,          1,          1,          3,             3),
	FPixelFormatInfo(PF_R8G8B8A8,           "R8G8B8A8",           		   1,          1,          1,          4,             4),
	FPixelFormatInfo(PF_R8G8B8A8_sRGB,      "R8G8B8A8_sRGB",      		   1,          1,          1,          4,             4),
	FPixelFormatInfo(PF_B8G8R8A8,           "B8G8R8A8",           		   1,          1,          1,          4,             4),
	FPixelFormatInfo(PF_B8G8R8A8_sRGB,      "B8G8R8A8_sRGB",      		   1,          1,          1,          4,             4),
	FPixelFormatInfo(PF_D24S8,              "D24S8",              		   1,          1,          1,          2,             1),
	FPixelFormatInfo(PF_D32F,               "D32F",               		   1,          1,          1,          4,             1),
	FPixelFormatInfo(PF_D32FS8,             "D32FS8",             		   1,          1,          1,          5,             1),
	FPixelFormatInfo(PF_DXT1,               "DXT1",               		   4,          4,          1,          8,             3),
	FPixelFormatInfo(PF_DXT1_sRGB,          "DXT1_sRGB",          		   4,          4,          1,          8,             3),
	FPixelFormatInfo(PF_DXT5,               "DXT5",               		   4,          4,          1,         16,             4),
	FPixelFormatInfo(PF_DXT5_sRGB,          "DXT5_sRGB",          		   4,          4,          1,         16,             4),
	FPixelFormatInfo(PF_R16F,               "R16F",               		   1,          1,          1,          2,             1),
	FPixelFormatInfo(PF_R16G16F,            "R16G16F",            		   1,          1,          1,          4,             2),
	FPixelFormatInfo(PF_R16G16B16F,         "R16G16B16F",         		   1,          1,          1,          6,             3),
	FPixelFormatInfo(PF_R16G16B16A16F,      "R16G16B16A16F",      		   1,          1,          1,          8,             4),
	FPixelFormatInfo(PF_R32F,               "R32F",               		   1,          1,          1,          4,             1),
	FPixelFormatInfo(PF_R32G32F,            "R32G32F",            		   1,          1,          1,          8,             2),
	FPixelFormatInfo(PF_R32G32B32F,         "R32G32B32F",         		   1,          1,          1,         12,             3),
	FPixelFormatInfo(PF_R32G32B32A32F,      "R32G32B32A32F",      		   1,          1,          1,         16,             4)
};

}