#include "../../framework/framework.h"

#include "imui/imui.h"

#include <stdlib.h>
#include <stdio.h>

static HwMinSizeHorizontal( ImUiWindow* window );
static HwMinSizeVertical( ImUiWindow* window );
static HwMinSizeElement( ImUiWindow* window );

static HwStretchStack( ImUiWindow* window );
static HwStretchHorizontal( ImUiWindow* window );
static HwStretchVertical( ImUiWindow* window );
static HwStretchElements( ImUiWindow* window, ImUiSize stretch1, ImUiSize stretch2, ImUiSize stretch3 );

static ImUiFont* s_font = NULL;

void ImUiFrameworkTick( ImUiSurface* surface )
{
	const ImUiSize surfaceSize = ImUiSurfaceGetSize( surface );
	ImUiWindow* window = ImUiWindowBegin( surface, ImUiStringViewCreate( "main" ), ImUiRectangleCreate( 0.0f, 0.0f, surfaceSize.width, surfaceSize.height ), 1 );

	ImUiWidget* hLayout = ImUiWidgetBeginNamed( window, ImUiStringViewCreate( "hMain" ) );
	ImUiWidgetSetMargin( hLayout, ImUiThicknessCreateAll( 50.0f ) );
	ImUiWidgetSetStretch( hLayout, ImUiSizeCreateOne() );
	ImUiWidgetSetLayoutHorizontalSpacing( hLayout, 50.0f );

	ImUiWidget* vLayout = ImUiWidgetBeginNamed( window, ImUiStringViewCreate( "vMain" ) );
	ImUiWidgetSetStretch( vLayout, ImUiSizeCreateOne() );
	ImUiWidgetSetLayoutVerticalSpacing( vLayout, 50.0f );

	HwMinSizeHorizontal( window );
	HwStretchStack( window );
	HwStretchHorizontal( window );

	ImUiWidgetEnd( vLayout );

	HwStretchVertical( window );

	ImUiWidgetEnd( hLayout );

	ImUiWindowEnd( window );
}

static HwMinSizeHorizontal( ImUiWindow* window )
{
	ImUiWidget* layout = ImUiWidgetBeginNamed( window, ImUiStringViewCreate( "min_horizontal" ) );
	ImUiWidgetSetPadding( layout, ImUiThicknessCreateAll( 20.0f ) );
	ImUiWidgetSetLayoutHorizontalSpacing( layout, 10.0f );

	ImUiDrawRectangleColor( layout, ImUiWidgetGetRectangle( layout ), ImUiColorCreateWhite( 1.0f ) );

	HwMinSizeElement( window );
	HwMinSizeElement( window );
	HwMinSizeElement( window );

	ImUiWidgetEnd( layout );
}

static HwMinSizeVertical( ImUiWindow* window )
{

}

static HwMinSizeElement( ImUiWindow* window )
{
	ImUiWidget* widget = ImUiWidgetBegin( window );
	ImUiWidgetSetMargin( widget, ImUiThicknessCreateAll( 10.0f ) );
	ImUiWidgetSetFixedSize( widget, ImUiSizeCreateAll( 20.0f ) );

	ImUiDrawRectangleColor( widget, ImUiWidgetGetRectangle( widget ), ImUiColorCreate( 0.0f, 1.0f, 1.0f, 1.0f ) );

	ImUiWidgetEnd( widget );
}

static HwStretchStack( ImUiWindow* window )
{
	ImUiWidget* layout = ImUiWidgetBeginNamed( window, ImUiStringViewCreate( "stack" ) );
	ImUiWidgetSetPadding( layout, ImUiThicknessCreateAll( 20.0f ) );
	ImUiWidgetSetStretch( layout, ImUiSizeCreateOne() );

	ImUiDrawRectangleColor( layout, ImUiWidgetGetRectangle( layout ), ImUiColorCreateWhite( 1.0f ) );

	{
		ImUiWidget* widget2 = ImUiWidgetBegin( window );
		ImUiWidgetSetMargin( widget2, ImUiThicknessCreateAll( 10.0f ) );
		ImUiWidgetSetStretch( widget2, ImUiSizeCreate( 0.5f, 0.5f ) );

		ImUiDrawRectangleColor( widget2, ImUiWidgetGetRectangle( widget2 ), ImUiColorCreate( 0.0f, 0.0f, 1.0f, 1.0f ) );

		ImUiWidgetEnd( widget2 );
	}

	{
		ImUiWidget* widget2 = ImUiWidgetBegin( window );
		ImUiWidgetSetMargin( widget2, ImUiThicknessCreateAll( 10.0f ) );
		ImUiWidgetSetStretch( widget2, ImUiSizeCreate( 0.5f, 0.5f ) );
		ImUiWidgetSetAlignment( widget2, ImUiAlignmentCreate( ImUiHorizintalAlignment_Right, ImUiVerticalAlignment_Bottom ) );

		ImUiDrawRectangleColor( widget2, ImUiWidgetGetRectangle( widget2 ), ImUiColorCreate( 0.0f, 1.0f, 0.0f, 1.0f ) );

		ImUiWidgetEnd( widget2 );
	}

	{
		ImUiWidget* widget2 = ImUiWidgetBegin( window );
		ImUiWidgetSetMargin( widget2, ImUiThicknessCreateAll( 10.0f ) );
		ImUiWidgetSetStretch( widget2, ImUiSizeCreate( 0.5f, 0.5f ) );
		ImUiWidgetSetAlignment( widget2, ImUiAlignmentCreate( ImUiHorizintalAlignment_Center, ImUiVerticalAlignment_Center ) );

		ImUiDrawRectangleColor( widget2, ImUiWidgetGetRectangle( widget2 ), ImUiColorCreate( 1.0f, 0.0f, 0.0f, 1.0f ) );

		ImUiWidgetEnd( widget2 );
	}

	ImUiWidgetEnd( layout );
}

