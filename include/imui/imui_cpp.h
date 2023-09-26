#pragma once

#include "imui/imui.h"
#include "imui/imui_toolbox.h"

namespace imui
{
	class UiFrame;
	class UiInput;
	class UiSurface;
	class UiWidget;
	class UiWindow;
	struct UiAlign;
	struct UiBorder;
	struct UiPos;
	struct UiRect;
	struct UiSize;

	struct UiStringView : public ImUiStringView
	{
						UiStringView();
		template< size_t TLen >
						UiStringView( const char (&str)[ TLen ] );
		explicit		UiStringView( const ImUiStringView& value );

		const char*		getData() const;
		size_t			getLength() const;
	};

	struct UiAlign : public ImUiAlign
	{
						UiAlign( ImUiHAlign hAlign, ImUiVAlign vAlign );
	};

	struct UiBorder : public ImUiBorder
	{
						UiBorder();
						UiBorder( float all );
						UiBorder( float horizontal, float vertical );
						UiBorder( float _top, float _left, float _bottom, float _right );
		explicit		UiBorder( const ImUiBorder& value );

		UiSize			getMinSize() const;
	};

	struct UiPos : public ImUiPos
	{
						UiPos();
						UiPos( float all );
						UiPos( float _x, float _y );
		explicit		UiPos( const ImUiPos& value );

		UiPos			add( float _x, float _y ) const;
		UiPos			add( UiPos add ) const;
		UiPos			sub( float _x, float _y ) const;
		UiPos			sub( UiPos sub ) const;
		UiPos			scale( float factor ) const;

		static UiPos	Zero;
	};

	struct UiRect : public ImUiRect
	{
						UiRect();
						UiRect( UiPos _pos, UiSize _size );
						UiRect( float x, float y, float width, float height );
		explicit		UiRect( const ImUiRect& value );

		UiRect			shrinkBorder( const ImUiBorder& border ) const;

		bool			includesPos( ImUiPos _pos ) const;
		bool			intersectsRect( const ImUiRect& rect2 ) const;

		float			getRight() const;
		float			getBottom() const;

		UiPos			getTopLeft() const;
		UiPos			getTopRight() const;
		UiPos			getBottomLeft() const;
		UiPos			getBottomRight() const;
	};

	struct UiSize : public ImUiSize
	{
						UiSize();
						UiSize( float all );
						UiSize( float _width, float _height );
		explicit		UiSize( const ImUiSize& value );

		UiSize			add( float _width, float _height ) const;
		UiSize			add( UiSize add ) const;
		UiSize			sub( float _width, float _height ) const;
		UiSize			sub( UiSize sub ) const;
		UiSize			scale( float factor ) const;
		UiSize			shrinkBorder( const UiBorder& border ) const;
		UiSize			expandBorder( const UiBorder& border ) const;

		static UiSize	lerp( UiSize a, UiSize b, float t );
		static UiSize	lerp( UiSize a, UiSize b, float widthT, float heightT );
		static UiSize	min( UiSize a, UiSize b );
		static UiSize	max( UiSize a, UiSize b );

		static UiSize	Zero;
		static UiSize	One;
		static UiSize	Horizontal;
		static UiSize	Vertical;
	};

	struct UiContextParameters : public ImUiParameters
	{
						UiContextParameters();
	};

	class UiContext
	{
	public:

						UiContext();
						UiContext( const UiContextParameters& parameters );
						~UiContext();

		bool			isValid() const;
		ImUiContext*	getInternal() const;

		UiInput&		beginInput();
		void			endInput();

	private:

		ImUiContext*	m_context;
	};

	class UiFrame
	{
	public:

						UiFrame();
						UiFrame( ImUiFrame* frame );
						UiFrame( UiContext& context, float timeInSeconds );
						~UiFrame();

		void			beginFrame( UiContext& context, float timeInSeconds );
		void			endFrame();

		bool			isValid() const;
		ImUiFrame*		getInternal() const;

	private:

		ImUiFrame*		m_frame;
	};

	class UiSurface
	{
	public:

						UiSurface();
						UiSurface( ImUiSurface* surface );
						UiSurface( ImUiFrame* frame, const UiStringView& name, const UiSize& size, float dpiScale );
						UiSurface( UiFrame& frame, const UiStringView& name, const UiSize& size, float dpiScale );
						~UiSurface();

		void			beginSurface( ImUiFrame* frame, const UiStringView& name, const UiSize& size, float dpiScale );
		void			beginSurface( UiFrame& frame, const UiStringView& name, const UiSize& size, float dpiScale );
		void			endSurface();

		bool			isValid() const;
		ImUiSurface*	getInternal() const;

		UiSize			getSize() const;
		float			getDpiScale() const;

	private:

		ImUiSurface*	m_surface;
	};

	class UiWindow
	{
	public:

						UiWindow();
						UiWindow( ImUiWindow* window );
						UiWindow( ImUiSurface* surface, const UiStringView& name, const UiRect& rect, uint32_t zOrder );
						UiWindow( UiSurface& surface, const UiStringView& name, const UiRect& rect, uint32_t zOrder );
						~UiWindow();

		void			beginWindow( ImUiSurface* surface, const UiStringView& name, const UiRect& rect, uint32_t zOrder );
		void			beginWindow( UiSurface& surface, const UiStringView& name, const UiRect& rect, uint32_t zOrder );
		void			endWindow();

		bool			isValid() const;
		ImUiWindow*		getInternal() const;

		float			getTime() const;
		UiRect			getRect() const;
		uint32_t		getZOrder() const;

	private:

		ImUiWindow*		m_window;
	};

	class UiWidget
	{
	public:

						UiWidget();
						UiWidget( ImUiWidget* widget );
						UiWidget( UiWindow& window );
						UiWidget( UiWindow& window, ImUiId id );
						UiWidget( UiWindow& window, const UiStringView& name );
						~UiWidget();

		void			beginWidget( UiWindow& window );
		void			beginWidget( UiWindow& window, ImUiId id );
		void			beginWidget( UiWindow& window, const UiStringView& name );
		void			endWidget();

		bool			isValid() const;
		ImUiWidget*		getInternal() const;

		void*			allocState( size_t size );
		void*			allocState( size_t size, bool& isNew );
		template< class T >
		T*				newState();
		template< class T >
		T*				newState( bool& isNew );

		ImUiLayout		getLayout();
		void			setLayoutStack();							// default
		void			setLayoutScroll( UiPos offset );
		void			setLayoutHorizontal();
		void			setLayoutHorizontalSpacing( float spacing );
		void			setLayoutVerical();
		void			setLayoutVerticalSpacing( float spacing );
		void			setLayoutGrid( size_t columnCount );

		float			getTime();

		UiBorder		getMargin();
		void			setMargin( const UiBorder& margin );
		UiBorder		getPadding();
		void			setPadding( const UiBorder& padding );

		UiSize			getMinSize();
		void			setMinSize( UiSize size );
		UiSize			getMaxSize();
		void			setMaxSize( UiSize size );

		void			setFixedWidth( float value );
		void			setFixedHeight( float value );
		void			setFixedSize( UiSize size );

		UiSize			getStretch();
		void			setStretch( UiSize stretch );

		UiAlign			getAlign();
		void			setAlign( UiAlign align );
		void			setHAlign( ImUiHAlign align );
		void			setVAlign( ImUiVAlign align );

		UiPos			getPos();
		UiSize			getSize();
		UiRect			getRect();
		UiSize			getInnerSize();
		UiRect			getInnerRect();

	private:

		ImUiWidget*		m_widget;
	};

	namespace toolbox
	{
		struct UiToolboxConfig : ImUiToolboxConfig
		{
						UiToolboxConfig();
		};

		void			setConfig( const UiToolboxConfig& config );

		bool			buttonLabel( UiWindow& window, const UiStringView& text );
		bool			buttonLabelFormat( UiWindow& window, const char* format, ... );

		bool			checkBox( UiWindow& window, bool& checked, const UiStringView& text );
		bool			checkBoxState( UiWindow& window, const UiStringView& text );

		void			label( UiWindow& window, ImUiStringView text );
		void			labelFormat( UiWindow& window, const char* format, ... );

		bool			slider( UiWindow& window, float& value );								// value range is 0 to 1
		bool			sliderMinMax( UiWindow& window, float& value, float min, float max );
		float			sliderState( UiWindow& window );										// value range is 0 to 1
		float			sliderStateMinMax( UiWindow& window, float min, float max );

		bool			textEdit( UiWindow& window, char* buffer, size_t bufferSize, size_t textLength );

		void			progressBar( UiWindow& window, float value ); // value range 0 to 1
		void			progressBarMinMax( UiWindow& window, float value, float min, float max );
	}

	template< size_t TLen >
	UiStringView::UiStringView( const char( &str )[ TLen ] )
	{
		data	= str;
		length	= TLen - 1u;
	}

	template< class T >
	T* UiWidget::newState()
	{
		bool isNew;
		return newState< T >( isNew );
	}

	template< class T >
	T* UiWidget::newState( bool& isNew )
	{
		void* memory = ImUiWidgetAllocStateNewDestruct( m_widget, sizeof( T ), &isNew, T::~T );
		if( !memory )
		{
			return nullptr;
		}

		T* state = (T*)memory;
		if( isNew )
		{
			new (memory) T();
		}

		return state;
	}
}
