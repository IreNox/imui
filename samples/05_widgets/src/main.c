#include "../../framework/framework.h"

#include "imui/imui.h"
#include "imui/imui_widgets.h"

#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

static ImUiTexture	s_skinTexture	= { NULL };
static ImUiTexture	s_fontTexture	= { NULL };
static ImUiFont*	s_font			= NULL;

static void			ImUiTestSetConfig();

void ImUiFrameworkTick( ImUiSurface* surface )
{
	ImUiTestSetConfig();

	const ImUiSize surfaceSize = ImUiSurfaceGetSize( surface );
	ImUiWindow* window = ImUiWindowBegin( surface, ImUiStringViewCreate( "main" ), ImUiRectangleCreate( 0.0f, 0.0f, surfaceSize.width, surfaceSize.height ), 1 );

	ImUiWidget* vLayout = ImUiWidgetBeginNamed( window, ImUiStringViewCreate( "vMain" ) );
	ImUiWidgetSetStretch( vLayout, ImUiSizeCreateZero() );
	ImUiWidgetSetMargin( vLayout, ImUiThicknessCreateAll( 25.0f ) );
	ImUiWidgetSetLayoutVerticalSpacing( vLayout, 10.0f );

	static bool checked[ 3u ] ={ false, true, false };
	{
		ImUiWidget* buttonsLayout = ImUiWidgetBeginNamed( window, ImUiStringViewCreate( "buttons" ) );
		ImUiWidgetSetStretch( buttonsLayout, ImUiSizeCreateHorizintal() );
		ImUiWidgetSetLayoutHorizontalSpacing( buttonsLayout, 10.0f );

		if( ImUiWidgetsButton( window, IMUI_STR( "Button 1" ) ) )
		{
			checked[ 0u ] = !checked[ 0u ];
		}

		if( ImUiWidgetsButton( window, IMUI_STR( "Button 1" ) ) )
		{
			checked[ 1u ] = !checked[ 1u ];
		}

		if( ImUiWidgetsButton( window, IMUI_STR( "Button 1" ) ) )
		{
			checked[ 2u ] = !checked[ 2u ];
		}

		ImUiWidgetEnd( buttonsLayout );
	}

	{
		ImUiWidget* checkLayout = ImUiWidgetBeginNamed( window, ImUiStringViewCreate( "checks" ) );
		ImUiWidgetSetStretch( checkLayout, ImUiSizeCreateHorizintal() );
		ImUiWidgetSetLayoutVerticalSpacing( checkLayout, 10.0f );

		ImUiWidgetsCheckBox( window, IMUI_STR( "Check 1" ), &checked[ 0u ] );
		ImUiWidgetsCheckBox( window, IMUI_STR( "Check 2" ), &checked[ 1u ] );
		ImUiWidgetsCheckBox( window, IMUI_STR( "Check 3" ), &checked[ 2u ] );

		char text[ 64u ];
		snprintf( text, sizeof( text ), "C1: %d, C2: %d, C3: %d", checked[ 0u ], checked[ 1u ], checked[ 2u ] );
		ImUiWidgetsLabel( window, IMUI_STR( text ) );

		ImUiWidgetEnd( checkLayout );
	}

	{
		ImUiWidget* sliderLayout = ImUiWidgetBeginNamed( window, ImUiStringViewCreate( "sliders" ) );
		ImUiWidgetSetStretch( sliderLayout, ImUiSizeCreateHorizintal() );
		ImUiWidgetSetLayoutVerticalSpacing( sliderLayout, 10.0f );

		static float value = 2.5f;
		ImUiWidgetsSliderMinMax( window, &value, 0.0f, 5.0f );

		char text[ 64u ];
		snprintf( text, sizeof( text ), "Value: %.2f", value );
		ImUiWidgetsLabel( window, IMUI_STR( text ) );

		ImUiWidgetEnd( sliderLayout );
	}

	//ImUiDrawRectangleTexture( vLayout, ImUiRectangleCreateSize( 50.0f, 50.0f, s_fontTexture.size ), s_fontTexture );

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
	const float fontSize = 14.0f;
	ImUiFontTrueTypeDataCalculateMinTextureSize( ttf, fontSize, &width, &height );
	width = (width + 4u - 1u) & (0 - 4);
	height = (height + 4u - 1u) & (0 - 4);

	void* textureData = malloc( width * height );
	ImUiFontTrueTypeImage* image = ImUiFontTrueTypeDataGenerateTextureData( ttf, fontSize, textureData, width * height, width, height );

	s_fontTexture.data = ImUiFrameworkTextureCreate( textureData, width, height, true );
	s_fontTexture.size = ImUiSizeCreate( (float)width, (float)height );

	free( textureData );

	s_font = ImUiFontCreateTrueType( imui, image, s_fontTexture );

	ImUiFontTrueTypeDataDestroy( ttf );
	free( fileData );

	uint8_t skinData[ 32u * 32u * 4u ];
	for( uint8_t y = 0u; y < 32u; ++y )
	{
		const float v = y / 32.0f;

		uint8_t* line = skinData + (y * 32u * 4u);
		for( uint8_t x = 0u; x < 32u; ++x )
		{
			const float u = x / 32.0f;

			static const float s_points[][ 2u ] =
			{
				{ 0.0f, 0.0f },
				{ 1.0f, 0.0f },
				{ 0.0f, 1.0f },
				{ 1.0f, 1.0f }
			};

			float minDis = FLT_MAX;
			for( uint8_t i = 0u; i < 4u; ++i )
			{
				const float udiff = u - s_points[ i ][ 0u ];
				const float vdiff = v - s_points[ i ][ 1u ];

				const float dis = sqrtf( (udiff * udiff) + (vdiff * vdiff) );
				minDis = minDis < dis ? minDis : dis;
			}

			minDis *= 750.0f;
			const uint8_t value = minDis > 255.0f ? 255u : (uint8_t)minDis;

			uint8_t* pixel = line + (x * 4u);
			pixel[ 0u ]		= value;
			pixel[ 1u ]		= value;
			pixel[ 2u ]		= value;
			pixel[ 3u ]		= 0xffu;
		}
	}

	s_skinTexture.data = ImUiFrameworkTextureCreate( skinData, 32u, 32u, false );
	s_skinTexture.size = ImUiSizeCreate( 32.0f, 32.0f );

	ImUiTestSetConfig();

	return true;
}

