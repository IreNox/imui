﻿#include "00_samples.h"

#ifndef IMUI_NO_SAMPLE_FRAMEWORK
#	include "framework.h"
#endif

#include "imui/imui.h"
#include "imui/imui_toolbox.h"

#include <float.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#define IMUI_ARRAY_COUNT( arr ) (sizeof( arr ) / sizeof( *(arr) ))

typedef struct ImUiToolboxSampleContext
{
	float		sliderValue1;

	ImUiFont*	font;
	ImUiImage	fontTexture;

	ImUiSkin	skinRect;
	ImUiImage	skinRectTexture;
	ImUiSkin	skinLine;
	ImUiImage	skinLineTexture;
} ImUiToolboxSampleContext;

static ImUiToolboxSampleContext s_toolboxContext = { 2.5f, NULL };

#ifndef IMUI_NO_SAMPLE_FRAMEWORK
static void			ImUiToolboxSampleSetConfig();
#endif

static void			ImUiToolboxSampleButtonsAndCheckBoxes( ImUiWindow* window, ImUiWidget* vLayout );
static void			ImUiToolboxSampleSlidersAndProgressBars( ImUiWindow* window );
static void			ImUiToolboxSampleTextEdit( ImUiWindow* window );
static void			ImUiToolboxSampleDropDown( ImUiWindow* window );
static void			ImUiToolboxSamplePopup( ImUiWindow* window );
static void			ImUiToolboxSampleScrollAndList( ImUiWindow* window );

typedef struct ImUiTestPopupState ImUiTestPopupState;
struct ImUiTestPopupState
{
	bool isOpen;
};

void ImUiToolboxSampleTick( ImUiSurface* surface )
{
#ifndef IMUI_NO_SAMPLE_FRAMEWORK
	ImUiToolboxSampleSetConfig();
#endif

	const ImUiSize surfaceSize = ImUiSurfaceGetSize( surface );
	ImUiWindow* window = ImUiWindowBegin( surface, "main", ImUiRectCreate( 0.0f, 0.0f, surfaceSize.width, surfaceSize.height ), 1 );

	ImUiWidget* hLayout = ImUiWidgetBeginNamed( window, "hMain" );
	ImUiWidgetSetStretchOne( hLayout );
	ImUiWidgetSetMargin( hLayout, ImUiBorderCreateAll( 25.0f ) );
	ImUiWidgetSetLayoutHorizontalSpacing( hLayout, 10.0f );

	ImUiWidget* vLayout = ImUiWidgetBeginNamed( window, "vMain" );
	ImUiWidgetSetHStretch( vLayout, 1.0f );
	ImUiWidgetSetLayoutVerticalSpacing( vLayout, 10.0f );

	ImUiToolboxSampleButtonsAndCheckBoxes( window, vLayout );
	ImUiToolboxSampleSlidersAndProgressBars( window );
	ImUiToolboxSampleTextEdit( window );
	ImUiToolboxSampleDropDown( window );
	ImUiToolboxSamplePopup( window );

	ImUiWidgetEnd( vLayout );

	ImUiToolboxSampleScrollAndList( window );

	const ImUiPos mousePos = ImUiInputGetMousePos( ImUiWindowGetContext( window ) );
	ImUiWidget* mouseLabel = ImUiToolboxLabelBeginFormat( window, "X: %.0f\nY: %.0f", mousePos.x, mousePos.y );
	ImUiWidgetSetFixedWidth( mouseLabel, 100.0f );
	ImUiWidgetSetVAlign( mouseLabel, 0.0f );
	ImUiToolboxLabelEnd( mouseLabel );

	//ImUiDrawRectTexture( vLayout, ImUiRectCreateSize( 50.0f, 50.0f, s_widgetContext.fontTexture.size ), s_widgetContext.fontTexture );

	ImUiWidgetEnd( hLayout );

	ImUiWindowEnd( window );
}

