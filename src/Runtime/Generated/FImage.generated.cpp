#include "D:/Nilou/src/Runtime/Framework/Common/Actor/ArrowActor.h"
#include <UDRefl/UDRefl.hpp>

using namespace Ubpa;
using namespace Ubpa::UDRefl;

std::unique_ptr<NClass> nilou::FImage::StaticClass_ = nullptr;
const NClass *nilou::FImage::GetClass() const 
{ 
    return nilou::FImage::StaticClass(); 
}
const NClass *nilou::FImage::StaticClass()
{
    return nilou::FImage::StaticClass_.get();
}

template<>
struct TClassRegistry<nilou::FImage>
{
    TClassRegistry(const std::string& InName)
    {
        nilou::FImage::StaticClass_ = std::make_unique<NClass>();
        Mngr.RegisterType<nilou::FImage>();
		Mngr.AddField<&nilou::FImage::Channel>("Channel");
		Mngr.AddField<&nilou::FImage::Data>("Data");
		Mngr.AddField<&nilou::FImage::Depth>("Depth");
		Mngr.AddField<&nilou::FImage::Height>("Height");
		Mngr.AddField<&nilou::FImage::ImageType>("ImageType");
		Mngr.AddField<&nilou::FImage::NumMips>("NumMips");
		Mngr.AddField<&nilou::FImage::PixelFormat>("PixelFormat");
		Mngr.AddField<&nilou::FImage::Width>("Width");
		Mngr.AddMethod<&nilou::FImage::Deserialize>("Deserialize");
		Mngr.AddMethod<&nilou::FImage::Serialize>("Serialize");
;
        nilou::FImage::StaticClass_->Type = Type_of<nilou::FImage>;
        nilou::FImage::StaticClass_->TypeInfo = Mngr.GetTypeInfo(Type_of<nilou::FImage>);
    }

    static TClassRegistry<nilou::FImage> Dummy;
};
TClassRegistry<nilou::FImage> Dummy = TClassRegistry<nilou::FImage>("nilou::FImage");



void nilou::FImage::Serialize(FArchive& Ar)
{
    
    nlohmann::json &content = Ar.Node;

    {
        FArchive local_Ar(content["Channel"], Ar);
        TStaticSerializer<decltype(this->Channel)>::Serialize(this->Channel, local_Ar);
    }
    {
        FArchive local_Ar(content["Data"], Ar);
        TStaticSerializer<decltype(this->Data)>::Serialize(this->Data, local_Ar);
    }
    {
        FArchive local_Ar(content["Depth"], Ar);
        TStaticSerializer<decltype(this->Depth)>::Serialize(this->Depth, local_Ar);
    }
    {
        FArchive local_Ar(content["Height"], Ar);
        TStaticSerializer<decltype(this->Height)>::Serialize(this->Height, local_Ar);
    }
    {
        FArchive local_Ar(content["ImageType"], Ar);
        TStaticSerializer<decltype(this->ImageType)>::Serialize(this->ImageType, local_Ar);
    }
    {
        FArchive local_Ar(content["NumMips"], Ar);
        TStaticSerializer<decltype(this->NumMips)>::Serialize(this->NumMips, local_Ar);
    }
    {
        FArchive local_Ar(content["PixelFormat"], Ar);
        TStaticSerializer<decltype(this->PixelFormat)>::Serialize(this->PixelFormat, local_Ar);
    }
    {
        FArchive local_Ar(content["Width"], Ar);
        TStaticSerializer<decltype(this->Width)>::Serialize(this->Width, local_Ar);
    }
}

void nilou::FImage::Deserialize(FArchive& Ar)
{
    nlohmann::json &content = Ar.Node;

    if (content.contains("Channel"))
    {
        FArchive local_Ar(content["Channel"], Ar);
        TStaticSerializer<decltype(this->Channel)>::Deserialize(this->Channel, local_Ar);
    }
    if (content.contains("Data"))
    {
        FArchive local_Ar(content["Data"], Ar);
        TStaticSerializer<decltype(this->Data)>::Deserialize(this->Data, local_Ar);
    }
    if (content.contains("Depth"))
    {
        FArchive local_Ar(content["Depth"], Ar);
        TStaticSerializer<decltype(this->Depth)>::Deserialize(this->Depth, local_Ar);
    }
    if (content.contains("Height"))
    {
        FArchive local_Ar(content["Height"], Ar);
        TStaticSerializer<decltype(this->Height)>::Deserialize(this->Height, local_Ar);
    }
    if (content.contains("ImageType"))
    {
        FArchive local_Ar(content["ImageType"], Ar);
        TStaticSerializer<decltype(this->ImageType)>::Deserialize(this->ImageType, local_Ar);
    }
    if (content.contains("NumMips"))
    {
        FArchive local_Ar(content["NumMips"], Ar);
        TStaticSerializer<decltype(this->NumMips)>::Deserialize(this->NumMips, local_Ar);
    }
    if (content.contains("PixelFormat"))
    {
        FArchive local_Ar(content["PixelFormat"], Ar);
        TStaticSerializer<decltype(this->PixelFormat)>::Deserialize(this->PixelFormat, local_Ar);
    }
    if (content.contains("Width"))
    {
        FArchive local_Ar(content["Width"], Ar);
        TStaticSerializer<decltype(this->Width)>::Deserialize(this->Width, local_Ar);
    }
    
}
