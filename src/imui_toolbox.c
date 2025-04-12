#include "imui/imui_toolbox.h"

#include "imui_internal.h"
#include "imui_memory.h"
#include "imui_types.h"

#include <assert.h>
#include <math.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#if defined( _MSC_VER )
#	pragma warning(push)
#	pragma warning(disable : 4996)
#endif

static ImUiToolboxTheme s_theme;

struct ImUiToolboxScrollAreaState
{
	ImUiPos			offset;
	bool			wasPressedX;
	bool			wasPressedY;
	ImUiPos			pressPoint;
};

typedef struct ImUiToolboxTextEditState ImUiToolboxTextEditState;
struct ImUiToolboxTextEditState
{
	bool			hasFocus;

	ImUiPos			offset;
	bool			wasPressedX;
	bool			wasPressedY;
	ImUiPos			pressPoint;

	uint32			selectionStart;
	uint32			selectionEnd;
	uint32			cursorPos;
};

typedef struct ImUiToolboxListState ImUiToolboxListState;
struct ImUiToolboxListState
{
	uintsize		selectedIndex;
};

typedef struct ImUiToolboxDropDownState ImUiToolboxDropDownState;
struct ImUiToolboxDropDownState
{
	bool			isOpen;

	uintsize		selectedIndex;
};

static void ImUiToolboxListItemEndInternal( ImUiToolboxListContext* list );

static const ImUiToolboxThemeReflectionField s_themeReflectionFields[] =
{
	{ "Text/Color",							ImUiToolboxThemeReflectionType_Color,	offsetof( ImUiToolboxTheme, colors[ ImUiToolboxColor_Text ] ) },
	{ "Button/Color",						ImUiToolboxThemeReflectionType_Color,	offsetof( ImUiToolboxTheme, colors[ ImUiToolboxColor_Button ] ) },
	{ "Button/Hover Color",					ImUiToolboxThemeReflectionType_Color,	offsetof( ImUiToolboxTheme, colors[ ImUiToolboxColor_ButtonHover ] ) },
	{ "Button/Clicked Color",				ImUiToolboxThemeReflectionType_Color,	offsetof( ImUiToolboxTheme, colors[ ImUiToolboxColor_ButtonClicked ] ) },
	{ "Button/Text Color",					ImUiToolboxThemeReflectionType_Color,	offsetof( ImUiToolboxTheme, colors[ ImUiToolboxColor_ButtonText ] ) },
	{ "Check Box/Color",					ImUiToolboxThemeReflectionType_Color,	offsetof( ImUiToolboxTheme, colors[ ImUiToolboxColor_CheckBox ] ) },
	{ "Check Box/Hover Color",				ImUiToolboxThemeReflectionType_Color,	offsetof( ImUiToolboxTheme, colors[ ImUiToolboxColor_CheckBoxHover ] ) },
	{ "Check Box/Clicked Color",			ImUiToolboxThemeReflectionType_Color,	offsetof( ImUiToolboxTheme, colors[ ImUiToolboxColor_CheckBoxClicked ] ) },
	{ "Check Box/Checked Color",			ImUiToolboxThemeReflectionType_Color,	offsetof( ImUiToolboxTheme, colors[ ImUiToolboxColor_CheckBoxChecked ] ) },
	{ "Slider/Background Color",			ImUiToolboxThemeReflectionType_Color,	offsetof( ImUiToolboxTheme, colors[ ImUiToolboxColor_SliderBackground ] ) },
	{ "Slider/Pivot Color",					ImUiToolboxThemeReflectionType_Color,	offsetof( ImUiToolboxTheme, colors[ ImUiToolboxColor_SliderPivot ] ) },
	{ "Slider/Pivot Hover Color",			ImUiToolboxThemeReflectionType_Color,	offsetof( ImUiToolboxTheme, colors[ ImUiToolboxColor_SliderPivotHover ] ) },
	{ "Slider/Pivot Clicked Color",			ImUiToolboxThemeReflectionType_Color,	offsetof( ImUiToolboxTheme, colors[ ImUiToolboxColor_SliderPivotClicked ] ) },
	{ "Text Edit/Background Color",			ImUiToolboxThemeReflectionType_Color,	offsetof( ImUiToolboxTheme, colors[ ImUiToolboxColor_TextEditBackground ] ) },
	{ "Text Edit/Text Color",				ImUiToolboxThemeReflectionType_Color,	offsetof( ImUiToolboxTheme, colors[ ImUiToolboxColor_TextEditText ] ) },
	{ "Text Edit/Cursor Color",				ImUiToolboxThemeReflectionType_Color,	offsetof( ImUiToolboxTheme, colors[ ImUiToolboxColor_TextEditCursor ] ) },
	{ "Text Edit/Selection Color",			ImUiToolboxThemeReflectionType_Color,	offsetof( ImUiToolboxTheme, colors[ ImUiToolboxColor_TextEditSelection ] ) },
	{ "Progress Bar/Background Color",		ImUiToolboxThemeReflectionType_Color,	offsetof( ImUiToolboxTheme, colors[ ImUiToolboxColor_ProgressBarBackground ] ) },
	{ "Progress Bar/Progress Color",		ImUiToolboxThemeReflectionType_Color,	offsetof( ImUiToolboxTheme, colors[ ImUiToolboxColor_ProgressBarProgress ] ) },
	{ "Scroll Area/Bar Background Color",	ImUiToolboxThemeReflectionType_Color,	offsetof( ImUiToolboxTheme, colors[ ImUiToolboxColor_ScrollAreaBarBackground ] ) },
	{ "Scroll Area/Bar Pivot Color",		ImUiToolboxThemeReflectionType_Color,	offsetof( ImUiToolboxTheme, colors[ ImUiToolboxColor_ScrollAreaBarPivot ] ) },
	{ "List/Item/Hover Color",				ImUiToolboxThemeReflectionType_Color,	offsetof( ImUiToolboxTheme, colors[ ImUiToolboxColor_ListItemHover ] ) },
	{ "List/Item/Clicked Color",			ImUiToolboxThemeReflectionType_Color,	offsetof( ImUiToolboxTheme, colors[ ImUiToolboxColor_ListItemClicked ] ) },
	{ "List/Item/Selected Color",			ImUiToolboxThemeReflectionType_Color,	offsetof( ImUiToolboxTheme, colors[ ImUiToolboxColor_ListItemSelected ] ) },
	{ "Drop Down/Background Color",			ImUiToolboxThemeReflectionType_Color,	offsetof( ImUiToolboxTheme, colors[ ImUiToolboxColor_DropDown ] ) },
	{ "Drop Down/Text Color",				ImUiToolboxThemeReflectionType_Color,	offsetof( ImUiToolboxTheme, colors[ ImUiToolboxColor_DropDownText ] ) },
	{ "Drop Down/Icon Color",				ImUiToolboxThemeReflectionType_Color,	offsetof( ImUiToolboxTheme, colors[ ImUiToolboxColor_DropDownIcon ] ) },
	{ "Drop Down/Hover Color",				ImUiToolboxThemeReflectionType_Color,	offsetof( ImUiToolboxTheme, colors[ ImUiToolboxColor_DropDownHover ] ) },
	{ "Drop Down/Clicked Color",			ImUiToolboxThemeReflectionType_Color,	offsetof( ImUiToolboxTheme, colors[ ImUiToolboxColor_DropDownClicked ] ) },
	{ "Drop Down/Open Color",				ImUiToolboxThemeReflectionType_Color,	offsetof( ImUiToolboxTheme, colors[ ImUiToolboxColor_DropDownOpen ] ) },
	{ "Drop Down/List/Color",				ImUiToolboxThemeReflectionType_Color,	offsetof( ImUiToolboxTheme, colors[ ImUiToolboxColor_DropDownList ] ) },
	{ "Drop Down/Item/Text Color",			ImUiToolboxThemeReflectionType_Color,	offsetof( ImUiToolboxTheme, colors[ ImUiToolboxColor_DropDownItemText ] ) },
	{ "Drop Down/Item/Hover Color",			ImUiToolboxThemeReflectionType_Color,	offsetof( ImUiToolboxTheme, colors[ ImUiToolboxColor_DropDownItemHover ] ) },
	{ "Drop Down/Item/Clicked Color",		ImUiToolboxThemeReflectionType_Color,	offsetof( ImUiToolboxTheme, colors[ ImUiToolboxColor_DropDownItemClicked ] ) },
	{ "Drop Down/Item/Selected Color",		ImUiToolboxThemeReflectionType_Color,	offsetof( ImUiToolboxTheme, colors[ ImUiToolboxColor_DropDownItemSelected ] ) },
	{ "Popup/Background Color",				ImUiToolboxThemeReflectionType_Color,	offsetof( ImUiToolboxTheme, colors[ ImUiToolboxColor_PopupBackground ] ) },
	{ "Popup/Color",						ImUiToolboxThemeReflectionType_Color,	offsetof( ImUiToolboxTheme, colors[ ImUiToolboxColor_Popup ] ) },

	{ "Button/Skin",						ImUiToolboxThemeReflectionType_Skin,	offsetof( ImUiToolboxTheme, skins[ ImUiToolboxSkin_Button ] ) },
	{ "Check Box/Skin",						ImUiToolboxThemeReflectionType_Skin,	offsetof( ImUiToolboxTheme, skins[ ImUiToolboxSkin_CheckBox ] ) },
	{ "Check Box/Checked Skin",				ImUiToolboxThemeReflectionType_Skin,	offsetof( ImUiToolboxTheme, skins[ ImUiToolboxSkin_CheckBoxChecked ] ) },
	{ "Slider/Background Skin",				ImUiToolboxThemeReflectionType_Skin,	offsetof( ImUiToolboxTheme, skins[ ImUiToolboxSkin_SliderBackground ] ) },
	{ "Slider/Pivot Skin",					ImUiToolboxThemeReflectionType_Skin,	offsetof( ImUiToolboxTheme, skins[ ImUiToolboxSkin_SliderPivot ] ) },
	{ "Text Edit/Background Skin",			ImUiToolboxThemeReflectionType_Skin,	offsetof( ImUiToolboxTheme, skins[ ImUiToolboxSkin_TextEditBackground ] ) },
	{ "Progress Bar/Background Skin",		ImUiToolboxThemeReflectionType_Skin,	offsetof( ImUiToolboxTheme, skins[ ImUiToolboxSkin_ProgressBarBackground ] ) },
	{ "Progress Bar/Progress Skin",			ImUiToolboxThemeReflectionType_Skin,	offsetof( ImUiToolboxTheme, skins[ ImUiToolboxSkin_ProgressBarProgress ] ) },
	{ "Scroll Area/Bar Background Skin",	ImUiToolboxThemeReflectionType_Skin,	offsetof( ImUiToolboxTheme, skins[ ImUiToolboxSkin_ScrollAreaBarBackground ] ) },
	{ "Scroll Area/Bar Pivot Skin",			ImUiToolboxThemeReflectionType_Skin,	offsetof( ImUiToolboxTheme, skins[ ImUiToolboxSkin_ScrollAreaBarPivot ] ) },
	{ "List/Item/Skin",						ImUiToolboxThemeReflectionType_Skin,	offsetof( ImUiToolboxTheme, skins[ ImUiToolboxSkin_ListItem ] ) },
	{ "List/Item/Selected Skin",			ImUiToolboxThemeReflectionType_Skin,	offsetof( ImUiToolboxTheme, skins[ ImUiToolboxSkin_ItemSelected ] ) },
	{ "Drop Down/Skin",						ImUiToolboxThemeReflectionType_Skin,	offsetof( ImUiToolboxTheme, skins[ ImUiToolboxSkin_DropDown ] ) },
	{ "Drop Down/List/Skin",				ImUiToolboxThemeReflectionType_Skin,	offsetof( ImUiToolboxTheme, skins[ ImUiToolboxSkin_DropDownList ] ) },
	{ "Drop Down/Item/Skin",				ImUiToolboxThemeReflectionType_Skin,	offsetof( ImUiToolboxTheme, skins[ ImUiToolboxSkin_DropDownItem ] ) },
	{ "Popup/Skin",							ImUiToolboxThemeReflectionType_Skin,	offsetof( ImUiToolboxTheme, skins[ ImUiToolboxSkin_Popup ] ) },

	{ "Check Box/Checked Icon",				ImUiToolboxThemeReflectionType_Image,	offsetof( ImUiToolboxTheme, icons[ ImUiToolboxIcon_CheckBoxChecked ] ) },
	{ "Drop Down/Open Icon",				ImUiToolboxThemeReflectionType_Image,	offsetof( ImUiToolboxTheme, icons[ ImUiToolboxIcon_DropDownOpen ] ) },
	{ "Drop Down/Close Icon",				ImUiToolboxThemeReflectionType_Image,	offsetof( ImUiToolboxTheme, icons[ ImUiToolboxIcon_DropDownClose ] ) },

	{ "Text/Font",							ImUiToolboxThemeReflectionType_Font,	offsetof( ImUiToolboxTheme, font ) },

	{ "Button/Height",						ImUiToolboxThemeReflectionType_Float,	offsetof( ImUiToolboxTheme, button.height ) },
	{ "Button/Padding",						ImUiToolboxThemeReflectionType_Border,	offsetof( ImUiToolboxTheme, button.padding ) },

	{ "Check Box/Size",						ImUiToolboxThemeReflectionType_Size,	offsetof( ImUiToolboxTheme, checkBox.size ) },
	{ "Check Box/Text Spacing",				ImUiToolboxThemeReflectionType_Float,	offsetof( ImUiToolboxTheme, checkBox.textSpacing ) },

	{ "Slider/Height",						ImUiToolboxThemeReflectionType_Float,	offsetof( ImUiToolboxTheme, slider.height ) },
	{ "Slider/Padding",						ImUiToolboxThemeReflectionType_Border,	offsetof( ImUiToolboxTheme, slider.padding ) },
	{ "Slider/Pivot Size",					ImUiToolboxThemeReflectionType_Size,	offsetof( ImUiToolboxTheme, slider.pivotSize ) },

	{"Text Edit/height",					ImUiToolboxThemeReflectionType_Float,	offsetof( ImUiToolboxTheme, textEdit.height ) },
	{"Text Edit/padding",					ImUiToolboxThemeReflectionType_Border,	offsetof( ImUiToolboxTheme, textEdit.padding ) },
	{"Text Edit/cursorSize",				ImUiToolboxThemeReflectionType_Size,	offsetof( ImUiToolboxTheme, textEdit.cursorSize ) },
	{"Text Edit/blinkTime",					ImUiToolboxThemeReflectionType_Double,	offsetof( ImUiToolboxTheme, textEdit.blinkTime ) },

	{ "Progress Bar/Height",				ImUiToolboxThemeReflectionType_Float,	offsetof( ImUiToolboxTheme, progressBar.height ) },
	{ "Progress Bar/Padding",				ImUiToolboxThemeReflectionType_Border,	offsetof( ImUiToolboxTheme, progressBar.padding ) },

	{ "Scroll Area/Bar Size",				ImUiToolboxThemeReflectionType_Float,	offsetof( ImUiToolboxTheme, scrollArea.barSize ) },
	{ "Scroll Area/Bar Spacing",			ImUiToolboxThemeReflectionType_Float,	offsetof( ImUiToolboxTheme, scrollArea.barSpacing ) },
	{ "Scroll Area/Bar MinSize",			ImUiToolboxThemeReflectionType_Float,	offsetof( ImUiToolboxTheme, scrollArea.barMinSize ) },

	{ "List/Item Spacing",					ImUiToolboxThemeReflectionType_Float,	offsetof( ImUiToolboxTheme, list.itemSpacing ) },

	{ "Drop Down/Height",					ImUiToolboxThemeReflectionType_Float,	offsetof( ImUiToolboxTheme, dropDown.height ) },
	{ "Drop Down/Padding",					ImUiToolboxThemeReflectionType_Border,	offsetof( ImUiToolboxTheme, dropDown.padding ) },
	{ "Drop Down/List/ZOrder",				ImUiToolboxThemeReflectionType_UInt32,	offsetof( ImUiToolboxTheme, dropDown.listZOrder ) },
	{ "Drop Down/List/Margin",				ImUiToolboxThemeReflectionType_Border,	offsetof( ImUiToolboxTheme, dropDown.listMargin ) },
	{ "Drop Down/List/MaxLength",			ImUiToolboxThemeReflectionType_UInt32,	offsetof( ImUiToolboxTheme, dropDown.listMaxLength ) },
	{ "Drop Down/Item/Padding",				ImUiToolboxThemeReflectionType_Border,	offsetof( ImUiToolboxTheme, dropDown.itemPadding ) },
	{ "Drop Down/Item/Size",				ImUiToolboxThemeReflectionType_Float,	offsetof( ImUiToolboxTheme, dropDown.itemSize ) },
	{ "Drop Down/Item/Spacing",				ImUiToolboxThemeReflectionType_Float,	offsetof( ImUiToolboxTheme, dropDown.itemSpacing ) },

	{ "Popup/Z Order",						ImUiToolboxThemeReflectionType_UInt32,	offsetof( ImUiToolboxTheme, popup.zOrder ) },
	{ "Popup/Padding",						ImUiToolboxThemeReflectionType_Border,	offsetof( ImUiToolboxTheme, popup.padding ) },
	{ "Popup/Button Spacing",				ImUiToolboxThemeReflectionType_Float,	offsetof( ImUiToolboxTheme, popup.buttonSpacing ) },
};
static_assert( ImUiToolboxColor_MAX == 37, "more colors" );
static_assert( ImUiToolboxSkin_MAX == 16, "more skins" );
static_assert( ImUiToolboxIcon_MAX == 3, "more icons" );
static_assert( sizeof( ImUiToolboxTheme ) == 1256u, "theme changed" );

