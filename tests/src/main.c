#include "tests.h"

#include <stdio.h>

ImuiContext* g_imuiContext = NULL;

static int runTests( ImuiFrame* frame )
{
	int failed = 0;
	failed += runBaseTests( frame );
	failed += runDataTypeTests( frame );
	failed += runInputTests( frame );
	failed += runSurfaceTests( frame );
	failed += runWindowTests( frame );
	failed += runWidgetTests( frame );
	failed += runLayoutTests( frame );

	return failed;
}

int main()
{
	ImuiParameters parameters = { 0 };
	g_imuiContext = imuiCreate( &parameters );
	if( !g_imuiContext )
	{
		fprintf( stderr, "Error: failed to create context.\n" );
		return 1;
	}

	ImuiFrame* frame = imuiBegin( g_imuiContext, 0.0 );

	const int failed = runTests( frame );

	imuiEnd( frame );
	imuiDestroy( g_imuiContext );
	g_imuiContext = NULL;
	return failed ? 1 : 0;
}
