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

#if defined( __STDC_VERSION__ ) && __STDC_VERSION__ < 201112L
#	define static_assert(x, m) _Static_assert(x, m)
#endif

static ImuiToolboxTheme s_theme;

struct ImuiToolboxScrollAreaState
{
	ImuiPos			offset;
	bool			wasPressedX;
	bool			wasPressedY;
	ImuiPos			pressPoint;
};

struct ImuiToolboxTextBuffer
{
	ImuiAllocator*	allocator;

	char*			data;
	uintsize		dataLength;
	uintsize		dataCapacity;

	uintsize*		lines;
	uintsize		linesLength;
	uintsize		linesCapacity;
};

typedef struct ImuiToolboxTextEditState
{
	bool			hasFocus;

	ImuiPos			offset;
	bool			wasPressedX;
	bool			wasPressedY;
	ImuiPos			pressPoint;

	uint32			selectionStart;
	uint32			selectionEnd;
	uint32			cursorPos;
} ImuiToolboxTextEditState;

struct ImuiToolboxListState
{
	bool			hasFocus;

	uintsize		selectedIndex;
};

struct ImuiToolboxDropDownState
{
	bool			isOpen;

	uintsize		selectedIndex;
};

struct ImuiToolboxTabViewState
{
	uintsize		selectedTab;
};

static void imuiToolboxListItemEndInternal( ImuiToolboxListContext* list );

static const ImuiToolboxThemeReflectionField s_themeReflectionFields[] =
{
	{ "Text/Color",							ImuiToolboxThemeReflectionType_Color,	offsetof( ImuiToolboxTheme, colors[ ImuiToolboxColor_Text ] ) },
	{ "Button/Color",						ImuiToolboxThemeReflectionType_Color,	offsetof( ImuiToolboxTheme, colors[ ImuiToolboxColor_Button ] ) },
	{ "Button/Hover Color",					ImuiToolboxThemeReflectionType_Color,	offsetof( ImuiToolboxTheme, colors[ ImuiToolboxColor_ButtonHover ] ) },
	{ "Button/Clicked Color",				ImuiToolboxThemeReflectionType_Color,	offsetof( ImuiToolboxTheme, colors[ ImuiToolboxColor_ButtonClicked ] ) },
	{ "Button/Text Color",					ImuiToolboxThemeReflectionType_Color,	offsetof( ImuiToolboxTheme, colors[ ImuiToolboxColor_ButtonText ] ) },
	{ "Check Box/Color",					ImuiToolboxThemeReflectionType_Color,	offsetof( ImuiToolboxTheme, colors[ ImuiToolboxColor_CheckBox ] ) },
	{ "Check Box/Hover Color",				ImuiToolboxThemeReflectionType_Color,	offsetof( ImuiToolboxTheme, colors[ ImuiToolboxColor_CheckBoxHover ] ) },
	{ "Check Box/Clicked Color",			ImuiToolboxThemeReflectionType_Color,	offsetof( ImuiToolboxTheme, colors[ ImuiToolboxColor_CheckBoxClicked ] ) },
	{ "Check Box/Unchecked Color",			ImuiToolboxThemeReflectionType_Color,	offsetof( ImuiToolboxTheme, colors[ ImuiToolboxColor_CheckBoxUnchecked ] ) },
	{ "Check Box/Checked Color",			ImuiToolboxThemeReflectionType_Color,	offsetof( ImuiToolboxTheme, colors[ ImuiToolboxColor_CheckBoxChecked ] ) },
	{ "Slider/Background Color",			ImuiToolboxThemeReflectionType_Color,	offsetof( ImuiToolboxTheme, colors[ ImuiToolboxColor_SliderBackground ] ) },
	{ "Slider/Pivot Color",					ImuiToolboxThemeReflectionType_Color,	offsetof( ImuiToolboxTheme, colors[ ImuiToolboxColor_SliderPivot ] ) },
	{ "Slider/Pivot Hover Color",			ImuiToolboxThemeReflectionType_Color,	offsetof( ImuiToolboxTheme, colors[ ImuiToolboxColor_SliderPivotHover ] ) },
	{ "Slider/Pivot Clicked Color",			ImuiToolboxThemeReflectionType_Color,	offsetof( ImuiToolboxTheme, colors[ ImuiToolboxColor_SliderPivotClicked ] ) },
	{ "Text Edit/Background Color",			ImuiToolboxThemeReflectionType_Color,	offsetof( ImuiToolboxTheme, colors[ ImuiToolboxColor_TextEditBackground ] ) },
	{ "Text Edit/Text Color",				ImuiToolboxThemeReflectionType_Color,	offsetof( ImuiToolboxTheme, colors[ ImuiToolboxColor_TextEditText ] ) },
	{ "Text Edit/Cursor Color",				ImuiToolboxThemeReflectionType_Color,	offsetof( ImuiToolboxTheme, colors[ ImuiToolboxColor_TextEditCursor ] ) },
	{ "Text Edit/Selection Color",			ImuiToolboxThemeReflectionType_Color,	offsetof( ImuiToolboxTheme, colors[ ImuiToolboxColor_TextEditSelection ] ) },
	{ "Progress Bar/Background Color",		ImuiToolboxThemeReflectionType_Color,	offsetof( ImuiToolboxTheme, colors[ ImuiToolboxColor_ProgressBarBackground ] ) },
	{ "Progress Bar/Progress Color",		ImuiToolboxThemeReflectionType_Color,	offsetof( ImuiToolboxTheme, colors[ ImuiToolboxColor_ProgressBarProgress ] ) },
	{ "Scroll Area/Bar Background Color",	ImuiToolboxThemeReflectionType_Color,	offsetof( ImuiToolboxTheme, colors[ ImuiToolboxColor_ScrollAreaBarBackground ] ) },
	{ "Scroll Area/Bar Pivot Color",		ImuiToolboxThemeReflectionType_Color,	offsetof( ImuiToolboxTheme, colors[ ImuiToolboxColor_ScrollAreaBarPivot ] ) },
	{ "List/Item/Hover Color",				ImuiToolboxThemeReflectionType_Color,	offsetof( ImuiToolboxTheme, colors[ ImuiToolboxColor_ListItemHover ] ) },
	{ "List/Item/Clicked Color",			ImuiToolboxThemeReflectionType_Color,	offsetof( ImuiToolboxTheme, colors[ ImuiToolboxColor_ListItemClicked ] ) },
	{ "List/Item/Selected Color",			ImuiToolboxThemeReflectionType_Color,	offsetof( ImuiToolboxTheme, colors[ ImuiToolboxColor_ListItemSelected ] ) },
	{ "Drop Down/Background Color",			ImuiToolboxThemeReflectionType_Color,	offsetof( ImuiToolboxTheme, colors[ ImuiToolboxColor_DropDown ] ) },
	{ "Drop Down/Text Color",				ImuiToolboxThemeReflectionType_Color,	offsetof( ImuiToolboxTheme, colors[ ImuiToolboxColor_DropDownText ] ) },
	{ "Drop Down/Icon Color",				ImuiToolboxThemeReflectionType_Color,	offsetof( ImuiToolboxTheme, colors[ ImuiToolboxColor_DropDownIcon ] ) },
	{ "Drop Down/Hover Color",				ImuiToolboxThemeReflectionType_Color,	offsetof( ImuiToolboxTheme, colors[ ImuiToolboxColor_DropDownHover ] ) },
	{ "Drop Down/Clicked Color",			ImuiToolboxThemeReflectionType_Color,	offsetof( ImuiToolboxTheme, colors[ ImuiToolboxColor_DropDownClicked ] ) },
	{ "Drop Down/Open Color",				ImuiToolboxThemeReflectionType_Color,	offsetof( ImuiToolboxTheme, colors[ ImuiToolboxColor_DropDownOpen ] ) },
	{ "Drop Down/List/Color",				ImuiToolboxThemeReflectionType_Color,	offsetof( ImuiToolboxTheme, colors[ ImuiToolboxColor_DropDownList ] ) },
	{ "Drop Down/Item/Text Color",			ImuiToolboxThemeReflectionType_Color,	offsetof( ImuiToolboxTheme, colors[ ImuiToolboxColor_DropDownItemText ] ) },
	{ "Drop Down/Item/Hover Color",			ImuiToolboxThemeReflectionType_Color,	offsetof( ImuiToolboxTheme, colors[ ImuiToolboxColor_DropDownItemHover ] ) },
	{ "Drop Down/Item/Clicked Color",		ImuiToolboxThemeReflectionType_Color,	offsetof( ImuiToolboxTheme, colors[ ImuiToolboxColor_DropDownItemClicked ] ) },
	{ "Drop Down/Item/Selected Color",		ImuiToolboxThemeReflectionType_Color,	offsetof( ImuiToolboxTheme, colors[ ImuiToolboxColor_DropDownItemSelected ] ) },
	{ "Popup/Background Color",				ImuiToolboxThemeReflectionType_Color,	offsetof( ImuiToolboxTheme, colors[ ImuiToolboxColor_PopupBackground ] ) },
	{ "Popup/Color",						ImuiToolboxThemeReflectionType_Color,	offsetof( ImuiToolboxTheme, colors[ ImuiToolboxColor_Popup ] ) },
	{ "Tab View/Head Background Color",		ImuiToolboxThemeReflectionType_Color,	offsetof( ImuiToolboxTheme, colors[ ImuiToolboxColor_TabViewHeadBackground ] ) },
	{ "Tab View/Header Active Color",		ImuiToolboxThemeReflectionType_Color,	offsetof( ImuiToolboxTheme, colors[ ImuiToolboxColor_TabViewHeaderActive ] ) },
	{ "Tab View/Header Inactive Color",		ImuiToolboxThemeReflectionType_Color,	offsetof( ImuiToolboxTheme, colors[ ImuiToolboxColor_TabViewHeaderInactive ] ) },
	{ "Tab View/Body Background Color",		ImuiToolboxThemeReflectionType_Color,	offsetof( ImuiToolboxTheme, colors[ ImuiToolboxColor_TabViewBody ] ) },

	{ "Button/Skin",						ImuiToolboxThemeReflectionType_Skin,	offsetof( ImuiToolboxTheme, skins[ ImuiToolboxSkin_Button ] ) },
	{ "Button/Hover Skin",					ImuiToolboxThemeReflectionType_Skin,	offsetof( ImuiToolboxTheme, skins[ ImuiToolboxSkin_ButtonHover ] ) },
	{ "Button/Clicked Skin",				ImuiToolboxThemeReflectionType_Skin,	offsetof( ImuiToolboxTheme, skins[ ImuiToolboxSkin_ButtonClicked ] ) },
	{ "Check Box/Skin",						ImuiToolboxThemeReflectionType_Skin,	offsetof( ImuiToolboxTheme, skins[ ImuiToolboxSkin_CheckBox ] ) },
	{ "Check Box/Checked Skin",				ImuiToolboxThemeReflectionType_Skin,	offsetof( ImuiToolboxTheme, skins[ ImuiToolboxSkin_CheckBoxChecked ] ) },
	{ "Slider/Background Skin",				ImuiToolboxThemeReflectionType_Skin,	offsetof( ImuiToolboxTheme, skins[ ImuiToolboxSkin_SliderBackground ] ) },
	{ "Slider/Pivot Skin",					ImuiToolboxThemeReflectionType_Skin,	offsetof( ImuiToolboxTheme, skins[ ImuiToolboxSkin_SliderPivot ] ) },
	{ "Text Edit/Background Skin",			ImuiToolboxThemeReflectionType_Skin,	offsetof( ImuiToolboxTheme, skins[ ImuiToolboxSkin_TextEditBackground ] ) },
	{ "Progress Bar/Background Skin",		ImuiToolboxThemeReflectionType_Skin,	offsetof( ImuiToolboxTheme, skins[ ImuiToolboxSkin_ProgressBarBackground ] ) },
	{ "Progress Bar/Progress Skin",			ImuiToolboxThemeReflectionType_Skin,	offsetof( ImuiToolboxTheme, skins[ ImuiToolboxSkin_ProgressBarProgress ] ) },
	{ "Scroll Area/Bar Background Skin",	ImuiToolboxThemeReflectionType_Skin,	offsetof( ImuiToolboxTheme, skins[ ImuiToolboxSkin_ScrollAreaBarBackground ] ) },
	{ "Scroll Area/Bar Pivot Skin",			ImuiToolboxThemeReflectionType_Skin,	offsetof( ImuiToolboxTheme, skins[ ImuiToolboxSkin_ScrollAreaBarPivot ] ) },
	{ "List/Item/Skin",						ImuiToolboxThemeReflectionType_Skin,	offsetof( ImuiToolboxTheme, skins[ ImuiToolboxSkin_ListItem ] ) },
	{ "List/Item/Selected Skin",			ImuiToolboxThemeReflectionType_Skin,	offsetof( ImuiToolboxTheme, skins[ ImuiToolboxSkin_ItemSelected ] ) },
	{ "Drop Down/Skin",						ImuiToolboxThemeReflectionType_Skin,	offsetof( ImuiToolboxTheme, skins[ ImuiToolboxSkin_DropDown ] ) },
	{ "Drop Down/List/Skin",				ImuiToolboxThemeReflectionType_Skin,	offsetof( ImuiToolboxTheme, skins[ ImuiToolboxSkin_DropDownList ] ) },
	{ "Drop Down/Item/Skin",				ImuiToolboxThemeReflectionType_Skin,	offsetof( ImuiToolboxTheme, skins[ ImuiToolboxSkin_DropDownItem ] ) },
	{ "Popup/Skin",							ImuiToolboxThemeReflectionType_Skin,	offsetof( ImuiToolboxTheme, skins[ ImuiToolboxSkin_Popup ] ) },
	{ "Tab View/Head Skin",					ImuiToolboxThemeReflectionType_Skin,	offsetof( ImuiToolboxTheme, skins[ ImuiToolboxSkin_TabViewHeadBackground ] ) },
	{ "Tab View/Header Active Skin",		ImuiToolboxThemeReflectionType_Skin,	offsetof( ImuiToolboxTheme, skins[ ImuiToolboxSkin_TabViewHeaderActive ] ) },
	{ "Tab View/Header Inactive Skin",		ImuiToolboxThemeReflectionType_Skin,	offsetof( ImuiToolboxTheme, skins[ ImuiToolboxSkin_TabViewHeaderInactive ] ) },
	{ "Tab View/Body Skin",					ImuiToolboxThemeReflectionType_Skin,	offsetof( ImuiToolboxTheme, skins[ ImuiToolboxSkin_TabViewBody ] ) },

	{ "Check Box/Unchecked Icon",			ImuiToolboxThemeReflectionType_Image,	offsetof( ImuiToolboxTheme, icons[ ImuiToolboxIcon_CheckBoxUnchecked ] ) },
	{ "Check Box/Checked Icon",				ImuiToolboxThemeReflectionType_Image,	offsetof( ImuiToolboxTheme, icons[ ImuiToolboxIcon_CheckBoxChecked ] ) },
	{ "Drop Down/Open Icon",				ImuiToolboxThemeReflectionType_Image,	offsetof( ImuiToolboxTheme, icons[ ImuiToolboxIcon_DropDownOpen ] ) },
	{ "Drop Down/Close Icon",				ImuiToolboxThemeReflectionType_Image,	offsetof( ImuiToolboxTheme, icons[ ImuiToolboxIcon_DropDownClose ] ) },

	{ "Text/Font",							ImuiToolboxThemeReflectionType_Font,	offsetof( ImuiToolboxTheme, font ) },

	{ "Button/Height",						ImuiToolboxThemeReflectionType_Float,	offsetof( ImuiToolboxTheme, button.height ) },
	{ "Button/Padding",						ImuiToolboxThemeReflectionType_Border,	offsetof( ImuiToolboxTheme, button.padding ) },

	{ "Check Box/Size",						ImuiToolboxThemeReflectionType_Size,	offsetof( ImuiToolboxTheme, checkBox.size ) },
	{ "Check Box/Text Spacing",				ImuiToolboxThemeReflectionType_Float,	offsetof( ImuiToolboxTheme, checkBox.textSpacing ) },

	{ "Slider/Height",						ImuiToolboxThemeReflectionType_Float,	offsetof( ImuiToolboxTheme, slider.height ) },
	{ "Slider/Padding",						ImuiToolboxThemeReflectionType_Border,	offsetof( ImuiToolboxTheme, slider.padding ) },
	{ "Slider/Pivot Size",					ImuiToolboxThemeReflectionType_Size,	offsetof( ImuiToolboxTheme, slider.pivotSize ) },

	{"Text Edit/height",					ImuiToolboxThemeReflectionType_Float,	offsetof( ImuiToolboxTheme, textEdit.height ) },
	{"Text Edit/padding",					ImuiToolboxThemeReflectionType_Border,	offsetof( ImuiToolboxTheme, textEdit.padding ) },
	{"Text Edit/cursorSize",				ImuiToolboxThemeReflectionType_Size,	offsetof( ImuiToolboxTheme, textEdit.cursorSize ) },
	{"Text Edit/blinkTime",					ImuiToolboxThemeReflectionType_Double,	offsetof( ImuiToolboxTheme, textEdit.blinkTime ) },

	{ "Progress Bar/Height",				ImuiToolboxThemeReflectionType_Float,	offsetof( ImuiToolboxTheme, progressBar.height ) },
	{ "Progress Bar/Padding",				ImuiToolboxThemeReflectionType_Border,	offsetof( ImuiToolboxTheme, progressBar.padding ) },

	{ "Scroll Area/Bar Size",				ImuiToolboxThemeReflectionType_Float,	offsetof( ImuiToolboxTheme, scrollArea.barSize ) },
	{ "Scroll Area/Bar Spacing",			ImuiToolboxThemeReflectionType_Float,	offsetof( ImuiToolboxTheme, scrollArea.barSpacing ) },
	{ "Scroll Area/Bar MinSize",			ImuiToolboxThemeReflectionType_Float,	offsetof( ImuiToolboxTheme, scrollArea.barMinSize ) },

	{ "List/Item Spacing",					ImuiToolboxThemeReflectionType_Float,	offsetof( ImuiToolboxTheme, list.itemSpacing ) },

	{ "Drop Down/Height",					ImuiToolboxThemeReflectionType_Float,	offsetof( ImuiToolboxTheme, dropDown.height ) },
	{ "Drop Down/Padding",					ImuiToolboxThemeReflectionType_Border,	offsetof( ImuiToolboxTheme, dropDown.padding ) },
	{ "Drop Down/List/ZOrder",				ImuiToolboxThemeReflectionType_UInt32,	offsetof( ImuiToolboxTheme, dropDown.listZOrder ) },
	{ "Drop Down/List/Margin",				ImuiToolboxThemeReflectionType_Border,	offsetof( ImuiToolboxTheme, dropDown.listMargin ) },
	{ "Drop Down/List/MaxLength",			ImuiToolboxThemeReflectionType_UInt32,	offsetof( ImuiToolboxTheme, dropDown.listMaxLength ) },
	{ "Drop Down/Item/Padding",				ImuiToolboxThemeReflectionType_Border,	offsetof( ImuiToolboxTheme, dropDown.itemPadding ) },
	{ "Drop Down/Item/Size",				ImuiToolboxThemeReflectionType_Float,	offsetof( ImuiToolboxTheme, dropDown.itemSize ) },
	{ "Drop Down/Item/Spacing",				ImuiToolboxThemeReflectionType_Float,	offsetof( ImuiToolboxTheme, dropDown.itemSpacing ) },

	{ "Popup/Z Order",						ImuiToolboxThemeReflectionType_UInt32,	offsetof( ImuiToolboxTheme, popup.zOrder ) },
	{ "Popup/Padding",						ImuiToolboxThemeReflectionType_Border,	offsetof( ImuiToolboxTheme, popup.padding ) },
	{ "Popup/Button Spacing",				ImuiToolboxThemeReflectionType_Float,	offsetof( ImuiToolboxTheme, popup.buttonSpacing ) },

	{ "Tab View/Header Spacing",			ImuiToolboxThemeReflectionType_Float,	offsetof( ImuiToolboxTheme, tabView.headerSpacing ) },
	{ "Tab View/Header Cut Extend Left",	ImuiToolboxThemeReflectionType_Float,	offsetof( ImuiToolboxTheme, tabView.headerCutLeft ) },
	{ "Tab View/Header Cut Extend Right",	ImuiToolboxThemeReflectionType_Float,	offsetof( ImuiToolboxTheme, tabView.headerCutRight ) },
	{ "Tab View/Header Padding",			ImuiToolboxThemeReflectionType_Border,	offsetof( ImuiToolboxTheme, tabView.headerPadding ) },
	{ "Tab View/Body Padding",				ImuiToolboxThemeReflectionType_Border,	offsetof( ImuiToolboxTheme, tabView.bodyPadding ) },
};
static_assert( ImuiToolboxColor_MAX == 42, "more colors" );
static_assert( ImuiToolboxSkin_MAX == 22, "more skins" );
static_assert( ImuiToolboxIcon_MAX == 4, "more icons" );
static_assert( sizeof( ImuiToolboxTheme ) == 1640u, "theme changed" );

