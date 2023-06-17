#pragma once
#include <cstdint>
#include <iostream>
#include <cwchar>
#include <filesystem>

#include "Common/Log.h"
#include "RHIResources.h"

namespace nilou {
	struct GfxConfiguration {
		/// Inline all-elements constructor.
		/// \param[in] r the red color depth in bits
		/// \param[in] g the green color depth in bits
		/// \param[in] b the blue color depth in bits
		/// \param[in] a the alpha color depth in bits
		/// \param[in] d the depth buffer depth in bits
		/// \param[in] s the stencil buffer depth in bits
		/// \param[in] msaa the msaa sample count
		/// \param[in] width the screen width in pixel
		/// \param[in] height the screen height in pixel
		GfxConfiguration(EPixelFormat InSwapChainFormat = EPixelFormat::PF_R8G8B8A8_sRGB, EPixelFormat InDepthFormat = EPixelFormat::PF_D24S8, uint32_t msaa = 0,
			uint32_t width = 1920, uint32_t height = 1080, const wchar_t *app_name = L"Nilou") :
			SwapChainFormat(InSwapChainFormat), DepthFormat(InDepthFormat), msaaSamples(msaa),
			screenWidth(width), screenHeight(height), appName(app_name)
		{
			workDir = std::filesystem::current_path();
		}

		EPixelFormat SwapChainFormat;
		EPixelFormat DepthFormat;
		uint32_t msaaSamples; ///< MSAA samples
		uint32_t screenWidth;
		uint32_t screenHeight;
		std::filesystem::path workDir;
		const wchar_t *appName;
		const char* defaultRHI = "opengl";

		friend std::ostream &operator<<(std::ostream &out, const GfxConfiguration &conf)
		{
			out << "App Name:" << conf.appName << std::endl;
			out << "Working Directory:" << conf.workDir << std::endl;
			out << "GfxConfiguration:" <<
				" SwapChainFormat: " << magic_enum::enum_name(conf.SwapChainFormat) << 
				" M:" << conf.msaaSamples <<
				" W:" << conf.screenWidth <<
				" H:" << conf.screenHeight <<
				std::endl;
			return out;
		}
	};
}