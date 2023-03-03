#pragma once

STargaImage::STargaImage( STargaImage&& RHS ) noexcept:
	RawData( std::move( RHS.RawData ) ),
	pHeader( std::move( RHS.pHeader ) ),
	DeveloperArea( std::move( RHS.DeveloperArea ) ),
	pDeveloperDirectory( std::move( RHS.pDeveloperDirectory ) ),
	ImageArea( std::move( RHS.ImageArea ) ),
	pExtensionArea( std::move( RHS.pExtensionArea ) ),
	pFooter( std::move( RHS.pFooter ) )
{
	RHS.RawData.clear();
	RHS.pHeader = nullptr;
	RHS.DeveloperArea = {};
	RHS.pDeveloperDirectory = nullptr;
	RHS.pExtensionArea = nullptr;
	RHS.ImageArea = { };
	RHS.pFooter = nullptr;
}

STargaImage::STargaImage( const STargaImage& RHS ) noexcept:
	RawData( RHS.RawData ),
	pHeader( nullptr ),
	DeveloperArea( {} ),
	pDeveloperDirectory( nullptr ),
	ImageArea( { } ),
	pExtensionArea( nullptr ),
	pFooter( nullptr )
{
	// Parse The Raw Data Into The Various Structures.
	ParseRawData();
}

STargaImage& STargaImage::operator=( STargaImage&& RHS ) noexcept
{
	RawData = std::move( RHS.RawData );
	pHeader = std::move( RHS.pHeader );
	DeveloperArea = std::move( RHS.DeveloperArea );
	pDeveloperDirectory = std::move( RHS.pDeveloperDirectory );
	ImageArea = std::move( RHS.ImageArea );
	pExtensionArea = std::move( RHS.pExtensionArea );
	pFooter = std::move( RHS.pFooter );

	RHS.RawData.clear();
	RHS.pHeader = nullptr;
	RHS.DeveloperArea = {};
	RHS.pDeveloperDirectory = nullptr;
	RHS.pExtensionArea = nullptr;
	RHS.ImageArea = { 0 };
	RHS.pFooter = nullptr;

	return *this;
}

STargaImage& STargaImage::operator=( const STargaImage& RHS ) noexcept
{
	return *this = STargaImage( RHS );
}

/*static*/ STargaImage STargaImage::Load( const char* szFilePath, bool bDecompress )
{
	STargaImage Image{};

	//Only Keep File Stream Open For A Limited Time.
	{
		//Open A File Stream.
		std::ifstream FileStream( szFilePath, std::ios::binary );

		//Seek To End Of File.
		FileStream.seekg( 0, std::ios::end );

		//Pre-Allocate Buffer Data From Size Of File (Performance).
		Image.RawData.resize( FileStream.tellg() );

		//Seek Back To Beginning So That We Can Read The File.
		FileStream.seekg( 0 );

		//Read In Entire Targa File.
		FileStream.read( reinterpret_cast<char*>( Image.RawData.data() ), Image.RawData.size() );
	}

	// Parse The Raw Data Into The Various Structures.
	Image.ParseRawData();

	if( bDecompress )
		Image.Decompress();

	return Image;
}

/*static*/ void STargaImage::Save( const STargaImage& Image, const char* szFilePath )
{
	//Open A File Stream.
	std::ofstream FileStream( szFilePath, std::ios::binary );

	//Write File.
	FileStream.write( reinterpret_cast<const char*>(Image.RawData.data()), Image.RawData.size() );

	//Write Footer.
	if( !Image.pFooter )
	{
		//No Footer. Create One.
		SFooter Footer{};
		Footer.BinaryZeroStringTerminator = 0;
		Footer.DeveloperDirectoryOffset = 0;
		Footer.ExtensionAreaOffset = 0;
		Footer.Reserved = '.';
		std::memcpy( Footer.Signature, "TRUEVISION-XFILE", sizeof( Footer.Signature ) );

		FileStream.write( (char*)&Footer, sizeof( SFooter ) );
	}
}