ImuiToolboxThemeReflection imuiToolboxThemeReflectionGet()
{
	ImuiToolboxThemeReflection reflection;
	reflection.fields	= s_themeReflectionFields;
	reflection.count	= IMUI_ARRAY_COUNT( s_themeReflectionFields );
	return reflection;
}

ImuiToolboxTheme* imuiToolboxThemeGet()
{
	return &s_theme;
}

void imuiToolboxThemeFillDefault( ImuiToolboxTheme* theme, ImuiFont* font )
{
	const ImuiColor textColor			= imuiColorCreateWhite();
	const ImuiColor elementColor		= imuiColorCreateGray( 0xb2u );
	const ImuiColor elementHoverColor	= imuiColorCreateGray( 0xe5u );
	const ImuiColor elementClickedColor	= imuiColorCreateGray( 0x66u );
	const ImuiColor backgroundColor		= imuiColorCreateGray( 0x4cu );
	const ImuiColor textEditCursorColor	= imuiColorCreateBlack();

	theme->colors[ ImuiToolboxColor_Text ]						= textColor;
	theme->colors[ ImuiToolboxColor_Button ]					= elementColor;
	theme->colors[ ImuiToolboxColor_ButtonHover ]				= elementHoverColor;
	theme->colors[ ImuiToolboxColor_ButtonClicked ]				= elementClickedColor;
	theme->colors[ ImuiToolboxColor_ButtonText ]				= textColor;
	theme->colors[ ImuiToolboxColor_CheckBox ]					= elementColor;
	theme->colors[ ImuiToolboxColor_CheckBoxHover ]				= elementHoverColor;
	theme->colors[ ImuiToolboxColor_CheckBoxClicked ]			= elementClickedColor;
	theme->colors[ ImuiToolboxColor_CheckBoxUnchecked ]			= imuiColorCreateTransparentBlack();
	theme->colors[ ImuiToolboxColor_CheckBoxChecked ]			= textColor;
	theme->colors[ ImuiToolboxColor_SliderBackground ]			= backgroundColor;
	theme->colors[ ImuiToolboxColor_SliderPivot ]				= elementColor;
	theme->colors[ ImuiToolboxColor_SliderPivotHover ]			= elementHoverColor;
	theme->colors[ ImuiToolboxColor_SliderPivotClicked ]		= elementClickedColor;
	theme->colors[ ImuiToolboxColor_TextEditBackground ]		= elementClickedColor;
	theme->colors[ ImuiToolboxColor_TextEditText ]				= textColor;
	theme->colors[ ImuiToolboxColor_TextEditCursor ]			= textEditCursorColor;
	theme->colors[ ImuiToolboxColor_TextEditSelection ]			= elementColor;
	theme->colors[ ImuiToolboxColor_ProgressBarBackground ]		= backgroundColor;
	theme->colors[ ImuiToolboxColor_ProgressBarProgress ]		= elementColor;
	theme->colors[ ImuiToolboxColor_ScrollAreaBarBackground ]	= backgroundColor;
	theme->colors[ ImuiToolboxColor_ScrollAreaBarPivot ]		= elementColor;
	theme->colors[ ImuiToolboxColor_ListItemHover ]				= elementHoverColor;
	theme->colors[ ImuiToolboxColor_ListItemClicked ]			= elementClickedColor;
	theme->colors[ ImuiToolboxColor_ListItemSelected ]			= elementColor;
	theme->colors[ ImuiToolboxColor_DropDown ]					= elementClickedColor;
	theme->colors[ ImuiToolboxColor_DropDownText ]				= textColor;
	theme->colors[ ImuiToolboxColor_DropDownIcon ]				= textColor;
	theme->colors[ ImuiToolboxColor_DropDownHover ]				= elementHoverColor;
	theme->colors[ ImuiToolboxColor_DropDownClicked ]			= elementClickedColor;
	theme->colors[ ImuiToolboxColor_DropDownOpen ]				= elementColor;
	theme->colors[ ImuiToolboxColor_DropDownList ]				= backgroundColor;
	theme->colors[ ImuiToolboxColor_DropDownItemText ]			= textColor;
	theme->colors[ ImuiToolboxColor_DropDownItemHover ]			= elementHoverColor;
	theme->colors[ ImuiToolboxColor_DropDownItemClicked ]		= elementClickedColor;
	theme->colors[ ImuiToolboxColor_DropDownItemSelected ]		= elementColor;
	theme->colors[ ImuiToolboxColor_PopupBackground ]			= imuiColorCreateFloat( 0.0f, 0.0f, 0.0f, 0.2f );
	theme->colors[ ImuiToolboxColor_Popup ]						= backgroundColor;
	theme->colors[ ImuiToolboxColor_TabViewHeadBackground ]		= backgroundColor;
	theme->colors[ ImuiToolboxColor_TabViewHeaderActive ]		= elementColor;
	theme->colors[ ImuiToolboxColor_TabViewHeaderInactive ]		= elementClickedColor;
	theme->colors[ ImuiToolboxColor_TabViewBody ]				= backgroundColor;
	static_assert( ImuiToolboxColor_MAX == 42, "more colors" );

	const ImuiSkin skin = { IMUI_TEXTURE_HANDLE_INVALID };

	theme->skins[ ImuiToolboxSkin_Button ]						= skin;
	theme->skins[ ImuiToolboxSkin_ButtonHover ]					= skin;
	theme->skins[ ImuiToolboxSkin_ButtonClicked ]				= skin;
	theme->skins[ ImuiToolboxSkin_CheckBox ]					= skin;
	theme->skins[ ImuiToolboxSkin_CheckBoxChecked ]				= skin;
	theme->skins[ ImuiToolboxSkin_SliderBackground ]			= skin;
	theme->skins[ ImuiToolboxSkin_SliderPivot ]					= skin;
	theme->skins[ ImuiToolboxSkin_TextEditBackground ]			= skin;
	theme->skins[ ImuiToolboxSkin_ProgressBarBackground ]		= skin;
	theme->skins[ ImuiToolboxSkin_ProgressBarProgress ]			= skin;
	theme->skins[ ImuiToolboxSkin_ScrollAreaBarBackground ]		= skin;
	theme->skins[ ImuiToolboxSkin_ScrollAreaBarPivot ]			= skin;
	theme->skins[ ImuiToolboxSkin_ListItem ]					= skin;
	theme->skins[ ImuiToolboxSkin_ItemSelected ]				= skin;
	theme->skins[ ImuiToolboxSkin_DropDown ]					= skin;
	theme->skins[ ImuiToolboxSkin_DropDownList ]				= skin;
	theme->skins[ ImuiToolboxSkin_DropDownItem ]				= skin;
	theme->skins[ ImuiToolboxSkin_Popup ]						= skin;
	theme->skins[ ImuiToolboxSkin_TabViewHeadBackground ]		= skin;
	theme->skins[ ImuiToolboxSkin_TabViewHeaderActive ]			= skin;
	theme->skins[ ImuiToolboxSkin_TabViewHeaderInactive ]		= skin;
	theme->skins[ ImuiToolboxSkin_TabViewBody ]					= skin;
	static_assert( ImuiToolboxSkin_MAX == 22, "more skins" );

	const ImuiImage image = { IMUI_TEXTURE_HANDLE_INVALID, 22u, 22u, { 0.0f, 0.0f, 1.0f, 1.0f } };

	theme->icons[ ImuiToolboxIcon_CheckBoxUnchecked ]			= image;
	theme->icons[ ImuiToolboxIcon_CheckBoxChecked ]				= image;
	theme->icons[ ImuiToolboxIcon_DropDownOpen ]				= image;
	theme->icons[ ImuiToolboxIcon_DropDownClose ]				= image;
	static_assert( ImuiToolboxIcon_MAX == 4, "more icons" );

	theme->font						= font;

	theme->button.height			= 25.0f;
	theme->button.padding			= imuiBorderCreate( 0.0f, 8.0f, 0.0f, 8.0f );

	theme->checkBox.size			= imuiSizeCreateAll( 25.0f );
	theme->checkBox.textSpacing		= 8.0f;

	theme->slider.height			= 25.0f;
	theme->slider.padding			= imuiBorderCreateHorizontalVertical( 5.0f, 0.0f );
	theme->slider.pivotSize			= imuiSizeCreate( 10.0f, 25.0f );

	theme->textEdit.height			= 25.0f;
	theme->textEdit.padding			= imuiBorderCreateAll( 2.0f );
	theme->textEdit.cursorSize		= imuiSizeCreate( 1.0f, 21.0f );
	theme->textEdit.blinkTime		= 0.53f;

	theme->progressBar.height		= 25.0f;
	theme->progressBar.padding		= imuiBorderCreateAll( 2.0f );

	theme->scrollArea.barSize		= 8.0f;
	theme->scrollArea.barSpacing	= 8.0f;
	theme->scrollArea.barMinSize	= 25.0f;

	theme->list.itemSpacing			= 8.0f;

	theme->dropDown.height			= 25.0f;
	theme->dropDown.padding			= imuiBorderCreate( 0.0f, 4.0f, 0.0f, 0.0f );
	theme->dropDown.listZOrder		= 20u;
	theme->dropDown.listMaxLength	= 12u;
	theme->dropDown.listMargin		= imuiBorderCreate( 0.0f, 0.0f, 0.0f, 0.0f );
	theme->dropDown.itemPadding		= imuiBorderCreate( 0.0f, 4.0f, 0.0f, 0.0f );
	theme->dropDown.itemSize		= 25.0f;
	theme->dropDown.itemSpacing		= 8.0f;

	theme->popup.zOrder				= 10u;
	theme->popup.padding			= imuiBorderCreateAll( 8.0f );
	theme->popup.buttonSpacing		= 4.0f;

	theme->tabView.headerSpacing	= 4.0f;
	theme->tabView.headerCutLeft	= 0.0f;
	theme->tabView.headerCutRight	= 0.0f;
	theme->tabView.headerPadding	= imuiBorderCreateAll( 8.0f );
	theme->tabView.bodyPadding		= imuiBorderCreateAll( 8.0f );
}

