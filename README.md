# I'm UI

A simple to use Immediate Mode UI library written in C. This is in an early state and all APIs may change.

## Features

- Five different layout methods
- Toolbox with a lot of generic widgets like: Button, Check Box etc.
- Bitmap font generation
- C++ API wrapper included
- a lot more

## Example

```
void doUi( ImUiContext* imui, float time )
{
	ImUiFrame* frame = ImUiBegin( imui, time );
	ImUiSurface* surface = ImUiSurfaceBegin( frame, IMUI_STR( "main" ), ImUiSizeCreate( (float)s_context.windowWidth, (float)s_context.windowHeight ), 1.0f );
	ImUiWindow* window = ImUiWindowBegin( surface, IMUI_STR( "main" ), ImUiRectCreateSize( 0.0f, 0.0f, surfaceSize ), 1 );

	ImUiWidget* buttonsLayout = ImUiWidgetBeginNamed( window, IMUI_STR( "buttons" ) );
	ImUiWidgetSetStretch( buttonsLayout, ImUiSizeCreateZero() );
	ImUiWidgetSetLayoutHorizontalSpacing( buttonsLayout, 10.0f );

	if( ImUiToolboxButtonLabel( window, IMUI_STR( "Button 1" ) ) )
	{
		// do stuff
	}

	if( ImUiToolboxButtonLabel( window, IMUI_STR( "Button 2" ) ) )
	{
		// do stuff
	}

	if( ImUiToolboxButtonLabel( window, IMUI_STR( "Button 3" ) ) )
	{
		// do stuff
	}

	ImUiWidgetEnd( buttonsLayout );
	
	ImUiWindowEnd( window );
	ImUiSurfaceEnd( surface );
	ImUiEnd( imui );	
}
```

## Building

Just import all files in `include` and `src` in your project and add `include` as include directory.

To build the samples is current [tiki_build](https://github.com/IreNox/tiki_build) required. Use the provided batch files to generate Visual Studio files. 

## TODO

- State id to allow multiple states per widget
- TextEdit mouse selection
- fix layout bugs
- more

## Used Libraries

- [stb_truetype](https://github.com/nothings/stb)
- [tiki_build](https://github.com/IreNox/tiki_build)