void STargaImage::Decompress()
{
	//Decompress The Image Data. (If It Is Compressed) (RLE Only) (True Color Only)
	if( this->pHeader->ImageType == EImageType::RLETrueColor )
		DecompressTrueColorRLE();

	//Decompress The Image Data. (If It Is Compressed) (RLE Only) (Black And White Only)
	else if( this->pHeader->ImageType == EImageType::RLEBlackAndWhite )
		DecompressBlackAndWhiteRLE();

	//Decompress The Image Data. (If It Is Compressed) (RLE Only) (Color Mapped Only)
	else if( this->pHeader->ImageType == EImageType::RLEColorMapped )
		DecompressColormappedRLE();
}

void STargaImage::ParseRawData()
{
	//Set The Header Pointer....Header Is Always The First 18 Bytes Of The File.
	this->pHeader = (SHeader*)&this->RawData[0];

	//Set The Image ID Pointer....Image ID Is Optional.
	this->ImageArea.ImageID = this->pHeader->IDLength ? (uint8_t*)&this->RawData[sizeof( SHeader )] : nullptr;

	//Set The Color Map Pointer....Color Map Is Optional.
	this->ImageArea.ColorMapData = this->pHeader->ColorMapSpec.ColorMapLength ? (uint8_t*)&this->RawData[sizeof( SHeader ) + this->pHeader->IDLength] : nullptr;

	//Set The Image Data Pointer....Image Data Is Optional.
	this->ImageArea.ImageData = this->pHeader->ImageSpec.ImageWidth && this->pHeader->ImageSpec.ImageHeight ? (uint8_t*)&this->RawData[sizeof( SHeader ) + this->pHeader->IDLength + ( (size_t)this->pHeader->ColorMapSpec.ColorMapLength * ( this->pHeader->ColorMapSpec.ColorMapEntrySize / 8 ) )] : nullptr;

	//Set The Footer Pointer....Footer Is The Last 26 Bytes Of The File. It Is Optional.
	this->pFooter = (SFooter*)( ( &this->RawData[0] ) + ( this->RawData.size() - 26 ) );
	if( strncmp( (const char*)this->pFooter->Signature, "TRUEVISION-XFILE.", sizeof( this->pFooter->Signature ) ) != 0 )
		this->pFooter = nullptr;//No Footer....Targa Is Version 1.

	//Set The Developer Directory Pointer....Developer Directory Is Optional.
	this->pDeveloperDirectory = this->pFooter && pFooter->DeveloperDirectoryOffset ? reinterpret_cast<SDeveloperDirectory*>( (uint8_t*)this->pHeader + this->pFooter->DeveloperDirectoryOffset ) : nullptr;

	//Set The Developer Directory Tag Pointer....Developer Directory Tag Is Optional.
	if( this->pDeveloperDirectory && this->pDeveloperDirectory->NumberOfTagsInDirectory )
		this->pDeveloperDirectory->Tags = reinterpret_cast<SDeveloperDirectory::STag*>( (uint8_t*)this->pDeveloperDirectory + sizeof( SDeveloperDirectory::NumberOfTagsInDirectory ) );

	//Set The Developer Data Pointer....Developer Data Is Optional.
	{
		//Calculate Developer Data Size By Iterating Through The Developer Directory Tags And Accumulating Their Sizes.
		size_t DeveloperDataSize = 0;
		if( this->pDeveloperDirectory && this->pDeveloperDirectory->NumberOfTagsInDirectory )
		{
			//Iterate Through The Developer Directory Tags And Accumulate Their Sizes.
			for( size_t i = 0; i < this->pDeveloperDirectory->NumberOfTagsInDirectory; ++i )
				DeveloperDataSize += this->pDeveloperDirectory->Tags[i].Size;

			//Set The Developer Data Pointer.
			this->DeveloperArea.DeveloperData = (uint8_t*)this->pDeveloperDirectory - DeveloperDataSize;
		}
	}

	//Set The Extension Area Pointer....Extension Area Is Optional.
	this->pExtensionArea = this->pFooter && this->pFooter->ExtensionAreaOffset ? reinterpret_cast<SExtensionArea*>( (uint8_t*)this->pHeader + this->pFooter->ExtensionAreaOffset ) : nullptr;

	//Set The Extension Area ColorCorrectionTable Pointer....Extension Area Tag Is Optional.
	if( this->pExtensionArea && this->pExtensionArea->ColorCorrectionOffset )
		this->pExtensionArea->pColorCorrectionTable = reinterpret_cast<SExtensionArea::SColorCorrectionTable*>( (uint8_t*)this->pHeader + this->pExtensionArea->ColorCorrectionOffset );

	//Set The Extension Area Tag Pointer....Extension Area Tag Is Optional.
	if( this->pExtensionArea && this->pExtensionArea->ScanLineOffset )
		this->pExtensionArea->pScanLineTable = (uint32_t*)( (uint8_t*)this->pHeader + this->pExtensionArea->ScanLineOffset );

	//Set The Extension Area Tag Pointer....Extension Area Tag Is Optional.
	if( this->pExtensionArea && this->pExtensionArea->PostageStampOffset )
		this->pExtensionArea->pPostageStampImage = (uint8_t*)( (uint8_t*)this->pHeader + this->pExtensionArea->PostageStampOffset );
}

