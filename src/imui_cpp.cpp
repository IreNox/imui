#include "imui/imui_cpp.h"

#include "imui_internal.h"

#include <cstdarg>
#include <cstring>

namespace imui
{
	UiSize UiSize::Zero			= UiSize();
	UiSize UiSize::One			= UiSize( 1.0f );

	UiSize::UiSize()
	{
		width	= 0.0f;
		height	= 0.0f;
	}

	UiSize::UiSize( float all )
	{
		width	= all;
		height	= all;
	}

	UiSize::UiSize( float _width, float _height )
	{
		width	= _width;
		height	= _height;
	}

	UiSize::UiSize( const ImuiSize& value )
	{
		width	= value.width;
		height	= value.height;
	}

	UiSize::UiSize( const ImuiSkin& value )
	{
		width	= (float)value.width;
		height	= (float)value.height;
	}

	UiSize::UiSize( const ImuiImage& value )
	{
		width	= (float)value.width;
		height	= (float)value.height;
	}

	UiSize UiSize::add( float _width, float _height ) const
	{
		return UiSize( width + _width, height + _height );
	}

	UiSize UiSize::add( UiSize add ) const
	{
		return UiSize( width + add.width, height + add.height );
	}

	UiSize UiSize::sub( float _width, float _height ) const
	{
		return UiSize( width - _width, height - _height );
	}

	UiSize UiSize::sub( UiSize sub ) const
	{
		return UiSize( width - sub.width, height - sub.height );
	}

	UiSize UiSize::scale( float factor ) const
	{
		return UiSize( width * factor, height * factor );
	}

	UiSize UiSize::shrinkBorder( const UiBorder& border ) const
	{
		return UiSize( width - (border.left + border.right), height - (border.top + border.bottom) );
	}

	UiSize UiSize::expandBorder( const UiBorder& border ) const
	{
		return UiSize( width + border.left + border.right, height + border.top + border.bottom );
	}

	UiSize UiSize::operator+( const UiSize& rhs ) const
	{
		return add( rhs );
	}

	UiSize UiSize::operator-( const UiSize& rhs ) const
	{
		return sub( rhs );
	}

	UiSize UiSize::operator*( float rhs ) const
	{
		return scale( rhs );
	}

	UiSize& UiSize::operator-=( const UiSize& rhs )
	{
		*this = sub( rhs );
		return *this;
	}

	UiSize& UiSize::operator+=( const UiSize& rhs )
	{
		*this = add( rhs );
		return *this;
	}

	UiSize& UiSize::operator*=( float rhs )
	{
		*this = scale( rhs );
		return *this;
	}

	UiSize UiSize::lerp( UiSize a, UiSize b, float t )
	{
		return UiSize(
			a.width + ((b.width - a.width) * t),
			a.height + ((b.height - a.height) * t)
		);
	}

	UiSize UiSize::lerp( UiSize a, UiSize b, float widthT, float heightT )
	{
		return UiSize(
			a.width + ((b.width - a.width) * widthT),
			a.height + ((b.height - a.height) * heightT)
		);
	}

	UiSize UiSize::min( UiSize a, UiSize b )
	{
		return UiSize(
			IMUI_MIN( a.width, b.width ),
			IMUI_MIN( a.height, b.height )
		);
	}

	UiSize UiSize::max( UiSize a, UiSize b )
	{
		return UiSize(
			IMUI_MAX( a.width, b.width ),
			IMUI_MAX( a.height, b.height )
		);
	}

	UiBorder::UiBorder()
	{
		top		= 0.0f;
		left	= 0.0f;
		bottom	= 0.0f;
		right	= 0.0f;
	}

	UiBorder::UiBorder( float all )
	{
		top		= all;
		left	= all;
		bottom	= all;
		right	= all;
	}

	UiBorder::UiBorder( float horizontal, float vertical )
	{
		top		= vertical;
		left	= horizontal;
		bottom	= vertical;
		right	= horizontal;
	}

	UiBorder::UiBorder( float _top, float _left, float _bottom, float _right )
	{
		top		= _top;
		left	= _left;
		bottom	= _bottom;
		right	= _right;
	}

	UiBorder::UiBorder( const ImuiBorder& value )
	{
		top		= value.top;
		left	= value.left;
		bottom	= value.bottom;
		right	= value.right;
	}

	UiSize UiBorder::getMinSize() const
	{
		return UiSize( left + right, top + bottom );
	}

	UiPos UiPos::Zero = UiPos();

	UiPos::UiPos()
	{
		x = 0.0f;
		y = 0.0f;
	}

	UiPos::UiPos( float all )
	{
		x = all;
		y = all;
	}

	UiPos::UiPos( float _x, float _y )
	{
		x = _x;
		y = _y;
	}

	UiPos::UiPos( const ImuiPos& value )
	{
		x = value.x;
		y = value.y;
	}

	UiPos::UiPos( const ImuiSize& value )
	{
		x = value.width;
		y = value.height;
	}

	UiPos UiPos::add( float _x, float _y ) const
	{
		return UiPos( x + _x, y + _y );
	}

	UiPos UiPos::add( UiPos add ) const
	{
		return UiPos( x + add.x, y + add.y );
	}

	UiPos UiPos::sub( float _x, float _y ) const
	{
		return UiPos( x - _x, y - _y );
	}

	UiPos UiPos::sub( UiPos sub ) const
	{
		return UiPos( x - sub.x, y - sub.y );
	}

	UiPos UiPos::scale( float factor ) const
	{
		return UiPos( x * factor, y * factor );
	}

	UiPos UiPos::operator+( const UiPos& rhs ) const
	{
		return add( rhs );
	}

	UiPos UiPos::operator-( const UiPos& rhs ) const
	{
		return sub( rhs );
	}

	UiPos UiPos::operator*( float rhs ) const
	{
		return scale( rhs );
	}

	UiPos& UiPos::operator+=( const UiPos& rhs )
	{
		*this = add( rhs );
		return *this;
	}

	UiPos& UiPos::operator-=( const UiPos& rhs )
	{
		*this = sub( rhs );
		return *this;
	}

	UiPos& UiPos::operator*=( float rhs )
	{
		*this = scale( rhs );
		return *this;
	}

	UiRect::UiRect()
	{
		pos		= UiPos::Zero;
		size	= UiSize::Zero;
	}

	UiRect::UiRect( UiPos _pos, UiSize _size )
	{
		pos		= _pos;
		size	= _size;
	}

