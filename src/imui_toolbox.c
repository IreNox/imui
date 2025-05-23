#include "imui/imui_toolbox.h"

#include "imui_font.h"
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

typedef struct ImUiToolboxTextBuffer
{
	ImUiAllocator*	allocator;

	char*			data;
	uintsize		dataLength;
	uintsize		dataCapacity;

	uintsize*		lines;
	uintsize		linesLength;
	uintsize		linesCapacity;
} ImUiToolboxTextBuffer;

typedef struct ImUiToolboxTextEditState
{
	bool			hasFocus;

	ImUiPos			offset;
	bool			wasPressedX;
	bool			wasPressedY;
	ImUiPos			pressPoint;

	uint32			selectionStart;
	uint32			selectionEnd;
	uint32			cursorPos;
} ImUiToolboxTextEditState;

struct ImUiToolboxListState
{
	bool			hasFocus;

	uintsize		selectedIndex;
};

struct ImUiToolboxDropDownState
{
	bool			isOpen;

	uintsize		selectedIndex;
};

struct ImUiToolboxTabViewState
{
	uintsize		selectedTab;
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
	{ "Tab View/Head Background Color",		ImUiToolboxThemeReflectionType_Color,	offsetof( ImUiToolboxTheme, colors[ ImUiToolboxColor_TabViewHeadBackground ] ) },
	{ "Tab View/Header Active Color",		ImUiToolboxThemeReflectionType_Color,	offsetof( ImUiToolboxTheme, colors[ ImUiToolboxColor_TabViewHeaderActive ] ) },
	{ "Tab View/Header Inactive Color",		ImUiToolboxThemeReflectionType_Color,	offsetof( ImUiToolboxTheme, colors[ ImUiToolboxColor_TabViewHeaderInactive ] ) },
	{ "Tab View/Body Background Color",		ImUiToolboxThemeReflectionType_Color,	offsetof( ImUiToolboxTheme, colors[ ImUiToolboxColor_TabViewBody ] ) },

	{ "Button/Skin",						ImUiToolboxThemeReflectionType_Skin,	offsetof( ImUiToolboxTheme, skins[ ImUiToolboxSkin_Button ] ) },
	{ "Button/Hover Skin",					ImUiToolboxThemeReflectionType_Skin,	offsetof( ImUiToolboxTheme, skins[ ImUiToolboxSkin_ButtonHover ] ) },
	{ "Button/Clicked Skin",				ImUiToolboxThemeReflectionType_Skin,	offsetof( ImUiToolboxTheme, skins[ ImUiToolboxSkin_ButtonClicked ] ) },
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
	{ "Tab View/Head Skin",					ImUiToolboxThemeReflectionType_Skin,	offsetof( ImUiToolboxTheme, skins[ ImUiToolboxSkin_TabViewHeadBackground ] ) },
	{ "Tab View/Header Active Skin",		ImUiToolboxThemeReflectionType_Skin,	offsetof( ImUiToolboxTheme, skins[ ImUiToolboxSkin_TabViewHeaderActive ] ) },
	{ "Tab View/Header Inactive Skin",		ImUiToolboxThemeReflectionType_Skin,	offsetof( ImUiToolboxTheme, skins[ ImUiToolboxSkin_TabViewHeaderInactive ] ) },
	{ "Tab View/Body Skin",					ImUiToolboxThemeReflectionType_Skin,	offsetof( ImUiToolboxTheme, skins[ ImUiToolboxSkin_TabViewBody ] ) },

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

