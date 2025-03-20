#include "00_samples.h"

#ifndef IMUI_NO_SAMPLE_FRAMEWORK
#	include "framework.h"
#endif

#include "imui/imui.h"
#include "imui/imui_toolbox.h"

typedef enum ImUiToolboxEditorSampleMode
{
	ImUiToolboxEditorSampleMode_Write,
	ImUiToolboxEditorSampleMode_Read,
	ImUiToolboxEditorSampleMode_Append,

	ImUiToolboxEditorSampleMode_Count
} ImUiToolboxEditorSampleMode;

typedef struct ImUiToolboxEditorSampleContext
{
	ImUiToolboxEditorSampleMode		mode;

#ifndef IMUI_NO_SAMPLE_FRAMEWORK
	ImUiFrameworkToolboxConfigData	configData;
#endif
} ImUiToolboxEditorSampleContext;

static ImUiToolboxEditorSampleContext s_toolboxEditorContext = { ImUiToolboxEditorSampleMode_Write };

void ImUiToolboxEditorSampleTick( ImUiWindow* window )
{
	ImUiWidget* vLayout = ImUiWidgetBeginNamed( window, "vMain" );
	ImUiWidgetSetHStretch( vLayout, 1.0f );
	ImUiWidgetSetLayoutVerticalSpacing( vLayout, 10.0f );

	ImUiWidget* hLayout = ImUiWidgetBeginNamed( window, "hMain" );
	ImUiWidgetSetStretchOne( hLayout );
	ImUiWidgetSetMargin( hLayout, ImUiBorderCreateAll( 25.0f ) );
	ImUiWidgetSetLayoutHorizontalSpacing( hLayout, 10.0f );

	for( size_t i = 0u; i < ImUiToolboxEditorSampleMode_Count; ++i )
	{
		const char* text = "Error";
		switch( i )
		{
		case ImUiToolboxEditorSampleMode_Write:		text = "Write"; break;
		case ImUiToolboxEditorSampleMode_Read:		text = "Read"; break;
		case ImUiToolboxEditorSampleMode_Append:	text = "Append"; break;
		default:									break;
		}

		bool checked = i == s_toolboxEditorContext.mode;
		if( ImUiToolboxCheckBox( window, &checked, text ) && checked )
		{
			s_toolboxEditorContext.mode = i;
		}
	}

	ImUiWidgetEnd( hLayout );

	switch( s_toolboxEditorContext.mode )
	{
	case ImUiToolboxEditorSampleMode_Write:
		break;

	case ImUiToolboxEditorSampleMode_Read:
		break;

	case ImUiToolboxEditorSampleMode_Append:
		break;

	default:
		break;
	}

	ImUiWidgetEnd( vLayout );
}

#ifndef IMUI_NO_SAMPLE_FRAMEWORK
bool ImUiToolboxEditorSampleInitialize( ImUiContext* imui )
{
	return ImUiFrameworkToolboxConfigDataInitialize( &s_toolboxEditorContext.configData, imui );
}

void ImUiToolboxEditorSampleShutdown( ImUiContext* imui )
{
	ImUiFrameworkToolboxConfigDataShutdown( &s_toolboxEditorContext.configData, imui );
}
#endif