	UiRect::UiRect( float x, float y, float width, float height )
	{
		pos		= UiPos( x, y );
		size	= UiSize( width, height );
	}

	UiRect::UiRect( const ImuiRect& value )
	{
		pos		= UiPos( value.pos );
		size	= UiSize( value.size );
	}

	UiRect UiRect::shrinkBorder( const ImuiBorder& border ) const
	{
		return (UiRect)imuiRectShrinkBorder( *this, border );
	}

	bool UiRect::includesPos( ImuiPos _pos ) const
	{
		return imuiRectIncludesPos( *this, _pos );
	}

	bool UiRect::intersectsRect( const ImuiRect& rect2 ) const
	{
		return imuiRectIntersectsRect( *this, rect2 );
	}

	float UiRect::getRight() const
	{
		return pos.x + size.width;
	}

	float UiRect::getBottom() const
	{
		return pos.y + size.height;
	}

	UiPos UiRect::getTopLeft() const
	{
		return (const UiPos&)pos;
	}

	UiPos UiRect::getTopRight() const
	{
		return UiPos( getRight(), pos.y );
	}

	UiPos UiRect::getBottomLeft() const
	{
		return UiPos( pos.x, getBottom() );
	}

	UiPos UiRect::getBottomRight() const
	{
		return UiPos( getRight(), getBottom() );
	}

	UiSize UiRect::getSize() const
	{
		return (const UiSize&)size;
	}

	UiColor UiColor::White				= UiColor( imuiColorCreateWhite() );
	UiColor UiColor::Black				= UiColor( imuiColorCreateBlack() );
	UiColor UiColor::TransparentBlack	= UiColor();

	UiColor::UiColor()
		: ImuiColor( imuiColorCreateTransparentBlack() )
	{
	}

	UiColor::UiColor( uint8_t _red, uint8_t _green, uint8_t _blue )
		: ImuiColor( imuiColorCreate( _red, _green, _blue, 0xffu ) )
	{
	}

	UiColor::UiColor( uint8_t _red, uint8_t _green, uint8_t _blue, uint8_t _alpha )
		: ImuiColor( imuiColorCreate( _red, _green, _blue, _alpha ) )
	{
	}

	UiColor::UiColor( float _red, float _green, float _blue )
		: ImuiColor( imuiColorCreateFloat( _red, _green, _blue, 1.0f ) )
	{
	}

	UiColor::UiColor( float _red, float _green, float _blue, float _alpha )
		: ImuiColor( imuiColorCreateFloat( _red, _green, _blue, _alpha ) )
	{
	}

	UiColor::UiColor( ImuiColor value )
		: ImuiColor( value )
	{
	}

	UiColor UiColor::createWhite( uint8_t _alpha )
	{
		return UiColor( imuiColorCreateWhiteA( _alpha ) );
	}

	UiColor UiColor::createBlack( uint8_t _alpha )
	{
		return UiColor( imuiColorCreateBlackA( _alpha ) );
	}

	UiColor UiColor::createGray( uint8_t gray )
	{
		return UiColor( imuiColorCreateGray( gray ) );
	}

	UiColor UiColor::createGray( uint8_t gray, uint8_t _alpha )
	{
		return UiColor( imuiColorCreateGrayA( gray, _alpha ) );
	}

	UiTexCoord UiTexCoord::ZeroToOne = UiTexCoord( 0.0f, 0.0f, 1.0f, 1.0f );

	UiTexCoord::UiTexCoord( float _u0, float _v0, float _u1, float _v1 )
	{
		u0 = _u0;
		v0 = _v0;
		u1 = _u1;
		v1 = _v1;
	}

	UiContextParameters::UiContextParameters()
	{
		memset( this, 0u, sizeof( *this ) );
	}

	UiContext::UiContext()
		: m_owner( false )
		, m_context( nullptr )
	{
	}

	UiContext::UiContext( ImuiContext* context )
		: m_owner( false )
		, m_context( context )
	{
	}

	UiContext::UiContext( const UiContextParameters& parameters )
	{
		m_owner = true;
		m_context = imuiCreate( &parameters );
	}

	UiContext::~UiContext()
	{
		if( m_owner && m_context )
		{
			imuiDestroy( m_context );
			m_context = nullptr;
			m_owner = false;
		}
	}

	bool UiContext::isValid() const
	{
		return m_context != nullptr;
	}

	ImuiContext* UiContext::getInternal() const
	{
		return m_context;
	}

	UiInput& UiContext::beginInput( const ImuiInputState* previousInput )
	{
		ImuiInput* input = imuiInputBegin( m_context, previousInput );
		return *(UiInput*)input;
	}

	void UiContext::endInput()
	{
		imuiInputEnd( m_context );
	}

	void UiContext::setMouseCursor( ImuiInputMouseCursor cursor )
	{
		imuiInputSetMouseCursor( m_context, cursor );
	}

	UiFrame::UiFrame()
		: m_owner( false )
		, m_frame( nullptr )
	{
	}

	UiFrame::UiFrame( UiContext& context, float timeInSeconds )
		: m_owner( false )
		, m_frame( nullptr )
	{
		beginFrame( context, timeInSeconds );
	}

	UiInputState::UiInputState( const ImuiInputState* input )
		: m_state( input )
	{
	}

	uint32_t UiInputState::getKeyModifiers() const
	{
		return imuiInputGetKeyModifiers( m_state );
	}

	bool UiInputState::isKeyDown( ImuiInputKey key ) const
	{
		return imuiInputIsKeyDown( m_state, key );
	}

	bool UiInputState::isKeyUp( ImuiInputKey key ) const
	{
		return imuiInputIsKeyUp( m_state, key );
	}

	bool UiInputState::hasKeyPressed( ImuiInputKey key ) const
	{
		return imuiInputHasKeyPressed( m_state, key );
	}

	bool UiInputState::hasKeyReleased( ImuiInputKey key ) const
	{
		return imuiInputHasKeyReleased( m_state, key );
	}

	const char* UiInputState::getText() const
	{
		return imuiInputGetText( m_state );
	}

	UiPos UiInputState::getMousePos() const
	{
		return (UiPos)imuiInputGetMousePos( m_state );
	}

	bool UiInputState::isMouseInRect( UiRect rect ) const
	{
		return imuiInputIsMouseInRect( m_state, rect );
	}

