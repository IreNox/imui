#include "../../framework/framework.h"

#include "imui/imui.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

static ImUiTexture s_fontTexture;
static ImUiFont* s_font = NULL;

static float s_time = 0.0f;

void ImUiFrameworkTick( ImUiSurface* surface )
{
	s_time += (1.0f / 60.0f);

	ImUiContext* imui = ImUiSurfaceGetContext( surface );

	ImUiTextLayout* text = ImUiTextLayoutCreate( imui, s_font, ImUiStringViewCreate( u8"ΑΒΓΔ Hello World! ΦΧΨΩ" ) );

	const ImUiSize surfaceSize = ImUiSurfaceGetSize( surface );
	ImUiWindow* window = ImUiWindowBegin( surface, ImUiStringViewCreate( "main" ), ImUiRectangleCreate( 0.0f, 0.0f, surfaceSize.width, surfaceSize.height ), 1 );

	const float timeSin = sinf( s_time / -2.0f );
	const float timeCos = cosf( s_time / -2.0f );

	const float timeLeft	= (timeSin < 0.0f ? -timeSin : 0.0f) + 1.0f;
	const float timeRight	= (timeSin > 0.0f ? timeSin : 0.0f) + 1.0f;
	const float timeTop		= (timeCos < 0.0f ? -timeCos : 0.0f) + 1.0f;
	const float timeBottom	= (timeCos > 0.0f ? timeCos : 0.0f) + 1.0f;

	const float timeR		= (timeSin * 0.5f) + 0.5f;
	const float timeG		= (timeCos * 0.5f) + 0.5f;
	const float timeB		= ((2.0f - (timeSin + timeCos)) * 0.25f) + 0.5f;

	ImUiWidget* vLayout = ImUiWidgetBeginNamed( window, ImUiStringViewCreate( "vMain" ) );
	ImUiWidgetSetStretch( vLayout, ImUiSizeCreateOne() );
	ImUiWidgetSetLayoutVerical( vLayout );

	{
		ImUiWidget* vTop = ImUiWidgetBeginNamed( window, ImUiStringViewCreate( "vTop" ) );
		ImUiWidgetSetStretch( vTop, ImUiSizeCreate( 1.0f, timeTop ) );
		ImUiWidgetEnd( vTop );
	}

	{
		ImUiWidget* hLayout = ImUiWidgetBeginNamed( window, ImUiStringViewCreate( "hMain" ) );
		ImUiWidgetSetStretch( hLayout, ImUiSizeCreate( 1.0f, 0.0f ) );
		ImUiWidgetSetLayoutHorizontal( hLayout );

		{
			ImUiWidget* hLeft = ImUiWidgetBeginNamed( window, ImUiStringViewCreate( "hLeft" ) );
			ImUiWidgetSetStretch( hLeft, ImUiSizeCreate( timeLeft, 1.0f ) );
			ImUiWidgetEnd( hLeft );

			ImUiWidget* hCenter = ImUiWidgetBeginNamed( window, ImUiStringViewCreate( "hCenter" ) );
			ImUiWidgetSetStretch( hCenter, ImUiSizeCreateZero() );
			ImUiWidgetSetPadding( hCenter, ImUiThicknessCreateAll( 50.0f ) );

			ImUiDrawWidgetColor( hCenter, ImUiColorCreate( timeR, timeG, timeB, 1.0f ) );

			ImUiWidget* centerText = ImUiWidgetBeginNamed( window, ImUiStringViewCreate( "centerText" ) );
			ImUiWidgetSetFixedSize( centerText, ImUiTextLayoutGetSize( text ) );

			ImUiDrawText( centerText, ImUiWidgetGetPosition( centerText ), text, ImUiColorCreateWhite( 1.0f ) );

			ImUiWidgetEnd( centerText );

			ImUiWidgetEnd( hCenter );

			ImUiWidget* hRight = ImUiWidgetBeginNamed( window, ImUiStringViewCreate( "hRight" ) );
			ImUiWidgetSetStretch( hRight, ImUiSizeCreate( timeRight, 1.0f ) );
			ImUiWidgetEnd( hRight );
		}

		ImUiWidgetEnd( hLayout );
	}

	{
		ImUiWidget* vBottom = ImUiWidgetBeginNamed( window, ImUiStringViewCreate( "vBottom" ) );
		ImUiWidgetSetStretch( vBottom, ImUiSizeCreate( 1.0f, timeBottom ) );
		ImUiWidgetEnd( vBottom );
	}

	ImUiWidgetEnd( vLayout );

	ImUiWindowEnd( window );
}

bool ImUiFrameworkInitialize( ImUiContext* imui )
{
	uint8_t* fileData;
	size_t fileSize;
	{
		FILE* file = fopen( "c:/windows/fonts/arial.ttf", "rb" );

		fseek( file, 0, SEEK_END );
		fpos_t fileSizeS;
		fgetpos( file, &fileSizeS );
		fileSize = (size_t)fileSizeS;
		fseek( file, 0, SEEK_SET );

		fileData = (uint8_t*)malloc( fileSize );
		fread( fileData, fileSize, 1, file );
		fclose( file );
	}

	ImUiFontTrueTypeData* ttf = ImUiFontTrueTypeDataCreate( imui, fileData, fileSize );

	ImUiFontTrueTypeDataAddCodepointRange( ttf, 0x20, 0x7e );
	ImUiFontTrueTypeDataAddCodepointRange( ttf, 0x370, 0x3ff );
	ImUiFontTrueTypeDataAddCodepointRange( ttf, 0xfffd, 0xfffd );

	uint32_t width;
	uint32_t height;
	const float fontSize = 32.0f;
	ImUiFontTrueTypeDataCalculateMinTextureSize( ttf, fontSize, &width, &height );
	width = (width + 4u - 1u) & (0u - 4u);
	height = (height + 4u - 1u) & (0u - 4u);

	void* textureData = malloc( width * height );
	ImUiFontTrueTypeImage* image = ImUiFontTrueTypeDataGenerateTextureData( ttf, fontSize, textureData, width * height, width, height );

	ImUiTexture texture;
	texture.data = (void*)(uint64_t)ImUiFrameworkTextureCreate( textureData, width, height );
	texture.size = ImUiSizeCreate( (float)width, (float)height );
	s_fontTexture = texture;

	free( textureData );

	s_font = ImUiFontCreateTrueType( imui, image, texture );

	ImUiFontTrueTypeDataDestroy( ttf );
	free( fileData );
	return true;
}

void ImUiFrameworkShutdown( ImUiContext* imui )
{
	ImUiFontDestroy( imui, s_font );
}
