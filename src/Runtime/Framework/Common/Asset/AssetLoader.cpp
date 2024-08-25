#include "GameStatics.h"
#include "Templates/ObjectMacros.h"
#include <filesystem>
#include <glad/glad.h>
#define TINYGLTF_ENABLE_DRACO
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <tinygltf/tiny_gltf.h>
// #define STBI_MSC_SECURE_CRT

#include <iostream>
#include <gdal.h>
#include <gdal_priv.h>

#include "DDS.h"

#include "AssetLoader.h"
#include "Common/Log.h"

namespace nilou {

	AssetLoader::AssetLoader()
	{
		GDALAllRegister();
	}


	AssetLoader *GetAssetLoader()
	{
		static AssetLoader *GAssetLoader = new AssetLoader;
		return GAssetLoader;
	}

    EPixelFormat TranslateToEPixelFormat(int channel, int bits, int pixel_type)
    {
        EPixelFormat PixelFormat = EPixelFormat::PF_Unknown;
        if (channel == 1)
        {
            switch (pixel_type)
            {
                case GL_UNSIGNED_BYTE: PixelFormat = EPixelFormat::PF_R8; break;
                case GL_FLOAT: 
                    if (bits == 16)
                        PixelFormat = EPixelFormat::PF_R16F;
                    else if (bits == 32)
                        PixelFormat = EPixelFormat::PF_R32F;
                    break;
                default: std::cout << "[ERROR] Not supported pixel type. channel: " << channel << " pixel type" << pixel_type << std::endl; break;
            }
        }
        else if (channel == 2)
        {
            switch (pixel_type)
            {
                case GL_UNSIGNED_BYTE: PixelFormat = EPixelFormat::PF_R8G8; break;
                case GL_FLOAT: PixelFormat = EPixelFormat::PF_R32G32F; break;
                case GL_HALF_FLOAT: PixelFormat = EPixelFormat::PF_R16G16F; break;
                default: std::cout << "[ERROR] Not supported pixel type. channel: " << channel << " pixel type" << pixel_type << std::endl; break;
            }
        }
        else if (channel == 3)
        {
            switch (pixel_type)
            {
                case GL_UNSIGNED_BYTE: PixelFormat = EPixelFormat::PF_R8G8B8; break;
                default: std::cout << "[ERROR] Not supported pixel type. channel: " << channel << " pixel type" << pixel_type << std::endl; break;
            }
        }
        else if (channel == 4)
        {
            switch (pixel_type)
            {
                case GL_UNSIGNED_BYTE: PixelFormat = EPixelFormat::PF_R8G8B8A8; break;
                case GL_FLOAT: PixelFormat = EPixelFormat::PF_R32G32B32A32F; break;
                case GL_HALF_FLOAT: PixelFormat = EPixelFormat::PF_R16G16B16A16F; break;
                default: std::cout << "[ERROR] Not supported pixel type. channel: " << channel << " pixel type" << pixel_type << std::endl; break;
            }
        }
        else 
        {
            std::cout << "[ERROR] Not supported pixel type. channel: " << channel << " pixel type" << pixel_type << std::endl;
        }
        return PixelFormat;
    }

	std::shared_ptr<tinygltf::Model> AssetLoader::SyncReadGLTFModel(const char *filePath)
	{
		
		tinygltf::Model *model = new tinygltf::Model;
		tinygltf::TinyGLTF loader;
		std::string err;
		std::string warn;

		bool ret = loader.LoadASCIIFromFile(model, &err, &warn, filePath);// "D:\\UnderwaterRendering\\UnderwaterSimulationSystem\\Assets\\simple_mesh.gltf");
		if (!ret)
		{
			std::cout << err << ' ' << warn << std::endl;
			return nullptr;
		}
		std::string model_name = std::filesystem::path(filePath).filename().string();
		for (int i = 0; i < model->meshes.size(); i++)
		{
			model->meshes[i].name = model_name + "_mesh_"+std::to_string(i);
		}
		for (int i = 0; i < model->materials.size(); i++)
		{
			model->materials[i].name = model_name + "_material_"+std::to_string(i);
		}
		for (int i = 0; i < model->textures.size(); i++)
		{
			model->textures[i].name = model_name + "_texture_"+std::to_string(i);
		}
		return std::shared_ptr<tinygltf::Model>(model);
	}

	std::string AssetLoader::SyncOpenAndReadText(const char *filePath)
	{
		std::filesystem::path absolute_path;
		std::filesystem::path FilePath = std::filesystem::path(filePath);
		if (FilePath.is_absolute())
		{
			if (std::filesystem::exists(FilePath) && std::filesystem::is_regular_file(FilePath))
				absolute_path = FilePath;
			else
			 	NILOU_LOG(Error, "File not found: {}", FilePath.generic_string());
		}
		else 
		{
			// for (const std::filesystem::path &WorkDir : AssetLoader::WorkDirectory)
			// {
			// 	absolute_path = WorkDir / FilePath;
				
			// 	if (std::filesystem::exists(FilePath) && std::filesystem::is_regular_file(FilePath))
			// 		break;
			// }
		}


		if (absolute_path.empty())
			return "";

        std::stringstream res;
        std::ifstream stream(absolute_path);
        char buffer[1024];
        while (stream.getline(buffer, sizeof(buffer)))
        {
            res << buffer << "\n";
        }
		return res.str();
		// FILE *fp = fopen(absolute_path.c_str(), "r");
		// long pos = ftell(fp);
		// fseek(fp, 0, SEEK_END);
		// size_t length = ftell(fp);
		// fseek(fp, pos, SEEK_SET);
		// char *data = new char[length + 1];
		// length = fread(data, 1, length, fp);
		// data[length] = '\0';
		// fclose(fp);
		// return data;
	}