	bool UiInputState::isMouseButtonDown( ImuiInputMouseButton button ) const
	{
		return imuiInputIsMouseButtonDown( m_state, button );
	}

	bool UiInputState::isMouseButtonUp( ImuiInputMouseButton button ) const
	{
		return imuiInputIsMouseButtonUp( m_state, button );
	}

	bool UiInputState::hasMouseButtonPressed( ImuiInputMouseButton button ) const
	{
		return imuiInputHasMouseButtonReleased( m_state, button );
	}

	bool UiInputState::hasMouseButtonReleased( ImuiInputMouseButton button ) const
	{
		return imuiInputHasMouseButtonReleased( m_state, button );
	}

	UiPos UiInputState::getMouseScrollDelta() const
	{
		return imuiInputGetMouseScrollDelta( m_state );
	}

	UiFrame::UiFrame( ImuiFrame* frame )
		: m_owner( false )
		, m_frame( frame )
	{
	}

	UiFrame::~UiFrame()
	{
		endFrame();
	}

	void UiFrame::beginFrame( UiContext& context, float timeInSeconds )
	{
		m_frame = imuiBegin( context.getInternal(), timeInSeconds );
		m_owner = true;
	}

	void UiFrame::endFrame()
	{
		if( m_owner && m_frame )
		{
			imuiEnd( m_frame );
			m_frame = nullptr;
			m_owner = false;
		}
	}

	bool UiFrame::isValid() const
	{
		return m_frame != nullptr;
	}

	ImuiFrame* UiFrame::getInternal() const
	{
		return m_frame;
	}

	UiSurface::UiSurface()
		: m_owner( false )
		, m_surface( nullptr )
	{
	}

	UiSurface::UiSurface( ImuiSurface* surface )
		: m_owner( false )
		, m_surface( surface )
	{
	}

	UiSurface::UiSurface( ImuiFrame* frame, const char* name, const ImuiSize& size, const ImuiInputState* input, float dpiScale )
	{
		beginSurface( frame, name, size, input, dpiScale );
	}

	UiSurface::UiSurface( UiFrame& frame, const char* name, const ImuiSize& size, const ImuiInputState* input, float dpiScale )
	{
		beginSurface( frame, name, size, input, dpiScale );
	}

	UiSurface::~UiSurface()
	{
		endSurface();
	}

	void UiSurface::beginSurface( ImuiFrame* frame, const char* name, const ImuiSize& size, const ImuiInputState* input, float dpiScale )
	{
		m_surface = imuiSurfaceBegin( frame, name, size, input, dpiScale );
		m_owner = true;
	}

	void UiSurface::beginSurface( UiFrame& frame, const char* name, const ImuiSize& size, const ImuiInputState* input, float dpiScale )
	{
		beginSurface( frame.getInternal(), name, size, input, dpiScale );
	}

	void UiSurface::endSurface()
	{
		if( m_owner && m_surface )
		{
			imuiSurfaceEnd( m_surface );
			m_surface = nullptr;
			m_owner = false;
		}
	}

	bool UiSurface::isValid() const
	{
		return m_surface != nullptr;
	}

	ImuiSurface* UiSurface::getInternal() const
	{
		return m_surface;
	}

	UiContext UiSurface::getContext() const
	{
		return UiContext( imuiSurfaceGetContext( m_surface ) );
	}

	double UiSurface::getTime() const
	{
		return imuiSurfaceGetTime( m_surface );
	}

	UiRect UiSurface::getRect() const
	{
		return UiRect( UiPos::Zero, UiSize( m_surface->size ) );
	}

	UiSize UiSurface::getSize() const
	{
		return (const UiSize&)m_surface->size;
	}

	UiInputState UiSurface::getInput() const
	{
		return UiInputState( imuiSurfaceGetInput( m_surface ) );
	}

	float UiSurface::getDpiScale() const
	{
		return m_surface->dpiScale;
	}

	UiWindow::UiWindow()
		: m_owner( false )
		, m_window( nullptr )
	{
	}

	UiWindow::UiWindow( ImuiWindow* window )
		: m_owner( false )
		, m_window( window )
	{
	}

	UiWindow::UiWindow( ImuiSurface* surface, const char* name, const ImuiRect& rect, uint32_t zOrder )
	{
		beginWindow( surface, name, rect, zOrder );
	}

	UiWindow::UiWindow( UiSurface& surface, const char* name, const ImuiRect& rect, uint32_t zOrder )
		: m_window( nullptr )
	{
		beginWindow( surface, name, rect, zOrder );
	}

	UiWindow::~UiWindow()
	{
		endWindow();
	}

	void UiWindow::beginWindow( ImuiSurface* surface, const char* name, const ImuiRect& rect, uint32_t zOrder )
	{
		m_window = imuiWindowBegin( surface, name, rect, zOrder );
		m_owner = true;
	}

	void UiWindow::beginWindow( UiSurface& surface, const char* name, const ImuiRect& rect, uint32_t zOrder )
	{
		beginWindow( surface.getInternal(), name, rect, zOrder );
	}

	void UiWindow::endWindow()
	{
		if( m_owner && m_window )
		{
			imuiWindowEnd( m_window );
			m_window = nullptr;
			m_owner = false;
		}
	}

	bool UiWindow::isValid() const
	{
		return m_window != nullptr;
	}

	ImuiWindow* UiWindow::getInternal() const
	{
		return m_window;
	}

	UiContext UiWindow::getContext() const
	{
		return UiContext( imuiWindowGetContext( m_window ) );
	}

	UiSurface UiWindow::getSurface() const
	{
		return UiSurface( imuiWindowGetSurface( m_window ) );
	}

	UiInputState UiWindow::getInput() const
	{
		return UiInputState( imuiWindowGetInput( m_window ) );
	}

	double UiWindow::getTime() const
	{
		return imuiWindowGetTime( m_window );
	}

	void* UiWindow::allocState( size_t size, ImuiId stateId )
	{
		return imuiWindowAllocState( m_window, size, stateId );
	}

	void* UiWindow::allocState( size_t size, ImuiId stateId, bool& isNew )
	{
		return imuiWindowAllocStateNew( m_window, size, stateId, &isNew );
	}

	UiRect UiWindow::getRect() const
	{
		return (const UiRect&)m_window->rect;
	}

	uint32_t UiWindow::getZOrder() const
	{
		return m_window->zOrder;
	}