	{ "Tab View/Header Spacing",			ImUiToolboxThemeReflectionType_Float,	offsetof( ImUiToolboxTheme, tabView.headerSpacing ) },
	{ "Tab View/Header Cut Extend Left",	ImUiToolboxThemeReflectionType_Float,	offsetof( ImUiToolboxTheme, tabView.headerCutLeft ) },
	{ "Tab View/Header Cut Extend Right",	ImUiToolboxThemeReflectionType_Float,	offsetof( ImUiToolboxTheme, tabView.headerCutRight ) },
	{ "Tab View/Header Padding",			ImUiToolboxThemeReflectionType_Border,	offsetof( ImUiToolboxTheme, tabView.headerPadding ) },
	{ "Tab View/Body Padding",				ImUiToolboxThemeReflectionType_Border,	offsetof( ImUiToolboxTheme, tabView.bodyPadding ) },
};
static_assert( ImUiToolboxColor_MAX == 41, "more colors" );
static_assert( ImUiToolboxSkin_MAX == 22, "more skins" );
static_assert( ImUiToolboxIcon_MAX == 3, "more icons" );
static_assert( sizeof( ImUiToolboxTheme ) == 1608u, "theme changed" );

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
	theme->colors[ ImUiToolboxColor_DropDownItemText ]			= textColor;
	theme->colors[ ImUiToolboxColor_DropDownItemHover ]			= elementHoverColor;
	theme->colors[ ImUiToolboxColor_DropDownItemClicked ]		= elementClickedColor;
	theme->colors[ ImUiToolboxColor_DropDownItemSelected ]		= elementColor;
	theme->colors[ ImUiToolboxColor_PopupBackground ]			= ImUiColorCreateFloat( 0.0f, 0.0f, 0.0f, 0.2f );
	theme->colors[ ImUiToolboxColor_Popup ]						= backgroundColor;
	theme->colors[ ImUiToolboxColor_TabViewHeadBackground ]		= backgroundColor;
	theme->colors[ ImUiToolboxColor_TabViewHeaderActive ]		= elementColor;
	theme->colors[ ImUiToolboxColor_TabViewHeaderInactive ]		= elementClickedColor;
	theme->colors[ ImUiToolboxColor_TabViewBody ]				= backgroundColor;
	static_assert( ImUiToolboxColor_MAX == 41, "more colors" );

	const ImUiSkin skin = { IMUI_TEXTURE_HANDLE_INVALID };

	theme->skins[ ImUiToolboxSkin_Button ]						= skin;
	theme->skins[ ImUiToolboxSkin_ButtonHover ]					= skin;
	theme->skins[ ImUiToolboxSkin_ButtonClicked ]				= skin;
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
	theme->skins[ ImUiToolboxSkin_ItemSelected ]				= skin;
	theme->skins[ ImUiToolboxSkin_DropDown ]					= skin;
	theme->skins[ ImUiToolboxSkin_DropDownList ]				= skin;
	theme->skins[ ImUiToolboxSkin_DropDownItem ]				= skin;
	theme->skins[ ImUiToolboxSkin_Popup ]						= skin;
	theme->skins[ ImUiToolboxSkin_TabViewHeadBackground ]		= skin;
	theme->skins[ ImUiToolboxSkin_TabViewHeaderActive ]			= skin;
	theme->skins[ ImUiToolboxSkin_TabViewHeaderInactive ]		= skin;
	theme->skins[ ImUiToolboxSkin_TabViewBody ]					= skin;
	static_assert( ImUiToolboxSkin_MAX == 22, "more skins" );

	const ImUiImage image = { IMUI_TEXTURE_HANDLE_INVALID, 22u, 22u, { 0.0f, 0.0f, 1.0f, 1.0f } };

	theme->icons[ ImUiToolboxIcon_CheckBoxChecked ]				= image;
	theme->icons[ ImUiToolboxIcon_DropDownOpen ]				= image;
	theme->icons[ ImUiToolboxIcon_DropDownClose ]				= image;
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

	theme->tabView.headerSpacing	= 4.0f;
	theme->tabView.headerCutLeft	= 0.0f;
	theme->tabView.headerCutRight	= 0.0f;
	theme->tabView.headerPadding	= ImUiBorderCreateAll( 8.0f );
	theme->tabView.bodyPadding		= ImUiBorderCreateAll( 8.0f );
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
	const ImUiSkin* skin = &s_theme.skins[ ImUiToolboxSkin_Button ];
	if( inputState.wasPressed && inputState.isMouseDown )
	{
		color = s_theme.colors[ ImUiToolboxColor_ButtonClicked ];
		skin = &s_theme.skins[ ImUiToolboxSkin_ButtonClicked ];
	}
	else if( inputState.isMouseOver )
	{
		color = s_theme.colors[ ImUiToolboxColor_ButtonHover ];
		skin = &s_theme.skins[ ImUiToolboxSkin_ButtonHover ];
	}

	ImUiWidgetDrawSkin( button, skin, color );

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
	return ImUiToolboxLabelBeginLengthColor( window, text, strlen( text ), color );
}

