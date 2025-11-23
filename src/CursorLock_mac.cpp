#include "CursorLock.hpp"

#ifdef GEODE_IS_MACOS

#include <CoreGraphics/CoreGraphics.h>
#include <algorithm>

namespace cursorlock {

class MacCursorLockAPI : public CursorLockAPI {
public:
	void applyBounds(PixelRect const& rect) override {
		m_rect = rect;
		m_active = true;
	}

	void release() override {
		m_active = false;
	}

	void tick() override {
		if (!m_active) {
			return;
		}

		CGEventRef event = CGEventCreate(nullptr);
		if (!event) {
			return;
		}
		CGPoint loc = CGEventGetLocation(event);
		CFRelease(event);

		CGPoint clamped{
			std::clamp(loc.x, m_rect.left, m_rect.right),
			std::clamp(loc.y, m_rect.top, m_rect.bottom)
		};

		if (clamped.x != loc.x || clamped.y != loc.y) {
			CGWarpMouseCursorPosition(clamped);
			CGAssociateMouseAndMouseCursorPosition(true);
		}
	}

private:
	PixelRect m_rect{0, 0, 0, 0};
	bool m_active = false;
};

ScreenRect getPrimaryScreenRect() {
	CGDirectDisplayID display = CGMainDisplayID();
	CGRect rect = CGDisplayBounds(display);
	return {rect.origin.x, rect.origin.y, rect.size.width, rect.size.height};
}

std::unique_ptr<CursorLockAPI> createCursorLockAPI() {
	return std::make_unique<MacCursorLockAPI>();
}

} // namespace cursorlock

#endif