void imuiToolboxThemeSet( const ImuiToolboxTheme* theme )
{
	s_theme = *theme;
}

void imuiToolboxSpacer( ImuiWindow* window, float width, float height )
{
	ImuiWidget* widget = imuiWidgetBegin( window );
	imuiWidgetSetFixedSizeFloat( widget, width, height );
	imuiWidgetEnd( widget );
}

void imuiToolboxStrecher( ImuiWindow* window, float horizontal, float vertical )
{
	ImuiWidget* widget = imuiWidgetBegin( window );
	imuiWidgetSetStretch( widget, horizontal, vertical );
	imuiWidgetEnd( widget );
}

ImuiWidget* imuiToolboxButtonBegin( ImuiWindow* window )
{
	ImuiWidget* button = imuiWidgetBegin( window );
	imuiWidgetSetFixedHeight( button, s_theme.button.height );
	imuiWidgetSetPadding( button, s_theme.button.padding );
	imuiWidgetSetCanHaveFocus( button );

	ImuiWidgetInputState inputState;
	imuiWidgetGetInputState( button, &inputState );

	ImuiColor color = s_theme.colors[ ImuiToolboxColor_Button ];
	const ImuiSkin* skin = &s_theme.skins[ ImuiToolboxSkin_Button ];
	if( inputState.wasPressed && inputState.isMouseDown )
	{
		color = s_theme.colors[ ImuiToolboxColor_ButtonClicked ];
		skin = &s_theme.skins[ ImuiToolboxSkin_ButtonClicked ];
	}
	else if( inputState.isMouseOver || inputState.hasFocus )
	{
		color = s_theme.colors[ ImuiToolboxColor_ButtonHover ];
		skin = &s_theme.skins[ ImuiToolboxSkin_ButtonHover ];
	}

	imuiWidgetDrawSkin( button, skin, color );

	return button;
}

bool imuiToolboxButtonEnd( ImuiWidget* button )
{
	imuiWidgetEnd( button );

	ImuiWidgetInputState inputState;
	imuiWidgetGetInputState( button, &inputState );

	return (inputState.wasPressed && inputState.hasMouseReleased) || (inputState.hasFocus && imuiInputGetShortcut( imuiWidgetGetInput( button ) ) == ImuiInputShortcut_Confirm);
}

ImuiWidget* imuiToolboxButtonLabelBegin( ImuiWindow* window, const char* text )
{
	ImuiWidget* buttonFrame = imuiToolboxButtonBegin( window );

	ImuiWidget* buttonText = imuiWidgetBegin( window );

	ImuiTextLayout* layout = imuiTextLayoutCreateWidget( buttonText, s_theme.font, text );
	const ImuiSize textSize = imuiTextLayoutGetSize( layout );
	imuiWidgetSetAlign( buttonText, 0.5f, 0.5f );
	imuiWidgetSetFixedSize( buttonText, textSize );

	if( layout )
	{
		imuiWidgetDrawText( buttonText, layout, s_theme.colors[ ImuiToolboxColor_ButtonText ] );
	}

	imuiWidgetEnd( buttonText );

	return buttonFrame;
}

ImuiWidget* imuiToolboxButtonLabelBeginFormat( ImuiWindow* window, const char* format, ... )
{
	va_list args;
	va_start( args, format );
	ImuiWidget* button = imuiToolboxButtonLabelBeginFormatArgs( window, format, args );
	va_end( args );

	return button;
}

ImuiWidget* imuiToolboxButtonLabelBeginFormatArgs( ImuiWindow* window, const char* format, va_list args )
{
	char buffer[ 256u ];

	int length = vsnprintf( buffer, sizeof( buffer ), format, args );
	if( length < 0 )
	{
		return false;
	}
	else if( length >= sizeof( buffer ) )
	{
		char* headBuffer = (char*)imuiMemoryAlloc( &window->context->allocator, length + 1u );
		if( !headBuffer )
		{
			return false;
		}

		length = vsnprintf( headBuffer, length + 1u, format, args );

		ImuiWidget* button = imuiToolboxButtonLabelBegin( window, headBuffer );

		imuiMemoryFree( &window->context->allocator, headBuffer );
		return button;
	}

	return imuiToolboxButtonLabelBegin( window, buffer );
}

bool imuiToolboxButtonLabel( ImuiWindow* window, const char* text )
{
	ImuiWidget* button = imuiToolboxButtonLabelBegin( window, text );
	return imuiToolboxButtonEnd( button );
}

bool imuiToolboxButtonLabelFormat( ImuiWindow* window, const char* format, ... )
{
	va_list args;
	va_start( args, format );
	const bool result = imuiToolboxButtonLabelFormatArgs( window, format, args );
	va_end( args );

	return result;
}

bool imuiToolboxButtonLabelFormatArgs( ImuiWindow* window, const char* format, va_list args )
{
	ImuiWidget* button = imuiToolboxButtonLabelBeginFormatArgs( window, format, args );
	return imuiToolboxButtonEnd( button );
}

ImuiWidget* imuiToolboxButtonIconBegin( ImuiWindow* window, const ImuiImage* icon, ImuiSize iconSize )
{
	IMUI_ASSERT( icon );

	ImuiWidget* buttonFrame = imuiToolboxButtonBegin( window );

	ImuiWidget* buttonIcon = imuiWidgetBegin( window );
	imuiWidgetSetAlign( buttonIcon, 0.5f, 0.5f );
	imuiWidgetSetFixedSize( buttonIcon, iconSize );

	imuiWidgetDrawImageColor( buttonIcon, icon, s_theme.colors[ ImuiToolboxColor_ButtonText ] );

	imuiWidgetEnd( buttonIcon );

	return buttonFrame;
}

bool imuiToolboxButtonIcon( ImuiWindow* window, const ImuiImage* icon )
{
	return imuiToolboxButtonIconSize( window, icon, imuiSizeCreateImage( icon ) );
}

bool imuiToolboxButtonIconSize( ImuiWindow* window, const ImuiImage* icon, ImuiSize iconSize )
{
	ImuiWidget* button = imuiToolboxButtonIconBegin( window, icon, iconSize );
	return imuiToolboxButtonEnd( button );
}

ImuiWidget* imuiToolboxCheckBoxBegin( ImuiWindow* window )
{
	ImuiWidget* checkBoxFrame = imuiWidgetBegin( window );
	imuiWidgetSetPadding( checkBoxFrame, imuiBorderCreate( 0.0f, s_theme.checkBox.size.width + s_theme.checkBox.textSpacing, 0.0f, 0.0f ) );
	imuiWidgetSetFixedHeight( checkBoxFrame, s_theme.checkBox.size.height );
	imuiWidgetSetVAlign( checkBoxFrame, 0.5f );

	return checkBoxFrame;
}

