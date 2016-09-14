#pragma once


#include <stdint.h>
#include <memory>



#ifndef FASTTARGA_HEADERONLY

////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// 	When Set To Non 0. FastTarga Is Compiled In Header-Only Mode.  No DLL Is Produced.  When
/// 	Compiling For Lanuage Wrappers, This Must Be Defined As 0.
/// </summary>
/// <remarks> Richard Gerard Marcoux III, September 14, 2016. </remarks>
////////////////////////////////////////////////////////////////////////////////////////////////////
#define FASTTARGA_HEADERONLY 1

#endif // !1





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
struct STargaImage
{

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>
	/// 	Loads The Specified Targa File And Returns An Instance Of The STargaImage Struct.
	/// </summary>
	/// <remarks> Richard Gerard Marcoux III, September 14, 2016. </remarks>
	/// <param name="FilePath"> The Full FilePath Of The Targa Image You Want To Load. </param>
	/// <returns> The Fully Loaded Targa Image. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	static STargaImage Load( const char* FilePath );

	struct SHeader
	{
		struct SColorMapSpecification
		{
			uint16_t FirstEntryIndex;
			uint16_t ColorMapLength;
			uint8_t ColorMapEntrySize;
		};

		struct SImageSpecification
		{
			uint16_t ImageXOrigin;
			uint16_t ImageYOrigin;
			uint16_t ImageWidth;
			uint16_t ImageHeight;
			uint8_t ImageDepth;
			uint8_t ImageDescriptor; //Bits 0-3 Tells How Many Bits In Alpha Channel.  Bit 4 Tells If Left To Right Ordering.  Bit 5 Tells If Top To Bottom Ordering.  Bits 6-7 Must Be Zero.
		};

		uint8_t IDLength;
		uint8_t ColorMapType;
		uint8_t ImageType;
		SColorMapSpecification ColorMapSpec;
		SImageSpecification ImageSpec;
	};

	struct SData
	{
		uint8_t* ImageID;
		uint8_t* ColorMapData;
		uint8_t* ImageData;
	};

	struct SDeveloperArea
	{
		uint8_t* DeveloperData;
	};

	struct SDeveloperDirectory
	{
		struct STag
		{
			uint16_t Data;
			uint32_t Offset;
			uint32_t Size;
		};

		uint16_t NumberOfTagsInDirectory;
		STag* Tags;
	};

	struct SExtensionArea
	{
		struct STimeStamp
		{
			uint16_t Month;
			uint16_t Day;
			uint16_t Year;
			uint16_t Hour;
			uint16_t Minute;
			uint16_t Second;
		};

		struct SJobTime
		{
			uint16_t Hours;
			uint16_t Minutes;
			uint16_t Seconds;
		};

		struct SSoftwareVersion
		{
			uint16_t VersionNumber;
			char VersionLetter;
		};

		struct SKeyColor
		{
			uint8_t A;
			uint8_t R;
			uint8_t G;
			uint8_t B;
		};

		struct SPixelAspectRatio
		{
			uint16_t Width;
			uint16_t Height;
		};

		struct SGammaValue
		{
			uint16_t Numerator;
			uint16_t Denominator;
		};

		struct SColorCorrectionTable
		{
			struct SColor
			{
				uint16_t A;
				uint16_t R;
				uint16_t G;
				uint16_t B;
			};

			SColor Colors[ 256 ];
		};

		uint16_t ExtensionSize;
		char AuthorName[ 41 ];
		char AuthorComments[ 324 ];
		STimeStamp TimeStamp;
		char JobName[ 41 ];
		SJobTime JobTime;
		char SoftwareID[ 41 ];
		SSoftwareVersion SoftwareVersion;
		SKeyColor KeyColor;
		SPixelAspectRatio PixelAspectRatio;
		SGammaValue GammaValue;
		uint32_t ColorCorrectionOffset;
		uint32_t PostageStampOffset;
		uint32_t ScanLineOffset;
		uint8_t AttributesType;
		uint8_t* pScanLineTable;
		uint8_t* pPostageStampImage;
		SColorCorrectionTable ColorCorrectioinTable;
	};

	struct SFooter
	{
		uint32_t ExtensionAreaOffset;
		uint32_t DeveloperDictionaryOffset;
		char Signature[ 16 ];
		char Reserved; //Must Contain A Period '.'.
		uint8_t BinaryZeroStringTerminator;
	};

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>
	/// 	Dynamically Allocated Array That Holds All Data From The Targa File.  This Is The Only
	/// 	Member That Actually Owns Any Memory.  All Other Member Pointers Are Simply
	/// 	Aliases/Offsets To Portions Of This Data.  This Allows Blazing Fast Memory Access And
	/// 	Keeps The Memory Footprint To The Absolute Minimum.  Make Sure To Allocate The Needed
	/// 	Memory For This Member To Hold All Data In The Targa File.
	/// </summary>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	std::unique_ptr<uint8_t> RawData;
	
	//Offset Into The pRawData Array That Points To The Header.
	SHeader* pHeader;

	//Offset Into The pRawData Array That Points To The Developer Area.
	SDeveloperArea* pDeveloperArea;

	//Offset Into The pRawData Array That Points To The Developer Directory.
	SDeveloperDirectory* pDeveloperDirectory;

	//Offset Into The pRawData Array That Points To The Image Area.
	SData* pImageArea;

	//Offset Into The pRawData Array That Points To The Extension Data.
	SExtensionArea* pExtensionArea;

	//Offset Into The pRawData Array That Points To The Footer.
	SFooter* pFooter;
};


//"Inline" Implementation For Header-Only Builds 
#if FASTTARGA_HEADERONLY


/*static*/ STargaImage STargaImage::Load( const char* FilePath )
{

}


#endif // FASTTARGA_HEADERONLY