ImUiToolboxThemeReflection ImUiToolboxThemeReflectionGet()
{
	ImUiToolboxThemeReflection reflection;
	reflection.fields	= s_themeReflectionFields;
	reflection.count	= IMUI_ARRAY_COUNT( s_themeReflectionFields );
	return reflection;
}

ImUiToolboxTheme* ImUiToolboxThemeGet()
{
	return &s_theme;
}

void ImUiToolboxThemeFillDefault( ImUiToolboxTheme* theme, ImUiFont* font )
{
	const ImUiColor textColor			= ImUiColorCreateWhite();
	const ImUiColor elementColor		= ImUiColorCreateGray( 0xb2u );
	const ImUiColor elementHoverColor	= ImUiColorCreateGray( 0xe5u );
	const ImUiColor elementClickedColor	= ImUiColorCreateGray( 0x66u );
	const ImUiColor backgroundColor		= ImUiColorCreateGray( 0x4cu );
	const ImUiColor textEditCursorColor	= ImUiColorCreateBlack();

	theme->colors[ ImUiToolboxColor_Text ]						= textColor;
	theme->colors[ ImUiToolboxColor_Button ]					= elementColor;
	theme->colors[ ImUiToolboxColor_ButtonHover ]				= elementHoverColor;
	theme->colors[ ImUiToolboxColor_ButtonClicked ]				= elementClickedColor;
	theme->colors[ ImUiToolboxColor_ButtonText ]				= textColor;
	theme->colors[ ImUiToolboxColor_CheckBox ]					= elementColor;
	theme->colors[ ImUiToolboxColor_CheckBoxHover ]				= elementHoverColor;
	theme->colors[ ImUiToolboxColor_CheckBoxClicked ]			= elementClickedColor;
	theme->colors[ ImUiToolboxColor_CheckBoxChecked ]			= textColor;
	theme->colors[ ImUiToolboxColor_SliderBackground ]			= backgroundColor;
	theme->colors[ ImUiToolboxColor_SliderPivot ]				= elementColor;
	theme->colors[ ImUiToolboxColor_SliderPivotHover ]			= elementHoverColor;
	theme->colors[ ImUiToolboxColor_SliderPivotClicked ]		= elementClickedColor;
	theme->colors[ ImUiToolboxColor_TextEditBackground ]		= elementClickedColor;
	theme->colors[ ImUiToolboxColor_TextEditText ]				= textColor;
	theme->colors[ ImUiToolboxColor_TextEditCursor ]			= textEditCursorColor;
	theme->colors[ ImUiToolboxColor_TextEditSelection ]			= elementColor;
	theme->colors[ ImUiToolboxColor_ProgressBarBackground ]		= backgroundColor;
	theme->colors[ ImUiToolboxColor_ProgressBarProgress ]		= elementColor;
	theme->colors[ ImUiToolboxColor_ScrollAreaBarBackground ]	= backgroundColor;
	theme->colors[ ImUiToolboxColor_ScrollAreaBarPivot ]		= elementColor;
	theme->colors[ ImUiToolboxColor_ListItemHover ]				= elementHoverColor;
	theme->colors[ ImUiToolboxColor_ListItemClicked ]			= elementClickedColor;
	theme->colors[ ImUiToolboxColor_ListItemSelected ]			= elementColor;
	theme->colors[ ImUiToolboxColor_DropDown ]					= elementClickedColor;
	theme->colors[ ImUiToolboxColor_DropDownText ]				= textColor;
	theme->colors[ ImUiToolboxColor_DropDownIcon ]				= textColor;
	theme->colors[ ImUiToolboxColor_DropDownHover ]				= elementHoverColor;
	theme->colors[ ImUiToolboxColor_DropDownClicked ]			= elementClickedColor;
	theme->colors[ ImUiToolboxColor_DropDownOpen ]				= elementColor;
	theme->colors[ ImUiToolboxColor_DropDownList ]				= backgroundColor;
	theme->colors[ ImUiToolboxColor_DropDownItemText ]		= textColor;
	theme->colors[ ImUiToolboxColor_DropDownItemHover ]		= elementHoverColor;
	theme->colors[ ImUiToolboxColor_DropDownItemClicked ]	= elementClickedColor;
	theme->colors[ ImUiToolboxColor_DropDownItemSelected ]	= elementColor;
	theme->colors[ ImUiToolboxColor_PopupBackground ]			= ImUiColorCreateFloat( 0.0f, 0.0f, 0.0f, 0.2f );
	theme->colors[ ImUiToolboxColor_Popup ]						= backgroundColor;
	static_assert( ImUiToolboxColor_MAX == 37, "more colors" );

	const ImUiSkin skin = { IMUI_TEXTURE_HANDLE_INVALID };

	theme->skins[ ImUiToolboxSkin_Button ]						= skin;
	theme->skins[ ImUiToolboxSkin_CheckBox ]					= skin;
	theme->skins[ ImUiToolboxSkin_CheckBoxChecked ]				= skin;
	theme->skins[ ImUiToolboxSkin_SliderBackground ]			= skin;
	theme->skins[ ImUiToolboxSkin_SliderPivot ]					= skin;
	theme->skins[ ImUiToolboxSkin_TextEditBackground ]			= skin;
	theme->skins[ ImUiToolboxSkin_ProgressBarBackground ]		= skin;
	theme->skins[ ImUiToolboxSkin_ProgressBarProgress ]			= skin;
	theme->skins[ ImUiToolboxSkin_ScrollAreaBarBackground ]		= skin;
	theme->skins[ ImUiToolboxSkin_ScrollAreaBarPivot ]			= skin;
	theme->skins[ ImUiToolboxSkin_ListItem ]					= skin;
	theme->skins[ ImUiToolboxSkin_ItemSelected ]			= skin;
	theme->skins[ ImUiToolboxSkin_DropDown ]					= skin;
	theme->skins[ ImUiToolboxSkin_DropDownList ]				= skin;
	theme->skins[ ImUiToolboxSkin_DropDownItem ]			= skin;
	theme->skins[ ImUiToolboxSkin_Popup ]						= skin;
	static_assert( ImUiToolboxSkin_MAX == 16, "more skins" );

	const ImUiImage image = { IMUI_TEXTURE_HANDLE_INVALID, 22u, 22u, { 0.0f, 0.0f, 1.0f, 1.0f } };

	theme->icons[ ImUiToolboxIcon_CheckBoxChecked ]				= image;
	theme->icons[ ImUiToolboxIcon_DropDownOpen ]			= image;
	theme->icons[ ImUiToolboxIcon_DropDownClose ]			= image;
	static_assert( ImUiToolboxIcon_MAX == 3, "more icons" );

	theme->font						= font;

	theme->button.height			= 25.0f;
	theme->button.padding			= ImUiBorderCreate( 0.0f, 8.0f, 0.0f, 8.0f );

	theme->checkBox.size			= ImUiSizeCreateAll( 25.0f );
	theme->checkBox.textSpacing		= 8.0f;

	theme->slider.height			= 25.0f;
	theme->slider.padding			= ImUiBorderCreateHorizontalVertical( 5.0f, 0.0f );
	theme->slider.pivotSize			= ImUiSizeCreate( 10.0f, 25.0f );

	theme->textEdit.height			= 25.0f;
	theme->textEdit.padding			= ImUiBorderCreateAll( 2.0f );
	theme->textEdit.cursorSize		= ImUiSizeCreate( 1.0f, 21.0f );
	theme->textEdit.blinkTime		= 0.53f;

	theme->progressBar.height		= 25.0f;
	theme->progressBar.padding		= ImUiBorderCreateAll( 2.0f );

	theme->scrollArea.barSize		= 8.0f;
	theme->scrollArea.barSpacing	= 8.0f;
	theme->scrollArea.barMinSize	= 25.0f;

	theme->list.itemSpacing			= 8.0f;

	theme->dropDown.height			= 25.0f;
	theme->dropDown.padding			= ImUiBorderCreate( 0.0f, 4.0f, 0.0f, 0.0f );
	theme->dropDown.listZOrder		= 20u;
	theme->dropDown.listMaxLength	= 12u;
	theme->dropDown.listMargin		= ImUiBorderCreate( 0.0f, 0.0f, 0.0f, 0.0f );
	theme->dropDown.itemPadding		= ImUiBorderCreate( 0.0f, 4.0f, 0.0f, 0.0f );
	theme->dropDown.itemSize		= 25.0f;
	theme->dropDown.itemSpacing		= 8.0f;

	theme->popup.zOrder				= 10u;
	theme->popup.padding			= ImUiBorderCreateAll( 8.0f );
	theme->popup.buttonSpacing		= 4.0f;
}

