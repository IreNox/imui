#include "00_samples.h"

typedef bool (*imuiSampleInitialize)( ImuiContext* imui );
typedef void (*imuiSampleShutdown)( ImuiContext* imui );
typedef void (*imuiSampleTick)( ImuiWindow* window );

typedef struct ImuiSample
{
	imuiSampleInitialize	initialize;
	imuiSampleShutdown		shutdown;
	imuiSampleTick			tick;
} ImuiSample;

static const ImuiSample s_samples[] =
{
	{ imuiHelloWorldSampleInitialize, imuiHelloWorldSampleShutdown, imuiHelloWorldSampleTick },
	{ imuiLayoutSampleInitialize, imuiLayoutSampleShutdown, imuiLayoutSampleTick },
	{ imuiInputSampleInitialize, imuiInputSampleShutdown, imuiInputSampleTick },
	{ imuiToolboxSampleInitialize, imuiToolboxSampleShutdown, imuiToolboxSampleTick },
	{ imuiToolboxCppSampleInitialize, imuiToolboxCppSampleShutdown, imuiToolboxCppSampleTick }
};

#define IMUI_SAMPLES_DEFAULT_INDEX 3

static const ImuiSample* s_currentSample = &s_samples[ IMUI_SAMPLES_DEFAULT_INDEX ];

bool imuiFrameworkInitialize( ImuiContext* imui )
{
	s_currentSample->initialize( imui );
	return true;
}

void imuiFrameworkShutdown( ImuiContext* imui )
{
	s_currentSample->shutdown( imui );
}

void imuiFrameworkTick( ImuiSurface* surface )
{
	ImuiContext* imui = imuiSurfaceGetContext( surface );

	for( int i = ImuiInputKey_1; i < ImuiInputKey_1 + (sizeof( s_samples ) / sizeof( *s_samples )); ++i )
	{
		const ImuiSample* sample = &s_samples[ i - ImuiInputKey_1 ];
		if( sample == s_currentSample )
		{
			continue;
		}

		if( !imuiInputHasKeyPressed( imuiSurfaceGetInput( surface ), (ImuiInputKey)i ) )
		{
			continue;
		}

		s_currentSample->shutdown( imui );
		s_currentSample = sample;
		s_currentSample->initialize( imui );
		break;
	}

	const ImuiSize surfaceSize = imuiSurfaceGetSize( surface );
	ImuiWindow* window = imuiWindowBegin( surface, "main", imuiRectCreate( 0.0f, 0.0f, surfaceSize.width, surfaceSize.height ), 1 );

	s_currentSample->tick( window );

	imuiWindowEnd( window );
}
