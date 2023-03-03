#include "TargaImage.h"
//Include SDL2
#include <SDL.h>

int main( int argc, char** argv )
{
	// Load The Targa Image
	auto TargaImage = STargaImage::Load( "C:\\Users\\guita\\Desktop\\Untitled.tga" );
	STargaImage::Save( TargaImage, R"(C:\Users\guita\Desktop\Untitled2.tga)" );
	
	//Display The Image Using The SDL2 Library
	SDL_Init( SDL_INIT_VIDEO );
	SDL_Window* pWindow = SDL_CreateWindow( "FastTarga Test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, TargaImage.pHeader->ImageSpec.ImageWidth, TargaImage.pHeader->ImageSpec.ImageHeight, SDL_WINDOW_SHOWN );
	SDL_Renderer* pRenderer = SDL_CreateRenderer( pWindow, -1, SDL_RENDERER_ACCELERATED );
	SDL_Texture* pTexture = SDL_CreateTexture( pRenderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STATIC, TargaImage.pHeader->ImageSpec.ImageWidth, TargaImage.pHeader->ImageSpec.ImageHeight );
	SDL_UpdateTexture( pTexture, nullptr, TargaImage.ImageArea.ImageData, TargaImage.pHeader->ImageSpec.ImageWidth * 3 );
	
	SDL_Event Event;
	while( true )
	{
		if( SDL_PollEvent( &Event ) )
		{
			if( Event.type == SDL_QUIT )
				break;
		}
		SDL_RenderClear( pRenderer );
		SDL_RenderCopy( pRenderer, pTexture, nullptr, nullptr );
		SDL_RenderPresent( pRenderer );
	}
	SDL_DestroyTexture( pTexture );
	SDL_DestroyRenderer( pRenderer );
	SDL_DestroyWindow( pWindow );
	SDL_Quit();
	
	return 1;
}