void ImUiToolboxThemeSet( const ImUiToolboxTheme* theme )
{
	s_theme = *theme;
}

void ImUiToolboxSpacer( ImUiWindow* window, float width, float height )
{
	ImUiWidget* widget = ImUiWidgetBegin( window );
	ImUiWidgetSetFixedSizeFloat( widget, width, height );
	ImUiWidgetEnd( widget );
}

void ImUiToolboxStrecher( ImUiWindow* window, float horizontal, float vertical )
{
	ImUiWidget* widget = ImUiWidgetBegin( window );
	ImUiWidgetSetStretch( widget, horizontal, vertical );
	ImUiWidgetEnd( widget );
}

ImUiWidget* ImUiToolboxButtonBegin( ImUiWindow* window )
{
	ImUiWidget* button = ImUiWidgetBegin( window );
	ImUiWidgetSetFixedHeight( button, s_theme.button.height );
	ImUiWidgetSetPadding( button, s_theme.button.padding );

	ImUiWidgetInputState inputState;
	ImUiWidgetGetInputState( button, &inputState );

	ImUiColor color = s_theme.colors[ ImUiToolboxColor_Button ];
	if( inputState.wasPressed && inputState.isMouseDown )
	{
		color = s_theme.colors[ ImUiToolboxColor_ButtonClicked ];
	}
	else if( inputState.isMouseOver )
	{
		color = s_theme.colors[ ImUiToolboxColor_ButtonHover ];
	}

	ImUiWidgetDrawSkin( button, &s_theme.skins[ ImUiToolboxSkin_Button ], color );

	return button;
}

bool ImUiToolboxButtonEnd( ImUiWidget* button )
{
	ImUiWidgetEnd( button );

	ImUiWidgetInputState inputState;
	ImUiWidgetGetInputState( button, &inputState );

	return inputState.wasPressed && inputState.hasMouseReleased;
}

ImUiWidget* ImUiToolboxButtonLabelBegin( ImUiWindow* window, const char* text )
{
	ImUiWidget* buttonFrame = ImUiToolboxButtonBegin( window );

	ImUiWidget* buttonText = ImUiWidgetBegin( window );

	ImUiTextLayout* layout = ImUiTextLayoutCreateWidget( buttonText, s_theme.font, text );
	const ImUiSize textSize = ImUiTextLayoutGetSize( layout );
	ImUiWidgetSetAlign( buttonText, 0.5f, 0.5f );
	ImUiWidgetSetFixedSize( buttonText, textSize );

	if( layout )
	{
		ImUiWidgetDrawText( buttonText, layout, s_theme.colors[ ImUiToolboxColor_ButtonText ] );
	}

	ImUiWidgetEnd( buttonText );

	return buttonFrame;
}

ImUiWidget* ImUiToolboxButtonLabelBeginFormat( ImUiWindow* window, const char* format, ... )
{
	va_list args;
	va_start( args, format );
	ImUiWidget* button = ImUiToolboxButtonLabelBeginFormatArgs( window, format, args );
	va_end( args );

	return button;
}

ImUiWidget* ImUiToolboxButtonLabelBeginFormatArgs( ImUiWindow* window, const char* format, va_list args )
{
	char buffer[ 256u ];

	int length = vsnprintf( buffer, sizeof( buffer ), format, args );
	if( length < 0 )
	{
		return false;
	}
	else if( length >= sizeof( buffer ) )
	{
		char* headBuffer = (char*)ImUiMemoryAlloc( &window->context->allocator, length + 1u );
		if( !headBuffer )
		{
			return false;
		}

		length = vsnprintf( headBuffer, length + 1u, format, args );

		ImUiWidget* button = ImUiToolboxButtonLabelBegin( window, headBuffer );

		ImUiMemoryFree( &window->context->allocator, headBuffer );
		return button;
	}

	return ImUiToolboxButtonLabelBegin( window, buffer );
}

bool ImUiToolboxButtonLabel( ImUiWindow* window, const char* text )
{
	ImUiWidget* button = ImUiToolboxButtonLabelBegin( window, text );
	return ImUiToolboxButtonEnd( button );
}

bool ImUiToolboxButtonLabelFormat( ImUiWindow* window, const char* format, ... )
{
	va_list args;
	va_start( args, format );
	const bool result = ImUiToolboxButtonLabelFormatArgs( window, format, args );
	va_end( args );

	return result;
}

bool ImUiToolboxButtonLabelFormatArgs( ImUiWindow* window, const char* format, va_list args )
{
	ImUiWidget* button = ImUiToolboxButtonLabelBeginFormatArgs( window, format, args );
	return ImUiToolboxButtonEnd( button );
}

ImUiWidget* ImUiToolboxButtonIconBegin( ImUiWindow* window, const ImUiImage* icon, ImUiSize iconSize )
{
	IMUI_ASSERT( icon );

	ImUiWidget* buttonFrame = ImUiToolboxButtonBegin( window );

	ImUiWidget* buttonIcon = ImUiWidgetBegin( window );
	ImUiWidgetSetAlign( buttonIcon, 0.5f, 0.5f );
	ImUiWidgetSetFixedSize( buttonIcon, iconSize );

	ImUiWidgetDrawImage( buttonIcon, icon );

	ImUiWidgetEnd( buttonIcon );

	return buttonFrame;
}

