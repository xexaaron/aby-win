#include "common.hpp"
#include "event.hpp"

#include <qapplication.h>
#include <qnamespace.h>
#include <qscreen_platform.h>
#if ABY_WIN_ENABLE_QT
#	include "window.hpp"
#	include <qmainwindow.h>
#	include <qpainter.h>
#	include <qpalette.h>
#	include <qpixmap.h>
#	include <qwindow.h>
#	include "backend/qt/qt-window.hpp"
#	include <QApplication>
#	include <QClipboard>
#	include <QCursor>
#	include <QGuiApplication>
#	include <QIcon>
#	include <QMainWindow>
#	include <QPalette>
#	include <QScreen>
#	include <QWindow>
#	include <QtGlobal>
#	ifdef Q_OS_WIN
#		ifndef WIN32_LEAN_AND_MEAN
#			define WIN32_LEAN_AND_MEAN
#		endif
#		include <QtGui/qguiapplication_platform.h>
#		include <Windows.h>
#	endif
#	if defined(Q_OS_LINUX) || defined(Q_OS_UNIX)
#		include <QtGui/qtguiglobal.h>
#		if QT_CONFIG(xcb)
#			include <xcb/xcb.h>
#		endif
#		if QT_CONFIG(wayland)
#			include <wayland-client.h>
#		endif
#		include <QtGui/qguiapplication_platform.h>
#		include <QtWaylandGlobal/QtWaylandGlobal>
#	else
#		error "currently unsupported platform"
#	endif
#	include <qnativeinterface.h>

#	include <QKeyEvent>

namespace aby::win::qt {

	class EventFilter final : public QObject {
	public:
		EventFilter(Window& window, QObject* parent) :
		    QObject(parent),
		    m_Window(window) {
		}
	protected:
		bool eventFilter(QObject* object, QEvent* event) override;
	private:
		Window& m_Window;
	};

} // namespace aby::win::qt

namespace aby::win::qt::detail {

	auto to_key(int key) -> EKey;
	auto to_mods(Qt::KeyboardModifiers mods) -> EMod;
	auto to_button(Qt::MouseButton button) -> EMouseButton;

} // namespace aby::win::qt::detail

namespace aby::win::qt {

	Window::Window(const Config& config) :
	    win::Window(config),
	    bMainWindow(!config.child) {
		// TODO: Themeing via config.theme and QPalette

		aby_win_assert(QCoreApplication::instance() != nullptr,
		               "[qt] the windowing library does not handle the creation of a qt application");

		if (!bMainWindow) {
			auto* window  = new QWindow();
			m_EventFilter = new EventFilter(*this, window);
			m_EventFilter->setParent(window);

			window->installEventFilter(m_EventFilter);

			window->setTitle(QString::fromStdString(config.name.data()));
			window->resize(config.width, config.height);

			if (!config.decorated) {
				window->setFlags(Qt::FramelessWindowHint);
			}

			if (!config.resizable) {
				window->setMinimumSize(QSize(config.width, config.height));
				window->setMaximumSize(QSize(config.width, config.height));
			}

			if (config.visible) {
				window->show();
			}

			if (config.focused) {
				window->requestActivate();
			}

			m_QT = window;

		} else {
			auto* window  = new QMainWindow();
			m_EventFilter = new EventFilter(*this, window);
			m_EventFilter->setParent(window);
			window->installEventFilter(m_EventFilter);
			window->setWindowTitle(QString::fromStdString(config.name.data()));
			window->resize(config.width, config.height);

			if (!config.decorated) {
				window->setWindowFlags(Qt::FramelessWindowHint);
			}

			if (!config.resizable) {
				window->setFixedSize(config.width, config.height);
			}

			if (config.visible) {
				window->show();
			}

			if (config.focused) {
				window->activateWindow();
				window->raise();
			}

			m_QT = window;
		}
	}

	Window::~Window() {
		if (bMainWindow) {
			delete std::get<QMainWindow*>(m_QT);
		} else {
			delete std::get<QWindow*>(m_QT);
		}

		if (m_EventFilter) {
			delete m_EventFilter;
		}
	}

