#include "TargaImage.h"


//"External" Implementation For Library Builds 
#if !FASTTARGA_HEADERONLY


STargaImage::STargaImage() :
	pHeader(nullptr),
	pDeveloperArea(nullptr),
	pDeveloperDirectory(nullptr),
	ImageArea({ 0 }),
	pExtensionArea(nullptr),
	pFooter(nullptr)
{

}




STargaImage::~STargaImage()
{

}




STargaImage::STargaImage(STargaImage&& RHS) :
	RawData(std::move(RHS.RawData)),
	pHeader(std::move(RHS.pHeader)),
	pDeveloperArea(std::move(RHS.pDeveloperArea)),
	pDeveloperDirectory(std::move(RHS.pDeveloperDirectory)),
	ImageArea(std::move(RHS.ImageArea)),
	pExtensionArea(std::move(RHS.pExtensionArea)),
	pFooter(std::move(RHS.pFooter))
{
	RHS.RawData.clear();
	RHS.pHeader = nullptr;
	RHS.pDeveloperArea = nullptr;
	RHS.pDeveloperDirectory = nullptr;
	RHS.pExtensionArea = nullptr;
	RHS.ImageArea = { 0 };
}




STargaImage& STargaImage::operator=(STargaImage&& RHS)
{
	RawData = std::move(RHS.RawData);
	pHeader = std::move(RHS.pHeader);
	pDeveloperArea = std::move(RHS.pDeveloperArea);
	pDeveloperDirectory = std::move(RHS.pDeveloperDirectory);
	ImageArea = std::move(RHS.ImageArea);
	pExtensionArea = std::move(RHS.pExtensionArea);
	pFooter = std::move(RHS.pFooter);

	RHS.RawData.clear();
	RHS.pHeader = nullptr;
	RHS.pDeveloperArea = nullptr;
	RHS.pDeveloperDirectory = nullptr;
	RHS.pExtensionArea = nullptr;
	RHS.ImageArea = { 0 };

	return *this;
}




/*static*/ STargaImage STargaImage::Load(const char* FilePath)
{
	STargaImage Image;

	//Read In Entire Targa File.
	Image.RawData = std::vector<uint8_t>(std::istreambuf_iterator<uint8_t>(std::basic_ifstream<uint8_t>(FilePath, std::ios_base::binary)), std::istreambuf_iterator<uint8_t>());

	//Set The Header Pointer....Header Is Always The First 18 Bytes Of The File.
	Image.pHeader = (SHeader*)&Image.RawData[0];

	//We Don't Support ColorMapped, Black And White, Or Compressed Images....Yet.
	if (Image.pHeader->ImageType != 2 /*UnCompressed True Color*/)
		return STargaImage();

	//Set The Image Data Pointer....Image Data Comes After Pointer
	Image.ImageArea.ImageData = (&Image.RawData[0]) + sizeof(SHeader);

	//Set The Footer Pointer....Footer Is Always The Last 26 Bytes Of The File.
	Image.pFooter = (SFooter*)((&Image.RawData[0]) + (Image.RawData.size() - 26));
	if (strcmp((const char*)Image.pFooter->Signature, "TRUEVISION-XFILE") != 0)
		Image.pFooter = nullptr;//No Footer....Targa Is Version 1.

	return std::move(Image);
}


#endif // FASTTARGA_HEADERONLY