bool ImUiToolboxButtonIcon( ImUiWindow* window, const ImUiImage* icon )
{
	return ImUiToolboxButtonIconSize( window, icon, ImUiSizeCreateImage( icon ) );
}

bool ImUiToolboxButtonIconSize( ImUiWindow* window, const ImUiImage* icon, ImUiSize iconSize )
{
	ImUiWidget* button = ImUiToolboxButtonIconBegin( window, icon, iconSize );
	return ImUiToolboxButtonEnd( button );
}

ImUiWidget* ImUiToolboxCheckBoxBegin( ImUiWindow* window )
{
	ImUiWidget* checkBoxFrame = ImUiWidgetBegin( window );
	ImUiWidgetSetPadding( checkBoxFrame, ImUiBorderCreate( 0.0f, s_theme.checkBox.size.width + s_theme.checkBox.textSpacing, 0.0f, 0.0f ) );
	ImUiWidgetSetFixedHeight( checkBoxFrame, s_theme.checkBox.size.height );
	ImUiWidgetSetVAlign( checkBoxFrame, 0.5f );

	return checkBoxFrame;
}

bool ImUiToolboxCheckBoxEnd( ImUiWidget* checkBox, bool* checked, const char* text )
{
	ImUiWidgetInputState inputState;
	ImUiWidgetGetInputState( checkBox, &inputState );

	ImUiColor color = s_theme.colors[ ImUiToolboxColor_CheckBox ];
	if( inputState.wasPressed && inputState.isMouseDown )
	{
		color = s_theme.colors[ ImUiToolboxColor_CheckBoxClicked ];
	}
	else if( inputState.isMouseOver )
	{
		color = s_theme.colors[ ImUiToolboxColor_CheckBoxHover ];
	}

	const float checkBackgroundY = (ImUiWidgetGetSizeHeight( checkBox ) / 2.0f) - (s_theme.checkBox.size.height / 2.0f);
	const ImUiRect checkBackgroundRect = ImUiRectCreatePosSize( ImUiPosCreate( 0.0f, checkBackgroundY ), s_theme.checkBox.size );
	ImUiWidgetDrawPartialSkin( checkBox, checkBackgroundRect, &s_theme.skins[ ImUiToolboxSkin_CheckBox ], color );

	if( *checked )
	{
		const ImUiRect checkIconRect = ImUiRectCreateCenterPosSize( ImUiRectGetCenter( checkBackgroundRect ), ImUiSizeCreateImage( &s_theme.icons[ ImUiToolboxIcon_CheckBoxChecked ] ) );
		ImUiWidgetDrawPartialImageColor( checkBox, checkIconRect, &s_theme.icons[ ImUiToolboxIcon_CheckBoxChecked ], s_theme.colors[ ImUiToolboxColor_CheckBoxChecked ] );
	}

	ImUiWidget* checkBoxText = ImUiWidgetBegin( ImUiWidgetGetWindow( checkBox ) );

	ImUiTextLayout* layout = ImUiTextLayoutCreateWidget( checkBoxText, s_theme.font, text );
	const ImUiSize textSize = ImUiTextLayoutGetSize( layout );
	ImUiWidgetSetFixedSize( checkBoxText, textSize );
	ImUiWidgetSetVAlign( checkBoxText, 0.5f );

	if( layout )
	{
		ImUiWidgetDrawText( checkBoxText, layout, s_theme.colors[ ImUiToolboxColor_Text ] );
	}

	ImUiWidgetEnd( checkBoxText );

	ImUiWidgetEnd( checkBox );

	if( inputState.wasPressed && inputState.hasMouseReleased )
	{
		*checked = !*checked;
		return true;
	}

	return false;
}

bool ImUiToolboxCheckBox( ImUiWindow* window, bool* checked, const char* text )
{
	ImUiWidget* checkBox = ImUiToolboxCheckBoxBegin( window );
	return ImUiToolboxCheckBoxEnd( checkBox, checked, text);
}

bool ImUiToolboxCheckBoxState( ImUiWindow* window, const char* text )
{
	return ImUiToolboxCheckBoxStateDefault( window, text, false );
}

bool ImUiToolboxCheckBoxStateDefault( ImUiWindow* window, const char* text, bool defaultValue )
{
	ImUiWidget* checkBox = ImUiToolboxCheckBoxBegin( window );

	bool isNew;
	bool* checked = (bool*)ImUiWidgetAllocStateNew( checkBox, sizeof( bool ), IMUI_ID_STR( "check box" ), &isNew);
	if( isNew )
	{
		*checked = defaultValue;
	}

	ImUiToolboxCheckBoxEnd( checkBox, checked, text );
	return *checked;
}

ImUiWidget* ImUiToolboxLabelBegin( ImUiWindow* window, const char* text )
{
	return ImUiToolboxLabelBeginColor( window, text, s_theme.colors[ ImUiToolboxColor_Text ] );
}

ImUiWidget* ImUiToolboxLabelBeginColor( ImUiWindow* window, const char* text, ImUiColor color )
{
	ImUiWidget* label = ImUiWidgetBegin( window );

	ImUiTextLayout* layout = ImUiTextLayoutCreateWidget( label, s_theme.font, text );
	const ImUiSize textSize = ImUiTextLayoutGetSize( layout );
	ImUiWidgetSetFixedSize( label, textSize );
	ImUiWidgetSetVAlign( label, 0.5f );

	if( layout )
	{
		ImUiWidgetDrawText( label, layout, color );
	}

	return label;
}

ImUiWidget* ImUiToolboxLabelBeginFormat( ImUiWindow* window, const char* format, ... )
{
	va_list args;
	va_start( args, format );
	ImUiWidget* label = ImUiToolboxLabelBeginFormatArgs( window, format, args );
	va_end( args );
	return label;
}

ImUiWidget* ImUiToolboxLabelBeginFormatArgs( ImUiWindow* window, const char* format, va_list args )
{
	char buffer[ 256u ];

	int length = vsnprintf( buffer, sizeof( buffer ), format, args );
	if( length < 0 )
	{
		return NULL;
	}
	else if( length >= sizeof( buffer ) )
	{
		char* headBuffer = (char*)ImUiMemoryAlloc( &window->context->allocator, length + 1u );
		if( !headBuffer )
		{
			return NULL;
		}

		length = vsnprintf( headBuffer, length + 1u, format, args );

		ImUiWidget* label = ImUiToolboxLabelBegin( window, headBuffer );

		ImUiMemoryFree( &window->context->allocator, headBuffer );
		return label;
	}

	return ImUiToolboxLabelBegin( window, buffer );
}

void ImUiToolboxLabelEnd( ImUiWidget* label )
{
	ImUiWidgetEnd( label );
}

void ImUiToolboxLabel( ImUiWindow* window, const char* text )
{
	ImUiWidget* label = ImUiToolboxLabelBegin( window, text );
	ImUiToolboxLabelEnd( label );
}

void ImUiToolboxLabelColor( ImUiWindow* window, const char* text, ImUiColor color )
{
	ImUiWidget* label = ImUiToolboxLabelBeginColor( window, text, color );
	ImUiToolboxLabelEnd( label );
}

void ImUiToolboxLabelFormat( ImUiWindow* window, const char* format, ... )
{
	va_list args;
	va_start( args, format );
	ImUiToolboxLabelFormatArgs( window, format, args );
	va_end( args );
}

void ImUiToolboxLabelFormatArgs( ImUiWindow* window, const char* format, va_list args )
{
	ImUiWidget* label = ImUiToolboxLabelBeginFormatArgs( window, format, args );
	ImUiToolboxLabelEnd( label );
}

ImUiWidget* ImUiToolboxImageBegin( ImUiWindow* window, ImUiSize imgSize )
{
	ImUiWidget* imgWidget = ImUiWidgetBegin( window );
	ImUiWidgetSetFixedSize( imgWidget, imgSize );

	return imgWidget;
}

void ImUiToolboxImageEnd( ImUiWidget* imgWidget, const ImUiImage* img )
{
	ImUiWidgetDrawImage( imgWidget, img );

	ImUiWidgetEnd( imgWidget );
}

void ImUiToolboxImage( ImUiWindow* window, const ImUiImage* img )
{
	ImUiToolboxImageSize( window, img, ImUiSizeCreateImage( img ) );
}

void ImUiToolboxImageSize( ImUiWindow* window, const ImUiImage* img, ImUiSize imgSize )
{
	ImUiWidget* imgWidget = ImUiToolboxImageBegin( window, imgSize );
	ImUiToolboxImageEnd( imgWidget, img );
}

ImUiWidget* ImUiToolboxSliderBegin( ImUiWindow* window )
{
	ImUiWidget* slider = ImUiWidgetBegin( window );
	ImUiWidgetSetHStretch( slider, 1.0f );
	ImUiWidgetSetPadding( slider, s_theme.slider.padding );
	ImUiWidgetSetFixedHeight( slider, s_theme.slider.height );

	return slider;
}

bool ImUiToolboxSliderEnd( ImUiWidget* slider, float* value, float min, float max )
{
	ImUiWidgetInputState frameInputState;
	ImUiWidgetGetInputState( slider, &frameInputState );

	ImUiWidgetDrawSkin( slider, &s_theme.skins[ ImUiToolboxSkin_SliderBackground ], s_theme.colors[ ImUiToolboxColor_SliderBackground ] );

	ImUiWidget* sliderPivot = ImUiWidgetBegin( ImUiWidgetGetWindow( slider ) );
	ImUiWidgetSetFixedSize( sliderPivot, s_theme.slider.pivotSize );

	const float normalizedValue = (*value - min) / (max - min);
	ImUiWidgetSetHAlign( sliderPivot, normalizedValue );

	ImUiWidgetInputState inputState;
	ImUiWidgetGetInputState( sliderPivot, &inputState );

	ImUiColor color = s_theme.colors[ ImUiToolboxColor_SliderPivot ];
	if( frameInputState.wasPressed )
	{
		color = s_theme.colors[ ImUiToolboxColor_SliderPivotClicked ];
	}
	else if( inputState.isMouseOver )
	{
		color = s_theme.colors[ ImUiToolboxColor_SliderPivotHover ];
	}

	bool changed = false;

	if( frameInputState.wasPressed )
	{
		const ImUiRect sliderInnerRect = ImUiWidgetGetInnerRect( slider );

		const float mouseValueNorm		= (frameInputState.relativeMousePos.x - s_theme.slider.pivotSize.width) / (sliderInnerRect.size.width - s_theme.slider.pivotSize.width);
		const float mouseValueNormClamp	= mouseValueNorm > 1.0f ? 1.0f : (mouseValueNorm < 0.0f ? 0.0f : mouseValueNorm);
		IMUI_ASSERT( mouseValueNormClamp >= 0.0f && mouseValueNormClamp <= 1.0f );
		const float mouseValue			= (mouseValueNormClamp * (max - min)) + min;

		*value = mouseValue;
		changed = true;
	}

	ImUiWidgetDrawSkin( sliderPivot, &s_theme.skins[ ImUiToolboxSkin_SliderPivot ], color );

	ImUiWidgetEnd( sliderPivot );

	ImUiWidgetEnd( slider );

	return changed;
}