	auto Window::set_name(std::string_view name) -> void {
		const auto title = QString::fromUtf8(name.data(), static_cast<qsizetype>(name.size()));

		if (bMainWindow) {
			std::get<QMainWindow*>(m_QT)->setWindowTitle(title);
		} else {
			std::get<QWindow*>(m_QT)->setTitle(title);
		}
	}

	auto Window::set_width(uint32_t w) -> void {
		set_size(w, height());
	}

	auto Window::set_height(uint32_t h) -> void {
		set_size(width(), h);
	}

	auto Window::set_size(uint32_t w, uint32_t h) -> void {
		if (bMainWindow) {
			std::get<QMainWindow*>(m_QT)->resize(
			    static_cast<int>(w),
			    static_cast<int>(h));
		} else {
			std::get<QWindow*>(m_QT)->resize(
			    static_cast<int>(w),
			    static_cast<int>(h));
		}
	}

	auto Window::set_position(int32_t x, int32_t y) -> void {
		if (bMainWindow) {
			std::get<QMainWindow*>(m_QT)->move(
			    static_cast<int>(x),
			    static_cast<int>(y));
		} else {
			std::get<QWindow*>(m_QT)->setPosition(
			    static_cast<int>(x),
			    static_cast<int>(y));
		}
	}

	auto Window::set_fullscreen(bool fullscreen) -> void {
		if (bMainWindow) {
			auto* window = std::get<QMainWindow*>(m_QT);

			if (fullscreen) {
				window->showFullScreen();
			} else {
				window->showNormal();
			}
		} else {
			auto* window = std::get<QWindow*>(m_QT);

			if (fullscreen) {
				window->showFullScreen();
			} else {
				window->showNormal();
			}
		}
	}

	auto Window::set_cursor_mode(ECursorMode mode) -> void {
		switch (mode) {
			case ECursorMode::normal:
				QGuiApplication::restoreOverrideCursor();
				break;

			case ECursorMode::hidden:
				QGuiApplication::setOverrideCursor(Qt::BlankCursor);
				break;

			case ECursorMode::disabled:
				QGuiApplication::setOverrideCursor(Qt::BlankCursor);
				break;
		}
	}

	auto Window::set_cursor_pos(float x, float y) -> void {
		QCursor::setPos(
		    static_cast<int>(x),
		    static_cast<int>(y));
	}

	auto Window::set_cursor(ECursor cursor) -> void {
		Qt::CursorShape shape = Qt::ArrowCursor;

		switch (cursor) {
			case ECursor::arrow:
				shape = Qt::ArrowCursor;
				break;

			case ECursor::ibeam:
				shape = Qt::IBeamCursor;
				break;

			case ECursor::crosshair:
				shape = Qt::CrossCursor;
				break;

			case ECursor::hand:
				shape = Qt::PointingHandCursor;
				break;

			case ECursor::hresize:
				shape = Qt::SizeHorCursor;
				break;

			case ECursor::vresize:
				shape = Qt::SizeVerCursor;
				break;

			case ECursor::nwse_resize:
				shape = Qt::SizeFDiagCursor;
				break;

			case ECursor::nesw_resize:
				shape = Qt::SizeBDiagCursor;
				break;

			case ECursor::move:
				shape = Qt::SizeAllCursor;
				break;

			case ECursor::not_allowed:
				shape = Qt::ForbiddenCursor;
				break;
		}

		if (bMainWindow) {
			std::get<QMainWindow*>(m_QT)->setCursor(shape);
		} else {
			std::get<QWindow*>(m_QT)->setCursor(shape);
		}
	}

	auto Window::set_theme(ETheme theme) -> void {
		aby_win_wrn("[qt] theme setting not supported");
	}

	auto Window::set_icon(const Icon& icon) -> void {
		const auto expected_size =
		    static_cast<size_t>(icon.width) *
		    static_cast<size_t>(icon.height) * 4;

		if (icon.pixels.size() < expected_size) {
			aby_win_err("[qt] icon pixel data does not align with icon pixel size");
			return;
		}

		const QImage image(
		    reinterpret_cast<const uchar*>(icon.pixels.data()),
		    static_cast<int>(icon.width),
		    static_cast<int>(icon.height),
		    static_cast<int>(icon.width * 4),
		    QImage::Format_RGBA8888);

		if (bMainWindow) {
			auto* window = std::get<QMainWindow*>(m_QT);
			window->setWindowIcon(QPixmap::fromImage(image));
		} else {
			auto* window = std::get<QWindow*>(m_QT);
			window->setIcon(QPixmap::fromImage(image));
		}
	}

