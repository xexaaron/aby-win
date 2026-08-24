#include <aby-win/window.hpp>

int main(int argc, char** argv) {
    using namespace aby::win;
    auto window = Window::create(EWindow::glfw, "aby-win-test", 800, 600, ERenderBackend::none);

    while (!window->should_close()) {
        window->poll();
    }

	return 0;
}