static void ImUiToolboxSampleButtonsAndCheckBoxes( ImUiWindow* window, ImUiWidget* vLayout )
{
	bool isNewState;
	bool* checked = (bool*)ImUiWidgetAllocStateNew( vLayout, sizeof( bool ) * 3u, IMUI_ID_STR( "check" ), &isNewState);
	if( isNewState )
	{
		checked[ 1u ] = true;
	}

	{
		ImUiWidget* buttonsLayout = ImUiWidgetBeginNamed( window, "buttons" );
		ImUiWidgetSetLayoutHorizontalSpacing( buttonsLayout, 10.0f );

		if( ImUiToolboxButtonLabel( window, "Button 1" ) )
		{
			checked[ 0u ] = !checked[ 0u ];
		}

		if( ImUiToolboxButtonLabel( window, "Button 2" ) )
		{
			checked[ 1u ] = !checked[ 1u ];
		}

		if( ImUiToolboxButtonLabel( window, "Button 3" ) )
		{
			checked[ 2u ] = !checked[ 2u ];
		}

		ImUiWidgetEnd( buttonsLayout );
	}

	{
		ImUiWidget* checkLayout = ImUiWidgetBeginNamed( window, "checks" );
		ImUiWidgetSetHStretch( checkLayout, 1.0f );
		ImUiWidgetSetLayoutVerticalSpacing( checkLayout, 10.0f );

		ImUiToolboxCheckBox( window, &checked[ 0u ], "Check 1" );
		ImUiToolboxCheckBox( window, &checked[ 1u ], "Check 2" );
		ImUiToolboxCheckBox( window, &checked[ 2u ], "Check 3" );

		ImUiToolboxCheckBoxState( window, "Check State" );

		ImUiToolboxLabelFormat( window, "C1: %d, C2: %d, C3: %d", checked[ 0u ], checked[ 1u ], checked[ 2u ] );

		ImUiWidgetEnd( checkLayout );
	}
}

static void ImUiToolboxSampleSlidersAndProgressBars( ImUiWindow* window )
{
	ImUiWidget* sliderLayout = ImUiWidgetBeginNamed( window, "sliders" );
	ImUiWidgetSetHStretch( sliderLayout, 1.0f );
	ImUiWidgetSetLayoutVerticalSpacing( sliderLayout, 10.0f );

	ImUiToolboxSliderMinMax( window, &s_toolboxContext.sliderValue1, 1.0f, 5.0f );

	const float value2 = ImUiToolboxSliderStateMinMax( window, 1.0f, 5.0f );

	ImUiToolboxLabelFormat( window, "V1: %.2f, V2: %.2f", s_toolboxContext.sliderValue1, value2 );

	ImUiToolboxProgressBarMinMax( window, s_toolboxContext.sliderValue1, 1.0f, 5.0f );
	ImUiToolboxProgressBar( window, -1.0f );

	ImUiWidgetEnd( sliderLayout );
}

static void ImUiToolboxSampleTextEdit( ImUiWindow* window )
{
	ImUiToolboxTextEditStateBuffer( window, 128u );
}

static void ImUiToolboxSampleScrollAndList( ImUiWindow* window )
{
	ImUiWidget* vLayout = ImUiWidgetBeginNamed( window, "vLayout" );
	ImUiWidgetSetHStretch( vLayout, 1.0f );
	ImUiWidgetSetLayoutVerticalSpacing( vLayout, 10.0f );

	ImUiToolboxLabel( window, "Item count:" );
	const float itemCount = ImUiToolboxSliderStateMinMaxDefault( window, 0.0f, 128.0f, 32.0f );

	const bool useList = ImUiToolboxCheckBoxStateDefault( window, "List", true );

	const size_t count = (size_t)itemCount;
	if( useList )
	{
		ImUiToolboxListContext list;
		ImUiToolboxListBegin( &list, window, 25.0f, count );
		ImUiWidgetSetMinSize( list.list, 200.0f, 200.0f );

		for( size_t i = ImUiToolboxListGetBeginIndex( &list ); i < ImUiToolboxListGetEndIndex( &list ); ++i )
		{
			ImUiWidget* item = ImUiToolboxListNextItem( &list );
			ImUiWidgetSetPadding( item, ImUiBorderCreateAll( 4.0f ) );

			ImUiWidget* label = ImUiToolboxLabelBeginFormat( window, "List Label %i", i );
			ImUiWidgetSetVAlign( label, 0.5f );
			ImUiToolboxLabelEnd( label );
		}

		ImUiToolboxListEnd( &list );
	}
	else
	{
		ImUiToolboxScrollAreaContext scrollArea;
		ImUiToolboxScrollAreaBegin( &scrollArea, window );
		ImUiWidgetSetMinSize( scrollArea.area, 200.0f, 200.0f );

		ImUiWidget* scrollLayout = ImUiWidgetBeginNamed( window, "scroll" );
		ImUiWidgetSetHStretch( scrollLayout, 1.0f );
		ImUiWidgetSetLayoutVerticalSpacing( scrollLayout, 4.0f );

		for( size_t i = 0; i < itemCount; ++i )
		{
			ImUiToolboxLabelFormat( window, "Scroll Label %i", i );
		}

		ImUiWidgetEnd( scrollLayout );
		ImUiToolboxScrollAreaEnd( &scrollArea );
	}

	ImUiWidgetEnd( vLayout );
}