ImUiWidget* ImUiToolboxLabelBeginLength( ImUiWindow* window, const char* text, size_t length )
{
	return ImUiToolboxLabelBeginLengthColor( window, text, length, s_theme.colors[ ImUiToolboxColor_Text ] );
}

ImUiWidget* ImUiToolboxLabelBeginLengthColor( ImUiWindow* window, const char* text, size_t length, ImUiColor color )
{
	ImUiWidget* label = ImUiWidgetBegin( window );

	ImUiTextLayout* layout = ImUiTextLayoutCreateWidgetLength( label, s_theme.font, text,length );
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

void ImUiToolboxLabelLength( ImUiWindow* window, const char* text, size_t length )
{
	ImUiWidget* label = ImUiToolboxLabelBeginLength( window, text, length );
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

	const float range = max - min;
	const float normalizedValue = range == 0.0f ? 0.0f : (*value - min) / range;
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

		const float scaledPaddingLeft	= s_theme.slider.padding.left * slider->window->surface->dpiScale;
		const float mouseValueNorm		= (frameInputState.relativeMousePos.x - scaledPaddingLeft) / sliderInnerRect.size.width;
		const float mouseValueNormClamp	= mouseValueNorm > 1.0f ? 1.0f : (mouseValueNorm < 0.0f ? 0.0f : mouseValueNorm);
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

ImUiToolboxTextBuffer* ImUiToolboxTextBufferCreate( ImUiWindow* window, const char* text )
{
	ImUiToolboxTextBuffer* textBuffer = IMUI_MEMORY_NEW_ZERO( &window->context->allocator, ImUiToolboxTextBuffer );

	textBuffer->allocator = &window->context->allocator;

	ImUiToolboxTextBufferAppend( textBuffer, text );

	return textBuffer;
}

void ImUiToolboxTextBufferFree( ImUiToolboxTextBuffer* textBuffer )
{
	if( !textBuffer )
	{
		return;
	}

	ImUiMemoryFree( textBuffer->allocator, textBuffer->data );
	ImUiMemoryFree( textBuffer->allocator, textBuffer->lines );
	ImUiMemoryFree( textBuffer->allocator, textBuffer );
}

void ImUiToolboxTextBufferSet( ImUiToolboxTextBuffer* textBuffer, const char* text )
{
	textBuffer->dataLength	= 0;
	textBuffer->linesLength	= 0;

	ImUiToolboxTextBufferAppend( textBuffer, text );
}

void ImUiToolboxTextBufferAppend( ImUiToolboxTextBuffer* textBuffer, const char* text )
{
	if( !text )
	{
		return;
	}

	const uintsize textLength = strlen( text );

	if( !IMUI_MEMORY_ARRAY_CHECK_CAPACITY( textBuffer->allocator, textBuffer->data, textBuffer->dataCapacity, textBuffer->dataLength + textLength + 1u ) )
	{
		return;
	}

	const char* firstLine = text;
	if( textBuffer->linesLength > 0 )
	{
		const char* lastLine = textBuffer->data + textBuffer->lines[ textBuffer->linesLength - 1u ];
		if( !strchr( lastLine, '\n' ) )
		{
			firstLine = strchr( text, '\n' );
			if( firstLine )
			{
				firstLine++;
			}
		}
	}

	bool first = true;
	uintsize linesLength = textBuffer->linesLength;
	for( const char* line = firstLine; line; line = strchr( line, '\n' ) )
	{
		if( !first )
		{
			line++;
		}

		if( !IMUI_MEMORY_ARRAY_CHECK_CAPACITY( textBuffer->allocator, textBuffer->lines, textBuffer->linesCapacity, linesLength + 1u ) )
		{
			return;
		}

		const uintsize offset = line - text;
		textBuffer->lines[ linesLength ] = textBuffer->dataLength + offset;
		linesLength++;
		first = false;
	}

	strncpy( textBuffer->data + textBuffer->dataLength, text, textLength + 1 );

	textBuffer->dataLength += textLength;
	textBuffer->linesLength = linesLength;
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
	const float scale = ImUiWidgetGetDpiScale( text );
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
				const uintsize selectionStartChar	= ImUiTextLayoutGetGlyphCharIndex( layout, state->selectionStart );
				const uintsize selectionEndChar		= ImUiTextLayoutGetGlyphCharIndex( layout, state->selectionEnd );

				memmove( buffer + selectionStartChar, buffer + selectionEndChar, textLengthInternal - selectionEndChar );
				state->cursorPos = state->selectionStart;

				textLengthInternal -= selectionEndChar - selectionStartChar;

				state->selectionStart	= 0u;
				state->selectionEnd		= 0u;
			}

			const uintsize cursorChar = ImUiTextLayoutGetGlyphCharIndex( layout, state->cursorPos );
			if( cursorChar != textLengthInternal )
			{
				memmove( buffer + cursorChar + newSize, buffer + cursorChar, textLengthInternal - cursorChar );
			}

			memcpy( buffer + cursorChar, textInput, newSize );
			textLengthInternal += newSize;
			buffer[ textLengthInternal ] = '\0';

			state->cursorPos += (uint32)ImUiTextLayoutCalculateGlyphCount( textInput, newSize );

			changed = true;
		}

		if( ImUiInputHasKeyPressed( imui, ImUiInputKey_Backspace ) )
		{
			if( state->selectionStart != state->selectionEnd )
			{
				const uintsize selectionStartChar	= ImUiTextLayoutGetGlyphCharIndex( layout, state->selectionStart );
				const uintsize selectionEndChar		= ImUiTextLayoutGetGlyphCharIndex( layout, state->selectionEnd );

				memmove( buffer + selectionStartChar, buffer + selectionEndChar, textLengthInternal - selectionEndChar );
				state->cursorPos = state->selectionStart;

				textLengthInternal -= selectionEndChar - selectionStartChar;
				buffer[ textLengthInternal ] = '\0';

				state->selectionStart	= 0u;
				state->selectionEnd		= 0u;

				changed = true;
			}
			else if( state->cursorPos > 0u )
			{
				const uintsize backspaceStartChar	= ImUiTextLayoutGetGlyphCharIndex( layout, state->cursorPos - 1u );
				const uintsize backspaceEndChar		= ImUiTextLayoutGetGlyphCharIndex( layout, state->cursorPos );

				memmove( buffer + backspaceStartChar, buffer + backspaceEndChar, textLengthInternal - backspaceEndChar );

				textLengthInternal -= backspaceEndChar - backspaceStartChar;
				buffer[ textLengthInternal ] = '\0';

				state->cursorPos--;

				changed = true;
			}
		}
		else if( ImUiInputHasKeyPressed( imui, ImUiInputKey_Delete ) )
		{
			if( state->selectionStart != state->selectionEnd )
			{
				const uintsize selectionStartChar	= ImUiTextLayoutGetGlyphCharIndex( layout, state->selectionStart );
				const uintsize selectionEndChar		= ImUiTextLayoutGetGlyphCharIndex( layout, state->selectionEnd );

				memmove( buffer + selectionStartChar, buffer + selectionEndChar, textLengthInternal - selectionEndChar );
				state->cursorPos = state->selectionStart;

				textLengthInternal -= selectionEndChar - selectionStartChar;
				buffer[ textLengthInternal ] = '\0';

				state->selectionStart	= 0u;
				state->selectionEnd		= 0u;

				changed = true;
			}
			else if( ImUiTextLayoutGetGlyphCount( layout ) > state->cursorPos )
			{
				const uintsize deleteStartChar	= ImUiTextLayoutGetGlyphCharIndex( layout, state->cursorPos );
				const uintsize deleteEndChar	= ImUiTextLayoutGetGlyphCharIndex( layout, state->cursorPos + 1u );

				memmove( buffer + deleteStartChar, buffer + deleteEndChar, textLengthInternal - deleteEndChar );

				textLengthInternal -= deleteEndChar - deleteStartChar;
				buffer[ textLengthInternal ] = '\0';

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
			nextCursorPos = (sint32)ImUiTextLayoutFindGlyphIndex( layout, textInputState.relativeMousePos, scale );

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

			const ImUiPos startPos			= ImUiTextLayoutGetGlyphPos( layout, state->selectionStart, scale );
			const ImUiPos endPos			= ImUiTextLayoutGetGlyphPos( layout, state->selectionEnd, scale );

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

			const ImUiPos cursorPos			= ImUiTextLayoutGetGlyphPos( layout, state->cursorPos, scale );
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

ImUiWidget* ImUiToolboxTextViewBegin( ImUiToolboxTextViewContext* textView, ImUiWindow* window, const char* text )
{
	ImUiToolboxTextBuffer* textBuffer = ImUiToolboxTextBufferCreate( window, text );

	ImUiWidget* textViewWidget = ImUiToolboxTextViewBeginBuffer( textView, window, textBuffer );
	textView->ownsBuffer = true;

	return textViewWidget;
}

ImUiWidget* ImUiToolboxTextViewBeginBuffer( ImUiToolboxTextViewContext* textView, ImUiWindow* window, const ImUiToolboxTextBuffer* textBuffer )
{
	ImUiToolboxListBegin( &textView->list, window, s_theme.font ? s_theme.font->fontSize : 1.0f, textBuffer->linesLength, false );

	//bool isNewState;
	textView->ownsBuffer	= false;
	textView->textBuffer	= textBuffer;

	for( uintsize i = ImUiToolboxListGetBeginIndex( &textView->list ); i < ImUiToolboxListGetEndIndex( &textView->list ); ++i )
	{
		ImUiToolboxListNextItem( &textView->list );

		const bool lastLine = i == textBuffer->linesLength - 1u;
		const uintsize lineOffset = textBuffer->lines[ i ];
		const uintsize nextLineOffset = lastLine ? textBuffer->dataLength : textBuffer->lines[ i + 1 ];
		const uintsize lineLength = (nextLineOffset - lineOffset) - (lastLine ? 0 : 1);

		const char* line = textBuffer->data + lineOffset;
		ImUiToolboxLabelLength( window, line, lineLength );
	}

	return textView->list.list;
}

void ImUiToolboxTextViewEnd( ImUiToolboxTextViewContext* textView )
{

	ImUiToolboxListEnd( &textView->list );

	if( textView->ownsBuffer )
	{
		ImUiToolboxTextBufferFree( (ImUiToolboxTextBuffer*)textView->textBuffer );
		textView->textBuffer = NULL;
	}
}

void ImUiToolboxTextView( ImUiWindow* window, const char* text )
{
	ImUiToolboxTextViewContext textView;
	ImUiToolboxTextViewBegin( &textView, window, text );
	ImUiToolboxTextViewEnd( &textView );
}

void ImUiToolboxTextViewBuffer( ImUiWindow* window, const ImUiToolboxTextBuffer* textBuffer )
{
	ImUiToolboxTextViewContext textView;
	ImUiToolboxTextViewBeginBuffer( &textView, window, textBuffer );
	ImUiToolboxTextViewEnd( &textView );
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

ImUiWidget* ImUiToolboxScrollAreaBegin( ImUiToolboxScrollAreaContext* scrollArea, ImUiWindow* window )
{
	scrollArea->horizontalSpacing	= false;
	scrollArea->verticalSpacing		= false;
	scrollArea->area				= ImUiWidgetBegin( window );
	scrollArea->state				= (ImUiToolboxScrollAreaState*)ImUiWidgetAllocState( scrollArea->area, sizeof( *scrollArea->state ), IMUI_ID_STR( "scroll area" ) );
	scrollArea->content				= ImUiWidgetBegin( window );

	ImUiWidgetSetStretch( scrollArea->content, 1.0f, 1.0f );
	ImUiWidgetSetLayoutScroll( scrollArea->content, scrollArea->state->offset.x, scrollArea->state->offset.y );

	return scrollArea->area;
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

ImUiWidget* ImUiToolboxListBegin( ImUiToolboxListContext* list, ImUiWindow* window, float itemSize, size_t itemCount, bool selection )
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
	if( isNew || !selection )
	{
		list->state->selectedIndex = (uintsize)-1;
	}

	const ImUiRect listRect		= ImUiWidgetGetRect( list->list );
	const ImUiRect layoutRect	= ImUiWidgetGetRect( list->listLayout );
	const float scaledItemSize	= totalItemSize * window->surface->dpiScale;

	list->itemSize		= itemSize;
	list->itemCount		= itemCount;
	list->beginIndex	= (uintsize)((listRect.pos.y - layoutRect.pos.y) / scaledItemSize);
	list->endIndex		= list->beginIndex + (uintsize)ceilf( (listRect.size.height + scaledItemSize) / scaledItemSize );
	list->endIndex		= IMUI_MIN( list->endIndex, itemCount );

	list->item			= NULL;
	list->itemIndex		= list->beginIndex - 1u;

	list->selection		= selection;
	list->changed		= false;

	ImUiWidgetInputState inputState;
	ImUiWidgetGetInputState( list->list, &inputState );

	ImUiContext* imui = ImUiWidgetGetContext( list->list );
	if( ImUiInputHasMouseButtonPressed( imui, ImUiInputMouseButton_Left ) )
	{
		list->state->hasFocus = inputState.hasMousePressed;
	}

	if( list->state->hasFocus )
	{
		if( ImUiInputHasKeyPressed( imui, ImUiInputKey_Up ) )
		{
			list->state->selectedIndex--;
			if( list->state->selectedIndex >= itemCount )
			{
				list->state->selectedIndex = 0;
			}
			list->changed = true;
		}
		else if( ImUiInputHasKeyPressed( imui, ImUiInputKey_Down ) )
		{
			list->state->selectedIndex++;
			if( list->state->selectedIndex >= itemCount )
			{
				list->state->selectedIndex = itemCount - 1;
			}
			list->changed = true;
		}
	}

	return list->list;
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
	if( !list->selection )
	{
		return;
	}

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

	if( list->selection )
	{
		ImUiWidgetInputState inputState;
		ImUiWidgetGetInputState( item, &inputState );

		const ImUiSkin* skin = &s_theme.skins[ list->itemIndex == list->state->selectedIndex ? ImUiToolboxSkin_ItemSelected : ImUiToolboxSkin_ListItem ];
		if( inputState.isMouseDown )
		{
			ImUiWidgetDrawSkin( item, skin, s_theme.colors[ ImUiToolboxColor_ListItemClicked ] );
		}
		else if( inputState.isMouseOver )
		{
			ImUiWidgetDrawSkin( item, skin, s_theme.colors[ ImUiToolboxColor_ListItemHover ] );
		}
		else if( list->itemIndex == list->state->selectedIndex )
		{
			ImUiWidgetDrawSkin( item, skin, s_theme.colors[ ImUiToolboxColor_ListItemSelected ] );
		}

		if( inputState.hasMouseReleased )
		{
			list->state->selectedIndex = list->itemIndex;
			list->changed = true;
		}
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

ImUiWidget* ImUiToolboxDropDownBegin( ImUiToolboxDropDownContext* dropDown, ImUiWindow* window, const char** items, size_t itemCount, size_t itemStride )
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
		ImUiToolboxListBegin( &list, listWindow, s_theme.dropDown.itemSize, itemCount, true );
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

	return dropDown->dropDown;
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

ImUiWidget* ImUiToolboxTabViewBegin( ImUiToolboxTabViewContext* tabView, ImUiWindow* window )
{
	tabView->view = ImUiWidgetBegin( window );
	ImUiWidgetSetLayoutVertical( tabView->view );

	tabView->head = ImUiWidgetBegin( window );
	ImUiWidgetSetLayoutHorizontalSpacing( tabView->head, s_theme.tabView.headerSpacing );

	tabView->body			= NULL;
	tabView->headerCount	= 0u;
	tabView->state			= (ImUiToolboxTabViewState*)ImUiWidgetAllocState( tabView->head, sizeof( ImUiToolboxTabViewState ), IMUI_ID_TYPE( ImUiToolboxTabViewState ) );

	return tabView->view;
}

bool ImUiToolboxTabViewHeader( ImUiToolboxTabViewContext* tabView, const char* text )
{
	ImUiWidget* tabHeader = ImUiToolboxTabViewHeaderBegin( tabView );
	ImUiToolboxLabel( ImUiWidgetGetWindow( tabHeader ), text );
	return ImUiToolboxTabViewHeaderEnd( tabView, tabHeader );
}

ImUiWidget* ImUiToolboxTabViewHeaderBegin( ImUiToolboxTabViewContext* tabView )
{
	IMUI_ASSERT( tabView->head );

	ImUiWidget* tabHeader = ImUiWidgetBegin( ImUiWidgetGetWindow( tabView->head ) );
	ImUiWidgetSetPadding( tabHeader, s_theme.tabView.headerPadding );

	ImUiColor color = s_theme.colors[ ImUiToolboxColor_TabViewHeaderInactive ];
	const ImUiSkin* skin = &s_theme.skins[ ImUiToolboxSkin_TabViewHeaderInactive ];

	if( tabView->state->selectedTab == tabView->headerCount )
	{
		color = s_theme.colors[ ImUiToolboxColor_TabViewHeaderActive ];
		skin = &s_theme.skins[ ImUiToolboxSkin_TabViewHeaderActive ];

		tabView->selectedHeaderOffset	= ImUiWidgetGetPosX( tabHeader ) - ImUiWidgetGetPosX( tabView->head );
		tabView->selectedHeaderWidth	= ImUiWidgetGetSizeWidth( tabHeader );
	}

	ImUiWidgetDrawSkin( tabHeader, skin, color );

	return tabHeader;
}

bool ImUiToolboxTabViewHeaderEnd( ImUiToolboxTabViewContext* tabView, ImUiWidget* tabHeader )
{
	ImUiWidgetInputState inputState;
	ImUiWidgetGetInputState( tabHeader, &inputState );

	if( inputState.hasMousePressed )
	{
		tabView->state->selectedTab = tabView->headerCount;
	}

	ImUiWidgetEnd( tabHeader );

	const bool selected = tabView->state->selectedTab == tabView->headerCount;
	tabView->headerCount++;
	return selected;
}

ImUiWidget* ImUiToolboxTabViewBodyBegin( ImUiToolboxTabViewContext* tabView )
{
	IMUI_ASSERT( tabView->head );

	ImUiWidgetEnd( tabView->head );
	tabView->head = NULL;

	tabView->body = ImUiWidgetBegin( ImUiWidgetGetWindow( tabView->view ) );
	ImUiWidgetSetStretchOne( tabView->body );
	ImUiWidgetSetPadding( tabView->body, s_theme.tabView.bodyPadding );

	const ImUiSkin* skin = &s_theme.skins[ ImUiToolboxSkin_TabViewBody ];

	const float uScale = skin->width ? (skin->uv.u1 - skin->uv.u0) / skin->width : 0.0f;
	const float vScale = skin->height ? (skin->uv.v1 - skin->uv.v0) / skin->height : 0.0f;

	ImUiBorder uvBorder = skin->border;
	uvBorder.top	*= vScale;
	uvBorder.left	*= uScale;
	uvBorder.bottom	*= vScale;
	uvBorder.right	*= uScale;

	ImUiImage image;
	image.textureHandle	= skin->textureHandle;
	image.width			= skin->width;
	image.height		= skin->height;
	image.uv			= skin->uv;

	ImUiRect rect = ImUiWidgetGetRect( tabView->body );
	rect.pos.x	= 0.0f;
	rect.pos.y	= 0.0f;

	const ImUiSize borderSize = ImUiBorderGetMinSize( skin->border );
	const float xScale = rect.size.width >= borderSize.width ? 1.0f : rect.size.width / borderSize.width;
	const float yScale = rect.size.height >= borderSize.height ? 1.0f : rect.size.height / borderSize.height;

	const float xLeft			= rect.pos.x;
	const float xCenterLeft		= xLeft + (skin->border.left * xScale);
	const float xRight			= xLeft + rect.size.width;
	const float xCenterRight	= xRight - (skin->border.right * xScale);
	const float yTop			= rect.pos.y;
	const float yCenterTop		= yTop + (skin->border.top * yScale);
	const float yBottom			= yTop + rect.size.height;
	const float yCenterBottom	= yBottom - (skin->border.bottom * yScale);

	const float uLeft			= skin->uv.u0;
	const float uCenterLeft		= uLeft + uvBorder.left;
	const float uRight			= skin->uv.u1;
	const float uCenterRight	= uRight - uvBorder.right;
	const float vTop			= skin->uv.v0;
	const float vCenterTop		= vTop + uvBorder.top;
	const float vBottom			= skin->uv.v1;
	const float vCenterBottom	= vBottom - uvBorder.bottom;

	const float xPositions[] =
	{
		xLeft,
		xCenterLeft,
		xCenterRight,
		xRight
	};

	const float yPositions[] =
	{
		yTop,
		yCenterTop,
		yCenterBottom,
		yBottom
	};

	const float uPositions[] =
	{
		uLeft,
		uCenterLeft,
		uCenterRight,
		uRight
	};

	const float vPositions[] =
	{
		vTop,
		vCenterTop,
		vCenterBottom,
		vBottom
	};

	const ImUiColor color = s_theme.colors[ ImUiToolboxColor_TabViewBody ];
	for( uintsize x = 0; x < 5u; ++x )
	{
		const uintsize nextX = x + 1u;

		uintsize uvX = x;
		uintsize uvY = 0u;
		float posX = xPositions[ x ];
		float nextPosX = xPositions[ nextX ];
		if( x == 0u && tabView->selectedHeaderOffset == 0.0f )
		{
			uvY = 1u;
		}
		else if( x == 1u )
		{
			posX = tabView->selectedHeaderOffset + tabView->selectedHeaderWidth - s_theme.tabView.headerCutRight;
		}
		else if( x == 3u )
		{
			uvX = 1u;
			uvY = 1u;

			posX = tabView->selectedHeaderOffset + s_theme.tabView.headerCutLeft;
			nextPosX = tabView->selectedHeaderOffset + tabView->selectedHeaderWidth - +s_theme.tabView.headerCutRight;
		}
		else if( x == 4u )
		{
			if( tabView->selectedHeaderOffset == 0.0f )
			{
				continue;
			}

			uvX = 1u;

			posX = xPositions[ 1u ];
			nextPosX = tabView->selectedHeaderOffset + s_theme.tabView.headerCutLeft;
		}

		const ImUiPos posTl = ImUiPosCreate( posX, yPositions[ 0u ] );
		const ImUiPos posBr = ImUiPosCreate( nextPosX, yPositions[ 1u ] );

		const ImUiTexCoord uv =
		{
			uPositions[ uvX ], vPositions[ uvY ],
			uPositions[ uvX + 1u ], vPositions[ uvY + 1u ]
		};

		image.uv = uv;

		rect.pos			= posTl;
		rect.size.width		= posBr.x - posTl.x;
		rect.size.height	= posBr.y - posTl.y;

		ImUiWidgetDrawPartialImageColor( tabView->body, rect, &image, color );
	}

	for( uintsize y = 1u; y < 3u; ++y )
	{
		const uintsize nextY = y + 1u;

		for( uintsize x = 0; x < 3; ++x )
		{
			const uintsize nextX = x + 1u;

			const ImUiPos posTl = ImUiPosCreate( xPositions[ x ], yPositions[ y ] );
			const ImUiPos posBr = ImUiPosCreate( xPositions[ nextX ], yPositions[ nextY ] );

			const ImUiTexCoord uv =
			{
				uPositions[ x ], vPositions[ y ],
				uPositions[ nextX ], vPositions[ nextY ]
			};

			image.uv = uv;

			rect.pos			= posTl;
			rect.size.width		= posBr.x - posTl.x;
			rect.size.height	= posBr.y - posTl.y;

			ImUiWidgetDrawPartialImageColor( tabView->body, rect, &image, color );
		}
	}

	return tabView->body;
}

void ImUiToolboxTabViewBodyEnd( ImUiToolboxTabViewContext* tabView )
{
	IMUI_ASSERT( tabView->body );

	ImUiWidgetEnd( tabView->body );
	tabView->body = NULL;
}

void ImUiToolboxTabViewEnd( ImUiToolboxTabViewContext* tabView )
{
	IMUI_ASSERT( !tabView->head );
	IMUI_ASSERT( !tabView->body );

	ImUiWidgetEnd( tabView->view );
	tabView->view = NULL;
}

#if defined( _MSC_VER )
#	pragma warning(pop)
#endif