bool ImUiToolboxSlider( ImUiWindow* window, float* value )
{
	return ImUiToolboxSliderMinMax( window, value, 0.0f, 1.0f );
}

bool ImUiToolboxSliderMinMax( ImUiWindow* window, float* value, float min, float max )
{
	ImUiWidget* sliderFrame = ImUiToolboxSliderBegin( window );
	return ImUiToolboxSliderEnd( sliderFrame, value, min, max );
}

float ImUiToolboxSliderState( ImUiWindow* window )
{
	return ImUiToolboxSliderStateMinMaxDefault( window, 0.0f, 1.0f, 0.0f );
}

float ImUiToolboxSliderStateDefault( ImUiWindow* window, float defaultValue )
{
	return ImUiToolboxSliderStateMinMaxDefault( window, 0.0f, 1.0f, defaultValue );
}

float ImUiToolboxSliderStateMinMax( ImUiWindow* window, float min, float max )
{
	return ImUiToolboxSliderStateMinMaxDefault( window, min, max, min );
}

float ImUiToolboxSliderStateMinMaxDefault( ImUiWindow* window, float min, float max, float defaultValue )
{
	IMUI_ASSERT( min <= max );
	IMUI_ASSERT( defaultValue >= min );
	IMUI_ASSERT( defaultValue <= max );

	ImUiWidget* sliderFrame = ImUiToolboxSliderBegin( window );

	bool isNew;
	float* value = (float*)ImUiWidgetAllocStateNew( sliderFrame, sizeof( float ), IMUI_ID_STR( "slider" ), &isNew );
	if( isNew )
	{
		*value = defaultValue;
	}

	ImUiToolboxSliderEnd( sliderFrame, value, min, max );
	return *value;
}

ImUiWidget* ImUiToolboxTextEditBegin( ImUiWindow* window )
{
	ImUiWidget* textEditFrame = ImUiWidgetBegin( window );
	ImUiWidgetSetHStretch( textEditFrame, 1.0f );
	ImUiWidgetSetPadding( textEditFrame, s_theme.textEdit.padding );
	ImUiWidgetSetFixedHeight( textEditFrame, s_theme.textEdit.height );

	ImUiWidgetDrawSkin( textEditFrame, &s_theme.skins[ ImUiToolboxSkin_TextEditBackground ], s_theme.colors[ ImUiToolboxColor_TextEditBackground ] );

	return textEditFrame;
}

bool ImUiToolboxTextEditEnd( ImUiWidget* textEdit, char* buffer, size_t bufferSize, size_t* textLength )
{
	IMUI_ASSERT( buffer );
	IMUI_ASSERT( bufferSize > 0u );

	ImUiContext* imui = ImUiWidgetGetContext( textEdit );

	uintsize textLengthInternal;
	if( textLength )
	{
		textLengthInternal = *textLength;
	}
	else
	{
		textLengthInternal = strlen( buffer );
	}

	ImUiWidgetInputState inputState;
	ImUiWidgetGetInputState( textEdit, &inputState );

	if( inputState.isMouseOver )
	{
		ImUiInputSetMouseCursor( imui, ImUiInputMouseCursor_IBeam );
	}

	ImUiWidget* text = ImUiWidgetBegin( ImUiWidgetGetWindow( textEdit ) );
	ImUiWidgetSetVAlign( text, 0.5f );

	bool isNew;
	ImUiToolboxTextEditState* state = (ImUiToolboxTextEditState*)ImUiWidgetAllocStateNew( text, sizeof( *state ), IMUI_ID_STR( "text edit" ), &isNew );

	const ImUiRect textEditRect = ImUiWidgetGetRect( textEdit );
	const ImUiRect textEditInnerRect = ImUiWidgetGetInnerRect( textEdit );

	ImUiTextLayout* layout = ImUiTextLayoutCreateWidget( text, s_theme.font, buffer );
	const ImUiSize textSize = ImUiTextLayoutGetSize( layout );
	ImUiWidgetSetFixedSize( text, textSize );

	if( ImUiInputHasMouseButtonPressed( imui, ImUiInputMouseButton_Left ) )
	{
		state->hasFocus = inputState.hasMousePressed;
	}

	bool changed = false;
	if( state->hasFocus )
	{
		const ImUiInputShortcut shortcut = ImUiInputGetShortcut( imui );
		const uint32 mods = ImUiInputGetKeyModifiers( imui );

		if( shortcut == ImUiInputShortcut_SelectAll )
		{
			state->selectionStart	= 0u;
			state->selectionEnd		= (uint32)textLengthInternal;
		}

		const char* textInput = ImUiInputGetText( imui );
		if( shortcut == ImUiInputShortcut_Paste )
		{
			textInput = ImUiInputGetPasteText( imui );
		}

		if( textInput )
		{
			const uintsize remainingSize	= bufferSize - textLengthInternal - 1u;
			const uintsize inputLength		= strlen( textInput );
			const uintsize newSize			= IMUI_MIN( inputLength, remainingSize );

			if( state->selectionStart != state->selectionEnd )
			{
				memmove( buffer + state->selectionStart, buffer + state->selectionEnd, textLengthInternal - state->selectionEnd );
				state->cursorPos = state->selectionStart;

				textLengthInternal -= state->selectionEnd - state->selectionStart;

				state->selectionStart	= 0u;
				state->selectionEnd		= 0u;
			}

			if( state->cursorPos != textLengthInternal )
			{
				memmove( buffer + state->cursorPos + newSize, buffer + state->cursorPos, textLengthInternal - state->cursorPos );
			}

			memcpy( buffer + state->cursorPos, textInput, newSize );
			textLengthInternal += newSize;
			buffer[ textLengthInternal ] = '\0';

			state->cursorPos += (uint32)newSize;

			changed = true;
		}

		if( ImUiInputHasKeyPressed( imui, ImUiInputKey_Backspace ) )
		{
			if( state->selectionStart != state->selectionEnd )
			{
				memmove( buffer + state->selectionStart, buffer + state->selectionEnd, textLengthInternal - state->selectionEnd );
				state->cursorPos = state->selectionStart;

				textLengthInternal -= state->selectionEnd - state->selectionStart;
				buffer[ textLengthInternal ] = '\0';

				state->selectionStart	= 0u;
				state->selectionEnd		= 0u;

				changed = true;
			}
			else if( state->cursorPos > 0u )
			{
				for( uintsize i = state->cursorPos - 1u; i < textLengthInternal; ++i )
				{
					buffer[ i ] = buffer[ i + 1u ];
				}

				textLengthInternal--;
				state->cursorPos--;

				changed = true;
			}
		}
		else if( ImUiInputHasKeyPressed( imui, ImUiInputKey_Delete ) )
		{
			if( state->selectionStart != state->selectionEnd )
			{
				memmove( buffer + state->selectionStart, buffer + state->selectionEnd, textLengthInternal - state->selectionEnd );
				state->cursorPos = state->selectionStart;

				textLengthInternal -= state->selectionEnd - state->selectionStart;
				buffer[ textLengthInternal ] = '\0';

				state->selectionStart	= 0u;
				state->selectionEnd		= 0u;

				changed = true;
			}
			else if( textLengthInternal > state->cursorPos )
			{
				for( uintsize i = state->cursorPos; i < textLengthInternal; ++i )
				{
					buffer[ i ] = buffer[ i + 1u ];
				}

				textLengthInternal--;

				changed = true;
			}
		}

		sint32 nextCursorPos = (sint32)state->cursorPos;
		const bool leftPressed = ImUiInputHasKeyPressed( imui, ImUiInputKey_Left );
		const bool shiftPressed = (mods & (ImUiInputModifier_LeftShift | ImUiInputModifier_RightShift)) != 0;
		if( leftPressed ||
			ImUiInputHasKeyPressed( imui, ImUiInputKey_Right ) )
		{
			const sint32 direction = leftPressed ? -1 : 1;
			if( mods & (ImUiInputModifier_LeftCtrl | ImUiInputModifier_RightCtrl) )
			{
				nextCursorPos += direction;

				for( ; nextCursorPos > 0 && nextCursorPos <= (sint32)textLengthInternal; nextCursorPos += direction )
				{
					const char c = buffer[ nextCursorPos ];
					if( (c>= 'a' && c <= 'z') ||
						(c>= 'A' && c <= 'Z') ||
						(c>= '0' && c <= '9') )
					{
						continue;
					}

					break;
				}
			}
			else if( !shiftPressed &&
					 state->selectionStart != state->selectionEnd )
			{
				nextCursorPos			= leftPressed ? state->selectionStart : state->selectionEnd;
				state->selectionStart	= 0u;
				state->selectionEnd		= 0u;
			}
			else
			{
				nextCursorPos += direction;
			}
		}
		else if( shortcut == ImUiInputShortcut_Home )
		{
			nextCursorPos = 0u;
		}
		else if( shortcut == ImUiInputShortcut_End )
		{
			nextCursorPos = (sint32)textLengthInternal;
		}

		if( inputState.wasPressed )
		{
			ImUiWidgetInputState textInputState;
			ImUiWidgetGetInputState( text, &textInputState );
			nextCursorPos = (sint32)ImUiTextLayoutFindGlyphIndex( layout, textInputState.relativeMousePos );

			if( inputState.hasMousePressed )
			{
				state->selectionStart	= 0u;
				state->selectionEnd		= 0u;
				state->cursorPos		= nextCursorPos;
			}
		}

		if( nextCursorPos != (sint32)state->cursorPos )
		{
			nextCursorPos = IMUI_MAX( nextCursorPos, 0 );
			nextCursorPos = IMUI_MIN( nextCursorPos, (sint32)textLengthInternal );

			const uint32 nextCursorPosU = (uint32)nextCursorPos;
			if( shiftPressed ||
				inputState.wasPressed )
			{
				if( state->selectionStart == state->selectionEnd )
				{
					state->selectionStart	= IMUI_MIN( state->cursorPos, nextCursorPosU );
					state->selectionEnd		= IMUI_MAX( state->cursorPos, nextCursorPosU );
				}
				else if( state->selectionStart == state->cursorPos )
				{
					if( nextCursorPosU > state->selectionEnd )
					{
						state->selectionStart	= state->selectionEnd;
						state->selectionEnd		= nextCursorPosU;
					}
					else
					{
						state->selectionStart	= nextCursorPosU;
					}
				}
				else
				{
					if( nextCursorPosU < state->selectionStart )
					{
						state->selectionEnd		= state->selectionStart;
						state->selectionStart	= nextCursorPosU;
					}
					else
					{
						state->selectionEnd		= nextCursorPosU;
					}
				}
			}
			else
			{
				state->selectionStart	= 0u;
				state->selectionEnd		= 0u;
			}

			state->cursorPos = nextCursorPosU;
		}

		if( state->selectionStart != state->selectionEnd )
		{
			if( shortcut == ImUiInputShortcut_Copy )
			{
				ImUiInputSetCopyText( imui, buffer + state->selectionStart, state->selectionEnd - state->selectionStart );
			}

			const ImUiPos startPos			= ImUiTextLayoutGetGlyphPos( layout, state->selectionStart );
			const ImUiPos endPos			= ImUiTextLayoutGetGlyphPos( layout, state->selectionEnd );

			const ImUiRect selection = ImUiRectCreate(
				(textEditInnerRect.pos.x - textEditRect.pos.x) + startPos.x,
				textEditInnerRect.pos.y - textEditRect.pos.y,
				endPos.x - startPos.x,
				textEditInnerRect.size.height
			);
			ImUiWidgetDrawPartialColor( textEdit, selection, s_theme.colors[ ImUiToolboxColor_TextEditSelection ] );
		}
	}

	if( layout )
	{
		ImUiWidgetDrawText( text, layout, s_theme.colors[ ImUiToolboxColor_TextEditText ] );
	}

	if( state->hasFocus )
	{
		const double blinkValue	= fmod( ImUiWidgetGetTime( textEdit ), s_theme.textEdit.blinkTime * 2.0 );
		const bool blink		= blinkValue > s_theme.textEdit.blinkTime;
		if( blink )
		{
			//const ImUiPos cursorPos			= ImUiPosAdd( ImUiTextLayoutGetGlyphPos( layout, state->cursorPos ), s_config.textEdit.padding.left, s_config.textEdit.padding.top );
			//const ImUiRect cursorRect		= ImUiRectCreatePosSize( cursorPos, s_config.textEdit.cursorSize );

			const ImUiPos cursorPos			= ImUiTextLayoutGetGlyphPos( layout, state->cursorPos );
			const ImUiPos cursorPosTop		= ImUiPosCreate( (textEditInnerRect.pos.x - textEditRect.pos.x) + cursorPos.x, s_theme.textEdit.padding.top );
			const ImUiPos cursorPosBottom	= ImUiPosCreate( cursorPosTop.x, cursorPosTop.y + textEditInnerRect.size.height );
			ImUiWidgetDrawLine( textEdit, cursorPosTop, cursorPosBottom, s_theme.colors[ ImUiToolboxColor_TextEditCursor ] );
		}
	}

	ImUiWidgetEnd( text );

	ImUiWidgetEnd( textEdit );

	if( textLength )
	{
		*textLength = textLengthInternal;
	}

	return changed;
}