static void ImUiToolboxSampleDropDown( ImUiWindow* window )
{
	const char* items[] =
	{
		"Item 1",
		"Item 2",
		"Item 3",
		"Item 4",
		"Item 5",
		"Item 6",
		"Item 7",
		"Item 8",
		"Item 9",
		"Item 10",
		"Item 11",
		"Item 12",
		"Item 13",
		"Item 14"
	};

	ImUiToolboxDropDown( window, items, IMUI_ARRAY_COUNT( items ), sizeof( const char* ) );
}

static void ImUiToolboxSamplePopup( ImUiWindow* window )
{
	ImUiWidget* button = ImUiToolboxButtonLabelBegin( window, "Open Popup" );

	ImUiTestPopupState* state = (ImUiTestPopupState*)ImUiWidgetAllocState( button, sizeof( *state ), IMUI_ID_TYPE( ImUiTestPopupState ) );

	if( ImUiToolboxButtonLabelEnd( button ) )
	{
		state->isOpen = !state->isOpen;
	}

	if( state->isOpen )
	{
		const char* buttons[] =
		{
			"Ok",
			"Cancel"
		};

		ImUiWindow* popup = ImUiToolboxPopupBegin( window );

		ImUiToolboxLabel( popup, "Hello from a popup window!" );

		const size_t clickedButton = ImUiToolboxPopupEndButtons( popup, buttons, IMUI_ARRAY_COUNT( buttons ) );
		if( clickedButton < IMUI_ARRAY_COUNT( buttons ) )
		{
			state->isOpen = false;
		}
	}
}

#ifndef IMUI_NO_SAMPLE_FRAMEWORK
bool ImUiToolboxSampleInitialize( ImUiContext* imui )
{
	if( !ImUiFrameworkFontCreate( &s_toolboxContext.font, &s_toolboxContext.fontTexture, "c:/windows/fonts/arial.ttf", 15.0f ) )
	{
		return false;
	}

	if( !ImUiFrameworkSkinCreate( &s_toolboxContext.skinRect, &s_toolboxContext.skinRectTexture, 32u, 8.0f, 128.0f, false ) )
	{
		return false;
	}

	if( !ImUiFrameworkSkinCreate( &s_toolboxContext.skinLine, &s_toolboxContext.skinLineTexture, 32u, 6.0f, 64.0f, true ) )
	{
		return false;
	}

	ImUiToolboxSampleSetConfig();

	return true;
}

void ImUiToolboxSampleShutdown( ImUiContext* imui )
{
	ImUiFrameworkFontDestroy( &s_toolboxContext.font, &s_toolboxContext.fontTexture );
	ImUiFrameworkSkinDestroy( &s_toolboxContext.skinRect, &s_toolboxContext.skinRectTexture );
	ImUiFrameworkSkinDestroy( &s_toolboxContext.skinLine, &s_toolboxContext.skinLineTexture );
}

