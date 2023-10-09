#include "imui/imui_cpp.h"

#include "imui_internal.h"

#include <cstdarg>
#include <cstring>

namespace imui
{
	UiStringView::UiStringView()
	{
		data	= nullptr;
		length	= 0u;
	}

	UiStringView::UiStringView( const ImUiStringView& value )
	{
		data	= value.data;
		length	= value.length;
	}

	UiStringView::UiStringView( const char* str, size_t _length )
	{
		data	= str;
		length	= _length;
	}

	const char* UiStringView::getData() const
	{
		return data;
	}

	size_t UiStringView::getLength() const
	{
		return length;
	}

	bool UiStringView::isEmpty() const
	{
		return length == 0u;
	}

	bool UiStringView::operator!=( const UiStringView& rhs ) const
	{
		return ImUiStringViewIsEquals( *this, rhs );
	}

	bool UiStringView::operator==( const UiStringView& rhs ) const
	{
		return !ImUiStringViewIsEquals( *this, rhs );
	}

	UiAlign::UiAlign( float hAlign, float vAlign )
	{
		horizontal	= hAlign;
		vertical	= vAlign;
	}

	UiSize UiSize::Zero			= UiSize();
	UiSize UiSize::One			= UiSize( 1.0f );
	UiSize UiSize::Horizontal	= UiSize( 1.0f, 0.0f );
	UiSize UiSize::Vertical		= UiSize( 0.0f, 1.0f );

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

