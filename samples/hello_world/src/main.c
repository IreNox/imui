#include "../../framework/framework.h"

#include "imui/imui.h"

void ImUiFrameworkTick( ImUiSurface* surface )
{
	const ImUiSize surfaceSize = ImUiSurfaceGetSize( surface );
	ImUiWindow* window = ImUiWindowBegin( surface, ImUiStringViewCreate( "main" ), ImUiRectangleCreate( 0.0f, 0.0f, surfaceSize.width, surfaceSize.height ), 1 );

	//ImUiDraw

	ImUiWidget* widget = ImUiWidgetCreate( ImUiWindowGetRootWidget( window ) );
	ImUiWidgetSetMargin( widget, ImUiThicknessCreateAll( 50.0f ) );
	ImUiWidgetSetFixedSize( widget, ImUiSizeCreate( 300.0f, 100.0f ) );

	ImUiDrawRectangleColor( widget, ImUiWidgetGetRectangle( widget ), ImUiColorCreateWhite( 1.0f ) );

	ImUiWindowEnd( window );
}
