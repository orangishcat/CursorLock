#include "CursorLock.hpp"

#ifdef GEODE_IS_WINDOWS

#define NOMINMAX
#include <Windows.h>

namespace cursorlock {

class WindowsCursorLockAPI : public CursorLockAPI {
public:
	void applyBounds(PixelRect const& rect) override {
		RECT r;
		r.left = static_cast<LONG>(rect.left);
		r.top = static_cast<LONG>(rect.top);
		r.right = static_cast<LONG>(rect.right);
		r.bottom = static_cast<LONG>(rect.bottom);
		ClipCursor(&r);
	}

	void release() override {
		ClipCursor(nullptr);
	}
};

ScreenRect getPrimaryScreenRect() {
	auto x = static_cast<double>(GetSystemMetrics(SM_XVIRTUALSCREEN));
	auto y = static_cast<double>(GetSystemMetrics(SM_YVIRTUALSCREEN));
	auto w = static_cast<double>(GetSystemMetrics(SM_CXVIRTUALSCREEN));
	auto h = static_cast<double>(GetSystemMetrics(SM_CYVIRTUALSCREEN));
	return {x, y, w, h};
}

std::unique_ptr<CursorLockAPI> createCursorLockAPI() {
	return std::make_unique<WindowsCursorLockAPI>();
}

} // namespace cursorlock

#endif
