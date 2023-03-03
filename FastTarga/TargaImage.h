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
/// 	Compiling For Language Wrappers (SWIG), This Must Be Defined As 0.
/// </summary>
/// <remarks> Richard Gerard Marcoux III, September 14, 2016. </remarks>
////////////////////////////////////////////////////////////////////////////////////////////////////
#define FASTTARGA_HEADERONLY 0

#endif // !FASTTARGA_HEADERONLY


//Force Minimum Struct Alignment To 1 Byte.
#pragma pack(push, 1)

//If We Are Compiling For A Language Wrapper (SWIG), We Must Define The Export Macro As Empty.
#if FASTTARGA_HEADERONLY || SWIG
#define FASTTARGA_API
#else
//Otherwise, We Are Compiling For A DLL, So We Must Define The Export Macro As __declspec(dllexport).
#if defined(FASTTARGA_EXPORTS)
//We Are Compiling The DLL, So We Must Define The Export Macro As __declspec(dllexport).
#define FASTTARGA_API __declspec(dllexport)
#else
//We Are Compiling A Client, So We Must Define The Export Macro As __declspec(dllimport).
#define FASTTARGA_API __declspec(dllimport)
#endif //FASTTARGA_EXPORTS
#endif //FASTTARGA_HEADERONLY || SWIG

/// <summary>
///		The Targa Image Type Enum. This Enum Is Used To Determine The Type Of Targa Image.
/// </summary>
/// <remarks>
///		 Richard Gerard Marcoux III, February 28, 2023.
/// </remarks>
/// <note> uint8_t Is Used To Ensure The Size Of The Enum Is 1 Byte. </note>
enum class FASTTARGA_API EImageType : uint8_t
{
	// No Image Data Is Present
	NoImageData = 0,

	// Uncompressed Color Mapped Image
	ColorMapped = 1,

	// Uncompressed True Color Image
	TrueColor = 2,

	// Uncompressed Black And White Image (Grayscale)
	BlackAndWhite = 3,

	// Run Length Encoded Color Mapped Image
	RLEColorMapped = 9,

	// Run Length Encoded True Color Image
	RLETrueColor = 10,

	// Run Length Encoded Black And White Image (Grayscale)
	RLEBlackAndWhite = 11
};

////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary>
/// 	A Cross-Platform Targa Image Struct.  This Struct Implements The Targa Image File Format
/// 	Specification: http://www.dca.fee.unicamp.br/~martino/disciplinas/ea978/tgaffs.pdf.  It
/// 	Is Used To Hold All The Data That A Targa Image Can Represent.  No Parsing Is Performed.
/// 	However, Pointers Are Parsed To Point To All Relevant Information. A User Friendly Targa
/// 	Class Should Be Created To Wrap This Struct To Modify/Interpret It's Data.
/// </summary>
/// <remarks> Richard Gerard Marcoux III, September 13, 2016. </remarks>
////////////////////////////////////////////////////////////////////////////////////////////////////
struct FASTTARGA_API ALIGNAS( 1 ) STargaImage
{
	STargaImage() = default;
	~STargaImage() = default;

	STargaImage( STargaImage && RHS ) noexcept;
	STargaImage& operator=( STargaImage && RHS ) noexcept;
	STargaImage( const STargaImage & RHS ) noexcept;
	STargaImage& operator=( const STargaImage & RHS ) noexcept;

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>
	/// 	Loads The Specified Targa File And Returns An Instance Of The STargaImage Struct.
	/// </summary>
	/// <remarks> Richard Gerard Marcoux III, September 14, 2016. </remarks>
	/// <param name="szFilePath"> The Full FilePath Of The Targa Image You Want To Load. </param>
	/// <param name="bDecompress"> If True, The Image Will Be Decompressed. </param>
	/// <returns> The Fully Loaded Targa Image. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	static STargaImage Load( const char* szFilePath, bool bDecompress = true );

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>
	/// 	Saves The Specified Targa Image.
	/// </summary>
	/// <remarks> Richard Gerard Marcoux III, January 5, 2019. </remarks>
	/// <param name="Image"> The Targa Image You Want To Save. </param>
	/// <param name="szFilePath"> The Full FilePath Of The Targa Image You Want To Save. </param>
	/// <returns> The Fully Loaded Targa Image. </returns>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	static void Save( const STargaImage & Image, const char* szFilePath );

	////////////////////////////////////////////////////////////////////////////////////////////////////
	/// <summary>
	/// 	Decompresses The Image.  This Is Only Necessary If The Targa Image Is Compressed.
	/// </summary>
	/// <remarks> Richard Gerard Marcoux III, February 28, 2023. </remarks>
	////////////////////////////////////////////////////////////////////////////////////////////////////
	void Decompress();

	struct FASTTARGA_API ALIGNAS( 1 ) SHeader
	{
		struct ALIGNAS( 1 ) SColorMapSpecification
		{
			uint16_t FirstEntryIndex;
			uint16_t ColorMapLength;
			uint8_t ColorMapEntrySize;
		};

		struct ALIGNAS( 1 ) SImageSpecification
		{
			uint16_t ImageXOrigin;
			uint16_t ImageYOrigin;
			uint16_t ImageWidth;
			uint16_t ImageHeight;
			uint8_t ImageDepth; //Bits Per Pixel.
			uint8_t ImageDescriptor; //Bits 0-3 Tells How Many Bits In Alpha Channel.  Bit 4 Tells If Left To Right Ordering.  Bit 5 Tells If Top To Bottom Ordering.  Bits 6-7 Must Be Zero.
		};