bool imuiToolboxCheckBoxEnd( ImuiWidget* checkBox, bool* checked, const char* text )
{
	ImuiWidgetInputState inputState;
	imuiWidgetGetInputState( checkBox, &inputState );

	ImuiColor color = s_theme.colors[ ImuiToolboxColor_CheckBox ];
	if( inputState.wasPressed && inputState.isMouseDown )
	{
		color = s_theme.colors[ ImuiToolboxColor_CheckBoxClicked ];
	}
	else if( inputState.isMouseOver )
	{
		color = s_theme.colors[ ImuiToolboxColor_CheckBoxHover ];
	}

	const float dpiScale = imuiWidgetGetDpiScale( checkBox );
	const float checkBackgroundY = (imuiWidgetGetSizeHeight( checkBox ) / 2.0f) - (s_theme.checkBox.size.height * dpiScale / 2.0f);
	const ImuiRect checkBackgroundRect = imuiRectCreatePosSize( imuiPosCreate( 0.0f, checkBackgroundY ), imuiSizeScale( s_theme.checkBox.size, dpiScale ) );
	imuiWidgetDrawPartialSkin( checkBox, checkBackgroundRect, &s_theme.skins[ ImuiToolboxSkin_CheckBox ], color );

	const ImuiImage* icon = &s_theme.icons[ *checked ? ImuiToolboxIcon_CheckBoxChecked : ImuiToolboxIcon_CheckBoxUnchecked ];
	const ImuiRect checkIconRect = imuiRectCreateCenterPosSize( imuiRectGetCenter( checkBackgroundRect ), imuiSizeScale( imuiSizeCreateImage( icon ), dpiScale ) );
	const ImuiColor checkIconColor = s_theme.colors[ *checked ? ImuiToolboxColor_CheckBoxChecked : ImuiToolboxColor_CheckBoxUnchecked ];
	imuiWidgetDrawPartialImageColor( checkBox, checkIconRect, icon, checkIconColor );

	if( text )
	{
		ImuiWidget* checkBoxText = imuiWidgetBegin( imuiWidgetGetWindow( checkBox ) );

		ImuiTextLayout* layout = imuiTextLayoutCreateWidget( checkBoxText, s_theme.font, text );
		const ImuiSize textSize = imuiTextLayoutGetSize( layout );
		imuiWidgetSetFixedSize( checkBoxText, textSize );
		imuiWidgetSetVAlign( checkBoxText, 0.5f );

		if( layout )
		{
			imuiWidgetDrawText( checkBoxText, layout, s_theme.colors[ ImuiToolboxColor_Text ] );
		}

		imuiWidgetEnd( checkBoxText );
	}

	imuiWidgetEnd( checkBox );

	if( inputState.wasPressed && inputState.hasMouseReleased )
	{
		*checked = !*checked;
		return true;
	}

	return false;
}

bool imuiToolboxCheckBox( ImuiWindow* window, bool* checked, const char* text )
{
	ImuiWidget* checkBox = imuiToolboxCheckBoxBegin( window );
	return imuiToolboxCheckBoxEnd( checkBox, checked, text);
}

bool imuiToolboxCheckBoxState( ImuiWindow* window, const char* text )
{
	return imuiToolboxCheckBoxStateDefault( window, text, false );
}

bool imuiToolboxCheckBoxStateDefault( ImuiWindow* window, const char* text, bool defaultValue )
{
	ImuiWidget* checkBox = imuiToolboxCheckBoxBegin( window );

	bool isNew;
	bool* checked = (bool*)imuiWidgetAllocStateNew( checkBox, sizeof( bool ), IMUI_ID_STR( "check box" ), &isNew);
	if( isNew )
	{
		*checked = defaultValue;
	}

	imuiToolboxCheckBoxEnd( checkBox, checked, text );
	return *checked;
}

ImuiWidget* imuiToolboxLabelBegin( ImuiWindow* window, const char* text )
{
	return imuiToolboxLabelBeginColor( window, text, s_theme.colors[ ImuiToolboxColor_Text ] );
}

ImuiWidget* imuiToolboxLabelBeginColor( ImuiWindow* window, const char* text, ImuiColor color )
{
	return imuiToolboxLabelBeginLengthColor( window, text, strlen( text ), color );
}

ImuiWidget* imuiToolboxLabelBeginLength( ImuiWindow* window, const char* text, size_t length )
{
	return imuiToolboxLabelBeginLengthColor( window, text, length, s_theme.colors[ ImuiToolboxColor_Text ] );
}

ImuiWidget* imuiToolboxLabelBeginLengthColor( ImuiWindow* window, const char* text, size_t length, ImuiColor color )
{
	ImuiWidget* label = imuiWidgetBegin( window );

	ImuiTextLayout* layout = imuiTextLayoutCreateWidgetLength( label, s_theme.font, text,length );
	const ImuiSize textSize = imuiTextLayoutGetSize( layout );
	imuiWidgetSetMinSize( label, textSize );
	imuiWidgetSetVAlign( label, 0.5f );

	if( layout )
	{
		imuiWidgetDrawText( label, layout, color );
	}

	return label;

}

ImuiWidget* imuiToolboxLabelBeginFormat( ImuiWindow* window, const char* format, ... )
{
	va_list args;
	va_start( args, format );
	ImuiWidget* label = imuiToolboxLabelBeginFormatArgs( window, format, args );
	va_end( args );
	return label;
}

ImuiWidget* imuiToolboxLabelBeginFormatArgs( ImuiWindow* window, const char* format, va_list args )
{
	char buffer[ 256u ];

	int length = vsnprintf( buffer, sizeof( buffer ), format, args );
	if( length < 0 )
	{
		return NULL;
	}
	else if( length >= sizeof( buffer ) )
	{
		char* headBuffer = (char*)imuiMemoryAlloc( &window->context->allocator, length + 1u );
		if( !headBuffer )
		{
			return NULL;
		}

		length = vsnprintf( headBuffer, length + 1u, format, args );

		ImuiWidget* label = imuiToolboxLabelBegin( window, headBuffer );

		imuiMemoryFree( &window->context->allocator, headBuffer );
		return label;
	}

	return imuiToolboxLabelBegin( window, buffer );
}

void imuiToolboxLabelEnd( ImuiWidget* label )
{
	imuiWidgetEnd( label );
}

void imuiToolboxLabel( ImuiWindow* window, const char* text )
{
	ImuiWidget* label = imuiToolboxLabelBegin( window, text );
	imuiToolboxLabelEnd( label );
}

void imuiToolboxLabelLength( ImuiWindow* window, const char* text, size_t length )
{
	ImuiWidget* label = imuiToolboxLabelBeginLength( window, text, length );
	imuiToolboxLabelEnd( label );
}

void imuiToolboxLabelColor( ImuiWindow* window, const char* text, ImuiColor color )
{
	ImuiWidget* label = imuiToolboxLabelBeginColor( window, text, color );
	imuiToolboxLabelEnd( label );
}

void imuiToolboxLabelFormat( ImuiWindow* window, const char* format, ... )
{
	va_list args;
	va_start( args, format );
	imuiToolboxLabelFormatArgs( window, format, args );
	va_end( args );
}

void imuiToolboxLabelFormatArgs( ImuiWindow* window, const char* format, va_list args )
{
	ImuiWidget* label = imuiToolboxLabelBeginFormatArgs( window, format, args );
	imuiToolboxLabelEnd( label );
}

ImuiWidget* imuiToolboxImageBegin( ImuiWindow* window, ImuiSize imgSize )
{
	ImuiWidget* imgWidget = imuiWidgetBegin( window );
	imuiWidgetSetFixedSize( imgWidget, imgSize );

	return imgWidget;
}

void imuiToolboxImageEnd( ImuiWidget* imgWidget, const ImuiImage* img )
{
	imuiWidgetDrawImage( imgWidget, img );

	imuiWidgetEnd( imgWidget );
}

void imuiToolboxImage( ImuiWindow* window, const ImuiImage* img )
{
	imuiToolboxImageSize( window, img, imuiSizeCreateImage( img ) );
}

void imuiToolboxImageSize( ImuiWindow* window, const ImuiImage* img, ImuiSize imgSize )
{
	ImuiWidget* imgWidget = imuiToolboxImageBegin( window, imgSize );
	imuiToolboxImageEnd( imgWidget, img );
}

ImuiWidget* imuiToolboxSliderBegin( ImuiWindow* window )
{
	ImuiWidget* slider = imuiWidgetBegin( window );
	imuiWidgetSetHStretch( slider, 1.0f );
	imuiWidgetSetPadding( slider, s_theme.slider.padding );
	imuiWidgetSetFixedHeight( slider, s_theme.slider.height );

	return slider;
}

bool imuiToolboxSliderEnd( ImuiWidget* slider, float* value, float min, float max )
{
	ImuiWidgetInputState frameInputState;
	imuiWidgetGetInputState( slider, &frameInputState );

	imuiWidgetDrawSkin( slider, &s_theme.skins[ ImuiToolboxSkin_SliderBackground ], s_theme.colors[ ImuiToolboxColor_SliderBackground ] );

	ImuiWidget* sliderPivot = imuiWidgetBegin( imuiWidgetGetWindow( slider ) );
	imuiWidgetSetFixedSize( sliderPivot, s_theme.slider.pivotSize );

	const float range = max - min;
	const float normalizedValue = range == 0.0f ? 0.0f : (*value - min) / range;
	imuiWidgetSetHAlign( sliderPivot, normalizedValue );

	ImuiWidgetInputState inputState;
	imuiWidgetGetInputState( sliderPivot, &inputState );

	ImuiColor color = s_theme.colors[ ImuiToolboxColor_SliderPivot ];
	if( frameInputState.wasPressed )
	{
		color = s_theme.colors[ ImuiToolboxColor_SliderPivotClicked ];
	}
	else if( inputState.isMouseOver )
	{
		color = s_theme.colors[ ImuiToolboxColor_SliderPivotHover ];
	}

	bool changed = false;

	if( frameInputState.wasPressed )
	{
		const ImuiRect sliderInnerRect = imuiWidgetGetInnerRect( slider );

		const float scaledPaddingLeft	= s_theme.slider.padding.left * slider->window->surface->dpiScale;
		const float mouseValueNorm		= (frameInputState.relativeMousePos.x - scaledPaddingLeft) / sliderInnerRect.size.width;
		const float mouseValueNormClamp	= mouseValueNorm > 1.0f ? 1.0f : (mouseValueNorm < 0.0f ? 0.0f : mouseValueNorm);
		const float mouseValue			= (mouseValueNormClamp * (max - min)) + min;

		*value = mouseValue;
		changed = true;
	}

	imuiWidgetDrawSkin( sliderPivot, &s_theme.skins[ ImuiToolboxSkin_SliderPivot ], color );

	imuiWidgetEnd( sliderPivot );

	imuiWidgetEnd( slider );

	return changed;
}

bool imuiToolboxSlider( ImuiWindow* window, float* value )
{
	return imuiToolboxSliderMinMax( window, value, 0.0f, 1.0f );
}

bool imuiToolboxSliderMinMax( ImuiWindow* window, float* value, float min, float max )
{
	ImuiWidget* sliderFrame = imuiToolboxSliderBegin( window );
	return imuiToolboxSliderEnd( sliderFrame, value, min, max );
}

float imuiToolboxSliderState( ImuiWindow* window )
{
	return imuiToolboxSliderStateMinMaxDefault( window, 0.0f, 1.0f, 0.0f );
}

float imuiToolboxSliderStateDefault( ImuiWindow* window, float defaultValue )
{
	return imuiToolboxSliderStateMinMaxDefault( window, 0.0f, 1.0f, defaultValue );
}

float imuiToolboxSliderStateMinMax( ImuiWindow* window, float min, float max )
{
	return imuiToolboxSliderStateMinMaxDefault( window, min, max, min );
}

float imuiToolboxSliderStateMinMaxDefault( ImuiWindow* window, float min, float max, float defaultValue )
{
	IMUI_ASSERT( min <= max );
	IMUI_ASSERT( defaultValue >= min );
	IMUI_ASSERT( defaultValue <= max );

	ImuiWidget* sliderFrame = imuiToolboxSliderBegin( window );

	bool isNew;
	float* value = (float*)imuiWidgetAllocStateNew( sliderFrame, sizeof( float ), IMUI_ID_STR( "slider" ), &isNew );
	if( isNew )
	{
		*value = defaultValue;
	}

	imuiToolboxSliderEnd( sliderFrame, value, min, max );
	return *value;
}

ImuiToolboxTextBuffer* imuiToolboxTextBufferCreate( ImuiContext* imui )
{
	ImuiToolboxTextBuffer* textBuffer = IMUI_MEMORY_NEW_ZERO( &imui->allocator, ImuiToolboxTextBuffer );

	textBuffer->allocator = &imui->allocator;

	return textBuffer;
}

ImuiToolboxTextBuffer* imuiToolboxTextBufferCreateText( ImuiContext* imui, const char* text )
{
	ImuiToolboxTextBuffer* textBuffer = imuiToolboxTextBufferCreate( imui );

	imuiToolboxTextBufferAppend( textBuffer, text );

	return textBuffer;
}