	UiWidget::UiWidget()
		: m_owner( false )
		, m_widget( nullptr )
	{
	}

	UiWidget::UiWidget( UiWindow& window )
	{
		beginWidget( window );
	}

	UiWidget::UiWidget( UiWindow& window, ImuiId id )
	{
		beginWidget( window, id );
	}

	UiWidget::UiWidget( UiWindow& window, const char* name )
	{
		beginWidget( window, name );
	}


	UiWidget::UiWidget( ImuiWidget* widget )
	{
		beginWidget( widget );
	}

	UiWidget::~UiWidget()
	{
		endWidget();
	}

	void UiWidget::beginWidget( UiWindow& window )
	{
		m_owner = true;
		m_widget = imuiWidgetBegin( window.getInternal() );
	}

	void UiWidget::beginWidget( UiWindow& window, ImuiId id )
	{
		m_owner = true;
		m_widget = imuiWidgetBeginId( window.getInternal(), id );
	}

	void UiWidget::beginWidget( UiWindow& window, const char* name )
	{
		m_owner = true;
		m_widget = imuiWidgetBeginNamed( window.getInternal(), name );
	}

	void UiWidget::beginWidget( ImuiWidget* widget )
	{
		m_owner = false;
		m_widget = widget;
	}

	void UiWidget::endWidget()
	{
		if( m_widget && m_owner )
		{
			imuiWidgetEnd( m_widget );
		}

		m_owner = false;
		m_widget = nullptr;
	}

	bool UiWidget::isValid() const
	{
		return m_widget != nullptr;
	}

	ImuiWidget* UiWidget::getInternal() const
	{
		return m_widget;
	}

	UiContext UiWidget::getContext() const
	{
		return UiContext( imuiWidgetGetContext( m_widget ) );
	}

	UiSurface UiWidget::getSurface() const
	{
		return UiSurface( imuiWidgetGetSurface( m_widget ) );
	}

	UiWindow UiWidget::getWindow() const
	{
		return UiWindow( imuiWidgetGetWindow( m_widget ) );
	}

	UiInputState UiWidget::getInput() const
	{
		return UiInputState( imuiWidgetGetInput( m_widget ) );
	}

	void* UiWidget::allocState( size_t size, ImuiId stateId )
	{
		return imuiWidgetAllocState( m_widget, size, stateId );
	}

	void* UiWidget::allocState( size_t size, ImuiId stateId, bool& isNew )
	{
		return imuiWidgetAllocStateNew( m_widget, size, stateId, &isNew );
	}

	ImuiLayout UiWidget::getLayout() const
	{
		return imuiWidgetGetLayout( m_widget );
	}

	void UiWidget::setLayoutStack()
	{
		imuiWidgetSetLayoutStack( m_widget );
	}

	void UiWidget::setLayoutScroll( float offsetX, float offsetY )
	{
		imuiWidgetSetLayoutScroll( m_widget, offsetX, offsetY );
	}

	void UiWidget::setLayoutHorizontal( float spacing /* = 0.0f */ )
	{
		imuiWidgetSetLayoutHorizontalSpacing( m_widget, spacing );
	}

	void UiWidget::setLayoutVertical( float spacing /* = 0.0f */ )
	{
		imuiWidgetSetLayoutVerticalSpacing( m_widget, spacing );
	}

	void UiWidget::setLayoutGrid( uint32_t columnCount, float colSpacing /* = 0.0f */, float rowSpacing /* = 0.0f */ )
	{
		imuiWidgetSetLayoutGrid( m_widget, columnCount, colSpacing, rowSpacing );
	}

	double UiWidget::getTime()
	{
		return imuiWidgetGetTime( m_widget );
	}

	UiBorder UiWidget::getMargin() const
	{
		return (const UiBorder&)m_widget->margin;
	}

	void UiWidget::setMargin( const ImuiBorder& margin )
	{
		imuiWidgetSetMargin( m_widget, margin );
	}

	UiBorder UiWidget::getPadding() const
	{
		return (const UiBorder&)m_widget->padding;
	}

	void UiWidget::setPadding( const ImuiBorder& padding )
	{
		imuiWidgetSetPadding( m_widget, padding );
	}

	UiSize UiWidget::getMinSize() const
	{
		return (const UiSize&)m_widget->minSize;
	}

	void UiWidget::setMinWidth( float value )
	{
		imuiWidgetSetMinWidth( m_widget, value );
	}

	void UiWidget::setMinHeight( float value )
	{
		imuiWidgetSetMinHeight( m_widget, value );
	}

	void UiWidget::setMinSize( UiSize size )
	{
		imuiWidgetSetMinSize( m_widget, size );
	}

	UiSize UiWidget::getMaxSize() const
	{
		return (const UiSize&)m_widget->maxSize;
	}

	void UiWidget::setMaxWidth( float value )
	{
		imuiWidgetSetMaxWidth( m_widget, value );
	}

	void UiWidget::setMaxHeight( float value )
	{
		imuiWidgetSetMaxHeight( m_widget, value );
	}

	void UiWidget::setMaxSize( UiSize size )
	{
		imuiWidgetSetMaxSize( m_widget, size );
	}

	void UiWidget::setFixedWidth( float value )
	{
		imuiWidgetSetFixedWidth( m_widget, value );
	}

	void UiWidget::setFixedHeight( float value )
	{
		imuiWidgetSetFixedHeight( m_widget, value );
	}

	void UiWidget::setFixedSize( UiSize size )
	{
		imuiWidgetSetFixedSize( m_widget, size );
	}

	void UiWidget::setStretch( float horizontal, float vertical )
	{
		imuiWidgetSetStretch( m_widget, horizontal, vertical );
	}

	void UiWidget::setStretchOne()
	{
		imuiWidgetSetStretchOne( m_widget );
	}

	float UiWidget::getHStretch() const
	{
		return imuiWidgetGetHStretch( m_widget );
	}

	void UiWidget::setHStretch( float stretch )
	{
		imuiWidgetSetHStretch( m_widget, stretch );
	}

	float UiWidget::getVStretch() const
	{
		return imuiWidgetGetVStretch( m_widget );
	}

	void UiWidget::setVStretch( float stretch )
	{
		imuiWidgetSetVStretch( m_widget, stretch );
	}

	void UiWidget::setAlign( float horizontal, float vertical )
	{
		imuiWidgetSetAlign( m_widget, horizontal, vertical );
	}