bool ImUiToolboxTextEdit( ImUiWindow* window, char* buffer, size_t bufferSize, size_t* textLength )
{
	ImUiWidget* textEdit = ImUiToolboxTextEditBegin( window );
	return ImUiToolboxTextEditEnd( textEdit, buffer, bufferSize, textLength );
}

const char* ImUiToolboxTextEditStateBuffer( ImUiWindow* window, size_t bufferSize )
{
	return ImUiToolboxTextEditStateBufferDefault( window, bufferSize, NULL );
}

const char* ImUiToolboxTextEditStateBufferDefault( ImUiWindow* window, size_t bufferSize, const char* defaultValue )
{
	ImUiWidget* textEdit = ImUiToolboxTextEditBegin( window );

	bool isNew;
	char* buffer = (char*)ImUiWidgetAllocStateNew( textEdit, bufferSize, IMUI_ID_STR( "text buffer" ), &isNew );
	if( isNew && defaultValue )
	{
		strncpy( buffer, defaultValue, bufferSize );
	}

	ImUiToolboxTextEditEnd( textEdit, buffer, bufferSize, NULL );
	return buffer;
}

void ImUiToolboxProgressBar( ImUiWindow* window, float value )
{
	ImUiToolboxProgressBarMinMax( window, value, 0.0f, 1.0f );
}

void ImUiToolboxProgressBarMinMax( ImUiWindow* window, float value, float min, float max )
{
	ImUiWidget* progressBar = ImUiWidgetBegin( window );
	ImUiWidgetSetHStretch( progressBar, 1.0f );
	ImUiWidgetSetPadding( progressBar, s_theme.progressBar.padding );
	ImUiWidgetSetFixedHeight( progressBar, s_theme.progressBar.height );

	ImUiWidgetDrawSkin( progressBar, &s_theme.skins[ ImUiToolboxSkin_ProgressBarBackground ], s_theme.colors[ ImUiToolboxColor_ProgressBarBackground ] );

	const ImUiRect barRect = ImUiWidgetGetInnerRect( progressBar );

	ImUiRect progressRect;
	if( value < min )
	{
		const double time		= ImUiWindowGetTime( window );
		const float cosv		= ((float)cos( time * 8.0 ) * 0.15f) + 0.15f;
		const float sinv		= ((float)sin( time * 4.0 ) * 0.5f) + 0.5f;
		const float width		= ceilf( barRect.size.width * (0.1f + cosv) );
		const float margin		= floorf( sinv * (barRect.size.width - width) );

		progressRect = ImUiRectCreate(
			margin + s_theme.progressBar.padding.left,
			s_theme.progressBar.padding.top,
			width,
			barRect.size.height
		);
	}
	else
	{
		const float valueNorm	= (value - min) / (max - min);
		const float width		= ceilf( barRect.size.width * valueNorm );

		progressRect = ImUiRectCreate(
			s_theme.progressBar.padding.left,
			s_theme.progressBar.padding.top,
			width,
			barRect.size.height
		);
	}

	ImUiWidgetDrawPartialSkin( progressBar, progressRect, &s_theme.skins[ ImUiToolboxSkin_ProgressBarProgress ], s_theme.colors[ ImUiToolboxColor_ProgressBarProgress ] );

	ImUiWidgetEnd( progressBar );
}

void ImUiToolboxScrollAreaBegin( ImUiToolboxScrollAreaContext* scrollArea, ImUiWindow* window )
{
	scrollArea->horizontalSpacing	= false;
	scrollArea->verticalSpacing		= false;
	scrollArea->area				= ImUiWidgetBegin( window );
	scrollArea->state				= (ImUiToolboxScrollAreaState*)ImUiWidgetAllocState( scrollArea->area, sizeof( *scrollArea->state ), IMUI_ID_STR( "scroll area" ) );
	scrollArea->content				= ImUiWidgetBegin( window );

	ImUiWidgetSetStretch( scrollArea->content, 1.0f, 1.0f );
	ImUiWidgetSetLayoutScroll( scrollArea->content, scrollArea->state->offset.x, scrollArea->state->offset.y );
}

void ImUiToolboxScrollAreaEnableSpacing( ImUiToolboxScrollAreaContext* scrollArea, bool horizontal, bool vertical )
{
	scrollArea->horizontalSpacing	= horizontal;
	scrollArea->verticalSpacing		= vertical;
}