void imuiToolboxTextBufferFree( ImuiToolboxTextBuffer* textBuffer )
{
	if( !textBuffer )
	{
		return;
	}

	imuiMemoryFree( textBuffer->allocator, textBuffer->data );
	imuiMemoryFree( textBuffer->allocator, textBuffer->lines );
	imuiMemoryFree( textBuffer->allocator, textBuffer );
}

void imuiToolboxTextBufferSet( ImuiToolboxTextBuffer* textBuffer, const char* text )
{
	textBuffer->dataLength	= 0;
	textBuffer->linesLength	= 0;

	imuiToolboxTextBufferAppend( textBuffer, text );
}

void imuiToolboxTextBufferAppend( ImuiToolboxTextBuffer* textBuffer, const char* text )
{
	const uintsize textLength = text ? strlen( text ) : 0;
	imuiToolboxTextBufferAppendLength( textBuffer, text, textLength );
}

void imuiToolboxTextBufferAppendLength( ImuiToolboxTextBuffer* textBuffer, const char* text, size_t textLength )
{
	if( !text || textLength == 0u )
	{
		return;
	}

	if( !IMUI_MEMORY_ARRAY_CHECK_CAPACITY( textBuffer->allocator, textBuffer->data, textBuffer->dataCapacity, textBuffer->dataLength + textLength ) )
	{
		return;
	}

	const char* firstLine = text;
	if( textBuffer->linesLength > 0 )
	{
		const uintsize lastLineOffset = textBuffer->lines[ textBuffer->linesLength - 1u ];
		const char* lastLine = textBuffer->data + lastLineOffset;
		if( !memchr( lastLine, '\n', textBuffer->dataLength - lastLineOffset ) )
		{
			firstLine = memchr( text, '\n', textLength );
			if( firstLine )
			{
				firstLine++;
			}
		}
	}

	bool first = true;
	uintsize linesLength = textBuffer->linesLength;
	for( const char* line = firstLine; line; line = memchr( line, '\n', textLength - (line - text) ) )
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
		IMUI_ASSERT( linesLength == 0u || textBuffer->lines[ linesLength ] > textBuffer->lines[ linesLength - 1u ] );
		linesLength++;
		first = false;
	}

	strncpy( textBuffer->data + textBuffer->dataLength, text, textLength );

	textBuffer->dataLength += textLength;
	textBuffer->linesLength = linesLength;
}

const char* imuiToolboxTextBufferGetData( const ImuiToolboxTextBuffer* textBuffer )
{
	if( !textBuffer )
	{
		return NULL;
	}

	return textBuffer->data;
}

size_t imuiToolboxTextBufferGetLength( const ImuiToolboxTextBuffer* textBuffer )
{
	if( !textBuffer )
	{
		return 0;
	}

	return textBuffer->dataLength;
}

ImuiWidget* imuiToolboxTextEditBegin( ImuiWindow* window )
{
	ImuiWidget* textEditFrame = imuiWidgetBegin( window );
	imuiWidgetSetHStretch( textEditFrame, 1.0f );
	imuiWidgetSetPadding( textEditFrame, s_theme.textEdit.padding );
	imuiWidgetSetFixedHeight( textEditFrame, s_theme.textEdit.height );

	imuiWidgetDrawSkin( textEditFrame, &s_theme.skins[ ImuiToolboxSkin_TextEditBackground ], s_theme.colors[ ImuiToolboxColor_TextEditBackground ] );

	return textEditFrame;
}

