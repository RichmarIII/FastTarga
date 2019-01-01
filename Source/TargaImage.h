#pragma once


#include <stdint.h>
#include <memory>
#include <fstream>
#include <vector>



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
struct alignas( 1 ) STargaImage
{
	STargaImage();
	~STargaImage();
	STargaImage( STargaImage&& RHS );
	STargaImage& operator=( STargaImage&& RHS );

	STargaImage( STargaImage& RHS ) = delete;
	STargaImage& operator=( const STargaImage& RHS ) = delete;



	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>
	/// 	Loads The Specified Targa File And Returns An Instance Of The STargaImage Struct.
	/// </summary>
	/// <remarks> Richard Gerard Marcoux III, September 14, 2016. </remarks>
	/// <param name="FilePath"> The Full FilePath Of The Targa Image You Want To Load. </param>
	/// <returns> The Fully Loaded Targa Image. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	static STargaImage Load( const char* FilePath );

	struct alignas( 1 ) SHeader
	{
		struct alignas( 1 ) SColorMapSpecification
		{
			uint16_t FirstEntryIndex;
			uint16_t ColorMapLength;
			uint8_t ColorMapEntrySize;
		};

		struct alignas( 1 ) SImageSpecification
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

	struct alignas( 1 ) SData
	{
		uint8_t* ImageID;
		uint8_t* ColorMapData;
		uint8_t* ImageData; //Data Is Stored In BGRA Order.
	};

	struct alignas( 1 ) SDeveloperArea
	{
		uint8_t* DeveloperData;
	};

	struct alignas( 1 ) SDeveloperDirectory
	{
		struct alignas( 1 ) STag
		{
			uint16_t Data;
			uint32_t Offset;
			uint32_t Size;
		};

		uint16_t NumberOfTagsInDirectory;
		STag* Tags;
	};

	struct alignas( 1 ) SExtensionArea
	{
		struct alignas( 1 ) STimeStamp
		{
			uint16_t Month;
			uint16_t Day;
			uint16_t Year;
			uint16_t Hour;
			uint16_t Minute;
			uint16_t Second;
		};

		struct alignas( 1 ) SJobTime
		{
			uint16_t Hours;
			uint16_t Minutes;
			uint16_t Seconds;
		};

		struct alignas( 1 ) SSoftwareVersion
		{
			uint16_t VersionNumber;
			uint8_t VersionLetter;
		};

		struct alignas( 1 ) SKeyColor
		{
			uint8_t A;
			uint8_t R;
			uint8_t G;
			uint8_t B;
		};

		struct alignas( 1 ) SPixelAspectRatio
		{
			uint16_t Width;
			uint16_t Height;
		};

		struct alignas( 1 ) SGammaValue
		{
			uint16_t Numerator;
			uint16_t Denominator;
		};

		struct alignas( 1 ) SColorCorrectionTable
		{
			struct alignas( 1 ) SColor
			{
				uint16_t A;
				uint16_t R;
				uint16_t G;
				uint16_t B;
			};

			SColor Colors[ 256 ];
		};

		uint16_t ExtensionSize;
		uint8_t AuthorName[ 41 ];
		uint8_t AuthorComments[ 324 ];
		STimeStamp TimeStamp;
		uint8_t JobName[ 41 ];
		SJobTime JobTime;
		uint8_t SoftwareID[ 41 ];
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

	struct alignas( 1 ) SFooter
	{
		uint32_t ExtensionAreaOffset;
		uint32_t DeveloperDictionaryOffset;
		uint8_t Signature[ 16 ];
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
	pHeader( nullptr ),
	pDeveloperArea( nullptr ),
	pDeveloperDirectory( nullptr ),
	ImageArea( { 0 } ),
	pExtensionArea( nullptr ),
	pFooter( nullptr )
{

}




STargaImage::~STargaImage()
{

}




STargaImage::STargaImage( STargaImage&& RHS ) :
	RawData( std::move( RHS.RawData ) ),
	pHeader( std::move( RHS.pHeader ) ),
	pDeveloperArea( std::move( RHS.pDeveloperArea ) ),
	pDeveloperDirectory( std::move( RHS.pDeveloperDirectory ) ),
	ImageArea( std::move( RHS.ImageArea ) ),
	pExtensionArea( std::move( RHS.pExtensionArea ) ),
	pFooter( std::move( RHS.pFooter ) )
{
	RHS.RawData.clear();
	RHS.pHeader = nullptr;
	RHS.pDeveloperArea = nullptr;
	RHS.pDeveloperDirectory = nullptr;
	RHS.pExtensionArea = nullptr;
	RHS.ImageArea = { 0 };
}




STargaImage& STargaImage::operator=( STargaImage&& RHS )
{
	RawData = std::move( RHS.RawData );
	pHeader = std::move( RHS.pHeader );
	pDeveloperArea = std::move( RHS.pDeveloperArea );
	pDeveloperDirectory = std::move( RHS.pDeveloperDirectory );
	ImageArea = std::move( RHS.ImageArea );
	pExtensionArea = std::move( RHS.pExtensionArea );
	pFooter = std::move( RHS.pFooter );

	RHS.RawData.clear();
	RHS.pHeader = nullptr;
	RHS.pDeveloperArea = nullptr;
	RHS.pDeveloperDirectory = nullptr;
	RHS.pExtensionArea = nullptr;
	RHS.ImageArea = { 0 };

	return *this;
}




/*static*/ STargaImage STargaImage::Load( const char* FilePath )
{
	STargaImage Image;

	//Read In Entire Targa File.
	Image.RawData = std::vector<uint8_t>( std::istreambuf_iterator<uint8_t>( std::basic_ifstream<uint8_t>( FilePath, std::ios_base::binary ) ), std::istreambuf_iterator<uint8_t>() );

	//Set The Header Pointer....Header Is Always The First 18 Bytes Of The File.
	Image.pHeader = ( SHeader* ) &Image.RawData[ 0 ];

	//We Don't Support ColorMapped, Black And White, Or Compressed Images....Yet.
	if( Image.pHeader->ImageType != 2 /*UnCompressed True Color*/ )
		return STargaImage();

	//Set The Image Data Pointer....Image Data Comes After Pointer
	Image.ImageArea.ImageData = ( &Image.RawData[ 0 ] ) + sizeof( SHeader );

	//Set The Footer Pointer....Footer Is Always The Last 26 Bytes Of The File.
	Image.pFooter = ( SFooter* ) ( ( &Image.RawData[ 0 ] ) + ( Image.RawData.size() - 26 ) );
	if( strcmp( ( const char* ) Image.pFooter->Signature, "TRUEVISION-XFILE" ) != 0 )
		Image.pFooter = nullptr;//No Footer....Targa Is Version 1.

	return std::move( Image );
}


#endif // FASTTARGA_HEADERONLY