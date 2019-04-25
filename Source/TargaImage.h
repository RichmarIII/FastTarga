#pragma once


#include <stdint.h>
#include <memory>
#include <fstream>
#include <iostream>
#include <vector>
#include <iterator>
#include <algorithm>


#if SWIG
#define ALIGNAS(a)
#else
#define ALIGNAS(a) alignas(a)
#endif //!SWIG


#ifndef FASTTARGA_HEADERONLY

////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// 	When Set To Non 0. FastTarga Is Compiled In Header-Only Mode.  No DLL Is Produced.  When
/// 	Compiling For Lanuage Wrappers, This Must Be Defined As 0.
/// </summary>
/// <remarks> Richard Gerard Marcoux III, September 14, 2016. </remarks>
////////////////////////////////////////////////////////////////////////////////////////////////////
#define FASTTARGA_HEADERONLY 0

#endif // !FASTTARGA_HEADERONLY


//Force Minimum Struct Alignment To 1 Byte.
#pragma pack(push, 1)


////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// 	A Cross-Platform Targa Image Struct.  This Struct Implements The Targa Image File Format
/// 	Specification: http://www.dca.fee.unicamp.br/~martino/disciplinas/ea978/tgaffs.pdf.  It
/// 	Is Used To Hold All The Data That A Targa Image Can Represent.  No Parsing Is Performed.
/// 	However, Pointers Are Parsed To Point To All Relevent Information. A User Friendly Targa
/// 	Class Should Be Created To Wrap This Struct To Modify/Interpret It's Data.
/// </summary>
/// <remarks> Richard Gerard Marcoux III, September 13, 2016. </remarks>
////////////////////////////////////////////////////////////////////////////////////////////////////
struct ALIGNAS(1) STargaImage
{
	STargaImage();
	~STargaImage();
    
	STargaImage(STargaImage&& RHS);
	STargaImage& operator=(STargaImage&& RHS); 
	STargaImage(const STargaImage& RHS);
	STargaImage& operator=(const STargaImage& RHS);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>
	/// 	Loads The Specified Targa File And Returns An Instance Of The STargaImage Struct.
	/// </summary>
	/// <remarks> Richard Gerard Marcoux III, September 14, 2016. </remarks>
	/// <param name="szFilePath"> The Full FilePath Of The Targa Image You Want To Load. </param>
	/// <returns> The Fully Loaded Targa Image. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	static STargaImage Load(const char* szFilePath);

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>
	/// 	Saves The Specified Targa Image.
	/// </summary>
	/// <remarks> Richard Gerard Marcoux III, January 5, 2019. </remarks>
	/// <param name="Image"> The Targa Image You Want To Save. </param>
	/// <param name="szFilePath"> The Full FilePath Of The Targa Image You Want To Save. </param>
	/// <returns> The Fully Loaded Targa Image. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	static void Save( const STargaImage& Image, const char* szFilePath);

	struct ALIGNAS(1) SHeader
	{
		struct ALIGNAS(1) SColorMapSpecification
		{
			uint16_t FirstEntryIndex;
			uint16_t ColorMapLength;
			uint8_t ColorMapEntrySize;
		};

		struct ALIGNAS(1) SImageSpecification
		{
			uint16_t ImageXOrigin;
			uint16_t ImageYOrigin;
			uint16_t ImageWidth;
			uint16_t ImageHeight;
			uint8_t ImageDepth; //Bits Per Pixel.
			uint8_t ImageDescriptor; //Bits 0-3 Tells How Many Bits In Alpha Channel.  Bit 4 Tells If Left To Right Ordering.  Bit 5 Tells If Top To Bottom Ordering.  Bits 6-7 Must Be Zero.
		};

		uint8_t IDLength;
		uint8_t ColorMapType;
		uint8_t ImageType;
		SColorMapSpecification ColorMapSpec;
		SImageSpecification ImageSpec;
	};

	struct ALIGNAS(1) SData
	{
		uint8_t* ImageID;
		uint8_t* ColorMapData;
		uint8_t* ImageData; //Data Is Stored In BGRA Order.
	};

	struct ALIGNAS(1) SDeveloperArea
	{
		uint8_t* DeveloperData;
	};

	struct ALIGNAS(1) SDeveloperDirectory
	{
		struct ALIGNAS(1) STag
		{
			uint16_t Data;
			uint32_t Offset;
			uint32_t Size;
		};

		uint16_t NumberOfTagsInDirectory;
		STag* Tags;
	};

	struct ALIGNAS(1) SExtensionArea
	{
		struct ALIGNAS(1) STimeStamp
		{
			uint16_t Month;
			uint16_t Day;
			uint16_t Year;
			uint16_t Hour;
			uint16_t Minute;
			uint16_t Second;
		};

		struct ALIGNAS(1) SJobTime
		{
			uint16_t Hours;
			uint16_t Minutes;
			uint16_t Seconds;
		};

		struct ALIGNAS(1) SSoftwareVersion
		{
			uint16_t VersionNumber;
			uint8_t VersionLetter;
		};

		struct ALIGNAS(1) SKeyColor
		{
			uint8_t A;
			uint8_t R;
			uint8_t G;
			uint8_t B;
		};