	float UiWidget::getHAlign() const
	{
		return imuiWidgetGetHAlign( m_widget );
	}

	void UiWidget::setHAlign( float align )
	{
		imuiWidgetSetHAlign( m_widget, align );
	}

	float UiWidget::getVAlign() const
	{
		return imuiWidgetGetVAlign( m_widget );
	}

	void UiWidget::setVAlign( float align )
	{
		imuiWidgetSetVAlign( m_widget, align );
	}

	UiPos UiWidget::getPos() const
	{
		return (const UiPos&)m_widget->rect.pos;
	}

	UiSize UiWidget::getSize() const
	{
		return (const UiSize&)m_widget->rect.size;
	}

	UiRect UiWidget::getRect() const
	{
		return (const UiRect&)m_widget->rect;
	}

	void UiWidget::getInputState( ImuiWidgetInputState& inputState ) const
	{
		imuiWidgetGetInputState( m_widget, &inputState );
	}

	void UiWidget::drawLine( ImuiPos p0, ImuiPos p1, ImuiColor color )
	{
		imuiWidgetDrawLine( m_widget, p0, p1, color );
	}

	void UiWidget::drawTriangle( ImuiPos p0, ImuiPos p1, ImuiPos p2, ImuiColor color )
	{
		imuiWidgetDrawTriangle( m_widget, p0, p1, p2, color );
	}

	void UiWidget::drawColor( ImuiColor color )
	{
		imuiWidgetDrawColor( m_widget, color );
	}

	void UiWidget::drawImage( const ImuiImage& image )
	{
		imuiWidgetDrawImage( m_widget, &image );
	}

	void UiWidget::drawImage( const ImuiImage& image, ImuiColor color )
	{
		imuiWidgetDrawImageColor( m_widget, &image, color );
	}

	void UiWidget::drawSkin( const ImuiSkin& skin, ImuiColor color )
	{
		imuiWidgetDrawSkin( m_widget, &skin, color );
	}

	void UiWidget::drawText( ImuiTextLayout* layout, ImuiColor color )
	{
		imuiWidgetDrawText( m_widget, layout, color );
	}

	void UiWidget::drawPartialColor( const ImuiRect& rect, ImuiColor color )
	{
		imuiWidgetDrawPartialColor( m_widget, rect, color );
	}

	void UiWidget::drawPartialImage( const ImuiRect& rect, const ImuiImage& image )
	{
		imuiWidgetDrawPartialImage( m_widget, rect, &image );
	}

	void UiWidget::drawPartialImage( const ImuiRect& rect, const ImuiImage& image, ImuiColor color )
	{
		imuiWidgetDrawPartialImageColor( m_widget, rect, &image, color );
	}

	void UiWidget::drawPartialSkin( const ImuiRect& rect, const ImuiSkin& skin, ImuiColor color )
	{
		imuiWidgetDrawPartialSkin( m_widget, rect, &skin, color );
	}

	void UiWidget::drawPositionText( ImuiPos pos, ImuiTextLayout* layout, ImuiColor color )
	{
		imuiWidgetDrawPositionText( m_widget, pos, layout, color );
	}

	UiWidgetLayoutHorizontal::UiWidgetLayoutHorizontal( UiWindow& window, float spacing /* = 0.0f */ )
		: UiWidget( window )
	{
		setLayoutHorizontal( spacing );
	}

	UiWidgetLayoutHorizontal::UiWidgetLayoutHorizontal( UiWindow& window, ImuiId id, float spacing /*= 0.0f */ )
		: UiWidget( window, id ) 
	{
		setLayoutHorizontal( spacing );
	}

	UiWidgetLayoutHorizontal::UiWidgetLayoutHorizontal( UiWindow& window, const char* name, float spacing /*= 0.0f */ )
		: UiWidget( window, name )
	{
		setLayoutHorizontal( spacing );
	}

	UiWidgetLayoutVertical::UiWidgetLayoutVertical( UiWindow& window, float spacing /* = 0.0f */ )
		: UiWidget( window )
	{
		setLayoutVertical( spacing );
	}

	UiWidgetLayoutVertical::UiWidgetLayoutVertical( UiWindow& window, ImuiId id, float spacing /*= 0.0f */ )
		: UiWidget( window, id )
	{
		setLayoutVertical( spacing );
	}

	UiWidgetLayoutVertical::UiWidgetLayoutVertical( UiWindow& window, const char* name, float spacing /*= 0.0f */ )
		: UiWidget( window, name )
	{
		setLayoutVertical( spacing );
	}

	UiWidgetLayoutGrid::UiWidgetLayoutGrid( UiWindow& window, uint32_t columnCount, float colSpacing /*= 0.0f*/, float rowSpacing /*= 0.0f */ )
		: UiWidget( window )
	{
		setLayoutGrid( columnCount, colSpacing, rowSpacing );
	}

	UiWidgetLayoutGrid::UiWidgetLayoutGrid( UiWindow& window, ImuiId id, uint32_t columnCount, float colSpacing /*= 0.0f*/, float rowSpacing /*= 0.0f */ )
		: UiWidget( window, id )
	{
		setLayoutGrid( columnCount, colSpacing, rowSpacing );
	}

	UiWidgetLayoutGrid::UiWidgetLayoutGrid( UiWindow& window, const char* name, uint32_t columnCount, float colSpacing /*= 0.0f*/, float rowSpacing /*= 0.0f */ )
		: UiWidget( window, name )
	{
		setLayoutGrid( columnCount, colSpacing, rowSpacing );
	}

	toolbox::UiToolboxTheme::UiToolboxTheme()
	{
	}

	toolbox::UiToolboxTheme::UiToolboxTheme( ImuiFont* inFont )
	{
		setDefault( inFont );
	}

	void toolbox::UiToolboxTheme::setDefault( ImuiFont* inFont )
	{
		imuiToolboxThemeFillDefault( this, inFont );
	}

	void toolbox::UiToolboxTheme::applyConfig()
	{
		imuiToolboxThemeSet( this );
	}

	const UiColor& toolbox::UiToolboxTheme::getColor( ImuiToolboxColor color )
	{
		return (const UiColor&)imuiToolboxThemeGet()->colors[ color ];
	}

	const ImuiSkin& toolbox::UiToolboxTheme::getSkin( ImuiToolboxSkin skin )
	{
		return imuiToolboxThemeGet()->skins[ skin ];
	}