void ImUiToolboxScrollAreaEnd( ImUiToolboxScrollAreaContext* scrollArea )
{
	ImUiWindow* window = ImUiWidgetGetWindow( scrollArea->area );
	ImUiToolboxScrollAreaState* state = scrollArea->state;

	const ImUiRect frameRect = ImUiWidgetGetRect( scrollArea->area );

	ImUiSize areaSize = ImUiSizeCreateZero();
	for( ImUiWidget* child = ImUiWidgetGetFirstChild( scrollArea->content ); child; child = ImUiWidgetGetNextSibling( child ) )
	{
		const ImUiRect childRect = ImUiWidgetGetRect( child );

		areaSize.width	= IMUI_MAX( areaSize.width, childRect.size.width );
		areaSize.height	= IMUI_MAX( areaSize.height, childRect.size.height );
	}

	ImUiWidgetInputState areaInputState;
	ImUiWidgetGetInputState( scrollArea->area, &areaInputState );

	const bool hasHorizontalBar	= areaSize.width > frameRect.size.width;
	const bool hasVerticalBar	= areaSize.height > frameRect.size.height;

	ImUiBorder margin = ImUiBorderCreateZero();
	margin.right = (hasVerticalBar ? s_theme.scrollArea.barSize + (scrollArea->horizontalSpacing ? s_theme.scrollArea.barSpacing : 0.0f) : 0.0f) ;
	margin.bottom = (hasHorizontalBar ? s_theme.scrollArea.barSize + (scrollArea->verticalSpacing ? s_theme.scrollArea.barSpacing : 0.0f) : 0.0f) ;

	const ImUiSize frameAreaSize = ImUiSizeMax( ImUiSizeCreateZero(), ImUiSizeShrinkBorder( frameRect.size, margin ) );

	ImUiWidgetSetMargin( scrollArea->content, margin );
	ImUiWidgetEnd( scrollArea->content );
	scrollArea->content = NULL;

	if( hasHorizontalBar )
	{
		float barWidth = frameRect.size.width - (hasVerticalBar ? s_theme.scrollArea.barSize : 0.0f);
		barWidth = IMUI_MAX( 0.0f, barWidth );

		ImUiWidget* scrollBar = ImUiWidgetBegin( window );
		ImUiWidgetSetVAlign( scrollBar, 1.0f );
		ImUiWidgetSetFixedHeight( scrollBar, s_theme.scrollArea.barSize );
		ImUiWidgetSetHStretch( scrollBar, frameRect.size.width > 0.0f ? barWidth / frameRect.size.width : 1.0f );

		const ImUiRect barRect		= ImUiWidgetGetRect( scrollBar );
		const float pivotSizeFactor	= frameAreaSize.width / areaSize.width;
		const float pivotSize		= barWidth * pivotSizeFactor;
		const float pivotSizeFinal	= IMUI_MAX( pivotSize, s_theme.scrollArea.barMinSize );
		const float pivotOffset		= (state->offset.x / areaSize.width) * (barWidth - (pivotSizeFinal - pivotSize));

		const ImUiRect barPivotRect = ImUiRectCreate(
			pivotOffset,
			0.0f,
			pivotSizeFinal,
			s_theme.scrollArea.barSize
		);

		ImUiWidgetInputState inputState;
		ImUiWidgetGetInputState( scrollBar, &inputState );

		if( inputState.wasPressed )
		{
			if( !state->wasPressedX &&
				ImUiRectIncludesPos( barPivotRect, inputState.relativeMousePos ) )
			{
				state->pressPoint.x	= inputState.relativeMousePos.x;
				state->pressPoint.x	-= pivotOffset;
				state->wasPressedX	= true;
			}

			if( state->wasPressedX )
			{
				const float newBarOffset	= inputState.relativeMousePos.x - state->pressPoint.x;
				const float newOffset		= (newBarOffset / barRect.size.width) * areaSize.width;

				state->offset.x = newOffset;
			}
		}
		else
		{
			state->wasPressedX = false;
		}

		ImUiWidgetDrawSkin( scrollBar, &s_theme.skins[ ImUiToolboxSkin_ScrollAreaBarBackground ], s_theme.colors[ ImUiToolboxColor_ScrollAreaBarBackground ] );
		ImUiWidgetDrawPartialSkin( scrollBar, barPivotRect, &s_theme.skins[ ImUiToolboxSkin_ScrollAreaBarPivot ], s_theme.colors[ ImUiToolboxColor_ScrollAreaBarPivot ] );

		ImUiWidgetEnd( scrollBar );
	}

	if( hasVerticalBar )
	{
		float barHeight = frameRect.size.height - (hasHorizontalBar ? s_theme.scrollArea.barSize : 0.0f);
		barHeight = IMUI_MAX( 0.0f, barHeight );

		ImUiWidget* scrollBar = ImUiWidgetBegin( window );
		ImUiWidgetSetHAlign( scrollBar, 1.0f );
		ImUiWidgetSetFixedWidth( scrollBar, s_theme.scrollArea.barSize );
		ImUiWidgetSetVStretch( scrollBar, frameRect.size.height > 0.0f ? barHeight / frameRect.size.height : 1.0f );

		const ImUiRect barRect		= ImUiWidgetGetRect( scrollBar );
		const float pivotSizeFactor	= frameAreaSize.height / areaSize.height;
		const float pivotSize		= barHeight * pivotSizeFactor;
		const float pivotSizeFinal	= IMUI_MAX( pivotSize, s_theme.scrollArea.barMinSize );
		const float pivotOffset		= (state->offset.y / areaSize.height) * (barHeight - (pivotSizeFinal - pivotSize));

		const ImUiRect barPivotRect = ImUiRectCreate(
			0.0f,
			pivotOffset,
			s_theme.scrollArea.barSize,
			pivotSizeFinal
		);

		ImUiWidgetInputState inputState;
		ImUiWidgetGetInputState( scrollBar, &inputState );

		if( inputState.wasPressed )
		{
			if( !state->wasPressedY &&
				ImUiRectIncludesPos( barPivotRect, inputState.relativeMousePos ) )
			{
				state->pressPoint.y	= inputState.relativeMousePos.y;
				state->pressPoint.y	-= pivotOffset;
				state->wasPressedY	= true;
			}

			if( state->wasPressedY )
			{
				const float newBarOffset	= inputState.relativeMousePos.y - state->pressPoint.y;
				const float newOffset		= (newBarOffset / barRect.size.height) * areaSize.height;

				state->offset.y = newOffset;
			}
		}
		else
		{
			state->wasPressedY = false;
		}

		ImUiWidgetDrawSkin( scrollBar, &s_theme.skins[ ImUiToolboxSkin_ScrollAreaBarBackground ], s_theme.colors[ ImUiToolboxColor_ScrollAreaBarBackground ] );
		ImUiWidgetDrawPartialSkin( scrollBar, barPivotRect, &s_theme.skins[ ImUiToolboxSkin_ScrollAreaBarPivot ], s_theme.colors[ ImUiToolboxColor_ScrollAreaBarPivot ] );

		ImUiWidgetEnd( scrollBar );
	}

	if( areaInputState.isMouseOver )
	{
		state->offset = ImUiPosSubPos( state->offset, ImUiPosScale( ImUiInputGetMouseScrollDelta( ImUiWindowGetContext( window ) ), 80.0f ) );
	}

	state->offset = ImUiPosMax( ImUiPosCreateZero(), ImUiPosMin( state->offset, ImUiSizeToPos( ImUiSizeSubSize( areaSize, frameAreaSize ) ) ) );

	ImUiWidgetEnd( scrollArea->area );
	scrollArea->area = NULL;
	scrollArea->state = NULL;
}

void ImUiToolboxListBegin( ImUiToolboxListContext* list, ImUiWindow* window, float itemSize, size_t itemCount )
{
	IMUI_ASSERT( list );

	const float totalItemSize = itemSize + s_theme.list.itemSpacing;

	ImUiToolboxScrollAreaBegin( &list->scrollArea, window );
	list->list = list->scrollArea.area;

	ImUiToolboxScrollAreaEnableSpacing( &list->scrollArea, true, false );

	list->listLayout = ImUiWidgetBegin( window );
	ImUiWidgetSetHStretch( list->listLayout, 1.0f );
	ImUiWidgetSetLayoutVerticalSpacing( list->listLayout, s_theme.list.itemSpacing );
	if( itemCount > 0u )
	{
		ImUiWidgetSetFixedHeight( list->listLayout, (totalItemSize * itemCount) - s_theme.list.itemSpacing );
	}

	bool isNew;
	list->state = (ImUiToolboxListState*)ImUiWidgetAllocStateNew( list->listLayout, sizeof( ImUiToolboxListState ), IMUI_ID_STR( "list" ), &isNew );
	if( isNew )
	{
		list->state->selectedIndex = (uintsize)-1;
	}

	const ImUiRect listRect		= ImUiWidgetGetRect( list->list );
	const ImUiRect layoutRect	= ImUiWidgetGetRect( list->listLayout );

	list->itemSize		= itemSize;
	list->itemCount		= itemCount;
	list->beginIndex	= (uintsize)((listRect.pos.y - layoutRect.pos.y) / totalItemSize);
	list->endIndex		= list->beginIndex + (uintsize)ceilf( (listRect.size.height + totalItemSize) / totalItemSize );
	list->endIndex		= IMUI_MIN( list->endIndex, itemCount );

	list->item			= NULL;
	list->itemIndex		= list->beginIndex - 1u;

	list->changed		= false;
}

size_t ImUiToolboxListGetBeginIndex( const ImUiToolboxListContext* list )
{
	return list->beginIndex;
}

size_t ImUiToolboxListGetEndIndex( const ImUiToolboxListContext* list )
{
	return list->endIndex;
}

size_t ImUiToolboxListGetSelectedIndex( const ImUiToolboxListContext* list )
{
	return list->state->selectedIndex;
}

void ImUiToolboxListSetSelectedIndex( ImUiToolboxListContext* list, size_t index )
{
	list->state->selectedIndex = index;
}

static void ImUiToolboxListItemEndInternal( ImUiToolboxListContext* list )
{
	ImUiWidget* item = ImUiWidgetGetLastChild( list->listLayout );
	if( item )
	{
		ImUiWidgetEnd( item );
		list->item = NULL;
	}
}

ImUiWidget* ImUiToolboxListNextItem( ImUiToolboxListContext* list )
{
	ImUiToolboxListItemEndInternal( list );

	list->itemIndex++;

	ImUiWidget* item = ImUiWidgetBegin( ImUiWidgetGetWindow( list->list ) );
	ImUiWidgetSetHStretch( item, 1.0f );
	ImUiWidgetSetFixedHeight( item, list->itemSize );

	if( list->beginIndex > 0 &&
		list->itemIndex == list->beginIndex )
	{
		const float totalItemSize = list->itemSize + s_theme.list.itemSpacing;
		ImUiWidgetSetMargin( item, ImUiBorderCreate( totalItemSize * list->beginIndex, 0.0f, 0.0f, 0.0f ) );
	}

	ImUiWidgetInputState inputState;
	ImUiWidgetGetInputState( item, &inputState );

	if( inputState.isMouseDown )
	{
		ImUiWidgetDrawSkin( item, &s_theme.skins[ ImUiToolboxSkin_ItemSelected ], s_theme.colors[ ImUiToolboxColor_ListItemClicked ] );
	}
	else if( inputState.isMouseOver )
	{
		ImUiWidgetDrawSkin( item, &s_theme.skins[ ImUiToolboxSkin_ListItem ], s_theme.colors[ ImUiToolboxColor_ListItemHover ] );
	}
	else if( list->itemIndex == list->state->selectedIndex )
	{
		ImUiWidgetDrawSkin( item, &s_theme.skins[ ImUiToolboxSkin_ItemSelected ], s_theme.colors[ ImUiToolboxColor_ListItemSelected ] );
	}

	if( inputState.hasMouseReleased )
	{
		list->state->selectedIndex = list->itemIndex;
		list->changed = true;
	}

	return item;
}

bool ImUiToolboxListEnd( ImUiToolboxListContext* list )
{
	ImUiToolboxListItemEndInternal( list );

	ImUiWidgetEnd( list->listLayout );
	list->listLayout = NULL;

	ImUiToolboxScrollAreaEnd( &list->scrollArea );
	list->list = NULL;

	return list->changed;
}