	auto Window::set_hit_test_config(const HitTestConfig& cfg) -> void {
		aby_win_wrn("[qt] custom hit test not supported currently");
		// Qt requires native event handling for custom hit testing.
		// This is platform-specific and should be implemented through
		// QWindow/QWidget native event handling.
	}

	auto Window::set_clipboard(std::string_view text) -> void {
		if (auto* clipboard = QGuiApplication::clipboard()) {
			clipboard->setText(
			    QString::fromUtf8(
			        text.data(),
			        static_cast<qsizetype>(text.size())));
		}
	}

	auto Window::set_resizable(bool value) -> void {
		if (bMainWindow) {
			auto* window = std::get<QMainWindow*>(m_QT);

			if (value) {
				window->setMinimumSize(0, 0);
				window->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
			} else {
				window->setFixedSize(window->size());
			}
		} else {
			auto* window = std::get<QWindow*>(m_QT);

			if (value) {
				window->setMinimumSize(QSize(0, 0));
			} else {
				const auto size = window->size();

				window->setMinimumSize(size);
				window->setMaximumSize(size);
			}
		}
	}

	auto Window::set_visible(bool value) -> void {
		if (bMainWindow) {
			auto* window = std::get<QMainWindow*>(m_QT);

			if (value) {
				window->show();
			} else {
				window->hide();
			}
		} else {
			auto* window = std::get<QWindow*>(m_QT);

			if (value) {
				window->show();
			} else {
				window->hide();
			}
		}
	}

	auto Window::set_decorated(bool value) -> void {
		if (bMainWindow) {
			auto* window = std::get<QMainWindow*>(m_QT);
			auto flags   = window->windowFlags();

			if (value) {
				flags &= ~Qt::FramelessWindowHint;
			} else {
				flags |= Qt::FramelessWindowHint;
			}

			window->setWindowFlags(flags);
		} else {
			auto* window = std::get<QWindow*>(m_QT);
			auto flags   = window->flags();

			if (value) {
				flags &= ~Qt::FramelessWindowHint;
			} else {
				flags |= Qt::FramelessWindowHint;
			}

			window->setFlags(flags);
		}
	}

	auto Window::set_focused(bool value) -> void {
		if (!value) {
			return;
		}

		focus();
	}

	auto Window::add_listener(WindowListener&& listener) -> void {
		m_Listeners.emplace_back(std::move(listener));
	}

	auto Window::focus() -> void {
		if (bMainWindow) {
			auto* window = std::get<QMainWindow*>(m_QT);
			window->show();
			window->raise();
			window->activateWindow();
		} else {
			auto* window = std::get<QWindow*>(m_QT);
			window->show();
			window->raise();
			window->requestActivate();
		}
	}

	auto Window::minimize() -> void {
		if (bMainWindow) {
			std::get<QMainWindow*>(m_QT)->showMinimized();
		} else {
			std::get<QWindow*>(m_QT)->showMinimized();
		}
	}

	auto Window::maximize() -> void {
		if (bMainWindow) {
			std::get<QMainWindow*>(m_QT)->showMaximized();
		} else {
			std::get<QWindow*>(m_QT)->showMaximized();
		}
	}

	auto Window::show() -> void {
		set_visible(true);
	}

	auto Window::hide() -> void {
		set_visible(false);
	}

	auto Window::close() -> void {
		bShouldClose = true;
	}

	auto Window::poll() -> void {
		QCoreApplication::processEvents();
	}

	auto Window::width() const -> uint32_t {
		if (bMainWindow) {
			return static_cast<uint32_t>(
			    std::get<QMainWindow*>(m_QT)->width());
		}

		return static_cast<uint32_t>(
		    std::get<QWindow*>(m_QT)->width());
	}

	auto Window::height() const -> uint32_t {
		if (bMainWindow) {
			return static_cast<uint32_t>(
			    std::get<QMainWindow*>(m_QT)->height());
		}

		return static_cast<uint32_t>(
		    std::get<QWindow*>(m_QT)->height());
	}