	UiSize::UiSize( const ImUiSize& value )
	{
		width	= value.width;
		height	= value.height;
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

	UiBorder::UiBorder( const ImUiBorder& value )
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

	UiPos::UiPos( const ImUiPos& value )
	{
		x = value.x;
		y = value.y;
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

	UiRect::UiRect( const ImUiRect& value )
	{
		pos		= UiPos( value.pos );
		size	= UiSize( value.size );
	}

	UiRect UiRect::shrinkBorder( const ImUiBorder& border ) const
	{
		return (UiRect&)ImUiRectShrinkBorder( *this, border );
	}

	bool UiRect::includesPos( ImUiPos _pos ) const
	{
		return ImUiRectIncludesPos( *this, _pos );
	}

	bool UiRect::intersectsRect( const ImUiRect& rect2 ) const
	{
		return ImUiRectIntersectsRect( *this, rect2 );
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
		return (UiPos&)pos;
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

	UiColor UiColor::White				= UiColor( ImUiColorCreateWhite() );
	UiColor UiColor::Black				= UiColor( ImUiColorCreateBlack() );
	UiColor UiColor::TransparentBlack	= UiColor();

	UiColor::UiColor()
		: ImUiColor( ImUiColorCreateTransparentBlack() )
	{
	}

	UiColor::UiColor( uint8_t _red, uint8_t _green, uint8_t _blue )
		: ImUiColor( ImUiColorCreate( _red, _green, _blue, 0xffu ) )
	{
	}

	UiColor::UiColor( uint8_t _red, uint8_t _green, uint8_t _blue, uint8_t _alpha )
		: ImUiColor( ImUiColorCreate( _red, _green, _blue, _alpha ) )
	{
	}

	UiColor::UiColor( float _red, float _green, float _blue )
		: ImUiColor( ImUiColorCreateFloat( _red, _green, _blue, 1.0f ) )
	{
	}

	UiColor::UiColor( float _red, float _green, float _blue, float _alpha )
		: ImUiColor( ImUiColorCreateFloat( _red, _green, _blue, _alpha ) )
	{
	}

	UiColor::UiColor( ImUiColor value )
		: ImUiColor( value )
	{
	}

	UiColor UiColor::CreateWhite( uint8_t _alpha )
	{
		return UiColor( ImUiColorCreateWhiteA( _alpha ) );
	}

	UiColor UiColor::CreateBlack( uint8_t _alpha )
	{
		return UiColor( ImUiColorCreateBlackA( _alpha ) );
	}

	UiColor UiColor::CreateGray( uint8_t gray )
	{
		return UiColor( ImUiColorCreateGray( gray ) );
	}

	UiColor UiColor::CreateGray( uint8_t gray, uint8_t _alpha )
	{
		return UiColor( ImUiColorCreateGrayA( gray, _alpha ) );
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

	UiContext::UiContext( ImUiContext* context )
		: m_owner( false )
		, m_context( context )
	{
	}

	UiContext::UiContext( const UiContextParameters& parameters )
	{
		m_owner = true;
		m_context = ImUiCreate( &parameters );
	}

	UiContext::~UiContext()
	{
		if( m_owner && m_context )
		{
			ImUiDestroy( m_context );
			m_context = nullptr;
			m_owner = false;
		}
	}

	bool UiContext::isValid() const
	{
		return m_context != nullptr;
	}

	ImUiContext* UiContext::getInternal() const
	{
		return m_context;
	}

	UiInput& UiContext::beginInput()
	{
		ImUiInput* input = ImUiInputBegin( m_context );
		return *(UiInput*)input;
	}

	void UiContext::endInput()
	{
		ImUiInputEnd( m_context );
	}

	UiInputState UiContext::getInput() const
	{
		return UiInputState( m_context );
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

	UiInputState::UiInputState( const ImUiContext* imui )
		: m_context( imui )
	{
	}

	uint32_t UiInputState::getKeyModifiers() const
	{
		return ImUiInputGetKeyModifiers( m_context );
	}

	bool UiInputState::isKeyDown( ImUiInputKey key ) const
	{
		return ImUiInputIsKeyDown( m_context, key );
	}

	bool UiInputState::isKeyUp( ImUiInputKey key ) const
	{
		return ImUiInputIsKeyUp( m_context, key );
	}

	bool UiInputState::hasKeyPressed( ImUiInputKey key ) const
	{
		return ImUiInputHasKeyPressed( m_context, key );
	}

	bool UiInputState::hasKeyReleased( ImUiInputKey key ) const
	{
		return ImUiInputHasKeyReleased( m_context, key );
	}

	UiStringView UiInputState::getText() const
	{
		return (UiStringView)ImUiInputGetText( m_context );
	}

	UiPos UiInputState::getMousePos() const
	{
		return (UiPos)ImUiInputGetMousePos( m_context );
	}

	bool UiInputState::isMouseInRect( UiRect rect ) const
	{
		return ImUiInputIsMouseInRect( m_context, rect );
	}

	bool UiInputState::isMouseButtonDown( ImUiInputMouseButton button ) const
	{
		return ImUiInputIsMouseButtonDown( m_context, button );
	}

	bool UiInputState::isMouseButtonUp( ImUiInputMouseButton button ) const
	{
		return ImUiInputIsMouseButtonUp( m_context, button );
	}

	bool UiInputState::hasMouseButtonPressed( ImUiInputMouseButton button ) const
	{
		return ImUiInputHasMouseButtonReleased( m_context, button );
	}

	bool UiInputState::hasMouseButtonReleased( ImUiInputMouseButton button ) const
	{
		return ImUiInputHasMouseButtonReleased( m_context, button );
	}

	UiPos UiInputState::getMouseScrollDelta() const
	{
		return ImUiInputGetMouseScrollDelta( m_context );
	}

	UiFrame::UiFrame( ImUiFrame* frame )
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
		m_frame = ImUiBegin( context.getInternal(), timeInSeconds );
		m_owner = true;
	}

	void UiFrame::endFrame()
	{
		if( m_owner && m_frame )
		{
			ImUiEnd( m_frame );
			m_frame = nullptr;
			m_owner = false;
		}
	}

	bool UiFrame::isValid() const
	{
		return m_frame != nullptr;
	}

	ImUiFrame* UiFrame::getInternal() const
	{
		return m_frame;
	}

	UiSurface::UiSurface()
		: m_owner( false )
		, m_surface( nullptr )
	{
	}

	UiSurface::UiSurface( ImUiSurface* surface )
		: m_owner( false )
		, m_surface( surface )
	{
	}

	UiSurface::UiSurface( ImUiFrame* frame, const UiStringView& name, const UiSize& size, float dpiScale )
	{
		beginSurface( frame, name, size, dpiScale );
	}

	UiSurface::UiSurface( UiFrame& frame, const UiStringView& name, const UiSize& size, float dpiScale )
	{
		beginSurface( frame, name, size, dpiScale );
	}

	UiSurface::~UiSurface()
	{
		endSurface();
	}

	void UiSurface::beginSurface( ImUiFrame* frame, const UiStringView& name, const UiSize& size, float dpiScale )
	{
		m_surface = ImUiSurfaceBegin( frame, name, size, dpiScale );
		m_owner = true;
	}

	void UiSurface::beginSurface( UiFrame& frame, const UiStringView& name, const UiSize& size, float dpiScale )
	{
		beginSurface( frame.getInternal(), name, size, dpiScale );
	}

	void UiSurface::endSurface()
	{
		if( m_owner && m_surface )
		{
			ImUiSurfaceEnd( m_surface );
			m_surface = nullptr;
			m_owner = false;
		}
	}

	bool UiSurface::isValid() const
	{
		return m_surface != nullptr;
	}

	ImUiSurface* UiSurface::getInternal() const
	{
		return m_surface;
	}

	UiContext UiSurface::getContext() const
	{
		return UiContext( ImUiSurfaceGetContext( m_surface ) );
	}

	float UiSurface::getTime() const
	{
		return ImUiSurfaceGetTime( m_surface );
	}

	UiRect UiSurface::getRect() const
	{
		return UiRect( UiPos::Zero, UiSize( m_surface->size ) );
	}

	UiSize UiSurface::getSize() const
	{
		return (const UiSize&)m_surface->size;
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

	UiWindow::UiWindow( ImUiWindow* window )
		: m_owner( false )
		, m_window( window )
	{
	}

	UiWindow::UiWindow( ImUiSurface* surface, const UiStringView& name, const UiRect& rect, uint32_t zOrder )
	{
		beginWindow( surface, name, rect, zOrder );
	}

	UiWindow::UiWindow( UiSurface& surface, const UiStringView& name, const UiRect& rect, uint32_t zOrder )
		: m_window( nullptr )
	{
		beginWindow( surface, name, rect, zOrder );
	}

	UiWindow::~UiWindow()
	{
		endWindow();
	}

	void UiWindow::beginWindow( ImUiSurface* surface, const UiStringView& name, const UiRect& rect, uint32_t zOrder )
	{
		m_window = ImUiWindowBegin( surface, name, rect, zOrder );
		m_owner = true;
	}

	void UiWindow::beginWindow( UiSurface& surface, const UiStringView& name, const UiRect& rect, uint32_t zOrder )
	{
		beginWindow( surface.getInternal(), name, rect, zOrder );
	}

	void UiWindow::endWindow()
	{
		if( m_owner && m_window )
		{
			ImUiWindowEnd( m_window );
			m_window = nullptr;
			m_owner = false;
		}
	}

	bool UiWindow::isValid() const
	{
		return m_window != nullptr;
	}

	ImUiWindow* UiWindow::getInternal() const
	{
		return m_window;
	}

	UiContext UiWindow::getContext() const
	{
		return UiContext( ImUiWindowGetContext( m_window ) );
	}

	UiSurface UiWindow::getSurface() const
	{
		return UiSurface( ImUiWindowGetSurface( m_window ) );
	}

	float UiWindow::getTime() const
	{
		return m_window->frame->timeInSeconds;
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
		: m_widget( nullptr )
	{
	}

	UiWidget::UiWidget( UiWindow& window )
	{
		beginWidget( window );
	}

	UiWidget::UiWidget( UiWindow& window, ImUiId id )
	{
		beginWidget( window, id );
	}

	UiWidget::UiWidget( UiWindow& window, const UiStringView& name )
	{
		beginWidget( window, name );
	}

	UiWidget::~UiWidget()
	{
		endWidget();
	}

	void UiWidget::beginWidget( UiWindow& window )
	{
		m_widget = ImUiWidgetBegin( window.getInternal() );
	}

	void UiWidget::beginWidget( UiWindow& window, ImUiId id )
	{
		m_widget = ImUiWidgetBeginId( window.getInternal(), id );
	}

	void UiWidget::beginWidget( UiWindow& window, const UiStringView& name )
	{
		m_widget = ImUiWidgetBeginNamed( window.getInternal(), name );
	}

	void UiWidget::endWidget()
	{
		if( m_widget )
		{
			ImUiWidgetEnd( m_widget );
			m_widget = nullptr;
		}
	}

	bool UiWidget::isValid() const
	{
		return m_widget != nullptr;
	}

	ImUiWidget* UiWidget::getInternal() const
	{
		return m_widget;
	}

	UiContext UiWidget::getContext() const
	{
		return UiContext( ImUiWidgetGetContext( m_widget ) );
	}

	UiSurface UiWidget::getSurface() const
	{
		return UiSurface( ImUiWidgetGetSurface( m_widget ) );
	}

	UiWindow UiWidget::getWindow() const
	{
		return UiWindow( ImUiWidgetGetWindow( m_widget ) );
	}

	void* UiWidget::allocState( size_t size )
	{
		return ImUiWidgetAllocState( m_widget, size );
	}

	void* UiWidget::allocState( size_t size, bool& isNew )
	{
		return ImUiWidgetAllocStateNew( m_widget, size, &isNew );
	}

	ImUiLayout UiWidget::getLayout() const
	{
		return ImUiWidgetGetLayout( m_widget );
	}

	void UiWidget::setLayoutStack()
	{
		ImUiWidgetSetLayoutStack( m_widget );
	}

	void UiWidget::setLayoutScroll( UiPos offset )
	{
		ImUiWidgetSetLayoutScroll( m_widget, offset );
	}

	void UiWidget::setLayoutHorizontal()
	{
		ImUiWidgetSetLayoutHorizontal( m_widget );
	}

	void UiWidget::setLayoutHorizontalSpacing( float spacing )
	{
		ImUiWidgetSetLayoutHorizontalSpacing( m_widget, spacing );
	}

	void UiWidget::setLayoutVertical()
	{
		ImUiWidgetSetLayoutVertical( m_widget );
	}

	void UiWidget::setLayoutVerticalSpacing( float spacing )
	{
		ImUiWidgetSetLayoutVerticalSpacing( m_widget, spacing );
	}

	void UiWidget::setLayoutGrid( size_t columnCount )
	{
		ImUiWidgetSetLayoutGrid( m_widget, columnCount );
	}

	float UiWidget::getTime()
	{
		return m_widget->window->frame->timeInSeconds;
	}

	UiBorder UiWidget::getMargin() const
	{
		return (const UiBorder&)m_widget->margin;
	}

	void UiWidget::setMargin( const UiBorder& margin )
	{
		ImUiWidgetSetMargin( m_widget, margin );
	}

	UiBorder UiWidget::getPadding() const
	{
		return (const UiBorder&)m_widget->padding;
	}

	void UiWidget::setPadding( const UiBorder& padding )
	{
		ImUiWidgetSetPadding( m_widget, padding );
	}

	UiSize UiWidget::getMinSize() const
	{
		return (const UiSize&)m_widget->minSize;
	}

	void UiWidget::setMinWidth( float value )
	{
		ImUiWidgetSetMinWidth( m_widget, value );
	}

	void UiWidget::setMinHeight( float value )
	{
		ImUiWidgetSetMinHeight( m_widget, value );
	}

	void UiWidget::setMinSize( UiSize size )
	{
		ImUiWidgetSetMinSize( m_widget, size );
	}

	UiSize UiWidget::getMaxSize() const
	{
		return (const UiSize&)m_widget->maxSize;
	}

	void UiWidget::setMaxWidth( float value )
	{
		ImUiWidgetSetMaxWidth( m_widget, value );
	}

	void UiWidget::setMaxHeight( float value )
	{
		ImUiWidgetSetMaxHeight( m_widget, value );
	}

	void UiWidget::setMaxSize( UiSize size )
	{
		ImUiWidgetSetMaxSize( m_widget, size );
	}

	void UiWidget::setFixedWidth( float value )
	{
		ImUiWidgetSetFixedWidth( m_widget, value );
	}

	void UiWidget::setFixedHeight( float value )
	{
		ImUiWidgetSetFixedHeight( m_widget, value );
	}

	void UiWidget::setFixedSize( UiSize size )
	{
		ImUiWidgetSetFixedSize( m_widget, size );
	}

	UiSize UiWidget::getStretch() const
	{
		return (const UiSize&)m_widget->stretch;
	}

	void UiWidget::setStretch( UiSize stretch )
	{
		ImUiWidgetSetStretch( m_widget, stretch );
	}

	UiAlign UiWidget::getAlign() const
	{
		return (const UiAlign&)m_widget->align;
	}

	void UiWidget::setAlign( UiAlign align )
	{
		ImUiWidgetSetAlign( m_widget, align );
	}

	void UiWidget::setHAlign( float align )
	{
		ImUiWidgetSetHAlign( m_widget, align );
	}

	void UiWidget::setVAlign( float align )
	{
		ImUiWidgetSetVAlign( m_widget, align );
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

	//UiSize UiWidget::getInnerSize()
	//{
	//	return getRect().shrinkBorder( m_widget->padding ).size;
	//}

	//UiRect UiWidget::getInnerRect()
	//{
	//}

	void UiWidget::getInputState( ImUiWidgetInputState& inputState ) const
	{
		ImUiWidgetGetInputState( m_widget, &inputState );
	}

	void UiWidget::drawLine( UiPos p0, UiPos p1, UiColor color )
	{
		ImUiDrawLine( m_widget, p0, p1, color );
	}

	void UiWidget::drawWidgetColor( UiColor color )
	{
		ImUiDrawWidgetColor( m_widget, color );
	}

	void UiWidget::drawWidgetTexture( const ImUiTexture& texture )
	{
		ImUiDrawWidgetTexture( m_widget, texture );
	}

	void UiWidget::drawWidgetTextureColor( const ImUiTexture& texture, UiColor color )
	{
		ImUiDrawWidgetTextureColor( m_widget, texture, color );
	}

	void UiWidget::drawWidgetSkin( const ImUiSkin& skin )
	{
		ImUiDrawWidgetSkin( m_widget, skin );
	}

	void UiWidget::drawWidgetSkinColor( const ImUiSkin& skin, UiColor color )
	{
		ImUiDrawWidgetSkinColor( m_widget, skin, color );
	}

	void UiWidget::drawWidgetText( ImUiTextLayout* layout )
	{
		ImUiDrawWidgetText( m_widget, layout );
	}

	void UiWidget::drawWidgetTextColor( ImUiTextLayout* layout, UiColor color )
	{
		ImUiDrawWidgetTextColor( m_widget, layout, color );
	}

	void UiWidget::drawRectColor( const UiRect& rect, ImUiColor color )
	{
		ImUiDrawRectColor( m_widget, rect, color );
	}

	void UiWidget::drawRectTexture( const UiRect& rect, const ImUiTexture& texture )
	{
		ImUiDrawRectTexture( m_widget, rect, texture );
	}

	void UiWidget::drawRectTextureUv( const UiRect& rect, const ImUiTexture& texture, const UiTexCoord& uv )
	{
		ImUiDrawRectTextureUv( m_widget, rect, texture, uv );
	}

	void UiWidget::drawRectTextureColor( const UiRect& rect, const ImUiTexture& texture, UiColor color )
	{
		ImUiDrawRectTextureColor( m_widget, rect, texture, color );
	}

	void UiWidget::drawRectTextureColorUv( const UiRect& rect, const ImUiTexture& texture, UiColor color, const UiTexCoord& uv )
	{
		ImUiDrawRectTextureColorUv( m_widget, rect, texture, color, uv );
	}

	void UiWidget::drawSkin( const UiRect& rect, const ImUiSkin& skin )
	{
		ImUiDrawSkin( m_widget, rect, skin );
	}

	void UiWidget::drawSkinColor( const UiRect& rect, const ImUiSkin& skin, UiColor color )
	{
		ImUiDrawSkinColor( m_widget, rect, skin, color );
	}

	void UiWidget::drawText( UiPos pos, ImUiTextLayout* layout )
	{
		ImUiDrawText( m_widget, pos, layout );
	}

	void UiWidget::drawTextColor( UiPos pos, ImUiTextLayout* layout, UiColor color )
	{
		ImUiDrawTextColor( m_widget, pos, layout, color );
	}

	UiWidgetLayoutHorizontal::UiWidgetLayoutHorizontal( UiWindow& window )
		: UiWidget( window )
	{
		setLayoutHorizontal();
	}

	UiWidgetLayoutHorizontal::UiWidgetLayoutHorizontal( UiWindow& window, float spacing )
		: UiWidget( window )
	{
		setLayoutHorizontalSpacing( spacing );
	}

	UiWidgetLayoutVertical::UiWidgetLayoutVertical( UiWindow& window )
		: UiWidget( window )
	{
		setLayoutVertical();
	}

	UiWidgetLayoutVertical::UiWidgetLayoutVertical( UiWindow& window, float spacing )
		: UiWidget( window )
	{
		setLayoutVerticalSpacing( spacing );
	}

	toolbox::UiToolboxConfig::UiToolboxConfig()
	{
	}

	toolbox::UiToolboxConfig::UiToolboxConfig( ImUiFont* font )
	{
		setDefault( font );
	}

	void toolbox::UiToolboxConfig::setDefault( ImUiFont* font )
	{
		ImUiToolboxFillDefaultConfig( this, font );
	}

	void toolbox::setConfig( const UiToolboxConfig& config )
	{
		ImUiToolboxSetConfig( &config );
	}

	toolbox::UiToolboxWindow::UiToolboxWindow()
	{
	}

	toolbox::UiToolboxWindow::UiToolboxWindow( UiWindow& window )
	{
		m_window	= window.getInternal();
		m_owner		= false;
	}

	toolbox::UiToolboxWindow::UiToolboxWindow( ImUiWindow* window )
		: UiWindow( window )
	{
	}

	toolbox::UiToolboxWindow::UiToolboxWindow( UiSurface& surface, const UiStringView& name, const UiRect& rect, uint32_t zOrder )
		: UiWindow( surface, name, rect, zOrder )
	{
	}

	toolbox::UiToolboxWindow::UiToolboxWindow( ImUiSurface* surface, const UiStringView& name, const UiRect& rect, uint32_t zOrder )
		: UiWindow( surface, name, rect, zOrder )
	{
	}

	void toolbox::UiToolboxWindow::spacer( float width, float height )
	{
		ImUiToolboxSpacer( m_window, width, height );
	}

	void toolbox::UiToolboxWindow::strecher( float horizontal, float vertical )
	{
		ImUiToolboxStrecher( m_window, horizontal, vertical );
	}

	bool toolbox::UiToolboxWindow::buttonLabel( const UiStringView& text )
	{
		return ImUiToolboxButtonLabel( m_window, text );
	}

	bool toolbox::UiToolboxWindow::buttonLabelFormat( const char* format, ... )
	{
		va_list args;
		va_start( args, format );
		const bool result = ImUiToolboxButtonLabelFormatArgs( m_window, format, args );
		va_end( args );

		return result;
	}

	bool toolbox::UiToolboxWindow::buttonIcon( const ImUiTexture& icon )
	{
		return ImUiToolboxButtonIcon( m_window, icon );
	}

	bool toolbox::UiToolboxWindow::buttonIcon( const ImUiTexture& icon, UiSize iconSize )
	{
		return ImUiToolboxButtonIconSize( m_window, icon, iconSize );
	}

	bool toolbox::UiToolboxWindow::checkBox( bool& checked, const UiStringView& text )
	{
		return ImUiToolboxCheckBox( m_window, &checked, text );
	}

	bool toolbox::UiToolboxWindow::checkBoxState( const UiStringView& text, bool defaultValue /*= false */ )
	{
		return ImUiToolboxCheckBoxStateDefault( m_window, text, defaultValue );
	}

	void toolbox::UiToolboxWindow::label( const UiStringView& text )
	{
		ImUiToolboxLabel( m_window, text );
	}

	void toolbox::UiToolboxWindow::labelFormat( const char* format, ... )
	{
		va_list args;
		va_start( args, format );
		ImUiToolboxLabelFormatArgs( m_window, format, args);
		va_end( args );
	}

	bool toolbox::UiToolboxWindow::slider( float& value, float min /*= 0.0f*/, float max /*= 1.0f*/ )
	{
		return ImUiToolboxSliderMinMax( m_window, &value, min, max );
	}

	float toolbox::UiToolboxWindow::sliderState( float min /*= 0.0f*/, float max /*= 1.0f */ )
	{
		return ImUiToolboxSliderStateMinMax( m_window, min, max );
	}

	float toolbox::UiToolboxWindow::sliderState( float min, float max, float defaultValue )
	{
		return ImUiToolboxSliderStateMinMaxDefault( m_window, min, max, defaultValue );
	}

	bool toolbox::UiToolboxWindow::textEdit( char* buffer, size_t bufferSize, size_t* textLength )
	{
		return ImUiToolboxTextEdit( m_window, buffer, bufferSize, textLength );
	}

	UiStringView toolbox::UiToolboxWindow::textEditState( size_t bufferSize, UiStringView defaultValue /* = UiStringView() */ )
	{
		return UiStringView( ImUiToolboxTextEditStateBufferDefault( m_window, bufferSize, defaultValue ) );
	}

	void toolbox::UiToolboxWindow::progressBar( float value, float min /*= 0.0f*/, float max /*= 1.0f */ )
	{
		ImUiToolboxProgressBarMinMax( m_window, value, min, max );
	}

	size_t toolbox::UiToolboxWindow::dropDown( const UiStringView* items, size_t itemCount )
	{
		return ImUiToolboxDropDown( m_window, items, itemCount );
	}

	toolbox::UiToolboxButtonLabel::UiToolboxButtonLabel()
	{
	}

	toolbox::UiToolboxButtonLabel::UiToolboxButtonLabel( UiWindow& window, const UiStringView& text )
	{
		begin( window, text );
	}

	toolbox::UiToolboxButtonLabel::~UiToolboxButtonLabel()
	{
		end();
	}

	void toolbox::UiToolboxButtonLabel::begin( UiWindow& window, const UiStringView& text )
	{
		m_widget = ImUiToolboxButtonLabelBegin( window.getInternal(), text );
	}

	void toolbox::UiToolboxButtonLabel::beginFormat( UiWindow& window, const char* format, ... )
	{
		va_list args;
		va_start( args, format );
		m_widget = ImUiToolboxLabelBeginFormatArgs( window.getInternal(), format, args );
		va_end( args );
	}

	bool toolbox::UiToolboxButtonLabel::end()
	{
		if( m_widget )
		{
			const bool result = ImUiToolboxButtonLabelEnd( m_widget );
			m_widget = nullptr;

			return result;
		}

		return false;
	}

	toolbox::UiToolboxLabel::UiToolboxLabel()
	{
	}

	toolbox::UiToolboxLabel::UiToolboxLabel( UiWindow& window, const UiStringView& text )
	{
		begin( window, text );
	}

	toolbox::UiToolboxLabel::~UiToolboxLabel()
	{
		end();
	}

	void toolbox::UiToolboxLabel::begin( UiWindow& window, const UiStringView& text )
	{
		m_widget = ImUiToolboxLabelBegin( window.getInternal(), text );
	}

	void toolbox::UiToolboxLabel::beginFormat( UiWindow& window, const char* format, ... )
	{
		va_list args;
		va_start( args, format );
		m_widget = ImUiToolboxLabelBeginFormatArgs( window.getInternal(), format, args );
		va_end( args );
	}

	void toolbox::UiToolboxLabel::end()
	{
		if( m_widget )
		{
			ImUiToolboxLabelEnd( m_widget );
			m_widget = nullptr;
		}
	}

	toolbox::UiToolboxScrollArea::UiToolboxScrollArea()
	{
	}

	toolbox::UiToolboxScrollArea::UiToolboxScrollArea( UiWindow& window )
	{
		m_widget = ImUiToolboxScrollAreaBegin( window.getInternal() );
	}

	toolbox::UiToolboxScrollArea::~UiToolboxScrollArea()
	{
		if( m_widget )
		{
			ImUiToolboxScrollAreaEnd( m_widget );
			m_widget = nullptr;
		}
	}

	toolbox::UiToolboxList::UiToolboxList( UiWindow& window, float itemSize, size_t itemCount )
	{
		ImUiToolboxListBegin( &m_list, window.getInternal(), itemSize, itemCount );
		m_widget = m_list.list;
	}

	toolbox::UiToolboxList::~UiToolboxList()
	{
		ImUiToolboxListEnd( &m_list );
		m_widget = nullptr;
	}

	size_t toolbox::UiToolboxList::getBeginIndex() const
	{
		return ImUiToolboxListGetBeginIndex( &m_list );
	}

	size_t toolbox::UiToolboxList::getEndIndex() const
	{
		return ImUiToolboxListGetEndIndex( &m_list );
	}

	size_t toolbox::UiToolboxList::getSelectedIndex() const
	{
		return ImUiToolboxListGetSelectedIndex( &m_list );
	}

	ImUiWidget* toolbox::UiToolboxList::nextItem()
	{
		return ImUiToolboxListNextItem( &m_list );
	}

	toolbox::UiToolboxDropdown::UiToolboxDropdown( UiWindow& window, const UiStringView* items, size_t itemCount )
	{
		m_widget = ImUiToolboxDropDownBegin( window.getInternal(), items, itemCount );
	}

	toolbox::UiToolboxDropdown::~UiToolboxDropdown()
	{
		ImUiToolboxDropDownEnd( m_widget );
		m_widget = nullptr;
	}

	toolbox::UiToolboxPopup::UiToolboxPopup( UiSurface& surface )
	{
		m_window = ImUiToolboxPopupBeginSurface( surface.getInternal() );
		m_owner = true;
	}

	toolbox::UiToolboxPopup::UiToolboxPopup( UiWindow& window )
	{
		m_window = ImUiToolboxPopupBegin( window.getInternal() );
		m_owner = true;
	}

	toolbox::UiToolboxPopup::~UiToolboxPopup()
	{
		end();
	}

	size_t toolbox::UiToolboxPopup::end( const UiStringView* buttons, size_t buttonCount )
	{
		const size_t result = ImUiToolboxPopupEndButtons( m_window, buttons, buttonCount );
		m_window = nullptr;

		return result;
	}

	void toolbox::UiToolboxPopup::end()
	{
		if( m_window )
		{
			ImUiToolboxPopupEnd( m_window );
			m_window = nullptr;
		}
	}
}