static HwStretchHorizontal( ImUiWindow* window )
{
	ImUiWidget* layout = ImUiWidgetBeginNamed( window, ImUiStringViewCreate( "horizontal" ) );
	ImUiWidgetSetPadding( layout, ImUiThicknessCreateAll( 20.0f ) );
	ImUiWidgetSetStretch( layout, ImUiSizeCreateOne() );
	ImUiWidgetSetLayoutHorizontalSpacing( layout, 10.0f );

	ImUiDrawRectangleColor( layout, ImUiWidgetGetRectangle( layout ), ImUiColorCreateWhite( 1.0f ) );

	HwStretchElements( window, ImUiSizeCreate( 1.0f, 1.0f ), ImUiSizeCreate( 2.0f, 2.0f ), ImUiSizeCreate( 1.0f, 1.0f ) );

	ImUiWidgetEnd( layout );
}

static HwStretchVertical( ImUiWindow* window )
{
	ImUiWidget* layout = ImUiWidgetBeginNamed( window, ImUiStringViewCreate( "vertical" ) );
	ImUiWidgetSetPadding( layout, ImUiThicknessCreateAll( 20.0f ) );
	ImUiWidgetSetStretch( layout, ImUiSizeCreateOne() );
	ImUiWidgetSetLayoutVerticalSpacing( layout, 10.0f );

	ImUiDrawRectangleColor( layout, ImUiWidgetGetRectangle( layout ), ImUiColorCreateWhite( 1.0f ) );

	HwStretchElements( window, ImUiSizeCreate( 1.0f, 1.0f ), ImUiSizeCreate( 2.0f, 2.0f ), ImUiSizeCreate( 1.0f, 1.0f ) );

	ImUiWidgetEnd( layout );
}

static HwStretchElements( ImUiWindow* window, ImUiSize stretch1, ImUiSize stretch2, ImUiSize stretch3 )
{
	{
		ImUiWidget* widget2 = ImUiWidgetBegin( window );
		ImUiWidgetSetMargin( widget2, ImUiThicknessCreateAll( 10.0f ) );
		ImUiWidgetSetStretch( widget2, stretch1 );

		ImUiDrawRectangleColor( widget2, ImUiWidgetGetRectangle( widget2 ), ImUiColorCreate( 0.0f, 0.0f, 1.0f, 1.0f ) );

		ImUiWidgetEnd( widget2 );
	}

	{
		ImUiWidget* widget2 = ImUiWidgetBegin( window );
		ImUiWidgetSetMargin( widget2, ImUiThicknessCreateAll( 10.0f ) );
		ImUiWidgetSetStretch( widget2, stretch2 );

		ImUiDrawRectangleColor( widget2, ImUiWidgetGetRectangle( widget2 ), ImUiColorCreate( 1.0f, 0.0f, 0.0f, 1.0f ) );

		ImUiWidgetEnd( widget2 );
	}

	{
		ImUiWidget* widget2 = ImUiWidgetBegin( window );
		ImUiWidgetSetMargin( widget2, ImUiThicknessCreateAll( 10.0f ) );
		ImUiWidgetSetStretch( widget2, stretch3 );
		ImUiWidgetSetHorizintalAlignment( widget2, ImUiHorizintalAlignment_Center );

		ImUiDrawRectangleColor( widget2, ImUiWidgetGetRectangle( widget2 ), ImUiColorCreate( 0.0f, 1.0f, 0.0f, 1.0f ) );

		ImUiWidgetEnd( widget2 );
	}
}

bool ImUiFrameworkInitialize( ImUiContext* imui )
{
	uint8_t* fileData;
	size_t fileSize;
	{
		FILE* file = fopen( "c:/windows/fonts/arialbd.ttf", "rb" );

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

	ImUiFontTrueTypeDataAddCodepointRange( ttf, 0x21, 0x7e );
	ImUiFontTrueTypeDataAddCodepointRange( ttf, 0x590, 0x5ff );

	uint32_t width;
	uint32_t height;
	ImUiFontTrueTypeDataCalculateMinTextureSize( ttf, 15.0f, &width, &height );

	void* textureData = malloc( width * height );
	ImUiFontTrueTypeImage* image = ImUiFontTrueTypeDataGenerateTextureData( ttf, 15.0f, textureData, width * height, width, height );

	ImUiTexture texture;
	texture.data = (void*)(uint64_t)ImUiFrameworkTextureCreate( textureData, width, height );
	texture.size = ImUiSizeCreate( (float)width, (float)height );

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