	auto Window::size() const -> std::pair<uint32_t, uint32_t> {
		return { width(), height() };
	}

	auto Window::position() const -> std::pair<int32_t, int32_t> {
		if (bMainWindow) {
			const auto pos = std::get<QMainWindow*>(m_QT)->pos();
			return { pos.x(), pos.y() };
		}

		const auto pos = std::get<QWindow*>(m_QT)->position();
		return { pos.x(), pos.y() };
	}

	auto Window::scale() const -> float {
		if (bMainWindow) {
			return static_cast<float>(
			    std::get<QMainWindow*>(m_QT)->devicePixelRatioF());
		}

		return static_cast<float>(
		    std::get<QWindow*>(m_QT)->devicePixelRatio());
	}

	auto Window::native() const -> NativeWindow {
		NativeWindow w;
		w.backend = EWindow::qt;

		if (bMainWindow) {
			auto* window     = std::get<QMainWindow*>(m_QT);
			w.backend_window = window;

#	ifdef _WIN32
			w.platform        = EPlatform::winapi;
			w.platform_window = reinterpret_cast<HWND>(window->winId());
#	elif defined(__linux__)
			if (QGuiApplication::platformName() == "wayland") {
				m_NativeHandles.first  = qApp->nativeInterface<QNativeInterface::QWaylandApplication>()->display();
				m_NativeHandles.second = reinterpret_cast<wl_surface*>(window->winId());
				w.platform             = EPlatform::wayland;
				w.platform_window      = &m_NativeHandles;
			}

			if (QGuiApplication::platformName() == "xcb") {
				m_NativeHandles.first  = qApp->nativeInterface<QNativeInterface::QX11Application>()->display();
				m_NativeHandles.second = reinterpret_cast<void*>(static_cast<uintptr_t>(window->winId()));
				w.platform             = EPlatform::x11;
				w.platform_window      = &m_NativeHandles;
			}
#	elif defined(__APPLE__)
#		error "currently unsupported (have to configure objective c++ at some point).
#	else
#		error "currently unsupported platform"
#	endif

		} else {
			auto* window     = std::get<QWindow*>(m_QT);
			w.backend_window = window;
#	ifdef _WIN32
			w.platform        = EPlatform::winapi;
			w.platform_window = reinterpret_cast<HWND>(window->winId());
#	elif defined(__linux__)
			if (QGuiApplication::platformName() == "wayland") {
				m_NativeHandles.first  = qApp->nativeInterface<QNativeInterface::QWaylandApplication>()->display();
				m_NativeHandles.second = reinterpret_cast<wl_surface*>(window->winId());
				w.platform             = EPlatform::wayland;
				w.platform_window      = &m_NativeHandles;
			}

			if (QGuiApplication::platformName() == "xcb") {
				// Display*
				// Window
				m_NativeHandles.first  = qApp->nativeInterface<QNativeInterface::QX11Application>()->display();
				m_NativeHandles.second = reinterpret_cast<void*>(static_cast<uintptr_t>(window->winId()));
				w.platform             = EPlatform::x11;
				w.platform_window      = &m_NativeHandles;
			}
#	elif defined(__APPLE__)
#		error "currently unsupported (have to configure objective c++ at some point).
#	else
#		error "currently unsupported platform"
#	endif
		}

		return w;
	}

	auto Window::fb_width() const -> uint32_t {
		return static_cast<uint32_t>(
		    static_cast<float>(width()) * scale());
	}

	auto Window::fb_height() const -> uint32_t {
		return static_cast<uint32_t>(
		    static_cast<float>(height()) * scale());
	}

	auto Window::fb_size() const -> std::pair<uint32_t, uint32_t> {
		return { fb_width(), fb_height() };
	}

	auto Window::monitor() const -> const Monitor* {
		// Use the screen containing the window.
		// Map this QScreen to your Monitor abstraction.
		return nullptr;
	}

	auto Window::id() const -> uint32_t {
		if (bMainWindow) {
			return static_cast<uint32_t>(
			    std::get<QMainWindow*>(m_QT)->winId());
		}

		return static_cast<uint32_t>(
		    std::get<QWindow*>(m_QT)->winId());
	}

