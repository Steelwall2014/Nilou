#include <fstream>
#include "Archive.h"

namespace nilou {

    // void FArchive::WriteToPath(const std::filesystem::path &Path)
    // {
    //     BinLength = 0;
    //     for (int i = 0; i < OutBuffers.Blocks.size(); i++)
    //     {
    //         auto &Block = OutBuffers.Blocks[i];
    //         Block.Json["BufferOffset"] = BinLength;
    //         BinLength += Block.BufferSize;
    //     }
    //     std::string json_str = json.dump();
    //     JsonLength = json_str.size();
    //     FileLength = BinLength + JsonLength + 
    //                 4 + // magic
    //                 4 + // version
    //                 4 + // length
    //                 4 + // json chunk length
    //                 4 + // json chunk type ('J', 'S', 'O', 'N')
    //                 4 + // binary chunk length
    //                 4;  // binary chunk type ('B', 'I', 'N', '\0')
    //     std::ofstream out{Path.generic_string(), std::ios::binary};
    //     magic[0] = {'n'};
    //     magic[1] = {'a'};
    //     magic[2] = {'s'};
    //     magic[3] = {'t'};
    //     out.write(magic, 4);
    //     version = 1;
    //     out.write((char*)&version, 4);
    //     out.write((char*)&FileLength, 4);

    //     out.write((char*)&JsonLength, 4);
    //     JsonChunkType[0] = {'J'};
    //     JsonChunkType[1] = {'S'};
    //     JsonChunkType[2] = {'O'};
    //     JsonChunkType[3] = {'N'};
    //     out.write(JsonChunkType, 4);
    //     out.write(json_str.c_str(), json_str.size());

    //     out.write((char*)&BinLength, 4);
    //     BinChunkType[0] = {'B'};
    //     BinChunkType[1] = {'I'};
    //     BinChunkType[2] = {'N'};
    //     BinChunkType[3] = {'\0'};
    //     out.write(BinChunkType, 4);
    //     for (int i = 0; i < OutBuffers.Blocks.size(); i++)
    //     {
    //         auto &Block = OutBuffers.Blocks[i];
    //         out.write((char*)Block.Buffer.get(), Block.BufferSize);
    //     }
    // }

    // void FArchive::LoadFromPath(const std::filesystem::path &Path)
    // {
    //     std::ifstream in{Path.generic_string(), std::ios::binary};
        
    //     in.read(magic, 4);
    //     if (magic[0] != 'n' || 
    //         magic[1] != 'a' || 
    //         magic[2] != 's' || 
    //         magic[3] != 't')
    //         return;
        
    //     in.read((char*)&version, 4);
        
    //     in.read((char*)&FileLength, 4);

        
    //     in.read((char*)&JsonLength, 4);
    //     in.read(JsonChunkType, 4);
    //     std::unique_ptr<char[]> json = std::make_unique<char[]>(JsonLength+1);
    //     in.read(json.get(), JsonLength);
    //     json[JsonLength] = '\0';
    //     std::stringstream(json.get()) >> this->json;

    //     in.read((char*)&BinLength, 4);
    //     in.read(BinChunkType, 4);
    //     InBuffer = std::make_unique<unsigned char[]>(BinLength);
    //     in.read((char*)InBuffer.get(), BinLength);
        

    // }

    // void FArchiveBuffers::AddBuffer(nlohmann::json &InJson, uint32 InBufferSize, void* InBuffer)
    // {
    //     std::unique_ptr<unsigned char[]> Buffer = std::make_unique<unsigned char[]>(InBufferSize);
    //     std::memcpy(Buffer.get(), InBuffer, InBufferSize);
    //     InJson["BufferOffset"] = 0;
    //     InJson["BufferLength"] = InBufferSize;
    //     Blocks.emplace_back(InJson, InBufferSize, std::move(Buffer));
    // }

}