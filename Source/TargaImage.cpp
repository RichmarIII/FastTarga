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




/*static*/ STargaImage STargaImage::Load(const char* szFilePath)
{
	STargaImage Image;

	//Only Keep File Stream Open For A Limited Time.
	{
		//Open A File Stream.
		std::ifstream FileStream(szFilePath, std::ios::binary);

		//Seek To End Of File.
		FileStream.seekg(0, std::ios::end);

		//Pre-Allocate Buffer Data From Size Of File (Performance).
		Image.RawData.resize(FileStream.tellg());

		//Seek Back To Beginning So That We Can Read The File.
		FileStream.seekg(0);

		//Read In Entire Targa File.
		FileStream.read(reinterpret_cast<char*>(Image.RawData.data()), Image.RawData.size());
	}

	//Set The Header Pointer....Header Is Always The First 18 Bytes Of The File.
	Image.pHeader = (SHeader*)&Image.RawData[0];

	//We Don't Support ColorMapped, Black And White, Or Compressed Images....Yet.
	if (Image.pHeader->ImageType != 2 /*UnCompressed True Color*/)
		return STargaImage();

	//Set The Image Data Pointer....Image Data Comes After Header.
	Image.ImageArea.ImageData = (&Image.RawData[0]) + sizeof(SHeader);

	//Set The Footer Pointer....Footer Is Always The Last 26 Bytes Of The File.
	Image.pFooter = (SFooter*)((&Image.RawData[0]) + (Image.RawData.size() - 26));
	if (strcmp((const char*)Image.pFooter->Signature, "TRUEVISION-XFILE") != 0)
		Image.pFooter = nullptr;//No Footer....Targa Is Version 1.

	return std::move(Image);
}




/*static*/ void STargaImage::Save(const STargaImage& Image, const char* szFilePath)
{
	//We Don't Support ColorMapped, Black And White, Or Compressed Images....Yet.
	if (Image.pHeader->ImageType != 2)
		return;

	//Open A File Stream.
	std::ofstream FileStream(szFilePath, std::ios::binary);

	//Write Header.
	FileStream.write((char*)Image.pHeader, sizeof(SHeader));

	//Write Pixel Data.
	FileStream.write((char*)Image.ImageArea.ImageData, (Image.pHeader->ImageSpec.ImageDepth / 8) * Image.pHeader->ImageSpec.ImageWidth * Image.pHeader->ImageSpec.ImageHeight);

	//Write Footer.
	if (Image.pFooter)
		FileStream.write((char*)Image.pFooter, sizeof(SFooter));
	else
	{
		//No Footer. Create One.
		SFooter Footer;
		Footer.BinaryZeroStringTerminator = 0;
		Footer.DeveloperDictionaryOffset = 0;
		Footer.ExtensionAreaOffset = 0;
		Footer.Reserved = '.';
		std::memcpy(Footer.Signature, "TRUEVISION-XFILE", sizeof(Footer.Signature));

		FileStream.write((char*)&Footer, sizeof(SFooter));
	}
}


#endif // FASTTARGA_HEADERONLY