		struct ALIGNAS(1) SPixelAspectRatio
		{
			uint16_t Width;
			uint16_t Height;
		};

		struct ALIGNAS(1) SGammaValue
		{
			uint16_t Numerator;
			uint16_t Denominator;
		};

		struct ALIGNAS(1) SColorCorrectionTable
		{
			struct ALIGNAS(1) SColor
			{
				uint16_t A;
				uint16_t R;
				uint16_t G;
				uint16_t B;
			};

			SColor Colors[256];
		};

		uint16_t ExtensionSize;
		uint8_t AuthorName[41];
		uint8_t AuthorComments[324];
		STimeStamp TimeStamp;
		uint8_t JobName[41];
		SJobTime JobTime;
		uint8_t SoftwareID[41];
		SSoftwareVersion SoftwareVersion;
		SKeyColor KeyColor;
		SPixelAspectRatio PixelAspectRatio;
		SGammaValue GammaValue;
		uint32_t ColorCorrectionOffset;
		uint32_t PostageStampOffset;
		uint8_t Padding5;
		uint8_t Padding6;
		uint8_t Padding7;
		uint32_t ScanLineOffset;
		uint8_t AttributesType;
		uint8_t* pScanLineTable;
		uint8_t* pPostageStampImage;
		SColorCorrectionTable ColorCorrectioinTable;
	};

	struct ALIGNAS(1) SFooter
	{
		uint32_t ExtensionAreaOffset;
		uint32_t DeveloperDictionaryOffset;
		uint8_t Signature[16];
		uint8_t Reserved; //Must Contain A Period '.'.
		uint8_t BinaryZeroStringTerminator;
	};

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>
	/// 	Array That Holds All Data From The Targa File.  This Is The Only Member That Actually
	/// 	Owns Any Memory.  All Other Member Pointers Are Simply Aliases/Offsets To Portions Of
	/// 	This Data.  This Allows Blazing Fast Memory Access And Keeps The Memory Footprint To The
	/// 	Absolute Minimum.  Make Sure To Allocate The Needed Memory For This Member To Hold All
	/// 	Data In The Targa File.
	/// </summary>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	std::vector<uint8_t> RawData;

	//Offset Into The pRawData Array That Points To The Header.
	SHeader* pHeader;

	//Offset Into The pRawData Array That Points To The Developer Area.
	SDeveloperArea* pDeveloperArea;

	//Offset Into The pRawData Array That Points To The Developer Directory.
	SDeveloperDirectory* pDeveloperDirectory;

	//Offset Into The pRawData Array That Points To The Image Area.
	SData ImageArea;

	//Offset Into The pRawData Array That Points To The Extension Data.
	SExtensionArea* pExtensionArea;

	//Offset Into The pRawData Array That Points To The Footer.
	SFooter* pFooter;
};


//"Inline" Implementation For Header-Only Builds 
#if FASTTARGA_HEADERONLY


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




STargaImage::STargaImage(const STargaImage& RHS) :
    RawData(RHS.RawData),
    pHeader(nullptr),
    pDeveloperArea(nullptr),
    pDeveloperDirectory(nullptr),
    ImageArea({ 0 }),
    pExtensionArea(nullptr),
    pFooter(nullptr)
{
    //Set The Header Pointer....Header Is Always The First 18 Bytes Of The File.
    this->pHeader = (SHeader*)&this->RawData[0];

    //Set The Image Data Pointer....Image Data Comes After Header.
    this->ImageArea.ImageData = (&this->RawData[0]) + sizeof(SHeader);

    //Set The Footer Pointer....Footer Is Always The Last 26 Bytes Of The File.
    this->pFooter = (SFooter*)((&this->RawData[0]) + (this->RawData.size() - 26));
    if (strncmp((const char*)this->pFooter->Signature, "TRUEVISION-XFILE", sizeof(this->pFooter->Signature)) != 0)
        this->pFooter = nullptr;//No Footer....Targa Is Version 1.
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




STargaImage& STargaImage::operator=(const STargaImage& RHS)
{
    //Copy Data From Other Image.
    this->RawData = RHS.RawData;

    //Set The Header Pointer....Header Is Always The First 18 Bytes Of The File.
    this->pHeader = (SHeader*)&this->RawData[0];

    //Set The Image Data Pointer....Image Data Comes After Header.
    this->ImageArea.ImageData = (&this->RawData[0]) + sizeof(SHeader);

    //Set The Footer Pointer....Footer Is Always The Last 26 Bytes Of The File.
    this->pFooter = (SFooter*)((&this->RawData[0]) + (this->RawData.size() - 26));
    if (strncmp((const char*)this->pFooter->Signature, "TRUEVISION-XFILE", sizeof(this->pFooter->Signature)) != 0)
        this->pFooter = nullptr;//No Footer....Targa Is Version 1.
        
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
	if (strncmp((const char*)Image.pFooter->Signature, "TRUEVISION-XFILE", sizeof(Image.pFooter->Signature)) != 0)
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


#pragma pack(pop)


#endif // FASTTARGA_HEADERONLY