		uint8_t IDLength;
		uint8_t ColorMapType; // 0 = No Color Map Data, 1 = Color Map Data Is Present.
		EImageType ImageType;
		SColorMapSpecification ColorMapSpec;
		SImageSpecification ImageSpec;
	};

	struct FASTTARGA_API ALIGNAS( 1 ) SData
	{
		uint8_t* ImageID;
		uint8_t* ColorMapData;
		uint8_t* ImageData; //Data Is Stored In BGRA Order.
	};

	struct FASTTARGA_API ALIGNAS( 1 ) SDeveloperArea
	{
		uint8_t* DeveloperData;
	};

	struct FASTTARGA_API ALIGNAS( 1 ) SDeveloperDirectory
	{
		struct FASTTARGA_API ALIGNAS( 1 ) STag
		{
			uint16_t Tag;
			uint32_t Offset;
			uint32_t Size;
		};

		uint16_t NumberOfTagsInDirectory;
		STag* Tags;
	};

	struct FASTTARGA_API ALIGNAS( 1 ) SExtensionArea
	{
		struct FASTTARGA_API ALIGNAS( 1 ) STimeStamp
		{
			uint16_t Month;
			uint16_t Day;
			uint16_t Year;
			uint16_t Hour;
			uint16_t Minute;
			uint16_t Second;
		};

		struct FASTTARGA_API ALIGNAS( 1 ) SJobTime
		{
			uint16_t Hours;
			uint16_t Minutes;
			uint16_t Seconds;
		};

		struct FASTTARGA_API ALIGNAS( 1 ) SSoftwareVersion
		{
			uint16_t VersionNumber;
			uint8_t VersionLetter;
		};

		struct FASTTARGA_API ALIGNAS( 1 ) SKeyColor
		{
			uint8_t A;
			uint8_t R;
			uint8_t G;
			uint8_t B;
		};

		struct FASTTARGA_API ALIGNAS( 1 ) SPixelAspectRatio
		{
			uint16_t Width;
			uint16_t Height;
		};

		struct FASTTARGA_API ALIGNAS( 1 ) SGammaValue
		{
			uint16_t Numerator;
			uint16_t Denominator;
		};

		struct FASTTARGA_API ALIGNAS( 1 ) SColorCorrectionTable
		{
			struct FASTTARGA_API ALIGNAS( 1 ) SColor
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
		uint32_t* pScanLineTable;
		uint8_t* pPostageStampImage;
		SColorCorrectionTable* pColorCorrectionTable;
	};

	struct FASTTARGA_API ALIGNAS( 1 ) SFooter
	{
		uint32_t ExtensionAreaOffset;
		uint32_t DeveloperDirectoryOffset;
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
	SDeveloperArea DeveloperArea;

	//Offset Into The pRawData Array That Points To The Developer Directory.
	SDeveloperDirectory* pDeveloperDirectory;

	//Offset Into The pRawData Array That Points To The Image Area.
	SData ImageArea;

	//Offset Into The pRawData Array That Points To The Extension Data.
	SExtensionArea* pExtensionArea;

	//Offset Into The pRawData Array That Points To The Footer.
	SFooter* pFooter;

private:

	/// <summary>
	/// 	Parses The Raw Data Into The Various Structures. This Needs To Be Done After Reading The Raw Data From The File.
	/// </summary>
	/// <remarks>
	/// 	This Is A Helper Function That Is Called Automatically By The Load() And Function. It Exists To Make Code More Readable.
	void ParseRawData();
	
	/// <summary>
	///		Converts The Compressed Raw Data To Decompressed Raw Data. This Needs To Be Done After Decompressing The Image Data.
	///		RawData Is Overwritten
	/// </summary>
	/// <param name="DecompImageDataa">The Decompressed Image Data.</param>
	/// <remarks>
	///		This Is A Helper Function That Is Called Automatically By The Decompress() Function. It Exists To Make Code More Readable.
	///	</remarks>
	void ConvertCompRawDataToDecompRawData( const std::vector<uint8_t>& DecompImageData );

	/// <summary>
	///		 Decompresses The Image Data.
	/// </summary>
	/// <remarks>
	///		This Is A Helper Function That Is Called Automatically By The Decompress() Function. It Exists To Make Code More Readable.
	/// </remarks>
	void DecompressTrueColorRLE();

	/// <summary>
	///		 Decompresses The Image Data.
	/// </summary>
	/// <remarks>
	///		This Is A Helper Function That Is Called Automatically By The Decompress() Function. It Exists To Make Code More Readable.
	/// </remarks>
	void DecompressBlackAndWhiteRLE();

	/// <summary>
	///		 Decompresses The Image Data.
	/// </summary>
	/// <remarks>
	///		This Is A Helper Function That Is Called Automatically By The Decompress() Function. It Exists To Make Code More Readable.
	/// </remarks>
	void DecompressColormappedRLE();
};


//"Inline" Implementation For Header-Only Builds 
#if FASTTARGA_HEADERONLY

#include "TargaImageImpl.h"

#endif // FASTTARGA_HEADERONLY

#pragma pack(pop)