	auto Window::clipboard() const -> std::string_view {
		static thread_local std::string text;

		if (auto* clipboard = QGuiApplication::clipboard()) {
			text = clipboard->text().toStdString();
		} else {
			text.clear();
		}

		return text;
	}

	auto Window::focused() const -> bool {
		if (bMainWindow) {
			return std::get<QMainWindow*>(m_QT)->isActiveWindow();
		}

		return std::get<QWindow*>(m_QT)->isActive();
	}

	auto Window::minimized() const -> bool {
		if (bMainWindow) {
			return std::get<QMainWindow*>(m_QT)->isMinimized();
		}

		return std::get<QWindow*>(m_QT)->windowState() & Qt::WindowMinimized;
	}

	auto Window::maximized() const -> bool {
		if (bMainWindow) {
			return std::get<QMainWindow*>(m_QT)->isMaximized();
		}

		return std::get<QWindow*>(m_QT)->windowState() & Qt::WindowMaximized;
	}

	auto Window::visible() const -> bool {
		if (bMainWindow) {
			return std::get<QMainWindow*>(m_QT)->isVisible();
		}

		return std::get<QWindow*>(m_QT)->isVisible();
	}

	auto Window::fullscreened() -> bool {
		if (bMainWindow) {
			return std::get<QMainWindow*>(m_QT)->isFullScreen();
		}

		return std::get<QWindow*>(m_QT)->windowState() & Qt::WindowFullScreen;
	}

	auto Window::should_close() const -> bool {
		return bShouldClose;
	}

	auto Window::dispatch(Event& event) -> bool {
		for (auto& listener : m_Listeners) {
			if (listener(event))
				return true;
		}

		return false;
	}

	auto Window::is_child() -> bool {
		return !bMainWindow;
	}

} // namespace aby::win::qt

namespace aby::win::qt {

	bool EventFilter::eventFilter(QObject*, QEvent* event) {
		switch (event->type()) {
			case QEvent::KeyPress: {
				auto* e = static_cast<QKeyEvent*>(event);

				KeyPressedEvent ev(
				    m_Window.id(),
				    detail::to_key(e->key()),
				    detail::to_mods(e->modifiers()));

				return m_Window.dispatch(ev);
			}
			case QEvent::KeyRelease: {
				auto* e = static_cast<QKeyEvent*>(event);

				KeyReleasedEvent ev(
				    m_Window.id(),
				    detail::to_key(e->key()),
				    detail::to_mods(e->modifiers()));

				return m_Window.dispatch(ev);
			}

			case QEvent::MouseMove: {
				auto* e = static_cast<QMouseEvent*>(event);

				MouseMovedEvent ev(
				    m_Window.id(),
				    static_cast<float>(e->position().x()),
				    static_cast<float>(e->position().y()));

				return m_Window.dispatch(ev);
			}

			case QEvent::MouseButtonPress: {
				auto* e = static_cast<QMouseEvent*>(event);

				MousePressedEvent ev(
				    m_Window.id(),
				    detail::to_button(e->button()),
				    detail::to_mods(e->modifiers()));

				return m_Window.dispatch(ev);
			}

			case QEvent::MouseButtonRelease: {
				auto* e = static_cast<QMouseEvent*>(event);

				MouseReleasedEvent ev(
				    m_Window.id(),
				    detail::to_button(e->button()),
				    detail::to_mods(e->modifiers()));

				return m_Window.dispatch(ev);
			}

			case QEvent::Wheel: {
				auto* e = static_cast<QWheelEvent*>(event);

				MouseScrolledEvent ev(
				    m_Window.id(),
				    static_cast<float>(e->angleDelta().x()) / 120.0f,
				    static_cast<float>(e->angleDelta().y()) / 120.0f);

				return m_Window.dispatch(ev);
			}

			case QEvent::Resize: {
				auto* e = static_cast<QResizeEvent*>(event);

				WindowResizedEvent ev(
				    m_Window.id(),
				    static_cast<uint32_t>(e->size().width()),
				    static_cast<uint32_t>(e->size().height()));

				return m_Window.dispatch(ev);
			}

			case QEvent::Move: {
				auto* e = static_cast<QMoveEvent*>(event);

				WindowMovedEvent ev(
				    m_Window.id(),
				    e->pos().x(),
				    e->pos().y());

				return m_Window.dispatch(ev);
			}
			case QEvent::Close: {
				WindowClosedEvent ev(m_Window.id());
				m_Window.close();
				return m_Window.dispatch(ev);
			}
			case QEvent::Enter: {
				MouseEnteredEvent ev(m_Window.id());
				return m_Window.dispatch(ev);
			}

			case QEvent::Leave: {
				MouseLeftEvent ev(m_Window.id());
				return m_Window.dispatch(ev);
			}

			case QEvent::FocusIn: {
				WindowFocusedEvent ev(m_Window.id(), true);
				return m_Window.dispatch(ev);
			}

			case QEvent::FocusOut: {
				WindowFocusedEvent ev(m_Window.id(), false);
				return m_Window.dispatch(ev);
			}

			default:
				break;
		}

		return false;
	}

} // namespace aby::win::qt

