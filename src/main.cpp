#include <Geode/Geode.hpp>
#include <Geode/modify/MenuLayer.hpp>
#include <Geode/modify/PlayLayer.hpp>
#include <Geode/modify/PauseLayer.hpp>

#include <functional>
#include <memory>
#include <string>

#include "CursorLock.hpp"

using namespace geode::prelude;
using namespace cursorlock;

namespace {
class CursorLockManager : public CCNode {
public:
	static CursorLockManager* get() {
		static CursorLockManager* s_instance = nullptr;
		if (!s_instance) {
			s_instance = new CursorLockManager();
			if (s_instance && s_instance->init()) {
				s_instance->retain();
				CCDirector::sharedDirector()->getScheduler()->scheduleUpdateForTarget(s_instance, 0, false);
			}
		}
		return s_instance;
	}

	bool init() override {
		if (!CCNode::init()) {
			return false;
		}
		m_api = createCursorLockAPI();
		return true;
	}

	void update(float) override {
		if (m_enabled && m_api) {
			applyToAPI(); // re-apply in case the platform clears the clip when pausing/resuming
			m_api->tick();
		}
	}

	void setBounds(PercentBounds bounds) {
		m_bounds = clampBounds(bounds);
		if (m_enabled) {
			applyToAPI();
		}
	}

	[[nodiscard]] PercentBounds getBounds() const {
		return m_bounds;
	}

	void activate() {
		m_enabled = true;
		applyToAPI();
	}

	void deactivate() {
		m_enabled = false;
		if (m_api) {
			m_api->release();
		}
	}

	[[nodiscard]] bool isActive() const {
		return m_enabled;
	}

private:
	void applyToAPI() {
		if (!m_api) {
			return;
		}
		if (m_bounds.right <= m_bounds.left || m_bounds.bottom <= m_bounds.top) {
			log::warn("Cursor lock bounds are invalid, skipping apply.");
			return;
		}
		m_api->applyBounds(percentToPixels(m_bounds));
	}

	std::unique_ptr<CursorLockAPI> m_api;
	PercentBounds m_bounds{};
	bool m_enabled = false;
};

class BoundsOptionsLayer : public CCLayerColor, public TextInputDelegate {
public:
	static BoundsOptionsLayer* create(std::function<void(PercentBounds)> onApply) {
		auto ret = new BoundsOptionsLayer();
		if (ret->init(onApply)) {
			ret->autorelease();
			return ret;
		}
		CC_SAFE_DELETE(ret);
		return nullptr;
	}

	bool init(std::function<void(PercentBounds)> onApply) {
		if (!CCLayerColor::initWithColor({0, 0, 0, 125})) {
			return false;
		}
		m_onApply = std::move(onApply);
		setTouchEnabled(true);
		setKeypadEnabled(true);

		auto size = CCDirector::sharedDirector()->getWinSize();
		auto panel = CCScale9Sprite::create("GJ_square01.png");
		panel->setContentSize({360.f, 220.f});
		panel->setPosition({size.width / 2, size.height / 2});
		addChild(panel, 1);

		addLabel("Cursor Lock Bounds (%)", {0, 70}, panel);
		addLabel("Top-Left X", {-120, 30}, panel);
		addLabel("Top-Left Y", {20, 30}, panel);
		addLabel("Bottom-Right X", {-120, -30}, panel);
		addLabel("Bottom-Right Y", {20, -30}, panel);

		auto bounds = CursorLockManager::get()->getBounds();
		m_left = createInput(std::to_string(bounds.left), {-30, 30}, panel);
		m_top = createInput(std::to_string(bounds.top), {110, 30}, panel);
		m_right = createInput(std::to_string(bounds.right), {-30, -30}, panel);
		m_bottom = createInput(std::to_string(bounds.bottom), {110, -30}, panel);

		auto okSprite = ButtonSprite::create("Apply", 0, false, "goldFont.fnt", "GJ_button_01.png", 30, 0.7f);
		auto okItem = CCMenuItemSpriteExtra::create(okSprite, this, menu_selector(BoundsOptionsLayer::onApply));
		auto closeSprite = ButtonSprite::create("Close", 0, false, "goldFont.fnt", "GJ_button_06.png", 30, 0.7f);
		auto closeItem = CCMenuItemSpriteExtra::create(closeSprite, this, menu_selector(BoundsOptionsLayer::onClose));

		auto menu = CCMenu::create();
		menu->addChild(okItem);
		menu->addChild(closeItem);
		menu->alignItemsHorizontallyWithPadding(40);
		menu->setPosition({size.width / 2, size.height / 2 - 80});
		addChild(menu, 2);

		return true;
	}

