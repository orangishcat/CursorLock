#include "CursorLock.hpp"

#include <algorithm>

namespace cursorlock {

PercentBounds clampBounds(PercentBounds const& bounds) {
	PercentBounds clamped = bounds;
	clamped.left = std::clamp(clamped.left, 0.0, 100.0);
	clamped.top = std::clamp(clamped.top, 0.0, 100.0);
	clamped.right = std::clamp(clamped.right, 0.0, 100.0);
	clamped.bottom = std::clamp(clamped.bottom, 0.0, 100.0);
	return clamped;
}

PixelRect percentToPixels(PercentBounds const& pct) {
	ScreenRect screen = getPrimaryScreenRect();
	PercentBounds clamped = clampBounds(pct);

	double left = screen.x + screen.width * (clamped.left / 100.0);
	double top = screen.y + screen.height * (clamped.top / 100.0);
	double right = screen.x + screen.width * (clamped.right / 100.0);
	double bottom = screen.y + screen.height * (clamped.bottom / 100.0);
	return {left, top, right, bottom};
}

} // namespace cursorlock
