#pragma once
#include <string>

#include <json/json.hpp>
#include "Platform.h"

namespace nilou {

    struct FArchiveBuffer
    {
        FArchiveBuffer(nlohmann::json &InJson, uint32 InBufferSize, std::unique_ptr<unsigned char[]> InBuffer)
            : Json(InJson), BufferSize(InBufferSize), Buffer(std::move(InBuffer))
        { }
        nlohmann::json &Json;
        uint32 BufferSize;
        std::unique_ptr<unsigned char[]> Buffer;
    };

    class FArchiveBuffers
    {
    public:
        std::vector<FArchiveBuffer> Blocks;
        void AddBuffer(nlohmann::json &InJson, uint32 InBufferSize, void* InBuffer);
    };

    class FArchive
    {
    public:
        nlohmann::json json;
        FArchiveBuffers OutBuffers;
        char magic[4];
        uint32 version;
        uint32 FileLength;
        uint32 JsonLength;
        char JsonChunkType[4];
        uint32 BinLength;
        char BinChunkType[4];
        std::unique_ptr<unsigned char[]> InBuffer = nullptr;
        void WriteToPath(const std::filesystem::path &Path);
        void LoadFromPath(const std::filesystem::path &Path);
    };

}