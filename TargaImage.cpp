#include "TargaImage.h"


#if !FASTTARGA_HEADERONLY


STargaImage::STargaImage() :
	pHeader( nullptr ),
	pDeveloperArea( nullptr ),
	pDeveloperDirectory( nullptr ),
	ImageArea( { 0 } ),
	pExtensionArea( nullptr ),
	pFooter( nullptr )
{

}




STargaImage::STargaImage( const char* FilePath ) :
	pHeader( nullptr ),
	pDeveloperArea( nullptr ),
	pDeveloperDirectory( nullptr ),
	ImageArea( { 0 } ),
	pExtensionArea( nullptr ),
	pFooter( nullptr )
{
	*this = std::move( STargaImage::Load( FilePath ) );
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




STargaImage::STargaImage( const STargaImage& RHS ) :
	RawData( RHS.RawData ),
	pHeader( RHS.pHeader ),
	pDeveloperArea( RHS.pDeveloperArea ),
	pDeveloperDirectory( RHS.pDeveloperDirectory ),
	ImageArea( RHS.ImageArea ),
	pExtensionArea( RHS.pExtensionArea ),
	pFooter( RHS.pFooter )
{

}




STargaImage& STargaImage::operator=( const STargaImage& RHS )
{
	RawData = RHS.RawData;
	pHeader = RHS.pHeader;
	pDeveloperArea = RHS.pDeveloperArea;
	pDeveloperDirectory = RHS.pDeveloperDirectory;
	ImageArea = RHS.ImageArea;
	pExtensionArea = RHS.pExtensionArea;
	pFooter = RHS.pFooter;
	return *this;
}




/*static*/ STargaImage STargaImage::Load( const char* FilePath )
{
	STargaImage Image;

	//Read In Entire Targa File.
	auto File = fopen( FilePath, "rb" );
	fseek(File, 0, SEEK_END);
	auto Size = ftell( File );
	rewind( File );
	Image.RawData.resize( Size );
	fread( &Image.RawData[0], 1, Size, File );
	fclose( File );

	//Set The Header Pointer....Header Is Always The First 18 Bytes Of The File.
	Image.pHeader = ( SHeader* ) &Image.RawData[ 0 ];

	//We Don't Support ColorMapped, Black And White, Or Compressed Images....Yet.
	if( Image.pHeader->ImageType != 2 /*UnCompressed True Color*/ )
		return STargaImage();

	//Set The Image Data Pointer....Image Data Comes After Pointer
	Image.ImageArea.ImageData = ( &Image.RawData[ 0 ] ) + sizeof( SHeader );
	
	auto b = Image.ImageArea.ImageData[ 0 ];
	auto g = Image.ImageArea.ImageData[ 1 ];
	auto r = Image.ImageArea.ImageData[ 2 ];
	auto a = Image.ImageArea.ImageData[ 3 ];

	//Set The Footer Pointer....Footer Is Always The Last 26 Bytes Of The File.
	Image.pFooter = ( SFooter* ) ( ( &Image.RawData[ 0 ] ) + ( Image.RawData.size() - 26 ) );
	if( strcmp( ( const char* ) Image.pFooter->Signature, "TRUEVISION-XFILE" ) != 0 )
		Image.pFooter = nullptr;//No Footer....Targa Is Version 1.

	return std::move( Image );
}


#endif // FASTTARGA_HEADERONLY