	std::filesystem::path AssetLoader::FileExistsInDir(const std::filesystem::path &Directory, const std::filesystem::path &FileName)
	{
		if (!std::filesystem::exists(Directory))
		{
			std::cout << "Directory: " << Directory << " doesn't exist" << std::endl;
			throw;
		}

		if (!FileName.has_filename())
		{
			std::cout << FileName << " is not a file" << std::endl;
			throw;
		}
                
		for (const std::filesystem::directory_entry & dir_entry : 
			std::filesystem::recursive_directory_iterator(Directory))
		{
			if (dir_entry.is_regular_file())
			{
				if (FileName.filename() == dir_entry.path().filename())
					return dir_entry.path();
			}
		}
	}

	FImage AssetLoader::SyncOpenAndReadImage(const char *filePath)
	{
		std::filesystem::path absolute_path = std::filesystem::path(filePath);

		std::string AbsolutePath = absolute_path.generic_string();

		FImage img; 
		if (GameStatics::EndsWith(AbsolutePath, ".tiff") || GameStatics::EndsWith(AbsolutePath, ".tif"))
		{
			GDALDataset *ds = (GDALDataset *)GDALOpen(AbsolutePath.c_str(), GA_ReadOnly);
			int width = ds->GetRasterXSize();
			int height = ds->GetRasterYSize();
			int channel = ds->GetRasterCount();
			EPixelFormat PixelFormat;
			GDALRasterBand *pBand = ds->GetRasterBand(1);
			GDALDataType type = pBand->GetRasterDataType();
			//unsigned char *data = nullptr;
			int byte_per_pixel_per_channel = 0;
			switch (type)
			{
			case GDT_Byte:
				byte_per_pixel_per_channel = 1;
				PixelFormat = TranslateToEPixelFormat(channel, byte_per_pixel_per_channel*8, TINYGLTF_COMPONENT_TYPE_BYTE);
				break;
			case GDT_UInt16:
				byte_per_pixel_per_channel = 2;
				PixelFormat = TranslateToEPixelFormat(channel, byte_per_pixel_per_channel*8, TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT);
				break;
			case GDT_Int16:
				byte_per_pixel_per_channel = 2;
				PixelFormat = TranslateToEPixelFormat(channel, byte_per_pixel_per_channel*8, TINYGLTF_COMPONENT_TYPE_SHORT);
				break;
			case GDT_UInt32:
				byte_per_pixel_per_channel = 4;
				PixelFormat = TranslateToEPixelFormat(channel, byte_per_pixel_per_channel*8, TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT);
				break;
			case GDT_Int32:
				byte_per_pixel_per_channel = 4;
				PixelFormat = TranslateToEPixelFormat(channel, byte_per_pixel_per_channel*8, TINYGLTF_COMPONENT_TYPE_INT);
				break;
			case GDT_Float32:
				byte_per_pixel_per_channel = 4;
				PixelFormat = TranslateToEPixelFormat(channel, byte_per_pixel_per_channel*8, TINYGLTF_COMPONENT_TYPE_FLOAT);
				break;
			default:
				return img;
			}
			img = FImage(width, height, PixelFormat, EImageType::IT_Image2D);
			img.AllocateSpace();
			for (int band = 1; band <= channel; band++)
			{
				pBand = ds->GetRasterBand(band);
				type = pBand->GetRasterDataType();
				std::unique_ptr<uint8[]> temp_data = std::make_unique<uint8[]>(width * height * byte_per_pixel_per_channel);
				pBand->RasterIO(GF_Read, 0, 0, width, height, temp_data.get(), width, height, type, 0, 0);
				for (int row = 0; row < img.GetHeight(); row++)
				{
					for (int col = 0; col < img.GetWidth(); col++)
					{
						uint8* ptr = (uint8*)img.GetPointer(row, col, 0);
						uint8* data_ptr = temp_data.get() + (width * row + col) * byte_per_pixel_per_channel;
						std::copy(data_ptr, data_ptr+byte_per_pixel_per_channel, ptr);
					}
				}
			}
		}
		else if (GameStatics::EndsWith(AbsolutePath, ".dds"))
		{
			dds::DDS_Image_Info info;

			if (!LoadFromDDS(AbsolutePath.c_str(), &info)) {
				std::cout << "Error: Could not load texture!\n";
				return img;
			}
			img = FImage(
				info.Width, info.Height, 
				info.Format, EImageType::IT_Image2D, info.MipLevels);
			img.AllocateSpace();
			std::copy((uint8*)info.Data, (uint8*)info.Data+info.DataSize, img.GetData());
			
		}
		else
		{
			int width, height, channel;
			uint8* temp_data = stbi_load(AbsolutePath.c_str(), (int *)&width, (int *)&height, (int *)&channel, 0);
			EPixelFormat PixelFormat = TranslateToEPixelFormat(channel, 8, GL_UNSIGNED_BYTE);
			img = FImage(width, height, PixelFormat, EImageType::IT_Image2D, 1);
			img.AllocateSpace();
			std::copy(temp_data, temp_data+img.GetDataSize(), img.GetData());
			stbi_image_free(temp_data);
		}
		return img;
	}
}