	void keyBackClicked() override {
		removeFromParent();
	}

private:
	void addLabel(std::string const& text, CCPoint offset, CCNode* parent) {
		auto label = CCLabelBMFont::create(text.c_str(), "goldFont.fnt");
		label->setScale(0.6f);
		auto size = parent->getContentSize();
		label->setPosition({size.width / 2 + offset.x, size.height / 2 + offset.y});
		parent->addChild(label, 2);
	}

	CCTextInputNode* createInput(std::string const& value, const CCPoint& offset, CCNode* parent) {
		auto input = CCTextInputNode::create(90, 34, "0-100", "bigFont.fnt", 20, "Thonburi");
		input->setDelegate(this);
		input->setAllowedChars("0123456789.");
		input->setString(value.c_str());
		auto size = parent->getContentSize();
		input->setPosition({size.width / 2 + offset.x, size.height / 2 + offset.y});
		parent->addChild(input, 2);
		return input;
	}

	static double parseOrDefault(CCTextInputNode* node, double fallback) {
		try {
			return std::stod(node->getString());
		} catch (...) {
			return fallback;
		}
	}

	void onApply(CCObject*) {
		PercentBounds bounds;
		bounds.left = parseOrDefault(m_left, 25.0);
		bounds.top = parseOrDefault(m_top, 25.0);
		bounds.right = parseOrDefault(m_right, 75.0);
		bounds.bottom = parseOrDefault(m_bottom, 75.0);

		if (bounds.right <= bounds.left || bounds.bottom <= bounds.top) {
			FLAlertLayer::create("Cursor Lock", "Right/Bottom must be larger than Left/Top.", "OK")->show();
			return;
		}

		if (m_onApply) {
			m_onApply(bounds);
		}
		removeFromParent();
	}

	void onClose(CCObject*) {
		removeFromParent();
	}

	CCTextInputNode* m_left = nullptr;
	CCTextInputNode* m_top = nullptr;
	CCTextInputNode* m_right = nullptr;
	CCTextInputNode* m_bottom = nullptr;
	std::function<void(PercentBounds)> m_onApply;
};

CCMenuItemSpriteExtra* makeConfigButton(CCObject* target, SEL_MenuHandler selector) {
	auto icon = CCSprite::createWithSpriteFrameName("GJ_optionsBtn_001.png");
	auto btn = CCMenuItemSpriteExtra::create(icon, target, selector);
	btn->setID("cursor-lock-config"_spr);
	btn->setEnabled(true);
	return btn;
}
} // namespace

class $modify(MyMenuLayer, MenuLayer) {
	bool init() {
		if (!MenuLayer::init()) {
			return false;
		}

		// Ensure manager exists so bounds persist.
		CursorLockManager::get();

		auto configButton = makeConfigButton(this, menu_selector(MyMenuLayer::onConfig));

		if (auto menu = this->getChildByID("bottom-menu")) {
			menu->addChild(configButton);
			menu->updateLayout();
		}

		return true;
	}

	void onConfig(CCObject*) {
		auto layer = BoundsOptionsLayer::create([](PercentBounds bounds) {
			if (auto manager = CursorLockManager::get()) {
				manager->setBounds(bounds);
			}
		});

		if (auto scene = CCDirector::sharedDirector()->getRunningScene()) {
			scene->addChild(layer, 1000);
		}
	}
};

class $modify(MyPlayLayer, PlayLayer) {
	bool init(GJGameLevel* level, bool useReplay, bool dontCreateObjects) {
		if (!PlayLayer::init(level, useReplay, dontCreateObjects)) {
			return false;
		}

		if (auto manager = CursorLockManager::get()) {
			manager->activate();
		}
		return true;
	}

	void onExit() override {
		if (auto manager = CursorLockManager::get()) {
			manager->deactivate();
		}
		PlayLayer::onExit();
	}
};

class $modify(MyPauseLayer, PauseLayer) {
	void customSetup() {
		PauseLayer::customSetup();

		// Add a config button to pause menu so settings are reachable in-level.
		auto btn = makeConfigButton(this, menu_selector(MyPauseLayer::onConfig));
		auto menu = CCMenu::create();
		menu->addChild(btn);
		auto size = CCDirector::sharedDirector()->getWinSize();
		menu->setPosition({size.width - 35.f, size.height - 35.f});
		addChild(menu, 20);
	}

	void onConfig(CCObject*) {
		auto layer = BoundsOptionsLayer::create([](PercentBounds bounds) {
			if (auto manager = CursorLockManager::get()) {
				manager->setBounds(bounds);
				if (manager->isActive()) {
					// immediately re-apply when adjusting mid-level
					manager->activate();
				}
			}
		});

		if (auto scene = CCDirector::sharedDirector()->getRunningScene()) {
			scene->addChild(layer, 1000);
		}
	}

	void onResume(CCObject* sender) {
		if (auto manager = CursorLockManager::get()) {
			manager->activate();
		}
		PauseLayer::onResume(sender);
	}
};