void STargaImage::ConvertCompRawDataToDecompRawData( const std::vector<uint8_t>& DecompImageData )
{
	//Calculate The Various Sizes Of The Areas In The RawData Buffer

	//Calculate The Size Of The Header.
	const auto HeaderSize = sizeof( SHeader );

	//Calculate Size Of Footer
	const auto FooterSize = this->pFooter ? sizeof( SFooter ) : 0;

	//Calculate The Size Of The Image ID.
	const auto ImageIDSize = this->pHeader->IDLength;

	//Calculate The Size Of The Color Map Data.
	const auto ColorMapDataSize = this->pHeader->ColorMapSpec.ColorMapLength * ( this->pHeader->ColorMapSpec.ColorMapEntrySize / 8 );

	//Calculate The Size Of The Image Data
	size_t ImageDataSize = 0;
	if( !this->pFooter ) //If There Is No Footer, Then The Image Data Is The Rest Of The Buffer.
		ImageDataSize = this->RawData.size() - HeaderSize - ImageIDSize - ColorMapDataSize;
	else
	{
		//If There Is A Footer, But No Developer Dictionary, Then The Image Data Is The Rest Of The Buffer Up To The Extension Area.
		if( !pDeveloperDirectory )
		{
			//If There Is A Footer, But No Developer Dictionary Or Extension Area, Then The Image Data Is The Rest Of The Buffer Up To The Footer.
			if( !pExtensionArea )
				ImageDataSize = this->RawData.size() - HeaderSize - ImageIDSize - ColorMapDataSize - FooterSize;
			else // If There Is A Footer, But No Developer Dictionary, Then The Image Data Is The Rest Of The Buffer Up To The Extension Area.
				ImageDataSize = pFooter->ExtensionAreaOffset - HeaderSize - ImageIDSize - ColorMapDataSize;
		}
		else //If There Is A Footer And A Developer Dictionary, Then The Image Data Is The Rest Of The Buffer Up To The Developer Dictionary.
			ImageDataSize = pFooter->DeveloperDirectoryOffset - HeaderSize - ImageIDSize - ColorMapDataSize;
	}

	//Calculate The Size Of The Developer Directory
	size_t DeveloperDirectorySize = 0;
	if( pDeveloperDirectory )
	{
		//We Can Calculate It By Pointer Arithmetic. The Size Is The Difference Between The Developer Directory And The Extension Area (If It Exists) Or The Footer.
		if( pExtensionArea )
			DeveloperDirectorySize = (uint8_t*)pExtensionArea - (uint8_t*)pDeveloperDirectory;
		else if( pFooter )
			DeveloperDirectorySize = (uint8_t*)pFooter - (uint8_t*)pDeveloperDirectory;
	}

	//Calculate The Size Of The Developer Data
	size_t DeveloperDataSize = 0;
	if( pDeveloperDirectory )
	{
		//We Can Calculate It By Pointer Arithmetic. The Size Is The Difference Between The Developer Directory And End Of Image Data.
		DeveloperDataSize = (uint8_t*)pHeader + HeaderSize + ImageIDSize + ColorMapDataSize + ImageDataSize - (uint8_t*)pDeveloperDirectory;
	}

	//Calculate The Size Of The Extension Area
	size_t ExtensionAreaSize = 0;
	if( pExtensionArea )
	{
		//We Can Calculate It By Pointer Arithmetic. The Size Is The Difference Between The Extension Area And The Footer (If It Exists)
		if( pFooter )
			ExtensionAreaSize = (uint8_t*)pFooter - (uint8_t*)pExtensionArea;
	}

	//Create A New RawData Buffer With The Correct Size.
	std::vector<uint8_t> NewRawData( HeaderSize + ImageIDSize + ColorMapDataSize + DecompImageData.size() + DeveloperDirectorySize + DeveloperDataSize + ExtensionAreaSize + FooterSize );

	//Copy Up To The Start Of The Image Data.
	memcpy( NewRawData.data(), this->RawData.data(), HeaderSize + ImageIDSize + ColorMapDataSize );

	//Copy The Decompressed Image Data.
	memcpy( NewRawData.data() + HeaderSize + ImageIDSize + ColorMapDataSize, DecompImageData.data(), DecompImageData.size() );

	//Copy From The End Of The Image Data To The End Of The Buffer.
	memcpy( NewRawData.data() + HeaderSize + ImageIDSize + ColorMapDataSize + DecompImageData.size(), this->RawData.data() + HeaderSize + ImageIDSize + ColorMapDataSize + ImageDataSize, FooterSize + DeveloperDirectorySize + DeveloperDataSize + ExtensionAreaSize );

	//Replace The Old RawData Buffer With The New One.
	this->RawData = std::move( NewRawData );
}

