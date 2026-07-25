# I'm UI

[![Build Samples](https://github.com/IreNox/imui/actions/workflows/build_samples.yml/badge.svg)](https://github.com/IreNox/imui/actions/workflows/build_samples.yml)

A simple to use Immediate Mode UI library written in C. This is in an early state and all APIs may change.

## Features

- Five different layout methods(Stack, Scroll, Horizintal, Vertical, Grid)
- Toolbox with a lot of generic widgets like: Button, Check Box, List etc
- Bitmap font generation
- C++ API wrapper included
- a lot more

## Sample

![samples](samples/screen_shot.png)

## Example

```c++
void doUi( ImuiContext* imui, float time )
{
	ImuiInput* input = imuiInputBegin( imui, NULL );
	// write input state
	const ImuiInputState* inputState = imuiInputEnd( imui );

	ImuiFrame* frame = imuiBegin( imui, time );
	ImuiSurface* surface = imuiSurfaceBegin( frame, "main", imuiSizeCreate( (float)windowWidth, (float)windowHeight ), inputState, 1.0f );
	ImuiWindow* window = imuiWindowBegin( surface, "main", imuiRectCreateSize( 0.0f, 0.0f, imuiSurfaceGetSize( surface ) ), 1 );

	ImuiWidget* buttonsLayout = imuiWidgetBeginNamed( window, "buttons" );
	imuiWidgetSetLayoutHorizontalSpacing( buttonsLayout, 10.0f );

	if( imuiToolboxButtonLabel( window, "Button 1" ) )
	{
		// do stuff
	}

	if( imuiToolboxButtonLabel( window, "Button 2" ) )
	{
		// do stuff
	}

	if( imuiToolboxButtonLabel( window, "Button 3" ) )
	{
		// do stuff
	}

	imuiWidgetEnd( buttonsLayout );

	imuiWindowEnd( window );
	imuiSurfaceEnd( surface );
	imuiEnd( frame );
}
```

## Building

Just import all files in `include` and `src` in your project and add `include` as include directory.

To build the samples with cmake run `cmake -B build` or tiki_build use the provided batch files to generate Visual Studio files.

## TODO

- multiline text
- fix: use pool for surface and window(broken pointer on resize)
- more

## Used Libraries

- [stb_truetype](https://github.com/nothings/stb)
- [tiki_build](https://github.com/IreNox/tiki_build)
