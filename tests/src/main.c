#include "tests.h"

#include <stdio.h>

extern ImuiContext* g_imuiContext = NULL;

static int runTests( ImuiFrame* frame )
{
	int failed = 0;
	failed += runBaseTests( frame );

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
}