void STargaImage::DecompressTrueColorRLE()
{
	//Create A New Buffer To Hold The Decompressed Image Data.
	std::vector<uint8_t> DecompressedData( ( this->pHeader->ImageSpec.ImageDepth / 8 ) * this->pHeader->ImageSpec.ImageWidth * this->pHeader->ImageSpec.ImageHeight );

	const auto pCompressedData = this->ImageArea.ImageData;
	const auto BytesPerPixel = this->pHeader->ImageSpec.ImageDepth / 8;

	// Calculate The Size Of The Decompress Image In Bytes
	const auto DecompressedImageSizeBytes = DecompressedData.size();

	// Decompress the image data.
	int OutputIndex = 0;
	int InputIndex = 0;
	while( OutputIndex < DecompressedImageSizeBytes )
	{
		// Read the RLE packet header byte.
		unsigned char header = pCompressedData[InputIndex++];
		int count = ( header & 0x7F ) + 1;

		if( header & 0x80 )
		{
			// RLE packet, read color data and repeat it.
			std::vector<uint8_t> color( BytesPerPixel, 0 );
			for( int i = 0; i < BytesPerPixel; i++ )
			{
				color[i] = pCompressedData[InputIndex++];
			}

			for( int i = 0; i < count; i++ )
			{
				for( int j = 0; j < BytesPerPixel; j++ )
				{
					DecompressedData[OutputIndex++] = color[j];
				}
			}
		}
		else
		{
			// Raw packet, read uncompressed color data.
			for( int i = 0; i < count; i++ )
			{
				for( int j = 0; j < BytesPerPixel; j++ )
				{
					DecompressedData[OutputIndex++] = pCompressedData[InputIndex++];
				}
			}
		}
	}

	//Convert To Decompressed Raw Data
	ConvertCompRawDataToDecompRawData( DecompressedData );

	//Update Image Type.
	reinterpret_cast<SHeader*>( this->RawData.data() )->ImageType = EImageType::TrueColor;

	//Parse RawData To Update The Pointers.
	ParseRawData();
}