	const ImuiImage& toolbox::UiToolboxTheme::getIcon( ImuiToolboxIcon icon )
	{
		return imuiToolboxThemeGet()->icons[ icon ];
	}

	ImuiToolboxTheme& toolbox::UiToolboxTheme::getTheme()
	{
		return *imuiToolboxThemeGet();
	}

	toolbox::UiToolboxConfigFloatScope::UiToolboxConfigFloatScope( float& value, float newValue, bool active /* = true */ )
		: m_value( value )
	{
		IMUI_ASSERT( (void*)&value >= imuiToolboxThemeGet() && (void*)&value < imuiToolboxThemeGet() + 1u );

		m_oldValue = m_value;
		m_value = active ? newValue : m_oldValue;
	}

	toolbox::UiToolboxConfigFloatScope::~UiToolboxConfigFloatScope()
	{
		m_value = m_oldValue;
	}

	toolbox::UiToolboxConfigColorScope::UiToolboxConfigColorScope( ImuiToolboxColor color, const ImuiColor& newValue, bool active /* = true */ )
		: m_color( color )
	{
		ImuiColor& valueRef = imuiToolboxThemeGet()->colors[ m_color ];
		m_oldValue = valueRef;
		valueRef = active ? newValue : m_oldValue;
	}

	toolbox::UiToolboxConfigColorScope::~UiToolboxConfigColorScope()
	{
		imuiToolboxThemeGet()->colors[ m_color ] = m_oldValue;
	}

	toolbox::UiToolboxConfigSkinScope::UiToolboxConfigSkinScope( ImuiToolboxSkin skin, const ImuiSkin& newValue, bool active /* = true */ )
		: m_skin( skin )
	{
		ImuiSkin& valueRef = imuiToolboxThemeGet()->skins[ m_skin ];
		m_oldValue = valueRef;
		valueRef = active ? newValue : m_oldValue;
	}

	toolbox::UiToolboxConfigSkinScope::~UiToolboxConfigSkinScope()
	{
		imuiToolboxThemeGet()->skins[ m_skin ] = m_oldValue;
	}

	toolbox::UiToolboxConfigIconScope::UiToolboxConfigIconScope( ImuiToolboxIcon icon, const ImuiImage& newValue, bool active /* = true */ )
		: m_icon( icon )
	{
		ImuiImage& valueRef = imuiToolboxThemeGet()->icons[ m_icon ];
		m_oldValue = valueRef;
		valueRef = active ? newValue : m_oldValue;
	}

	toolbox::UiToolboxConfigIconScope::~UiToolboxConfigIconScope()
	{
		imuiToolboxThemeGet()->icons[ m_icon ] = m_oldValue;
	}

	toolbox::UiToolboxConfigBorderScope::UiToolboxConfigBorderScope( ImuiBorder& value, UiBorder newValue, bool active /* = true */ )
		: m_value( value )
	{
		IMUI_ASSERT( (void*)&value >= imuiToolboxThemeGet() && (void*)&value < imuiToolboxThemeGet() + 1u );

		m_oldValue = m_value;
		m_value = active ? newValue : m_oldValue;
	}

	toolbox::UiToolboxConfigBorderScope::~UiToolboxConfigBorderScope()
	{
		m_value = m_oldValue;
	}

	toolbox::UiToolboxWindow::UiToolboxWindow()
	{
	}

	toolbox::UiToolboxWindow::UiToolboxWindow( UiWindow& window )
	{
		m_window	= window.getInternal();
		m_owner		= false;
	}

	toolbox::UiToolboxWindow::UiToolboxWindow( ImuiWindow* window )
		: UiWindow( window )
	{
	}

	toolbox::UiToolboxWindow::UiToolboxWindow( UiSurface& surface, const char* name, const ImuiRect& rect, uint32_t zOrder )
		: UiWindow( surface, name, rect, zOrder )
	{
	}

	toolbox::UiToolboxWindow::UiToolboxWindow( ImuiSurface* surface, const char* name, const ImuiRect& rect, uint32_t zOrder )
		: UiWindow( surface, name, rect, zOrder )
	{
	}

	void toolbox::UiToolboxWindow::spacer( float width, float height )
	{
		imuiToolboxSpacer( m_window, width, height );
	}

	void toolbox::UiToolboxWindow::strecher( float horizontal, float vertical )
	{
		imuiToolboxStrecher( m_window, horizontal, vertical );
	}

	bool toolbox::UiToolboxWindow::buttonLabel( const char* text )
	{
		return imuiToolboxButtonLabel( m_window, text );
	}

	bool toolbox::UiToolboxWindow::buttonLabelFormat( const char* format, ... )
	{
		va_list args;
		va_start( args, format );
		const bool result = imuiToolboxButtonLabelFormatArgs( m_window, format, args );
		va_end( args );

		return result;
	}

	bool toolbox::UiToolboxWindow::buttonIcon( const ImuiImage& icon )
	{
		return imuiToolboxButtonIcon( m_window, &icon );
	}

	bool toolbox::UiToolboxWindow::buttonIcon( const ImuiImage& icon, ImuiSize iconSize )
	{
		return imuiToolboxButtonIconSize( m_window, &icon, iconSize );
	}

	bool toolbox::UiToolboxWindow::checkBox( bool& checked, const char* text )
	{
		return imuiToolboxCheckBox( m_window, &checked, text );
	}

	bool toolbox::UiToolboxWindow::checkBoxState( const char* text, bool defaultValue /*= false */ )
	{
		return imuiToolboxCheckBoxStateDefault( m_window, text, defaultValue );
	}

	void toolbox::UiToolboxWindow::label( const char* text )
	{
		imuiToolboxLabel( m_window, text );
	}

	void toolbox::UiToolboxWindow::label( const char* text, size_t length )
	{
		imuiToolboxLabelLength( m_window, text, length );
	}

	void toolbox::UiToolboxWindow::labelFormat( const char* format, ... )
	{
		va_list args;
		va_start( args, format );
		imuiToolboxLabelFormatArgs( m_window, format, args);
		va_end( args );
	}

	void toolbox::UiToolboxWindow::image( const ImuiImage& img )
	{
		image( img, UiSize( img ) );
	}

	void toolbox::UiToolboxWindow::image( const ImuiImage& img, const ImuiSize& size )
	{
		imuiToolboxImageSize( m_window, &img, size );
	}

