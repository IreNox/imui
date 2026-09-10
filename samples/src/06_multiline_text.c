#include "00_samples.h"

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

typedef struct ImuiMultilineTextSampleContext
{
	float							textWidth;

#ifndef IMUI_NO_SAMPLE_FRAMEWORK
	ImuiFrameworkToolboxConfigData	configData;
#endif
} ImuiMultilineTextSampleContext;

static ImuiMultilineTextSampleContext s_multilineTextContext =
{
	.textWidth = 400.0f
};

static const char s_multilineText[] = "Lorem ipsum dolor sit amet, consectetuer adipiscing elit. Aenean commodo ligula eget dolor. Aenean massa. Cum sociis natoque penatibus et magnis dis parturient montes, nascetur ridiculus mus. Donec quam felis, ultricies nec, pellentesque eu, pretium quis, sem. Nulla consequat massa quis enim. Donec pede justo, fringilla vel, aliquet nec, vulputate eget, arcu. In enim justo, rhoncus ut, imperdiet a, venenatis vitae, justo. Nullam dictum felis eu pede mollis pretium. Integer tincidunt. Cras dapibus. Vivamus elementum semper nisi. Aenean vulputate eleifend tellus. Aenean leo ligula, porttitor eu, consequat vitae, eleifend ac, enim. Aliquam lorem ante, dapibus in, viverra quis, feugiat a, tellus. Phasellus viverra nulla ut metus varius laoreet. Quisque rutrum. Aenean imperdiet. Etiam ultricies nisi vel augue. Curabitur ullamcorper ultricies nisi. Nam eget dui. Etiam rhoncus. Maecenas tempus, tellus eget condimentum rhoncus, sem quam semper libero, sit amet adipiscing sem neque sed ipsum. Nam quam nunc, blandit vel, luctus pulvinar, hendrerit id, lorem. Maecenas nec odio et ante tincidunt tempus. Donec vitae sapien ut libero venenatis faucibus. Nullam quis ante. Etiam sit amet orci eget eros faucibus tincidunt. Duis leo. Sed fringilla mauris sit amet nibh. Donec sodales sagittis magna. Sed consequat, leo eget bibendum sodales, augue velit cursus nunc, quis gravida magna mi a libero. Fusce vulputate eleifend sapien. Vestibulum purus quam, scelerisque ut, mollis sed, nonummy id, metus. Nullam accumsan lorem in dui. Cras ultricies mi eu turpis hendrerit fringilla. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia Curae; In ac dui quis mi consectetuer lacinia. Nam pretium turpis et arcu. Duis arcu tortor, suscipit eget, imperdiet nec, imperdiet iaculis, ipsum. Sed aliquam ultrices mauris. Integer ante arcu, accumsan a, consectetuer eget, posuere ut, mauris. Praesent adipiscing. Phasellus ullamcorper ipsum rutrum nunc. Nunc nonummy metus. Vestibulum volutpat pretium libero. Cras id dui. Aenean ut eros et nisl sagittis vestibulum. Nullam nulla eros, ultricies sit amet, nonummy id, imperdiet feugiat, pede. Sed lectus. Donec mollis hendrerit risus. Phasellus nec sem in justo pellentesque facilisis. Etiam imperdiet imperdiet orci. Nunc nec neque. Phasellus leo dolor, tempus non, auctor et, hendrerit quis, nisi. Curabitur ligula sapien, tincidunt non, euismod vitae, posuere imperdiet, leo. Maecenas malesuada. Praesent congue erat at massa. Sed cursus turpis vitae tortor. Donec posuere vulputate arcu. Phasellus accumsan cursus velit. Vestibulum ante ipsum primis in faucibus orci luctus et ultrices posuere cubilia Curae; Sed aliquam, nisi quis porttitor congue, elit erat euismod orci, ac";

void imuiMultilineTextSampleTick( ImuiWindow* window )
{
#ifndef IMUI_NO_SAMPLE_FRAMEWORK
	imuiFrameworkToolboxConfigDataApply( &s_multilineTextContext.configData );
#endif

	ImuiWidget* hLayout = imuiWidgetBeginNamed( window, "hMain" );
	imuiWidgetSetStretchOne( hLayout );
	imuiWidgetSetMargin( hLayout, imuiBorderCreateAll( 25.0f ) );
	imuiWidgetSetLayoutVerticalSpacing( hLayout, 10.0f );

	imuiToolboxSliderMinMax( window, &s_multilineTextContext.textWidth, 10.0f, imuiWidgetGetSizeWidth( hLayout ) );

	ImuiWidget* text = imuiWidgetBegin( window );
	imuiWidgetSetStretchOne( text );
	imuiWidgetSetMaxSizeFloat( text, s_multilineTextContext.textWidth, 100000000.0f );

	ImuiTextLayout* layout = imuiTextLayoutCreateWidgetLengthSize( text, imuiToolboxThemeGet()->font, s_multilineText, sizeof( s_multilineText ), imuiWidgetGetSize( text ) );
	imuiWidgetDrawText( text, layout, imuiColorCreateWhite() );

	imuiWidgetEnd( text );

	imuiWidgetEnd( hLayout );
}

bool imuiMultilineTextSampleInitialize( ImuiContext* imui )
{
#ifndef IMUI_NO_SAMPLE_FRAMEWORK
	return imuiFrameworkToolboxConfigDataInitialize( &s_multilineTextContext.configData, imui );
#else
	return true;
#endif
}

void imuiMultilineTextSampleShutdown( ImuiContext* imui )
{
#ifndef IMUI_NO_SAMPLE_FRAMEWORK
	imuiFrameworkToolboxConfigDataShutdown( &s_multilineTextContext.configData, imui );
#endif
}
