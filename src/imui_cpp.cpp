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

	const char* UiStringView::getData() const
	{
		return data;
	}

	size_t UiStringView::getLength() const
	{
		return length;
	}

	UiAlign::UiAlign( ImUiHAlign hAlign, ImUiVAlign vAlign )
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

	UiContext::UiContext()
		: m_context( nullptr )
	{
	}

	UiContextParameters::UiContextParameters()
	{
		memset( this, 0u, sizeof( *this ) );
	}

	UiContext::UiContext( const UiContextParameters& parameters )
	{
		m_context = ImUiCreate( &parameters );
	}

	UiContext::~UiContext()
	{
		if( m_context )
		{
			ImUiDestroy( m_context );
			m_context = nullptr;
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

	void* UiWidget::allocState( size_t size )
	{
		return ImUiWidgetAllocState( m_widget, size );
	}

	void* UiWidget::allocState( size_t size, bool& isNew )
	{
		return ImUiWidgetAllocStateNew( m_widget, size, &isNew );
	}

	ImUiLayout UiWidget::getLayout()
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

	UiBorder UiWidget::getMargin()
	{
		return (const UiBorder&)m_widget->margin;
	}

	void UiWidget::setMargin( const UiBorder& margin )
	{
		ImUiWidgetSetMargin( m_widget, margin );
	}

	UiBorder UiWidget::getPadding()
	{
		return (const UiBorder&)m_widget->padding;
	}

	void UiWidget::setPadding( const UiBorder& padding )
	{
		ImUiWidgetSetPadding( m_widget, padding );
	}

	UiSize UiWidget::getMinSize()
	{
		return (const UiSize&)m_widget->minSize;
	}

	void UiWidget::setMinSize( UiSize size )
	{
		ImUiWidgetSetMinSize( m_widget, size );
	}

	UiSize UiWidget::getMaxSize()
	{
		return (const UiSize&)m_widget->maxSize;
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

	UiSize UiWidget::getStretch()
	{
		return (const UiSize&)m_widget->stretch;
	}

	void UiWidget::setStretch( UiSize stretch )
	{
		ImUiWidgetSetStretch( m_widget, stretch );
	}

	UiAlign UiWidget::getAlign()
	{
		return (const UiAlign&)m_widget->align;
	}

	void UiWidget::setAlign( UiAlign align )
	{
		ImUiWidgetSetAlign( m_widget, align );
	}

	void UiWidget::setHAlign( ImUiHAlign align )
	{
		ImUiWidgetSetHAlign( m_widget, align );
	}

	void UiWidget::setVAlign( ImUiVAlign align )
	{
		ImUiWidgetSetVAlign( m_widget, align );
	}

	UiPos UiWidget::getPos()
	{
		return (const UiPos&)m_widget->rect.pos;
	}

	UiSize UiWidget::getSize()
	{
		return (const UiSize&)m_widget->rect.size;
	}

	UiRect UiWidget::getRect()
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

	toolbox::UiToolboxConfig::UiToolboxConfig( ImUiFont* font )
	{
		ImUiToolboxFillDefaultConfig( this, font );
	}

	void toolbox::setConfig( const UiToolboxConfig& config )
	{
		ImUiToolboxSetConfig( &config );
	}

	bool toolbox::buttonLabel( UiWindow& window, const UiStringView& text )
	{
		return ImUiToolboxButtonLabel( window.getInternal(), text );
	}

	bool toolbox::buttonLabelFormat( UiWindow& window, const char* format, ... )
	{
		va_list args;
		va_start( args, format );
		const bool result = ImUiToolboxButtonLabelFormatArgs( window.getInternal(), format, args );
		va_end( args );

		return result;
	}

	bool toolbox::checkBox( UiWindow& window, bool& checked, const UiStringView& text )
	{
		return ImUiToolboxCheckBox( window.getInternal(), &checked, text );
	}

	bool toolbox::checkBoxState( UiWindow& window, const UiStringView& text )
	{
		return ImUiToolboxCheckBoxState( window.getInternal(), text );
	}

	void toolbox::label( UiWindow& window, const UiStringView& text )
	{
		ImUiToolboxLabel( window.getInternal(), text );
	}

	void toolbox::labelFormat( UiWindow& window, const char* format, ... )
	{
		va_list args;
		va_start( args, format );
		ImUiToolboxLabelFormatArgs( window.getInternal(), format, args);
		va_end( args );
	}

	bool toolbox::slider( UiWindow& window, float& value )
	{
		return ImUiToolboxSlider( window.getInternal(), &value );
	}

	bool toolbox::sliderMinMax( UiWindow& window, float& value, float min, float max )
	{
		return ImUiToolboxSliderMinMax( window.getInternal(), &value, min, max );
	}

	float toolbox::sliderState( UiWindow& window )
	{
		return ImUiToolboxSliderState( window.getInternal() );
	}

	float toolbox::sliderStateMinMax( UiWindow& window, float min, float max )
	{
		return ImUiToolboxSliderStateMinMax( window.getInternal(), min, max );
	}

	bool toolbox::textEdit( UiWindow& window, char* buffer, size_t bufferSize, size_t textLength )
	{
		return ImUiToolboxTextEdit( window.getInternal(), buffer, bufferSize, textLength );;
	}

	void toolbox::progressBar( UiWindow& window, float value )
	{
		ImUiToolboxProgressBar( window.getInternal(), value );
	}

	void toolbox::progressBarMinMax( UiWindow& window, float value, float min, float max )
	{
		ImUiToolboxProgressBarMinMax( window.getInternal(), value, min, max );
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
}