bool imuiToolboxTextEditEnd( ImuiWidget* textEdit, char* buffer, size_t bufferSize, size_t* textLength )
{
	IMUI_ASSERT( buffer );
	IMUI_ASSERT( bufferSize > 0u );

	ImuiContext* imui = imuiWidgetGetContext( textEdit );
	const ImuiInputState* input = imuiWidgetGetInput( textEdit );

	uintsize textLengthInternal;
	if( textLength )
	{
		textLengthInternal = *textLength;
	}
	else
	{
		textLengthInternal = strlen( buffer );
	}

	ImuiWidgetInputState inputState;
	imuiWidgetGetInputState( textEdit, &inputState );

	if( inputState.isMouseOver )
	{
		imuiInputSetMouseCursor( imui, ImuiInputMouseCursor_IBeam );
	}

	ImuiWidget* text = imuiWidgetBegin( imuiWidgetGetWindow( textEdit ) );
	imuiWidgetSetVAlign( text, 0.5f );

	bool isNew;
	ImuiToolboxTextEditState* state = (ImuiToolboxTextEditState*)imuiWidgetAllocStateNew( text, sizeof( *state ), IMUI_ID_STR( "text edit" ), &isNew );

	const ImuiRect textEditRect = imuiWidgetGetRect( textEdit );
	const ImuiRect textEditInnerRect = imuiWidgetGetInnerRect( textEdit );

	ImuiTextLayout* layout = imuiTextLayoutCreateWidget( text, s_theme.font, buffer );
	const ImuiSize textSize = imuiTextLayoutGetSize( layout );
	imuiWidgetSetFixedSize( text, textSize );

	if( imuiInputHasMouseButtonPressed( input, ImuiInputMouseButton_Left ) )
	{
		state->hasFocus = inputState.hasMousePressed;
	}

	bool changed = false;
	const float scale = imuiWidgetGetDpiScale( text );
	if( state->hasFocus )
	{
		const ImuiInputShortcut shortcut = imuiInputGetShortcut( input );
		const uint32 mods = imuiInputGetKeyModifiers( input );

		if( shortcut == ImuiInputShortcut_SelectAll )
		{
			state->selectionStart	= 0u;
			state->selectionEnd		= (uint32)textLengthInternal;
		}

		const char* textInput = imuiInputGetText( input );
		if( shortcut == ImuiInputShortcut_Paste )
		{
			textInput = imuiInputGetPasteText( imui );
		}

		if( textInput )
		{
			const uintsize remainingSize	= bufferSize - textLengthInternal - 1u;
			const uintsize inputLength		= strlen( textInput );
			const uintsize newSize			= IMUI_MIN( inputLength, remainingSize );

			if( state->selectionStart != state->selectionEnd )
			{
				const uintsize selectionStartChar	= imuiTextLayoutGetGlyphCharIndex( layout, state->selectionStart );
				const uintsize selectionEndChar		= imuiTextLayoutGetGlyphCharIndex( layout, state->selectionEnd );

				memmove( buffer + selectionStartChar, buffer + selectionEndChar, textLengthInternal - selectionEndChar );
				state->cursorPos = state->selectionStart;

				textLengthInternal -= selectionEndChar - selectionStartChar;

				state->selectionStart	= 0u;
				state->selectionEnd		= 0u;
			}

			const uintsize cursorChar = imuiTextLayoutGetGlyphCharIndex( layout, state->cursorPos );
			if( cursorChar != textLengthInternal )
			{
				memmove( buffer + cursorChar + newSize, buffer + cursorChar, textLengthInternal - cursorChar );
			}

			memcpy( buffer + cursorChar, textInput, newSize );
			textLengthInternal += newSize;
			buffer[ textLengthInternal ] = '\0';

			state->cursorPos += (uint32)imuiTextLayoutCalculateGlyphCount( textInput, newSize );

			changed = true;
		}

		if( imuiInputHasKeyPressed( input, ImuiInputKey_Backspace ) )
		{
			if( state->selectionStart != state->selectionEnd )
			{
				const uintsize selectionStartChar	= imuiTextLayoutGetGlyphCharIndex( layout, state->selectionStart );
				const uintsize selectionEndChar		= imuiTextLayoutGetGlyphCharIndex( layout, state->selectionEnd );

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
				const uintsize backspaceStartChar	= imuiTextLayoutGetGlyphCharIndex( layout, state->cursorPos - 1u );
				const uintsize backspaceEndChar		= imuiTextLayoutGetGlyphCharIndex( layout, state->cursorPos );

				memmove( buffer + backspaceStartChar, buffer + backspaceEndChar, textLengthInternal - backspaceEndChar );

				textLengthInternal -= backspaceEndChar - backspaceStartChar;
				buffer[ textLengthInternal ] = '\0';

				state->cursorPos--;

				changed = true;
			}
		}
		else if( imuiInputHasKeyPressed( input, ImuiInputKey_Delete ) )
		{
			if( state->selectionStart != state->selectionEnd )
			{
				const uintsize selectionStartChar	= imuiTextLayoutGetGlyphCharIndex( layout, state->selectionStart );
				const uintsize selectionEndChar		= imuiTextLayoutGetGlyphCharIndex( layout, state->selectionEnd );

				memmove( buffer + selectionStartChar, buffer + selectionEndChar, textLengthInternal - selectionEndChar );
				state->cursorPos = state->selectionStart;

				textLengthInternal -= selectionEndChar - selectionStartChar;
				buffer[ textLengthInternal ] = '\0';

				state->selectionStart	= 0u;
				state->selectionEnd		= 0u;

				changed = true;
			}
			else if( imuiTextLayoutGetGlyphCount( layout ) > state->cursorPos )
			{
				const uintsize deleteStartChar	= imuiTextLayoutGetGlyphCharIndex( layout, state->cursorPos );
				const uintsize deleteEndChar	= imuiTextLayoutGetGlyphCharIndex( layout, state->cursorPos + 1u );

				memmove( buffer + deleteStartChar, buffer + deleteEndChar, textLengthInternal - deleteEndChar );

				textLengthInternal -= deleteEndChar - deleteStartChar;
				buffer[ textLengthInternal ] = '\0';

				changed = true;
			}
		}

		sint32 nextCursorPos = (sint32)state->cursorPos;
		const bool leftPressed = imuiInputHasKeyPressed( input, ImuiInputKey_Left );
		const bool shiftPressed = (mods & (ImuiInputModifier_LeftShift | ImuiInputModifier_RightShift)) != 0;
		if( leftPressed ||
			imuiInputHasKeyPressed( input, ImuiInputKey_Right ) )
		{
			const sint32 direction = leftPressed ? -1 : 1;
			if( mods & (ImuiInputModifier_LeftCtrl | ImuiInputModifier_RightCtrl) )
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
		else if( shortcut == ImuiInputShortcut_Home )
		{
			nextCursorPos = 0u;
		}
		else if( shortcut == ImuiInputShortcut_End )
		{
			nextCursorPos = (sint32)textLengthInternal;
		}

		if( inputState.wasPressed )
		{
			ImuiWidgetInputState textInputState;
			imuiWidgetGetInputState( text, &textInputState );
			nextCursorPos = (sint32)imuiTextLayoutFindGlyphIndex( layout, textInputState.relativeMousePos, scale );

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
			if( shortcut == ImuiInputShortcut_Copy )
			{
				imuiInputSetCopyText( imui, buffer + state->selectionStart, state->selectionEnd - state->selectionStart );
			}

			const ImuiPos startPos			= imuiTextLayoutGetGlyphPos( layout, state->selectionStart, scale );
			const ImuiPos endPos			= imuiTextLayoutGetGlyphPos( layout, state->selectionEnd, scale );

			const ImuiRect selection = imuiRectCreate(
				(textEditInnerRect.pos.x - textEditRect.pos.x) + startPos.x,
				textEditInnerRect.pos.y - textEditRect.pos.y,
				endPos.x - startPos.x,
				textEditInnerRect.size.height
			);
			imuiWidgetDrawPartialColor( textEdit, selection, s_theme.colors[ ImuiToolboxColor_TextEditSelection ] );
		}
	}

	if( layout )
	{
		imuiWidgetDrawText( text, layout, s_theme.colors[ ImuiToolboxColor_TextEditText ] );
	}

	if( state->hasFocus )
	{
		const double blinkValue	= fmod( imuiWidgetGetTime( textEdit ), s_theme.textEdit.blinkTime * 2.0 );
		const bool blink		= blinkValue > s_theme.textEdit.blinkTime;
		if( blink )
		{
			//const imuiPos cursorPos			= imuiPosAdd( imuiTextLayoutGetGlyphPos( layout, state->cursorPos ), s_config.textEdit.padding.left, s_config.textEdit.padding.top );
			//const imuiRect cursorRect		= imuiRectCreatePosSize( cursorPos, s_config.textEdit.cursorSize );

			const ImuiPos cursorPos			= imuiTextLayoutGetGlyphPos( layout, state->cursorPos, scale );
			const ImuiPos cursorPosTop		= imuiPosCreate( (textEditInnerRect.pos.x - textEditRect.pos.x) + cursorPos.x, s_theme.textEdit.padding.top );
			const ImuiPos cursorPosBottom	= imuiPosCreate( cursorPosTop.x, cursorPosTop.y + textEditInnerRect.size.height );
			imuiWidgetDrawLine( textEdit, cursorPosTop, cursorPosBottom, s_theme.colors[ ImuiToolboxColor_TextEditCursor ] );
		}
	}

	imuiWidgetEnd( text );

	imuiWidgetEnd( textEdit );

	if( textLength )
	{
		*textLength = textLengthInternal;
	}

	return changed;
}

bool imuiToolboxTextEdit( ImuiWindow* window, char* buffer, size_t bufferSize, size_t* textLength )
{
	ImuiWidget* textEdit = imuiToolboxTextEditBegin( window );
	return imuiToolboxTextEditEnd( textEdit, buffer, bufferSize, textLength );
}

const char* imuiToolboxTextEditStateBuffer( ImuiWindow* window, size_t bufferSize )
{
	return imuiToolboxTextEditStateBufferDefault( window, bufferSize, NULL );
}

const char* imuiToolboxTextEditStateBufferDefault( ImuiWindow* window, size_t bufferSize, const char* defaultValue )
{
	ImuiWidget* textEdit = imuiToolboxTextEditBegin( window );

	bool isNew;
	char* buffer = (char*)imuiWidgetAllocStateNew( textEdit, bufferSize, IMUI_ID_STR( "text buffer" ), &isNew );
	if( isNew && defaultValue )
	{
		strncpy( buffer, defaultValue, bufferSize );
	}

	imuiToolboxTextEditEnd( textEdit, buffer, bufferSize, NULL );
	return buffer;
}

ImuiWidget* imuiToolboxTextViewBegin( ImuiToolboxTextViewContext* textView, ImuiWindow* window, const char* text )
{
	ImuiToolboxTextBuffer* textBuffer = imuiToolboxTextBufferCreateText( window->context, text );

	ImuiWidget* textViewWidget = imuiToolboxTextViewBeginBuffer( textView, window, textBuffer );
	textView->ownsBuffer = true;

	return textViewWidget;
}

ImuiWidget* imuiToolboxTextViewBeginBuffer( ImuiToolboxTextViewContext* textView, ImuiWindow* window, const ImuiToolboxTextBuffer* textBuffer )
{
	ImuiWidget* list = imuiToolboxListBegin( &textView->list, window, s_theme.font ? s_theme.font->fontSize : 1.0f, textBuffer->linesLength, false );

	imuiWidgetSetStretchOne( list );

	//bool isNewState;
	textView->ownsBuffer	= false;
	textView->textBuffer	= textBuffer;

	for( uintsize i = imuiToolboxListGetBeginIndex( &textView->list ); i < imuiToolboxListGetEndIndex( &textView->list ); ++i )
	{
		imuiToolboxListNextItem( &textView->list );

		const bool lastLine = i == textBuffer->linesLength - 1u;
		const uintsize lineOffset = textBuffer->lines[ i ];
		const uintsize nextLineOffset = lastLine ? textBuffer->dataLength : textBuffer->lines[ i + 1 ];
		const uintsize lineLength = (nextLineOffset - lineOffset) - (lastLine ? 0 : 1);
		IMUI_ASSERT( nextLineOffset <= textBuffer->dataLength );
		IMUI_ASSERT( lineLength <= textBuffer->dataLength );

		const char* line = textBuffer->data + lineOffset;
		imuiToolboxLabelLength( window, line, lineLength );
	}

	return textView->list.list;
}

void imuiToolboxTextViewEnd( ImuiToolboxTextViewContext* textView )
{

	imuiToolboxListEnd( &textView->list );

	if( textView->ownsBuffer )
	{
		imuiToolboxTextBufferFree( (ImuiToolboxTextBuffer*)textView->textBuffer );
		textView->textBuffer = NULL;
	}
}

void imuiToolboxTextView( ImuiWindow* window, const char* text )
{
	ImuiToolboxTextViewContext textView;
	imuiToolboxTextViewBegin( &textView, window, text );
	imuiToolboxTextViewEnd( &textView );
}

void imuiToolboxTextViewBuffer( ImuiWindow* window, const ImuiToolboxTextBuffer* textBuffer )
{
	ImuiToolboxTextViewContext textView;
	imuiToolboxTextViewBeginBuffer( &textView, window, textBuffer );
	imuiToolboxTextViewEnd( &textView );
}

void imuiToolboxProgressBar( ImuiWindow* window, float value )
{
	imuiToolboxProgressBarMinMax( window, value, 0.0f, 1.0f );
}

void imuiToolboxProgressBarMinMax( ImuiWindow* window, float value, float min, float max )
{
	ImuiWidget* progressBar = imuiWidgetBegin( window );
	imuiWidgetSetHStretch( progressBar, 1.0f );
	imuiWidgetSetPadding( progressBar, s_theme.progressBar.padding );
	imuiWidgetSetFixedHeight( progressBar, s_theme.progressBar.height );

	imuiWidgetDrawSkin( progressBar, &s_theme.skins[ ImuiToolboxSkin_ProgressBarBackground ], s_theme.colors[ ImuiToolboxColor_ProgressBarBackground ] );

	const ImuiRect barRect = imuiWidgetGetInnerRect( progressBar );

	ImuiRect progressRect;
	if( value < min )
	{
		const double time		= imuiWindowGetTime( window );
		const float cosv		= ((float)cos( time * 8.0 ) * 0.15f) + 0.15f;
		const float sinv		= ((float)sin( time * 4.0 ) * 0.5f) + 0.5f;
		const float width		= ceilf( barRect.size.width * (0.1f + cosv) );
		const float margin		= floorf( sinv * (barRect.size.width - width) );

		progressRect = imuiRectCreate(
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

		progressRect = imuiRectCreate(
			s_theme.progressBar.padding.left,
			s_theme.progressBar.padding.top,
			width,
			barRect.size.height
		);
	}

	imuiWidgetDrawPartialSkin( progressBar, progressRect, &s_theme.skins[ ImuiToolboxSkin_ProgressBarProgress ], s_theme.colors[ ImuiToolboxColor_ProgressBarProgress ] );

	imuiWidgetEnd( progressBar );
}

ImuiWidget* imuiToolboxScrollAreaBegin( ImuiToolboxScrollAreaContext* scrollArea, ImuiWindow* window )
{
	scrollArea->horizontalSpacing	= false;
	scrollArea->verticalSpacing		= false;
	scrollArea->area				= imuiWidgetBegin( window );
	scrollArea->state				= (ImuiToolboxScrollAreaState*)imuiWidgetAllocState( scrollArea->area, sizeof( *scrollArea->state ), IMUI_ID_STR( "scroll area" ) );
	scrollArea->content				= imuiWidgetBegin( window );

	imuiWidgetSetStretch( scrollArea->content, 1.0f, 1.0f );
	imuiWidgetSetLayoutScroll( scrollArea->content, scrollArea->state->offset.x, scrollArea->state->offset.y );

	return scrollArea->area;
}

void imuiToolboxScrollAreaEnableSpacing( ImuiToolboxScrollAreaContext* scrollArea, bool horizontal, bool vertical )
{
	scrollArea->horizontalSpacing	= horizontal;
	scrollArea->verticalSpacing		= vertical;
}

void imuiToolboxScrollAreaSetOffset( ImuiToolboxScrollAreaContext* scrollArea, float offsetX, float offsetY )
{
	scrollArea->state->offset.x = offsetX;
	scrollArea->state->offset.y = offsetY;
}

void imuiToolboxScrollAreaMoveOffset( ImuiToolboxScrollAreaContext* scrollArea, float offsetX, float offsetY )
{
	scrollArea->state->offset.x += offsetX;
	scrollArea->state->offset.y += offsetY;
}

void imuiToolboxScrollAreaOffsetTo( ImuiToolboxScrollAreaContext* scrollArea, const ImuiWidget* widgetToScrollTo )
{
	const ImuiSize scrollSize = imuiWidgetGetSize( scrollArea->area );

	ImuiRect widgetRect = imuiWidgetGetRect( widgetToScrollTo );
	widgetRect.pos = imuiPosAddPos( imuiPosSubPos( widgetRect.pos, imuiWidgetGetPos( scrollArea->area ) ), scrollArea->state->offset );

	if( widgetRect.pos.y - scrollArea->state->offset.y < 0.0f )
	{
		scrollArea->state->offset.y = widgetRect.pos.y;
	}
	else if( imuiRectGetBottom( widgetRect ) > scrollArea->state->offset.y + scrollSize.height )
	{
		scrollArea->state->offset.y = imuiRectGetBottom( widgetRect ) - scrollSize.height;
	}

	if( widgetRect.pos.x - scrollArea->state->offset.x < 0.0f )
	{
		scrollArea->state->offset.x = widgetRect.pos.x;
	}
	else if( imuiRectGetRight( widgetRect ) > scrollArea->state->offset.x + scrollSize.width )
	{
		scrollArea->state->offset.x = imuiRectGetRight( widgetRect ) - scrollSize.width;
	}
}

void imuiToolboxScrollAreaEnd( ImuiToolboxScrollAreaContext* scrollArea )
{
	ImuiWindow* window = imuiWidgetGetWindow( scrollArea->area );
	ImuiToolboxScrollAreaState* state = scrollArea->state;

	const ImuiRect frameRect = imuiWidgetGetRect( scrollArea->area );

	ImuiSize areaSize = imuiSizeCreateZero();
	for( const ImuiWidget* child = imuiWidgetGetFirstChild( scrollArea->content ); child; child = imuiWidgetGetNextSibling( child ) )
	{
		const ImuiRect childRect = imuiWidgetGetRect( child );

		areaSize.width	= IMUI_MAX( areaSize.width, childRect.size.width );
		areaSize.height	= IMUI_MAX( areaSize.height, childRect.size.height );
	}

	const float dpiBarSize = s_theme.scrollArea.barSize * imuiWidgetGetDpiScale( scrollArea->content );
	const float dpiBarMinSize = s_theme.scrollArea.barMinSize * imuiWidgetGetDpiScale( scrollArea->content );

	ImuiWidgetInputState areaInputState;
	imuiWidgetGetInputState( scrollArea->area, &areaInputState );

	const bool hasHorizontalBar	= areaSize.width > frameRect.size.width;
	const bool hasVerticalBar	= areaSize.height > frameRect.size.height;

	ImuiBorder margin = imuiBorderCreateZero();
	margin.right = (hasVerticalBar ? s_theme.scrollArea.barSize + (scrollArea->horizontalSpacing ? s_theme.scrollArea.barSpacing : 0.0f) : 0.0f) ;
	margin.bottom = (hasHorizontalBar ? s_theme.scrollArea.barSize + (scrollArea->verticalSpacing ? s_theme.scrollArea.barSpacing : 0.0f) : 0.0f) ;

	const ImuiSize frameAreaSize = imuiSizeMax( imuiSizeCreateZero(), imuiSizeShrinkBorder( frameRect.size, margin ) );

	imuiWidgetSetMargin( scrollArea->content, margin );
	imuiWidgetEnd( scrollArea->content );

	if( hasHorizontalBar )
	{
		float barWidth = frameRect.size.width - (hasVerticalBar ? dpiBarSize : 0.0f);
		barWidth = IMUI_MAX( 0.0f, barWidth );

		ImuiWidget* scrollBar = imuiWidgetBegin( window );
		imuiWidgetSetVAlign( scrollBar, 1.0f );
		imuiWidgetSetFixedHeight( scrollBar, s_theme.scrollArea.barSize );
		imuiWidgetSetHStretch( scrollBar, frameRect.size.width > 0.0f ? barWidth / frameRect.size.width : 1.0f );

		const ImuiRect barRect		= imuiWidgetGetRect( scrollBar );
		const float pivotSizeFactor	= frameAreaSize.width / areaSize.width;
		const float pivotSize		= barWidth * pivotSizeFactor;
		const float pivotSizeFinal	= IMUI_MAX( pivotSize, dpiBarMinSize );
		const float pivotOffset		= (state->offset.x / areaSize.width) * (barWidth - (pivotSizeFinal - pivotSize));

		const ImuiRect barPivotRect = imuiRectCreate(
			pivotOffset,
			0.0f,
			pivotSizeFinal,
			dpiBarSize
		);

		ImuiWidgetInputState inputState;
		imuiWidgetGetInputState( scrollBar, &inputState );

		if( inputState.wasPressed )
		{
			if( !state->wasPressedX &&
				imuiRectIncludesPos( barPivotRect, inputState.relativeMousePos ) )
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

		imuiWidgetDrawSkin( scrollBar, &s_theme.skins[ ImuiToolboxSkin_ScrollAreaBarBackground ], s_theme.colors[ ImuiToolboxColor_ScrollAreaBarBackground ] );
		imuiWidgetDrawPartialSkin( scrollBar, barPivotRect, &s_theme.skins[ ImuiToolboxSkin_ScrollAreaBarPivot ], s_theme.colors[ ImuiToolboxColor_ScrollAreaBarPivot ] );

		imuiWidgetEnd( scrollBar );
	}

	if( hasVerticalBar )
	{
		float barHeight = frameRect.size.height - (hasHorizontalBar ? dpiBarSize : 0.0f);
		barHeight = IMUI_MAX( 0.0f, barHeight );

		ImuiWidget* scrollBar = imuiWidgetBegin( window );
		imuiWidgetSetHAlign( scrollBar, 1.0f );
		imuiWidgetSetFixedWidth( scrollBar, s_theme.scrollArea.barSize );
		imuiWidgetSetVStretch( scrollBar, frameRect.size.height > 0.0f ? barHeight / frameRect.size.height : 1.0f );

		const ImuiRect barRect		= imuiWidgetGetRect( scrollBar );
		const float pivotSizeFactor	= frameAreaSize.height / areaSize.height;
		const float pivotSize		= barHeight * pivotSizeFactor;
		const float pivotSizeFinal	= IMUI_MAX( pivotSize, dpiBarMinSize );
		const float pivotOffset		= (state->offset.y / areaSize.height) * (barHeight - (pivotSizeFinal - pivotSize));

		const ImuiRect barPivotRect = imuiRectCreate(
			0.0f,
			pivotOffset,
			dpiBarSize,
			pivotSizeFinal
		);

		ImuiWidgetInputState inputState;
		imuiWidgetGetInputState( scrollBar, &inputState );

		if( inputState.wasPressed )
		{
			if( !state->wasPressedY &&
				imuiRectIncludesPos( barPivotRect, inputState.relativeMousePos ) )
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

		imuiWidgetDrawSkin( scrollBar, &s_theme.skins[ ImuiToolboxSkin_ScrollAreaBarBackground ], s_theme.colors[ ImuiToolboxColor_ScrollAreaBarBackground ] );
		imuiWidgetDrawPartialSkin( scrollBar, barPivotRect, &s_theme.skins[ ImuiToolboxSkin_ScrollAreaBarPivot ], s_theme.colors[ ImuiToolboxColor_ScrollAreaBarPivot ] );

		imuiWidgetEnd( scrollBar );
	}

	if( areaInputState.isMouseOver )
	{
		state->offset = imuiPosSubPos( state->offset, imuiPosScale( imuiInputGetMouseScrollDelta( imuiWindowGetInput( window ) ), 80.0f ) );
	}

	state->offset = imuiPosMax( imuiPosCreateZero(), imuiPosMin( state->offset, imuiSizeToPos( imuiSizeSubSize( areaSize, frameAreaSize ) ) ) );

	imuiWidgetEnd( scrollArea->area );
}

ImuiWidget* imuiToolboxListBegin( ImuiToolboxListContext* list, ImuiWindow* window, float itemSize, size_t itemCount, bool selection )
{
	IMUI_ASSERT( list );

	const float totalItemSize = itemSize + s_theme.list.itemSpacing;

	imuiToolboxScrollAreaBegin( &list->scrollArea, window );
	list->list = list->scrollArea.area;

	imuiToolboxScrollAreaEnableSpacing( &list->scrollArea, true, false );

	list->listLayout = imuiWidgetBegin( window );
	imuiWidgetSetHStretch( list->listLayout, 1.0f );
	imuiWidgetSetLayoutVerticalSpacing( list->listLayout, s_theme.list.itemSpacing );
	if( itemCount > 0u )
	{
		imuiWidgetSetFixedHeight( list->listLayout, (totalItemSize * itemCount) - s_theme.list.itemSpacing );
	}

	bool isNew;
	list->state = (ImuiToolboxListState*)imuiWidgetAllocStateNew( list->listLayout, sizeof( ImuiToolboxListState ), IMUI_ID_STR( "list" ), &isNew );
	if( isNew || !selection )
	{
		list->state->selectedIndex = (uintsize)-1;
	}

	const ImuiRect listRect		= imuiWidgetGetRect( list->list );
	const ImuiRect layoutRect	= imuiWidgetGetRect( list->listLayout );
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

	ImuiWidgetInputState inputState;
	imuiWidgetGetInputState( list->list, &inputState );

	const ImuiInputState* input = imuiWidgetGetInput( list->list );
	if( imuiInputHasMouseButtonPressed( input, ImuiInputMouseButton_Left ) )
	{
		list->state->hasFocus = inputState.hasMousePressed;
	}

	if( list->state->hasFocus )
	{
		if( imuiInputHasKeyPressed( input, ImuiInputKey_Up ) )
		{
			list->state->selectedIndex--;
			if( list->state->selectedIndex >= itemCount )
			{
				list->state->selectedIndex = 0;
			}
			list->changed = true;
		}
		else if( imuiInputHasKeyPressed( input, ImuiInputKey_Down ) )
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

size_t imuiToolboxListGetBeginIndex( const ImuiToolboxListContext* list )
{
	return list->beginIndex;
}

size_t imuiToolboxListGetEndIndex( const ImuiToolboxListContext* list )
{
	return list->endIndex;
}

size_t imuiToolboxListGetSelectedIndex( const ImuiToolboxListContext* list )
{
	return list->state->selectedIndex;
}

void imuiToolboxListSetSelectedIndex( ImuiToolboxListContext* list, size_t index )
{
	if( !list->selection )
	{
		return;
	}

	list->state->selectedIndex = index;
}

static void imuiToolboxListItemEndInternal( ImuiToolboxListContext* list )
{
	ImuiWidget* item = imuiWidgetGetLastChild( list->listLayout );
	if( item )
	{
		imuiWidgetEnd( item );
		list->item = NULL;
	}
}

ImuiWidget* imuiToolboxListNextItem( ImuiToolboxListContext* list )
{
	return imuiToolboxListNextItemId( list, IMUI_ID_DEFAULT );
}

ImuiWidget* imuiToolboxListNextItemId( ImuiToolboxListContext* list, ImuiId id )
{
	imuiToolboxListItemEndInternal( list );

	list->itemIndex++;

	ImuiWidget* item = imuiWidgetBeginId( imuiWidgetGetWindow( list->list ), id );
	imuiWidgetSetHStretch( item, 1.0f );
	imuiWidgetSetFixedHeight( item, list->itemSize );

	if( list->beginIndex > 0 &&
		list->itemIndex == list->beginIndex )
	{
		const float totalItemSize = list->itemSize + s_theme.list.itemSpacing;
		imuiWidgetSetMargin( item, imuiBorderCreate( totalItemSize * list->beginIndex, 0.0f, 0.0f, 0.0f ) );
	}

	if( list->selection )
	{
		imuiWidgetSetCanHaveFocus( item );

		ImuiWidgetInputState inputState;
		imuiWidgetGetInputState( item, &inputState );

		const ImuiSkin* skin = &s_theme.skins[ list->itemIndex == list->state->selectedIndex ? ImuiToolboxSkin_ItemSelected : ImuiToolboxSkin_ListItem ];
		if( inputState.isMouseDown )
		{
			imuiWidgetDrawSkin( item, skin, s_theme.colors[ ImuiToolboxColor_ListItemClicked ] );
		}
		else if( inputState.isMouseOver || inputState.hasFocus )
		{
			imuiWidgetDrawSkin( item, skin, s_theme.colors[ ImuiToolboxColor_ListItemHover ] );
		}
		else if( list->itemIndex == list->state->selectedIndex )
		{
			imuiWidgetDrawSkin( item, skin, s_theme.colors[ ImuiToolboxColor_ListItemSelected ] );
		}

		if( inputState.hasMouseReleased )
		{
			list->state->selectedIndex = list->itemIndex;
			list->changed = true;
		}
	}

	return item;
}

bool imuiToolboxListEnd( ImuiToolboxListContext* list )
{
	imuiToolboxListItemEndInternal( list );

	imuiWidgetEnd( list->listLayout );
	imuiToolboxScrollAreaEnd( &list->scrollArea );

	return list->changed;
}

ImuiWidget* imuiToolboxDropDownBegin( ImuiToolboxDropDownContext* dropDown, ImuiWindow* window, const char** items, size_t itemCount, size_t itemStride )
{
	if( itemStride == 0u )
	{
		itemStride = sizeof( const char* );
	}

	dropDown->dropDown = imuiWidgetBegin( window );
	imuiWidgetSetPadding( dropDown->dropDown, s_theme.dropDown.padding );
	imuiWidgetSetFixedHeight( dropDown->dropDown, s_theme.dropDown.height );

	bool isNew;
	dropDown->state = (ImuiToolboxDropDownState*)imuiWidgetAllocStateNew( dropDown->dropDown, sizeof( *dropDown->state ), IMUI_ID_STR( "drop down" ), &isNew );
	if( isNew )
	{
		dropDown->state->selectedIndex = (uintsize)-1;
	}

	ImuiWidgetInputState inputState;
	imuiWidgetGetInputState( dropDown->dropDown, &inputState );

	ImuiColor color = s_theme.colors[ ImuiToolboxColor_DropDown ];
	if( dropDown->state->isOpen )
	{
		color = s_theme.colors[ ImuiToolboxColor_DropDownOpen ];
	}
	else if( inputState.isMouseDown )
	{
		color = s_theme.colors[ ImuiToolboxColor_DropDownClicked ];
	}
	else if( inputState.isMouseOver )
	{
		color = s_theme.colors[ ImuiToolboxColor_DropDownHover ];
	}

	imuiWidgetDrawSkin( dropDown->dropDown, &s_theme.skins[ ImuiToolboxSkin_DropDown ], color );

	ImuiWidget* icon = imuiWidgetBegin( window );
	const ImuiImage iconImage = s_theme.icons[ dropDown->state->isOpen ? ImuiToolboxIcon_DropDownClose : ImuiToolboxIcon_DropDownOpen ];
	imuiWidgetSetFixedSize( icon, imuiSizeCreateImage( &iconImage ) );
	imuiWidgetSetHAlign( icon, 1.0f );
	imuiWidgetSetVAlign( icon, 0.5f );
	imuiWidgetDrawImageColor( icon, &iconImage, s_theme.colors[ ImuiToolboxColor_DropDownIcon ] );
	imuiWidgetEnd( icon );

	ImuiSize maxSize = imuiSizeCreateZero();
	ImuiTextLayout* selectedTextLayout = NULL;
	const uint8* itemsBytes = (const byte*)items;
	for( uintsize i = 0; i < itemCount; ++i )
	{
		const char* itemText = *(const char**)itemsBytes;
		ImuiTextLayout* textLayout = imuiTextLayoutCreateWidget( dropDown->dropDown, s_theme.font, itemText );

		maxSize = imuiSizeMax( maxSize, imuiTextLayoutGetSize( textLayout ) );

		if( i == dropDown->state->selectedIndex )
		{
			selectedTextLayout = textLayout;
		}

		itemsBytes += itemStride;
	}

	imuiWidgetSetMinWidth( dropDown->dropDown, maxSize.width + imuiBorderGetMinSize( s_theme.dropDown.padding ).width + s_theme.dropDown.padding.left + imuiWidgetGetSize( icon ).width );

	ImuiWidget* text = imuiWidgetBegin( window );
	imuiWidgetSetFixedSize( text, maxSize );
	imuiWidgetSetVAlign( text, 0.5f );

	if( selectedTextLayout )
	{
		imuiWidgetDrawText( text, selectedTextLayout, s_theme.colors[ ImuiToolboxColor_DropDownText ] );
	}

	imuiWidgetEnd( text );

	if( inputState.hasMousePressed )
	{
		dropDown->state->isOpen = !dropDown->state->isOpen;
	}

	if( dropDown->state->isOpen && itemCount > 0u )
	{
		ImuiSurface* surface = imuiWindowGetSurface( window );
		const ImuiSize surfaceSize = imuiSurfaceGetSize( surface );

		const ImuiRect dropDownRect		= imuiWidgetGetRect( dropDown->dropDown );
		const ImuiSize listMaginSize	= imuiBorderGetMinSize( s_theme.dropDown.listMargin );

		const float listHeight			= listMaginSize.height + (((s_theme.dropDown.itemSize + s_theme.dropDown.itemSpacing) * IMUI_MIN( itemCount, s_theme.dropDown.listMaxLength )) - s_theme.dropDown.itemSpacing);
		const float listWidth			= dropDownRect.size.width + listMaginSize.width;
		const float dropDownBottom		= dropDownRect.pos.y + dropDownRect.size.height;

		ImuiRect listRect;
		if( dropDownBottom + listHeight > surfaceSize.height )
		{
			if( dropDownRect.pos.y - listHeight < 0.0f )
			{
				if( dropDownBottom > surfaceSize.height / 2.0f )
				{
					listRect = imuiRectCreate( dropDownRect.pos.x, 0.0f, listWidth, dropDownRect.pos.y );
				}
				else
				{
					listRect = imuiRectCreate( dropDownRect.pos.x, dropDownBottom, listWidth, surfaceSize.height - dropDownBottom );
				}
			}
			else
			{
				listRect = imuiRectCreate( dropDownRect.pos.x, dropDownRect.pos.y - listHeight, listWidth, listHeight );
			}
		}
		else
		{
			listRect = imuiRectCreate( dropDownRect.pos.x, dropDownBottom, listWidth, listHeight  );
		}
		ImuiWindow* listWindow = imuiWindowBegin( imuiWindowGetSurface( window ), "dropDownList", listRect, s_theme.dropDown.listZOrder );

		const float oldItemSpacing = s_theme.list.itemSpacing;
		s_theme.list.itemSpacing = s_theme.dropDown.itemSpacing;

		ImuiToolboxListContext list;
		imuiToolboxListBegin( &list, listWindow, s_theme.dropDown.itemSize, itemCount, true );
		imuiWidgetSetStretch( list.list, 1.0f, 1.0f );
		imuiWidgetSetMargin( list.scrollArea.area, s_theme.dropDown.listMargin );

		ImuiWidgetInputState listInputState;
		imuiWidgetGetInputState( list.list, &listInputState );

		imuiWidgetDrawSkin( listWindow->rootWidget, &s_theme.skins[ ImuiToolboxSkin_DropDownList ], s_theme.colors[ ImuiToolboxColor_DropDownList ] );

		imuiToolboxListSetSelectedIndex( &list, dropDown->state->selectedIndex );

		itemsBytes = (const byte*)items;
		for( uintsize i = imuiToolboxListGetBeginIndex( &list ); i < imuiToolboxListGetEndIndex( &list ); ++i )
		{
			const char* itemText = *(const char**)(itemsBytes + (i * itemStride));

			ImuiWidget* item = imuiToolboxListNextItem( &list );
			imuiWidgetSetPadding( item, s_theme.dropDown.itemPadding );

			ImuiWidget* label = imuiToolboxLabelBegin( listWindow, itemText );
			imuiWidgetSetVAlign( label, 0.5f );
			imuiWidgetEnd( label );
		}

		dropDown->state->selectedIndex = imuiToolboxListGetSelectedIndex( &list );

		if( imuiToolboxListEnd( &list ) )
		{
			dropDown->state->isOpen = false;
			dropDown->changed = true;
		}
		else
		{
			dropDown->changed = false;
		}

		s_theme.list.itemSpacing = oldItemSpacing;

		imuiWindowEnd( listWindow );

		if( !inputState.wasPressed &&
			!listInputState.wasPressed &&
			imuiInputHasMouseButtonReleased( imuiWindowGetInput( window ), ImuiInputMouseButton_Left ) )
		{
			dropDown->state->isOpen = false;
		}
	}

	return dropDown->dropDown;
}

size_t imuiToolboxDropDownGetSelectedIndex( const ImuiToolboxDropDownContext* dropDown )
{
	return dropDown->state->selectedIndex;
}

void imuiToolboxDropDownSetSelectedIndex( const ImuiToolboxDropDownContext* dropDown, size_t index )
{
	dropDown->state->selectedIndex = index;
}

bool imuiToolboxDropDownEnd( ImuiToolboxDropDownContext* dropDown )
{
	imuiWidgetEnd( dropDown->dropDown );

	return dropDown->changed;
}

size_t imuiToolboxDropDown( ImuiWindow* window, const char** items, size_t itemCount, size_t itemStride )
{
	ImuiToolboxDropDownContext dropDown;
	imuiToolboxDropDownBegin( &dropDown, window, items, itemCount, itemStride );
	const size_t selectedIndex = imuiToolboxDropDownGetSelectedIndex( &dropDown );
	imuiToolboxDropDownEnd( &dropDown );
	return selectedIndex;
}

ImuiWindow* imuiToolboxPopupBegin( ImuiWindow* window )
{
	return imuiToolboxPopupBeginSurface( imuiWindowGetSurface( window ) );
}

ImuiWindow* imuiToolboxPopupBeginSurface( ImuiSurface* surface )
{
	const ImuiRect windowRect = imuiRectCreatePosSize( imuiPosCreateZero(), imuiSurfaceGetSize( surface ) );
	ImuiWindow* popupWindow = imuiWindowBegin( surface, "popup", windowRect, s_theme.popup.zOrder );

	ImuiWidget* background = imuiWidgetBegin( popupWindow );
	imuiWidgetSetStretch( background, 1.0f, 1.0f );

	imuiWidgetDrawColor( background, s_theme.colors[ ImuiToolboxColor_PopupBackground ] );

	ImuiWidget* popup = imuiWidgetBegin( popupWindow );
	imuiWidgetSetAlign( popup, 0.5f, 0.5f );
	imuiWidgetSetPadding( popup, s_theme.popup.padding );
	imuiWidgetSetLayoutVertical( popup );

	imuiWidgetDrawSkin( popup, &s_theme.skins[ ImuiToolboxSkin_Popup ], s_theme.colors[ ImuiToolboxColor_Popup ] );

	return popupWindow;
}

size_t imuiToolboxPopupEndButtons( ImuiWindow* popupWindow, const char** buttons, size_t buttonCount )
{
	ImuiWidget* buttonsLayout = imuiWidgetBegin( popupWindow );
	imuiWidgetSetHAlign( buttonsLayout, 1.0f );
	imuiWidgetSetLayoutHorizontalSpacing( buttonsLayout, s_theme.popup.buttonSpacing );

	uintsize clickedButton = (uintsize)-1;
	for( uintsize i = 0; i < buttonCount; ++i )
	{
		if( imuiToolboxButtonLabel( popupWindow, buttons[ i ] ) )
		{
			clickedButton = i;
		}
	}

	imuiWidgetEnd( buttonsLayout );

	imuiToolboxPopupEnd( popupWindow );

	return clickedButton;
}

void imuiToolboxPopupEnd( ImuiWindow* popupWindow )
{
	ImuiWidget* background = imuiWindowGetFirstChild( popupWindow );
	ImuiWidget* popup = imuiWidgetGetFirstChild( background );
	imuiWidgetEnd( popup );
	imuiWidgetEnd( background );
	imuiWindowEnd( popupWindow );
}

ImuiWidget* imuiToolboxTabViewBegin( ImuiToolboxTabViewContext* tabView, ImuiWindow* window )
{
	tabView->view = imuiWidgetBegin( window );
	imuiWidgetSetLayoutVertical( tabView->view );

	tabView->head = imuiWidgetBegin( window );
	imuiWidgetSetLayoutHorizontalSpacing( tabView->head, s_theme.tabView.headerSpacing );

	tabView->body					= NULL;
	tabView->headerCount			= 0u;
	tabView->state					= (ImuiToolboxTabViewState*)imuiWidgetAllocState( tabView->head, sizeof( ImuiToolboxTabViewState ), IMUI_ID_TYPE( ImuiToolboxTabViewState ) );
	tabView->selectedHeaderOffset	= 0.0f;
	tabView->selectedHeaderWidth	= 0.0f;

	return tabView->view;
}

size_t imuiToolboxTabViewGetSelectedIndex( const ImuiToolboxTabViewContext* tabView )
{
	return tabView->state->selectedTab;
}

void imuiToolboxTabViewSetSelectedIndex( ImuiToolboxTabViewContext* tabView, size_t index )
{
	tabView->state->selectedTab = index;
}

bool imuiToolboxTabViewHeader( ImuiToolboxTabViewContext* tabView, const char* text )
{
	ImuiWidget* tabHeader = imuiToolboxTabViewHeaderBegin( tabView );
	imuiToolboxLabel( imuiWidgetGetWindow( tabHeader ), text );
	return imuiToolboxTabViewHeaderEnd( tabView, tabHeader );
}

ImuiWidget* imuiToolboxTabViewHeaderBegin( ImuiToolboxTabViewContext* tabView )
{
	IMUI_ASSERT( tabView->head );

	ImuiWidget* tabHeader = imuiWidgetBegin( imuiWidgetGetWindow( tabView->head ) );
	imuiWidgetSetPadding( tabHeader, s_theme.tabView.headerPadding );

	ImuiColor color = s_theme.colors[ ImuiToolboxColor_TabViewHeaderInactive ];
	const ImuiSkin* skin = &s_theme.skins[ ImuiToolboxSkin_TabViewHeaderInactive ];

	if( tabView->state->selectedTab == tabView->headerCount )
	{
		color = s_theme.colors[ ImuiToolboxColor_TabViewHeaderActive ];
		skin = &s_theme.skins[ ImuiToolboxSkin_TabViewHeaderActive ];

		tabView->selectedHeaderOffset	= imuiWidgetGetPosX( tabHeader ) - imuiWidgetGetPosX( tabView->head );
		tabView->selectedHeaderWidth	= imuiWidgetGetSizeWidth( tabHeader );
	}

	imuiWidgetDrawSkin( tabHeader, skin, color );

	return tabHeader;
}

bool imuiToolboxTabViewHeaderEnd( ImuiToolboxTabViewContext* tabView, ImuiWidget* tabHeader )
{
	ImuiWidgetInputState inputState;
	imuiWidgetGetInputState( tabHeader, &inputState );

	if( inputState.hasMousePressed )
	{
		tabView->state->selectedTab = tabView->headerCount;
	}

	imuiWidgetEnd( tabHeader );

	const bool selected = tabView->state->selectedTab == tabView->headerCount;
	tabView->headerCount++;
	return selected;
}

ImuiWidget* imuiToolboxTabViewBodyBegin( ImuiToolboxTabViewContext* tabView )
{
	IMUI_ASSERT( tabView->head );

	imuiWidgetEnd( tabView->head );
	tabView->head = NULL;

	tabView->body = imuiWidgetBegin( imuiWidgetGetWindow( tabView->view ) );
	imuiWidgetSetStretchOne( tabView->body );
	imuiWidgetSetPadding( tabView->body, s_theme.tabView.bodyPadding );

	const ImuiSkin* skin = &s_theme.skins[ ImuiToolboxSkin_TabViewBody ];

	const float uScale = skin->width ? (skin->uv.u1 - skin->uv.u0) / skin->width : 0.0f;
	const float vScale = skin->height ? (skin->uv.v1 - skin->uv.v0) / skin->height : 0.0f;

	ImuiBorder uvBorder = skin->border;
	uvBorder.top	*= vScale;
	uvBorder.left	*= uScale;
	uvBorder.bottom	*= vScale;
	uvBorder.right	*= uScale;

	ImuiImage image;
	image.textureHandle	= skin->textureHandle;
	image.width			= skin->width;
	image.height		= skin->height;
	image.uv			= skin->uv;

	ImuiRect rect = imuiWidgetGetRect( tabView->body );
	rect.pos.x	= 0.0f;
	rect.pos.y	= 0.0f;

	const ImuiSize borderSize = imuiBorderGetMinSize( skin->border );
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

	const ImuiColor color = s_theme.colors[ ImuiToolboxColor_TabViewBody ];
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

		const ImuiPos posTl = imuiPosCreate( posX, yPositions[ 0u ] );
		const ImuiPos posBr = imuiPosCreate( nextPosX, yPositions[ 1u ] );

		const ImuiTexCoord uv =
		{
			uPositions[ uvX ], vPositions[ uvY ],
			uPositions[ uvX + 1u ], vPositions[ uvY + 1u ]
		};

		image.uv = uv;

		rect.pos			= posTl;
		rect.size.width		= posBr.x - posTl.x;
		rect.size.height	= posBr.y - posTl.y;

		imuiWidgetDrawPartialImageColor( tabView->body, rect, &image, color );
	}

	for( uintsize y = 1u; y < 3u; ++y )
	{
		const uintsize nextY = y + 1u;

		for( uintsize x = 0; x < 3; ++x )
		{
			const uintsize nextX = x + 1u;

			const ImuiPos posTl = imuiPosCreate( xPositions[ x ], yPositions[ y ] );
			const ImuiPos posBr = imuiPosCreate( xPositions[ nextX ], yPositions[ nextY ] );

			const ImuiTexCoord uv =
			{
				uPositions[ x ], vPositions[ y ],
				uPositions[ nextX ], vPositions[ nextY ]
			};

			image.uv = uv;

			rect.pos			= posTl;
			rect.size.width		= posBr.x - posTl.x;
			rect.size.height	= posBr.y - posTl.y;

			imuiWidgetDrawPartialImageColor( tabView->body, rect, &image, color );
		}
	}

	return tabView->body;
}

void imuiToolboxTabViewBodyEnd( ImuiToolboxTabViewContext* tabView )
{
	IMUI_ASSERT( tabView->body );

	imuiWidgetEnd( tabView->body );
	tabView->body = NULL;
}

void imuiToolboxTabViewEnd( ImuiToolboxTabViewContext* tabView )
{
	IMUI_ASSERT( !tabView->head );
	IMUI_ASSERT( !tabView->body );

	imuiWidgetEnd( tabView->view );
}

#if defined( _MSC_VER )
#	pragma warning(pop)
#endif