	bool toolbox::UiToolboxWindow::slider( float& value, float min /*= 0.0f*/, float max /*= 1.0f*/ )
	{
		return imuiToolboxSliderMinMax( m_window, &value, min, max );
	}

	float toolbox::UiToolboxWindow::sliderState( float min /*= 0.0f*/, float max /*= 1.0f */ )
	{
		return imuiToolboxSliderStateMinMax( m_window, min, max );
	}

	float toolbox::UiToolboxWindow::sliderState( float min, float max, float defaultValue )
	{
		return imuiToolboxSliderStateMinMaxDefault( m_window, min, max, defaultValue );
	}

	bool toolbox::UiToolboxWindow::textEdit( char* buffer, size_t bufferSize, size_t* textLength /* = nullptr */ )
	{
		return imuiToolboxTextEdit( m_window, buffer, bufferSize, textLength );
	}

	const char* toolbox::UiToolboxWindow::textEditState( size_t bufferSize, const char* defaultValue /* = NULL */ )
	{
		return imuiToolboxTextEditStateBufferDefault( m_window, bufferSize, defaultValue );
	}

	void toolbox::UiToolboxWindow::progressBar( float value, float min /*= 0.0f*/, float max /*= 1.0f */ )
	{
		imuiToolboxProgressBarMinMax( m_window, value, min, max );
	}

	size_t toolbox::UiToolboxWindow::dropDown( const char* const* items, size_t itemCount, size_t itemStride /* = sizeof( const char* ) */ )
	{
		return imuiToolboxDropDown( m_window, (const char**)items, itemCount, itemStride );
	}

	toolbox::UiToolboxButton::UiToolboxButton()
	{
	}

	toolbox::UiToolboxButton::UiToolboxButton( UiWindow& window )
	{
		begin( window );
	}

	toolbox::UiToolboxButton::~UiToolboxButton()
	{
		end();
	}

	void toolbox::UiToolboxButton::begin( UiWindow& window )
	{
		m_widget = imuiToolboxButtonBegin( window.getInternal() );
	}

	bool toolbox::UiToolboxButton::end()
	{
		if( m_widget )
		{
			const bool result = imuiToolboxButtonEnd( m_widget );
			m_widget = nullptr;

			return result;
		}

		return false;
	}

	toolbox::UiToolboxButtonLabel::UiToolboxButtonLabel()
	{
	}

	toolbox::UiToolboxButtonLabel::UiToolboxButtonLabel( UiWindow& window, const char* text )
	{
		begin( window, text );
	}

	toolbox::UiToolboxButtonLabel::~UiToolboxButtonLabel()
	{
		end();
	}

	void toolbox::UiToolboxButtonLabel::begin( UiWindow& window, const char* text )
	{
		m_widget = imuiToolboxButtonLabelBegin( window.getInternal(), text );
	}

	void toolbox::UiToolboxButtonLabel::beginFormat( UiWindow& window, const char* format, ... )
	{
		va_list args;
		va_start( args, format );
		m_widget = imuiToolboxLabelBeginFormatArgs( window.getInternal(), format, args );
		va_end( args );
	}

	bool toolbox::UiToolboxButtonLabel::end()
	{
		if( m_widget )
		{
			const bool result = imuiToolboxButtonEnd( m_widget );
			m_widget = nullptr;

			return result;
		}

		return false;
	}

	toolbox::UiToolboxLabel::UiToolboxLabel()
	{
	}

	toolbox::UiToolboxLabel::UiToolboxLabel( UiWindow& window, const char* text )
	{
		begin( window, text );
	}

	toolbox::UiToolboxLabel::UiToolboxLabel( UiWindow& window, const char* text, size_t length )
	{
		begin( window, text, length );
	}

	toolbox::UiToolboxLabel::~UiToolboxLabel()
	{
		end();
	}

	void toolbox::UiToolboxLabel::begin( UiWindow& window, const char* text )
	{
		m_widget = imuiToolboxLabelBegin( window.getInternal(), text );
	}

	void toolbox::UiToolboxLabel::begin( UiWindow& window, const char* text, size_t length )
	{
		m_widget = imuiToolboxLabelBeginLength( window.getInternal(), text, length );
	}

	void toolbox::UiToolboxLabel::beginFormat( UiWindow& window, const char* format, ... )
	{
		va_list args;
		va_start( args, format );
		m_widget = imuiToolboxLabelBeginFormatArgs( window.getInternal(), format, args );
		va_end( args );
	}

	void toolbox::UiToolboxLabel::end()
	{
		if( m_widget )
		{
			imuiToolboxLabelEnd( m_widget );
			m_widget = nullptr;
		}
	}

	toolbox::UiToolboxSlider::UiToolboxSlider()
	{
	}

	toolbox::UiToolboxSlider::UiToolboxSlider( UiWindow& window, float& value, float min /*= 0.0f*/, float max /*= 1.0f */ )
	{
		begin( window, value, min, max );
	}

	toolbox::UiToolboxSlider::~UiToolboxSlider()
	{
		end();
	}

	void toolbox::UiToolboxSlider::begin( UiWindow& window, float& value, float min /*= 0.0f*/, float max /*= 1.0f */ )
	{
		m_value		= &value;
		m_min		= min;
		m_max		= max;

		m_widget	= imuiToolboxSliderBegin( window.getInternal() );
	}

	bool toolbox::UiToolboxSlider::end()
	{
		bool changed = false;
		if( m_widget )
		{
			changed = imuiToolboxSliderEnd( m_widget, m_value, m_min, m_max );
			m_widget = nullptr;
		}

		return changed;
	}

	toolbox::UiToolboxTextEdit::UiToolboxTextEdit( UiWindow& window )
		: m_buffer( nullptr )
		, m_bufferSize( 0u )
	{
		m_widget = imuiToolboxTextEditBegin( window.getInternal() );
	}

	toolbox::UiToolboxTextEdit::UiToolboxTextEdit( UiWindow& window, size_t bufferSize )
	{
		m_widget		= imuiToolboxTextEditBegin( window.getInternal() );
		m_buffer		= (char*)imuiWidgetAllocState( m_widget, bufferSize, IMUI_ID_STR( "text buffer" ) );
		m_bufferSize	= bufferSize;
	}

	toolbox::UiToolboxTextEdit::UiToolboxTextEdit( UiWindow& window, char* buffer, size_t bufferSize )
		: m_buffer( buffer )
		, m_bufferSize( bufferSize )
	{
		m_widget = imuiToolboxTextEditBegin( window.getInternal() );
	}

