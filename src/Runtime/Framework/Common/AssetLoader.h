#pragma once
#include <filesystem>
#include <set>
#include <string>

#include "Interface/IRuntimeModule.h"
#include "Texture.h"


namespace tinygltf {
	class Model;
}
namespace nilou {
    EPixelFormat TranslateToEPixelFormat(int channel, int bits, int pixel_type);

	class Image;
	class AssetLoader : public IRuntimeModule
	{
	public:
		AssetLoader();

		virtual ~AssetLoader() {};

		virtual int StartupModule() override;
		virtual void ShutdownModule() override;

		virtual void Tick(double DeltaTime);

		std::shared_ptr<tinygltf::Model> SyncReadGLTFModel(const char *filePath);

		std::string SyncOpenAndReadText(const char *filePath);

		std::shared_ptr<nilou::FImage> SyncOpenAndReadImage(const char *filePath);
		// static std::string SolutionDir; 
		// static std::string AssetDir;
		// static std::filesystem::path WorkDirectory;
		// static std::set<std::filesystem::path> ShaderDirectories;
		// static std::set<std::filesystem::path> AssetDirectories;
		static std::set<std::filesystem::path> GetAllSearchDirectories();
		static std::filesystem::path FileExistsInDir(const std::filesystem::path &Directory, const std::filesystem::path &FileName);
    private:
        std::vector<std::string> m_strSearchPath;
	};

	extern AssetLoader *g_pAssetLoader;
}