void ImUiToolboxDropDownBegin( ImUiToolboxDropDownContext* dropDown, ImUiWindow* window, const char** items, size_t itemCount, size_t itemStride )
{
	if( itemStride == 0u )
	{
		itemStride = sizeof( const char* );
	}

	dropDown->dropDown = ImUiWidgetBegin( window );
	ImUiWidgetSetPadding( dropDown->dropDown, s_theme.dropDown.padding );
	ImUiWidgetSetFixedHeight( dropDown->dropDown, s_theme.dropDown.height );

	bool isNew;
	dropDown->state = (ImUiToolboxDropDownState*)ImUiWidgetAllocStateNew( dropDown->dropDown, sizeof( *dropDown->state ), IMUI_ID_STR( "drop down" ), &isNew );
	if( isNew )
	{
		dropDown->state->selectedIndex = (uintsize)-1;
	}

	ImUiWidgetInputState inputState;
	ImUiWidgetGetInputState( dropDown->dropDown, &inputState );

	ImUiColor color = s_theme.colors[ ImUiToolboxColor_DropDown ];
	if( dropDown->state->isOpen )
	{
		color = s_theme.colors[ ImUiToolboxColor_DropDownOpen ];
	}
	else if( inputState.isMouseDown )
	{
		color = s_theme.colors[ ImUiToolboxColor_DropDownClicked ];
	}
	else if( inputState.isMouseOver )
	{
		color = s_theme.colors[ ImUiToolboxColor_DropDownHover ];
	}

	ImUiWidgetDrawSkin( dropDown->dropDown, &s_theme.skins[ ImUiToolboxSkin_DropDown ], color );

	ImUiWidget* icon = ImUiWidgetBegin( window );
	const ImUiImage iconImage = s_theme.icons[ dropDown->state->isOpen ? ImUiToolboxIcon_DropDownClose : ImUiToolboxIcon_DropDownOpen ];
	ImUiWidgetSetFixedSize( icon, ImUiSizeCreateImage( &iconImage ) );
	ImUiWidgetSetHAlign( icon, 1.0f );
	ImUiWidgetSetVAlign( icon, 0.5f );
	ImUiWidgetDrawImageColor( icon, &iconImage, s_theme.colors[ ImUiToolboxColor_DropDownIcon ] );
	ImUiWidgetEnd( icon );

	ImUiSize maxSize = ImUiSizeCreateZero();
	ImUiTextLayout* selectedTextLayout = NULL;
	const uint8* itemsBytes = (const byte*)items;
	for( uintsize i = 0; i < itemCount; ++i )
	{
		const char* itemText = *(const char**)itemsBytes;
		ImUiTextLayout* textLayout = ImUiTextLayoutCreateWidget( dropDown->dropDown, s_theme.font, itemText );

		maxSize = ImUiSizeMax( maxSize, ImUiTextLayoutGetSize( textLayout ) );

		if( i == dropDown->state->selectedIndex )
		{
			selectedTextLayout = textLayout;
		}

		itemsBytes += itemStride;
	}

	ImUiWidgetSetMinWidth( dropDown->dropDown, maxSize.width + ImUiBorderGetMinSize( s_theme.dropDown.padding ).width + s_theme.dropDown.padding.left + ImUiWidgetGetSize( icon ).width );

	ImUiWidget* text = ImUiWidgetBegin( window );
	ImUiWidgetSetFixedSize( text, maxSize );
	ImUiWidgetSetVAlign( text, 0.5f );

	if( selectedTextLayout )
	{
		ImUiWidgetDrawText( text, selectedTextLayout, s_theme.colors[ ImUiToolboxColor_DropDownText ] );
	}

	ImUiWidgetEnd( text );

	if( inputState.hasMousePressed )
	{
		dropDown->state->isOpen = !dropDown->state->isOpen;
	}

	if( dropDown->state->isOpen && itemCount > 0u )
	{
		ImUiSurface* surface = ImUiWindowGetSurface( window );
		const ImUiSize surfaceSize = ImUiSurfaceGetSize( surface );

		const ImUiRect dropDownRect		= ImUiWidgetGetRect( dropDown->dropDown );
		const ImUiSize listMaginSize	= ImUiBorderGetMinSize( s_theme.dropDown.listMargin );

		const float listHeight			= listMaginSize.height + (((s_theme.dropDown.itemSize + s_theme.dropDown.itemSpacing) * IMUI_MIN( itemCount, s_theme.dropDown.listMaxLength )) - s_theme.dropDown.itemSpacing);
		const float listWidth			= dropDownRect.size.width + listMaginSize.width;
		const float dropDownBottom		= dropDownRect.pos.y + dropDownRect.size.height;

		ImUiRect listRect;
		if( dropDownBottom + listHeight > surfaceSize.height )
		{
			if( dropDownRect.pos.y - listHeight < 0.0f )
			{
				if( dropDownBottom > surfaceSize.height / 2.0f )
				{
					listRect = ImUiRectCreate( dropDownRect.pos.x, 0.0f, listWidth, dropDownRect.pos.y );
				}
				else
				{
					listRect = ImUiRectCreate( dropDownRect.pos.x, dropDownBottom, listWidth, surfaceSize.height - dropDownBottom );
				}
			}
			else
			{
				listRect = ImUiRectCreate( dropDownRect.pos.x, dropDownRect.pos.y - listHeight, listWidth, listHeight );
			}
		}
		else
		{
			listRect = ImUiRectCreate( dropDownRect.pos.x, dropDownBottom, listWidth, listHeight  );
		}
		ImUiWindow* listWindow = ImUiWindowBegin( ImUiWindowGetSurface( window ), "dropDownList", listRect, s_theme.dropDown.listZOrder );

		const float oldItemSpacing = s_theme.list.itemSpacing;
		s_theme.list.itemSpacing = s_theme.dropDown.itemSpacing;

		ImUiToolboxListContext list;
		ImUiToolboxListBegin( &list, listWindow, s_theme.dropDown.itemSize, itemCount );
		ImUiWidgetSetStretch( list.list, 1.0f, 1.0f );
		ImUiWidgetSetMargin( list.scrollArea.area, s_theme.dropDown.listMargin );

		ImUiWidgetInputState listInputState;
		ImUiWidgetGetInputState( list.list, &listInputState );

		ImUiWidgetDrawSkin( listWindow->rootWidget, &s_theme.skins[ ImUiToolboxSkin_DropDownList ], s_theme.colors[ ImUiToolboxColor_DropDownList ] );

		ImUiToolboxListSetSelectedIndex( &list, dropDown->state->selectedIndex );

		itemsBytes = (const byte*)items;
		for( uintsize i = ImUiToolboxListGetBeginIndex( &list ); i < ImUiToolboxListGetEndIndex( &list ); ++i )
		{
			const char* itemText = *(const char**)(itemsBytes + (i * itemStride));

			ImUiWidget* item = ImUiToolboxListNextItem( &list );
			ImUiWidgetSetPadding( item, s_theme.dropDown.itemPadding );

			ImUiWidget* label = ImUiToolboxLabelBegin( listWindow, itemText );
			ImUiWidgetSetVAlign( label, 0.5f );
			ImUiWidgetEnd( label );
		}

		dropDown->state->selectedIndex = ImUiToolboxListGetSelectedIndex( &list );

		if( ImUiToolboxListEnd( &list ) )
		{
			dropDown->state->isOpen = false;
			dropDown->changed = true;
		}
		else
		{
			dropDown->changed = false;
		}

		s_theme.list.itemSpacing = oldItemSpacing;

		ImUiWindowEnd( listWindow );

		if( !inputState.wasPressed &&
			!listInputState.wasPressed &&
			ImUiInputHasMouseButtonReleased( ImUiWindowGetContext( window ), ImUiInputMouseButton_Left ) )
		{
			dropDown->state->isOpen = false;
		}
	}
}

size_t ImUiToolboxDropDownGetSelectedIndex( const ImUiToolboxDropDownContext* dropDown )
{
	return dropDown->state->selectedIndex;
}

void ImUiToolboxDropDownSetSelectedIndex( const ImUiToolboxDropDownContext* dropDown, size_t index )
{
	dropDown->state->selectedIndex = index;
}

bool ImUiToolboxDropDownEnd( ImUiToolboxDropDownContext* dropDown )
{
	ImUiWidgetEnd( dropDown->dropDown );

	return dropDown->changed;
}

size_t ImUiToolboxDropDown( ImUiWindow* window, const char** items, size_t itemCount, size_t itemStride )
{
	ImUiToolboxDropDownContext dropDown;
	ImUiToolboxDropDownBegin( &dropDown, window, items, itemCount, itemStride );
	const size_t selectedIndex = ImUiToolboxDropDownGetSelectedIndex( &dropDown );
	ImUiToolboxDropDownEnd( &dropDown );
	return selectedIndex;
}

ImUiWindow* ImUiToolboxPopupBegin( ImUiWindow* window )
{
	return ImUiToolboxPopupBeginSurface( ImUiWindowGetSurface( window ) );
}

ImUiWindow* ImUiToolboxPopupBeginSurface( ImUiSurface* surface )
{
	const ImUiRect windowRect = ImUiRectCreatePosSize( ImUiPosCreateZero(), ImUiSurfaceGetSize( surface ) );
	ImUiWindow* popupWindow = ImUiWindowBegin( surface, "popup", windowRect, s_theme.popup.zOrder );

	ImUiWidget* background = ImUiWidgetBegin( popupWindow );
	ImUiWidgetSetStretch( background, 1.0f, 1.0f );

	ImUiWidgetDrawColor( background, s_theme.colors[ ImUiToolboxColor_PopupBackground ] );

	ImUiWidget* popup = ImUiWidgetBegin( popupWindow );
	ImUiWidgetSetAlign( popup, 0.5f, 0.5f );
	ImUiWidgetSetPadding( popup, s_theme.popup.padding );
	ImUiWidgetSetLayoutVertical( popup );

	ImUiWidgetDrawSkin( popup, &s_theme.skins[ ImUiToolboxSkin_Popup ], s_theme.colors[ ImUiToolboxColor_Popup ] );

	return popupWindow;
}

size_t ImUiToolboxPopupEndButtons( ImUiWindow* popupWindow, const char** buttons, size_t buttonCount )
{
	ImUiWidget* buttonsLayout = ImUiWidgetBegin( popupWindow );
	ImUiWidgetSetHAlign( buttonsLayout, 1.0f );
	ImUiWidgetSetLayoutHorizontalSpacing( buttonsLayout, s_theme.popup.buttonSpacing );

	uintsize clickedButton = (uintsize)-1;
	for( uintsize i = 0; i < buttonCount; ++i )
	{
		if( ImUiToolboxButtonLabel( popupWindow, buttons[ i ] ) )
		{
			clickedButton = i;
		}
	}

	ImUiWidgetEnd( buttonsLayout );

	ImUiToolboxPopupEnd( popupWindow );

	return clickedButton;
}

void ImUiToolboxPopupEnd( ImUiWindow* popupWindow )
{
	ImUiWidget* background = ImUiWindowGetFirstChild( popupWindow );
	ImUiWidget* popup = ImUiWidgetGetFirstChild( background );
	ImUiWidgetEnd( popup );
	ImUiWidgetEnd( background );
	ImUiWindowEnd( popupWindow );
}

#if defined( _MSC_VER )
#	pragma warning(pop)
#endif
