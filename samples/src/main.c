#include "00_samples.h"

typedef bool (*ImUiSampleInitialize)( ImUiContext* imui );
typedef void (*ImUiSampleShutdown)( ImUiContext* imui );
typedef void (*ImUiSampleTick)( ImUiWindow* window );

typedef struct ImUiSample
{
	ImUiSampleInitialize	initialize;
	ImUiSampleShutdown		shutdown;
	ImUiSampleTick			tick;
} ImUiSample;

static const ImUiSample s_samples[] =
{
	{ ImUiHelloWorldSampleInitialize, ImUiHelloWorldSampleShutdown, ImUiHelloWorldSampleTick },
	{ ImUiLayoutSampleInitialize, ImUiLayoutSampleShutdown, ImUiLayoutSampleTick },
	{ ImUiInputSampleInitialize, ImUiInputSampleShutdown, ImUiInputSampleTick },
	{ ImUiToolboxSampleInitialize, ImUiToolboxSampleShutdown, ImUiToolboxSampleTick },
	{ ImUiToolboxCppSampleInitialize, ImUiToolboxCppSampleShutdown, ImUiToolboxCppSampleTick }
};

#define IMUI_SAMPLES_DEFAULT_INDEX 3

static const ImUiSample* s_currentSample = &s_samples[ IMUI_SAMPLES_DEFAULT_INDEX ];

bool ImUiFrameworkInitialize( ImUiContext* imui )
{
	s_currentSample->initialize( imui );
	return true;
}

void ImUiFrameworkShutdown( ImUiContext* imui )
{
	s_currentSample->shutdown( imui );
}

void ImUiFrameworkTick( ImUiSurface* surface )
{
	ImUiContext* imui = ImUiSurfaceGetContext( surface );

	for( int i = ImUiInputKey_1; i < ImUiInputKey_1 + (sizeof( s_samples ) / sizeof( *s_samples )); ++i )
	{
		const ImUiSample* sample = &s_samples[ i - ImUiInputKey_1 ];
		if( sample == s_currentSample )
		{
			continue;
		}

		if( !ImUiInputHasKeyPressed( imui, (ImUiInputKey)i ) )
		{
			continue;
		}

		s_currentSample->shutdown( imui );
		s_currentSample = sample;
		s_currentSample->initialize( imui );
		break;
	}

	const ImUiSize surfaceSize = ImUiSurfaceGetSize( surface );
	ImUiWindow* window = ImUiWindowBegin( surface, "main", ImUiRectCreate( 0.0f, 0.0f, surfaceSize.width, surfaceSize.height ), 1 );

	s_currentSample->tick( window );

	ImUiWindowEnd( window );
}
