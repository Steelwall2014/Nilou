#include "GameStatics.h"
#include "Templates/ObjectMacros.h"
#include <filesystem>
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


	AssetLoader *GetAssetLoader()
	{
		static AssetLoader *GAssetLoader = new AssetLoader;
		return GAssetLoader;
	}

    EPixelFormat TranslateToEPixelFormat(int channel, int bits, int pixel_type)
    {
        EPixelFormat PixelFormat = EPixelFormat::PF_UNKNOWN;
        if (channel == 1)
        {
            switch (pixel_type)
            {
                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: PixelFormat = EPixelFormat::PF_R8; break;
                case TINYGLTF_COMPONENT_TYPE_FLOAT: 
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
                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: PixelFormat = EPixelFormat::PF_R8G8; break;
                case TINYGLTF_COMPONENT_TYPE_FLOAT: 
                    if (bits == 16)
                        PixelFormat = EPixelFormat::PF_R16G16F;
                    else if (bits == 32)
                        PixelFormat = EPixelFormat::PF_R32G32F;
                    break;
                default: std::cout << "[ERROR] Not supported pixel type. channel: " << channel << " pixel type" << pixel_type << std::endl; break;
            }
        }
        else if (channel == 3)
        {
            switch (pixel_type)
            {
                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: PixelFormat = EPixelFormat::PF_R8G8B8; break;
                default: std::cout << "[ERROR] Not supported pixel type. channel: " << channel << " pixel type" << pixel_type << std::endl; break;
            }
        }
        else if (channel == 4)
        {
            switch (pixel_type)
            {
                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE: PixelFormat = EPixelFormat::PF_R8G8B8A8; break;
                case TINYGLTF_COMPONENT_TYPE_FLOAT:
                    if (bits == 16)
                        PixelFormat = EPixelFormat::PF_R16G16B16A16F;
                    else if (bits == 32)
                        PixelFormat = EPixelFormat::PF_R32G32B32A32F;
                    break;
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
			 	NILOU_LOG(Error, "File not found: " + FilePath.generic_string());
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

	std::shared_ptr<FImage> AssetLoader::SyncOpenAndReadImage(const char *filePath)
	{
		std::filesystem::path absolute_path;
		std::filesystem::path FilePath = std::filesystem::path(filePath);
		if (FilePath.is_absolute())
		{
			if (std::filesystem::exists(absolute_path) && std::filesystem::is_regular_file(absolute_path))
				absolute_path = FilePath;
		}
		else 
		{
			// for (const std::filesystem::path &WorkDir : AssetLoader::WorkDirectory)
			// {
			// 	absolute_path = WorkDir / FilePath;
				
			// 	if (std::filesystem::exists(absolute_path) && std::filesystem::is_regular_file(absolute_path))
			// 		break;
			// }
		}

		std::string AbsolutePath = absolute_path.generic_string();

		std::shared_ptr<FImage> img = nullptr; 
		if (GameStatics::EndsWith(AbsolutePath, ".tiff") || GameStatics::EndsWith(AbsolutePath, ".tif"))
		{
			img = std::make_shared<FImage>();
			GDALDataset *ds = (GDALDataset *)GDALOpen(AbsolutePath.c_str(), GA_ReadOnly);
			int width = img->Width = ds->GetRasterXSize();
			int height = img->Height = ds->GetRasterYSize();
			int channel = img->Channel = ds->GetRasterCount();
			GDALRasterBand *pBand = ds->GetRasterBand(1);
			GDALDataType type = pBand->GetRasterDataType();
			//unsigned char *data = nullptr;
			int byte_per_pixel = 0;
			switch (type)
			{
			case GDT_Int16:
				img->data = (unsigned char *)new short[width * height * channel];
				byte_per_pixel = 2;
				img->PixelFormat = TranslateToEPixelFormat(channel, byte_per_pixel*8, TINYGLTF_COMPONENT_TYPE_SHORT);
				break;
			case GDT_Float32:
				img->data = (unsigned char *)new float[width * height * channel];
				byte_per_pixel = 4;
				img->PixelFormat = TranslateToEPixelFormat(channel, byte_per_pixel*8, TINYGLTF_COMPONENT_TYPE_FLOAT);
				break;
			default:
				throw("function SyncOpenAndReadImage: DataType not implemented");
				break;
			}
			img->data_size = width * height * channel * byte_per_pixel;
			for (int band = 1; band <= channel; band++)
			{
				pBand = ds->GetRasterBand(band);
				type = pBand->GetRasterDataType();
				int offset = width * height * byte_per_pixel * (band-1);
				pBand->RasterIO(GF_Read, 0, 0, width, height, img->data + offset, width, height, type, 0, 0);
			}
		}
		else if (GameStatics::EndsWith(AbsolutePath, ".dds"))
		{
			dds::DDS_Image_Info info;

			if (!LoadFromDDS(AbsolutePath.c_str(), &info)) {
				std::cout << "Error: Could not load texture!\n";
				return nullptr;
			}
			img = std::make_shared<FImage>();
			img->data = (unsigned char *)info.Data;
			img->data_size = info.DataSize;
			img->Width = info.Width;
			img->Height = info.Height;
			img->Channel = 4;			// 因为dds纹理目前只有perlin噪声纹理用到了，所以这边写死
			img->PixelFormat = TranslateToEPixelFormat(4, info.DataSize/info.Width/info.Height/img->Channel * 8, dds::MapFormat_GLType(info.Format));
			
		}
		else
		{
			img = std::make_shared<FImage>();
			img->data = stbi_load(AbsolutePath.c_str(), (int *)&img->Width, (int *)&img->Height, (int *)&img->Channel, 0);
			img->PixelFormat = TranslateToEPixelFormat(img->Channel, 8, TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE);
		}
		return img;
	}
}