void STargaImage::DecompressBlackAndWhiteRLE()
{
	// Calculate the size of the decompressed image data in bytes
	const auto DecompressedImageSizeBits = this->pHeader->ImageSpec.ImageWidth * this->pHeader->ImageSpec.ImageHeight;
	const auto DecompressedImageSizeBytes = ( DecompressedImageSizeBits + 7 ) / 8;

	//Create A New Buffer To Hold The Decompressed Image Data.
	std::vector<uint8_t> DecompressedData( DecompressedImageSizeBytes );
	const auto CompressedData = this->ImageArea.ImageData;

	// Decompress the image data.
	int OutputIndex = 0;
	int InputIndex = 0;
	while( OutputIndex < DecompressedImageSizeBits )
	{
		// Read the RLE packet header byte.
		const auto Header = CompressedData[InputIndex++];
		const auto Count = ( Header & 0x7F ) + 1;

		if( Header & 0x80 )
		{
			// RLE packet, read the color (bit) and repeat it.
			unsigned char color = CompressedData[InputIndex++];

			for( int i = 0; i < Count; i++ )
			{
				// Store the color in the output buffer
				if( color )
				{
					// Set the bit to 1
					DecompressedData[OutputIndex / 8] |= 0x80 >> ( OutputIndex % 8 );
				}
				else
				{
					// Set the bit to 0
					DecompressedData[OutputIndex / 8] &= ~( 0x80 >> ( OutputIndex % 8 ) );
				}
				OutputIndex++;
			}
		}
		else
		{
			// Raw packet, read uncompressed colors (bits).
			for( int i = 0; i < Count; i++ )
			{
				const auto Color = CompressedData[InputIndex++];

				// Store the color in the output buffer
				if( Color )
				{
					// Set the bit to 1
					DecompressedData[OutputIndex / 8] |= 0x80 >> ( OutputIndex % 8 );
				}
				else
				{
					// Set the bit to 0
					DecompressedData[OutputIndex / 8] &= ~( 0x80 >> ( OutputIndex % 8 ) );
				}
				OutputIndex++;
			}
		}
	}

	//Convert To Decompressed Raw Data
	ConvertCompRawDataToDecompRawData( DecompressedData );

	//Update Image Type.
	reinterpret_cast<SHeader*>( this->RawData.data() )->ImageType = EImageType::BlackAndWhite;

	//Parse RawData To Update The Pointers.
	ParseRawData();
}

void STargaImage::DecompressColormappedRLE()
{
	//Create A New Buffer To Hold The Decompressed Image Data.
	std::vector<uint8_t> DecompressedData( this->pHeader->ImageSpec.ImageWidth * this->pHeader->ImageSpec.ImageHeight );
	const auto pCompressedData = this->ImageArea.ImageData;

	const size_t DecompressedImageSizeBytes = DecompressedData.size();

	// Decompress the image data.
	int OutputIndex = 0;
	int InputIndex = 0;
	while( OutputIndex < DecompressedImageSizeBytes )
	{
		// Read the RLE packet header byte.
		const auto Header = pCompressedData[InputIndex++];
		const auto Count = ( Header & 0x7F ) + 1;

		if( Header & 0x80 )
		{
			// RLE packet, read index into color palette and repeat it.
			const auto ColorIndex = pCompressedData[InputIndex++];

			for( int i = 0; i < Count; i++ )
			{
				DecompressedData[OutputIndex++] = ColorIndex;
			}
		}
		else
		{
			// Raw packet, read uncompressed color indices.
			for( int i = 0; i < Count; i++ )
			{
				const auto ColorIndex = pCompressedData[InputIndex++];
				DecompressedData[OutputIndex++] = ColorIndex;
			}
		}
	}

	//Convert To Decompressed Raw Data
	ConvertCompRawDataToDecompRawData( DecompressedData );

	//Update Image Type.
	reinterpret_cast<SHeader*>( this->RawData.data() )->ImageType = EImageType::ColorMapped;

	//Parse RawData To Update The Pointers.
	ParseRawData();
}