	toolbox::UiToolboxTextEdit::~UiToolboxTextEdit()
	{
		end();
	}

	bool toolbox::UiToolboxTextEdit::end( size_t* textLength /* = nullptr */ )
	{
		if( !m_widget )
		{
			return false;
		}

		const bool changed = imuiToolboxTextEditEnd( m_widget, m_buffer, m_bufferSize, textLength );
		m_widget = nullptr;
		return changed;
	}

	void toolbox::UiToolboxTextEdit::setBuffer( char* buffer, size_t bufferSize )
	{
		m_buffer		= buffer;
		m_bufferSize	= bufferSize;
	}

	toolbox::UiToolboxScrollArea::UiToolboxScrollArea( UiWindow& window )
	{
		imuiToolboxScrollAreaBegin( &m_scrollArea, window.getInternal() );
		m_widget = m_scrollArea.area;
	}

	toolbox::UiToolboxScrollArea::~UiToolboxScrollArea()
	{
		if( m_widget )
		{
			imuiToolboxScrollAreaEnd( &m_scrollArea );
			m_widget = nullptr;
		}
	}

	void toolbox::UiToolboxScrollArea::enableSpacing( bool horizontal, bool vertical )
	{
		imuiToolboxScrollAreaEnableSpacing( &m_scrollArea, horizontal, vertical );
	}

	toolbox::UiToolboxList::UiToolboxList( UiWindow& window, float itemSize, size_t itemCount, bool selection )
	{
		imuiToolboxListBegin( &m_list, window.getInternal(), itemSize, itemCount, selection );
		m_widget = m_list.list;
	}

	toolbox::UiToolboxList::~UiToolboxList()
	{
		end();
	}

	bool toolbox::UiToolboxList::end()
	{
		const bool changed = imuiToolboxListEnd( &m_list );
		endWidget();

		return changed;
	}

	size_t toolbox::UiToolboxList::getBeginIndex() const
	{
		return imuiToolboxListGetBeginIndex( &m_list );
	}

	size_t toolbox::UiToolboxList::getEndIndex() const
	{
		return imuiToolboxListGetEndIndex( &m_list );
	}

	size_t toolbox::UiToolboxList::getSelectedIndex() const
	{
		return imuiToolboxListGetSelectedIndex( &m_list );
	}

	void toolbox::UiToolboxList::setSelectedIndex( size_t index )
	{
		imuiToolboxListSetSelectedIndex( &m_list, index );
	}

	void toolbox::UiToolboxList::nextItem( UiWidget* widget /* = nullptr */, ImuiId id /* = IMUI_ID_DEFAULT */ )
	{
		ImuiWidget* itemWidget = imuiToolboxListNextItemId( &m_list, id );
		if( widget )
		{
			widget->beginWidget( itemWidget );
		}
	}

	toolbox::UiToolboxDropdown::UiToolboxDropdown( UiWindow& window, const char* const* items, size_t itemCount, size_t itemStride /* = sizeof( const char* ) */ )
	{
		imuiToolboxDropDownBegin( &m_dropDown, window.getInternal(), (const char**)items, itemCount, itemStride );
		m_widget = m_dropDown.dropDown;
	}

	toolbox::UiToolboxDropdown::~UiToolboxDropdown()
	{
		imuiToolboxDropDownEnd( &m_dropDown );
		m_widget = nullptr;
	}

	size_t toolbox::UiToolboxDropdown::getSelectedIndex() const
	{
		return imuiToolboxDropDownGetSelectedIndex( &m_dropDown );
	}

	void toolbox::UiToolboxDropdown::setSelectedIndex( size_t index )
	{
		imuiToolboxDropDownSetSelectedIndex( &m_dropDown, index );
	}

	toolbox::UiToolboxPopup::UiToolboxPopup( UiSurface& surface )
	{
		m_window = imuiToolboxPopupBeginSurface( surface.getInternal() );
		m_owner = true;
	}

	toolbox::UiToolboxPopup::UiToolboxPopup( UiWindow& window )
	{
		m_window = imuiToolboxPopupBegin( window.getInternal() );
		m_owner = true;
	}

	toolbox::UiToolboxPopup::~UiToolboxPopup()
	{
		end();
	}

	size_t toolbox::UiToolboxPopup::end( const char** buttons, size_t buttonCount )
	{
		const size_t result = imuiToolboxPopupEndButtons( m_window, buttons, buttonCount );
		m_window = nullptr;

		return result;
	}

	void toolbox::UiToolboxPopup::end()
	{
		if( m_window )
		{
			imuiToolboxPopupEnd( m_window );
			m_window = nullptr;
		}
	}

	toolbox::UiToolboxTabView::UiToolboxTabView()
	{
	}

	toolbox::UiToolboxTabView::UiToolboxTabView( UiWindow& window )
	{
		m_widget = imuiToolboxTabViewBegin( &m_context, window.getInternal() );
	}

	toolbox::UiToolboxTabView::~UiToolboxTabView()
	{
		imuiToolboxTabViewEnd( &m_context );
		m_widget = nullptr;
	}

	size_t toolbox::UiToolboxTabView::getSelectedIndex() const
	{
		return imuiToolboxTabViewGetSelectedIndex( &m_context );
	}

	void toolbox::UiToolboxTabView::setSelectedIndex( size_t index )
	{
		imuiToolboxTabViewSetSelectedIndex( &m_context, index );
	}

	bool toolbox::UiToolboxTabView::header( const char* text )
	{
		return imuiToolboxTabViewHeader( &m_context, text );
	}

	void toolbox::UiToolboxTabView::beginHeader( UiWidget& header )
	{
		header.beginWidget( imuiToolboxTabViewHeaderBegin( &m_context ) );
	}

	bool toolbox::UiToolboxTabView::endHeader( UiWidget& header )
	{
		const bool selected = imuiToolboxTabViewHeaderEnd( &m_context, header.getInternal() );
		header.endWidget();

		return selected;
	}

	void toolbox::UiToolboxTabView::beginBody( UiWidget& body )
	{
		body.beginWidget( imuiToolboxTabViewBodyBegin( &m_context ) );
	}

	void toolbox::UiToolboxTabView::endBody( UiWidget& body )
	{
		imuiToolboxTabViewBodyEnd( &m_context );
		body.endWidget();
	}
}