static void ImUiToolboxSampleSetConfig()
{
	const ImUiColor textColor			= ImUiColorCreateWhite();
	const ImUiColor elementColor		= ImUiColorCreateFloat( 0.1f, 0.5f, 0.7f, 1.0f );
	const ImUiColor elementHoverColor	= ImUiColorCreateFloat( 0.3f, 0.7f, 0.9f, 1.0f );
	const ImUiColor elementClickedColor	= ImUiColorCreateFloat( 0.0f, 0.4f, 0.6f, 1.0f );
	const ImUiColor backgroundColor		= ImUiColorCreateFloat( 0.0f, 0.3f, 0.5f, 1.0f );
	const ImUiColor checkedColor		= ImUiColorCreateFloat( 1.0f, 0.5f, 0.7f, 1.0f );
	const ImUiColor textEditCursorColor	= ImUiColorCreateWhite();

	ImUiToolboxConfig config;
	config.colors[ ImUiToolboxColor_Text ]						= ImUiColorCreateFloat( 1.0f, 1.0f, 1.0f, 1.0f );
	config.colors[ ImUiToolboxColor_Button ]					= elementColor;
	config.colors[ ImUiToolboxColor_ButtonHover ]				= elementHoverColor;
	config.colors[ ImUiToolboxColor_ButtonClicked ]				= elementClickedColor;
	config.colors[ ImUiToolboxColor_ButtonText ]				= textColor;
	config.colors[ ImUiToolboxColor_CheckBox ]					= elementColor;
	config.colors[ ImUiToolboxColor_CheckBoxHover ]				= elementHoverColor;
	config.colors[ ImUiToolboxColor_CheckBoxClicked ]			= elementClickedColor;
	config.colors[ ImUiToolboxColor_CheckBoxChecked ]			= textColor;
	config.colors[ ImUiToolboxColor_SliderBackground ]			= backgroundColor;
	config.colors[ ImUiToolboxColor_SliderPivot ]				= elementColor;
	config.colors[ ImUiToolboxColor_SliderPivotHover ]			= elementHoverColor;
	config.colors[ ImUiToolboxColor_SliderPivotClicked ]		= checkedColor;
	config.colors[ ImUiToolboxColor_TextEditBackground ]		= backgroundColor;
	config.colors[ ImUiToolboxColor_TextEditText ]				= textColor;
	config.colors[ ImUiToolboxColor_TextEditCursor ]			= textEditCursorColor;
	config.colors[ ImUiToolboxColor_TextEditSelection ]			= elementColor;
	config.colors[ ImUiToolboxColor_ProgressBarBackground ]		= backgroundColor;
	config.colors[ ImUiToolboxColor_ProgressBarProgress ]		= elementColor;
	config.colors[ ImUiToolboxColor_ScrollAreaBarBackground ]	= backgroundColor;
	config.colors[ ImUiToolboxColor_ScrollAreaBarPivot ]		= elementColor;
	config.colors[ ImUiToolboxColor_ListItemHover ]				= elementHoverColor;
	config.colors[ ImUiToolboxColor_ListItemClicked ]			= elementClickedColor;
	config.colors[ ImUiToolboxColor_ListItemSelected ]			= elementColor;
	config.colors[ ImUiToolboxColor_DropDown ]					= backgroundColor;
	config.colors[ ImUiToolboxColor_DropDownText ]				= textColor;
	config.colors[ ImUiToolboxColor_DropDownIcon ]				= textColor;
	config.colors[ ImUiToolboxColor_DropDownHover ]				= elementHoverColor;
	config.colors[ ImUiToolboxColor_DropDownClicked ]			= elementClickedColor;
	config.colors[ ImUiToolboxColor_DropDownOpen ]				= elementColor;
	config.colors[ ImUiToolboxColor_DropDownList ]				= backgroundColor;
	config.colors[ ImUiToolboxColor_DropDownListItemText ]		= textColor;
	config.colors[ ImUiToolboxColor_DropDownListItemHover ]		= elementHoverColor;
	config.colors[ ImUiToolboxColor_DropDownListItemClicked ]	= elementClickedColor;
	config.colors[ ImUiToolboxColor_DropDownListItemSelected ]	= elementColor;
	config.colors[ ImUiToolboxColor_PopupBackground ]			= ImUiColorCreateFloat( 0.0f, 0.0f, 0.0f, 0.4f );
	config.colors[ ImUiToolboxColor_Popup ]						= backgroundColor;
	_STATIC_ASSERT( ImUiToolboxColor_MAX == 37 );

	config.skins[ ImUiToolboxSkin_Button ]						= s_toolboxContext.skinRect;
	config.skins[ ImUiToolboxSkin_CheckBox ]					= s_toolboxContext.skinRect;
	config.skins[ ImUiToolboxSkin_CheckBoxChecked ]				= s_toolboxContext.skinRect;
	config.skins[ ImUiToolboxSkin_SliderBackground ]			= s_toolboxContext.skinLine;
	config.skins[ ImUiToolboxSkin_SliderPivot ]					= s_toolboxContext.skinRect;
	config.skins[ ImUiToolboxSkin_TextEditBackground ]			= s_toolboxContext.skinRect;
	config.skins[ ImUiToolboxSkin_ProgressBarBackground ]		= s_toolboxContext.skinLine;
	config.skins[ ImUiToolboxSkin_ProgressBarProgress ]			= s_toolboxContext.skinRect;
	config.skins[ ImUiToolboxSkin_ScrollAreaBarBackground ]		= s_toolboxContext.skinRect;
	config.skins[ ImUiToolboxSkin_ScrollAreaBarPivot ]			= s_toolboxContext.skinRect;
	config.skins[ ImUiToolboxSkin_ListItem ]					= s_toolboxContext.skinRect;
	config.skins[ ImUiToolboxSkin_ListItemSelected ]			= s_toolboxContext.skinRect;
	config.skins[ ImUiToolboxSkin_DropDown ]					= s_toolboxContext.skinRect;
	config.skins[ ImUiToolboxSkin_DropDownList ]				= s_toolboxContext.skinRect;
	config.skins[ ImUiToolboxSkin_DropDownListItem ]			= s_toolboxContext.skinRect;
	config.skins[ ImUiToolboxSkin_Popup ]						= s_toolboxContext.skinRect;
	_STATIC_ASSERT( ImUiToolboxSkin_MAX == 16 );

	const ImUiImage image = { NULL, 16u, 16u };

	config.icons[ ImUiToolboxIcon_CheckBoxChecked ] = image;
	config.icons[ ImUiToolboxIcon_DropDownOpenIcon ] = image;
	config.icons[ ImUiToolboxIcon_DropDownCloseIcon ] = image;

	config.font						= s_toolboxContext.font;

	config.button.height			= 20.0f;
	config.button.padding			= ImUiBorderCreateAll( 8.0f );

	config.checkBox.size			= ImUiSizeCreateAll( 20.0f );
	config.checkBox.textSpacing		= 5.0f;

	config.slider.height			= 20.0f;
	config.slider.padding			= ImUiBorderCreateHorizontalVertical( 0.0f, 8.0f );
	config.slider.pivotSize			= ImUiSizeCreate( 12.0f, 20.0f );

	config.textEdit.height			= 25.0f;
	config.textEdit.padding			= ImUiBorderCreateAll( 4.0f );
	config.textEdit.cursorSize		= ImUiSizeCreate( 1.0f, 12.0f );
	config.textEdit.blinkTime		= 0.53f;

	config.progressBar.height		= 20.0f;
	config.progressBar.padding		= ImUiBorderCreateHorizontalVertical( 0.0f, 4.0f );

	config.scrollArea.barSize		= 8.0f;
	config.scrollArea.barSpacing	= 4.0f;
	config.scrollArea.barMinSize	= 20.0f;

	config.list.itemSpacing			= 4.0f;

	config.dropDown.height			= 25.0f;
	config.dropDown.padding			= ImUiBorderCreate( 0.0f, 4.0f, 0.0f, 4.0f );
	config.dropDown.listZOrder		= 10u;
	config.dropDown.maxListLength	= 8u;
	config.dropDown.itemPadding		= ImUiBorderCreate( 0.0f, 4.0f, 0.0f, 0.0f );
	config.dropDown.itemSize		= 25.0f;
	config.dropDown.itemSpacing		= 8.0f;

	config.popup.zOrder				= 20u;
	config.popup.padding			= ImUiBorderCreateAll( 8.0f );
	config.popup.buttonSpacing		= 4.0f;

	ImUiToolboxSetConfig( &config );
}
#endif
