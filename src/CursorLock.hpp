#pragma once

#include <Geode/Geode.hpp>
#include <memory>

namespace cursorlock {

struct PercentBounds {
	double left = 25.0;
	double top = 25.0;
	double right = 75.0;
	double bottom = 75.0;
};

struct PixelRect {
	double left;
	double top;
	double right;
	double bottom;
};

struct ScreenRect {
	double x;
	double y;
	double width;
	double height;
};

class CursorLockAPI {
public:
	virtual ~CursorLockAPI() = default;
	virtual void applyBounds(PixelRect const& rect) = 0;
	virtual void release() = 0;
	virtual void tick() {}
};

PercentBounds clampBounds(PercentBounds const& bounds);
ScreenRect getPrimaryScreenRect();
PixelRect percentToPixels(PercentBounds const& pct);
std::unique_ptr<CursorLockAPI> createCursorLockAPI();

} // namespace cursorlock