void ImUiFrameworkShutdown( ImUiContext* imui )
{
	ImUiFrameworkTextureDestroy( (ImUiFrameworkTexture*)s_skinTexture.data );
	ImUiFrameworkTextureDestroy( (ImUiFrameworkTexture*)s_fontTexture.data );

	ImUiFontDestroy( imui, s_font );
}

static void ImUiTestSetConfig()
{
	ImUiWidgetsConfig config;
	config.colors[ ImUiWidgetsColor_Text ]						= ImUiColorCreate( 1.0f, 1.0f, 1.0f, 1.0f );
	config.colors[ ImUiWidgetsColor_Button ]					= ImUiColorCreate( 0.1f, 0.5f, 0.7f, 1.0f );
	config.colors[ ImUiWidgetsColor_ButtonHover ]				= ImUiColorCreate( 0.3f, 0.7f, 0.9f, 1.0f );
	config.colors[ ImUiWidgetsColor_ButtonClicked ]				= ImUiColorCreate( 0.0f, 0.3f, 0.5f, 1.0f );
	config.colors[ ImUiWidgetsColor_ButtonText ]				= ImUiColorCreate( 1.0f, 1.0f, 1.0f, 1.0f );
	config.colors[ ImUiWidgetsColor_CheckBox ]					= ImUiColorCreate( 0.1f, 0.5f, 0.7f, 1.0f );
	config.colors[ ImUiWidgetsColor_CheckBoxHover ]				= ImUiColorCreate( 0.3f, 0.7f, 0.9f, 1.0f );
	config.colors[ ImUiWidgetsColor_CheckBoxClicked ]			= ImUiColorCreate( 0.0f, 0.3f, 0.5f, 1.0f );
	config.colors[ ImUiWidgetsColor_CheckBoxChecked ]			= ImUiColorCreate( 1.0f, 0.5f, 0.7f, 1.0f );
	config.colors[ ImUiWidgetsColor_CheckBoxCheckedHover ]		= ImUiColorCreate( 1.0f, 0.7f, 0.9f, 1.0f );
	config.colors[ ImUiWidgetsColor_CheckBoxCheckedClicked ]	= ImUiColorCreate( 1.0f, 0.3f, 0.5f, 1.0f );
	config.colors[ ImUiWidgetsColor_SliderBackground ]			= ImUiColorCreate( 0.0f, 0.3f, 0.5f, 1.0f );
	config.colors[ ImUiWidgetsColor_SliderPivot ]				= ImUiColorCreate( 0.1f, 0.5f, 0.7f, 1.0f );
	config.colors[ ImUiWidgetsColor_SliderPivotHover ]			= ImUiColorCreate( 0.3f, 0.7f, 0.9f, 1.0f );
	config.colors[ ImUiWidgetsColor_SliderPivotClicked ]		= ImUiColorCreate( 0.0f, 0.3f, 0.5f, 1.0f );

	ImUiSkin skin;
	skin.texture	= s_skinTexture;
	skin.border		= ImUiThicknessCreateAll( 6.0f );
	skin.uv.u0		= 0.0f;
	skin.uv.v0		= 0.0f;
	skin.uv.u1		= 1.0f;
	skin.uv.v1		= 1.0f;

	config.skins[ ImUiWidgetsSkin_Button ]						= skin;
	config.skins[ ImUiWidgetsSkin_CheckBox ]					= skin;
	config.skins[ ImUiWidgetsSkin_CheckBoxChecked ]				= skin;
	config.skins[ ImUiWidgetsSkin_SliderBackground ]			= skin;
	config.skins[ ImUiWidgetsSkin_SliderPivot ]					= skin;

	config.font					= s_font;

	config.buttonPadding		= ImUiThicknessCreateAll( 8.0f );

	config.checkBoxSize			= ImUiSizeCreateAll( 20.0f );
	config.checkBoxTextSpacing	= 5.0f;

	config.sliderPadding		= ImUiThicknessCreateVerticalHorizontal( 8.0f, 0.0f );
	config.sliderPivotSize		= 12.0f;
	config.sliderHeight			= 20.0f;

	ImUiWidgetsSetConfig( &config );
}