namespace aby::win::qt::detail {

	auto to_key(int key) -> EKey {
		switch (key) {
			// Letters
			case Qt::Key_A:
				return EKey::a;
			case Qt::Key_B:
				return EKey::b;
			case Qt::Key_C:
				return EKey::c;
			case Qt::Key_D:
				return EKey::d;
			case Qt::Key_E:
				return EKey::e;
			case Qt::Key_F:
				return EKey::f;
			case Qt::Key_G:
				return EKey::g;
			case Qt::Key_H:
				return EKey::h;
			case Qt::Key_I:
				return EKey::i;
			case Qt::Key_J:
				return EKey::j;
			case Qt::Key_K:
				return EKey::k;
			case Qt::Key_L:
				return EKey::l;
			case Qt::Key_M:
				return EKey::m;
			case Qt::Key_N:
				return EKey::n;
			case Qt::Key_O:
				return EKey::o;
			case Qt::Key_P:
				return EKey::p;
			case Qt::Key_Q:
				return EKey::q;
			case Qt::Key_R:
				return EKey::r;
			case Qt::Key_S:
				return EKey::s;
			case Qt::Key_T:
				return EKey::t;
			case Qt::Key_U:
				return EKey::u;
			case Qt::Key_V:
				return EKey::v;
			case Qt::Key_W:
				return EKey::w;
			case Qt::Key_X:
				return EKey::x;
			case Qt::Key_Y:
				return EKey::y;
			case Qt::Key_Z:
				return EKey::z;

			// Numbers
			case Qt::Key_0:
				return EKey::num_0;
			case Qt::Key_1:
				return EKey::num_1;
			case Qt::Key_2:
				return EKey::num_2;
			case Qt::Key_3:
				return EKey::num_3;
			case Qt::Key_4:
				return EKey::num_4;
			case Qt::Key_5:
				return EKey::num_5;
			case Qt::Key_6:
				return EKey::num_6;
			case Qt::Key_7:
				return EKey::num_7;
			case Qt::Key_8:
				return EKey::num_8;
			case Qt::Key_9:
				return EKey::num_9;

			// Function
			case Qt::Key_F1:
				return EKey::f1;
			case Qt::Key_F2:
				return EKey::f2;
			case Qt::Key_F3:
				return EKey::f3;
			case Qt::Key_F4:
				return EKey::f4;
			case Qt::Key_F5:
				return EKey::f5;
			case Qt::Key_F6:
				return EKey::f6;
			case Qt::Key_F7:
				return EKey::f7;
			case Qt::Key_F8:
				return EKey::f8;
			case Qt::Key_F9:
				return EKey::f9;
			case Qt::Key_F10:
				return EKey::f10;
			case Qt::Key_F11:
				return EKey::f11;
			case Qt::Key_F12:
				return EKey::f12;

			// Modifiers
			case Qt::Key_Shift:
				return EKey::left_shift;
			case Qt::Key_Control:
				return EKey::left_ctrl;
			case Qt::Key_Alt:
				return EKey::left_alt;
			case Qt::Key_Meta:
				return EKey::left_super;

			// Navigation
			case Qt::Key_Up:
				return EKey::up;
			case Qt::Key_Down:
				return EKey::down;
			case Qt::Key_Left:
				return EKey::left;
			case Qt::Key_Right:
				return EKey::right;

			case Qt::Key_Home:
				return EKey::home;
			case Qt::Key_End:
				return EKey::end;
			case Qt::Key_PageUp:
				return EKey::page_up;
			case Qt::Key_PageDown:
				return EKey::page_down;

			case Qt::Key_Insert:
				return EKey::insert;
			case Qt::Key_Delete:
				return EKey::del;

			// Editing
			case Qt::Key_Backspace:
				return EKey::backspace;
			case Qt::Key_Return:
			case Qt::Key_Enter:
				return EKey::enter;
			case Qt::Key_Tab:
				return EKey::tab;
			case Qt::Key_Escape:
				return EKey::escape;
			case Qt::Key_Space:
				return EKey::space;

			// Punctuation
			case Qt::Key_Apostrophe:
				return EKey::apostrophe;
			case Qt::Key_Comma:
				return EKey::comma;
			case Qt::Key_Minus:
				return EKey::minus;
			case Qt::Key_Period:
				return EKey::period;
			case Qt::Key_Slash:
				return EKey::slash;
			case Qt::Key_Semicolon:
				return EKey::semicolon;
			case Qt::Key_Equal:
				return EKey::equal;
			case Qt::Key_BracketLeft:
				return EKey::left_bracket;
			case Qt::Key_Backslash:
				return EKey::backslash;
			case Qt::Key_BracketRight:
				return EKey::right_bracket;
			case Qt::Key_QuoteLeft:
				return EKey::grave_accent;

			// Locks
			case Qt::Key_CapsLock:
				return EKey::caps_lock;
			case Qt::Key_NumLock:
				return EKey::num_lock;
			case Qt::Key_ScrollLock:
				return EKey::scroll_lock;

				// Numpad
				// case Qt::Key_NumPad0:
				// 	return EKey::kp_0;
				// case Qt::Key_NumPad1:
				// 	return EKey::kp_1;
				// case Qt::Key_NumPad2:
				// 	return EKey::kp_2;
				// case Qt::Key_NumPad3:
				// 	return EKey::kp_3;
				// case Qt::Key_NumPad4:
				// 	return EKey::kp_4;
				// case Qt::Key_NumPad5:
				// 	return EKey::kp_5;
				// case Qt::Key_NumPad6:
				// 	return EKey::kp_6;
				// case Qt::Key_NumPad7:
				// 	return EKey::kp_7;
				// case Qt::Key_np:
				// 	return EKey::kp_8;
				// case Qt::Key_NumPad9:
				// 	return EKey::kp_9;
				// case Qt::Key_n:
				// 	return EKey::kp_decimal;
				// case Qt::Key_Slash:
				// 	return EKey::slash;

			case Qt::Key_Print:
				return EKey::print_screen;
			case Qt::Key_Pause:
				return EKey::pause;

			case Qt::Key_Menu:
				return EKey::menu;

			default:
				return EKey::unknown;
		}
	}

	auto to_mods(Qt::KeyboardModifiers mods) -> EMod {
		EMod result = EMod::none;

		if (mods & Qt::ShiftModifier)
			result |= EMod::shift;

		if (mods & Qt::ControlModifier)
			result |= EMod::ctrl;

		if (mods & Qt::AltModifier)
			result |= EMod::alt;

		if (mods & Qt::MetaModifier)
			result |= EMod::super;

		if (mods & Qt::KeypadModifier) {
			// No direct EMod equivalent.
			// Keypad-ness belongs to the key itself.
		}

		return result;
	}

	auto to_button(Qt::MouseButton button) -> EMouseButton {
		switch (button) {
			case Qt::LeftButton:
				return EMouseButton::left;
			case Qt::RightButton:
				return EMouseButton::right;
			case Qt::MiddleButton:
				return EMouseButton::middle;

			case Qt::XButton1:
				return EMouseButton::button_4;
			case Qt::XButton2:
				return EMouseButton::button_5;

			default:
				return EMouseButton::none;
		}
	}

} // namespace aby::win::qt::detail

#endif
