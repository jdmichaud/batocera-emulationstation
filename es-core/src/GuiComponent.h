#pragma once
#ifndef ES_CORE_GUI_COMPONENT_H
#define ES_CORE_GUI_COMPONENT_H

#include "math/Misc.h"
#include "math/Transform4x4f.h"
#include "HelpPrompt.h"
#include "HelpStyle.h"
#include "InputConfig.h"
#include <functional>
#include "ThemeData.h"
#include <memory>

class Animation;
class AnimationController;
class Font;
class InputConfig;
class ThemeData;
class Window;
class StoryboardAnimator;

namespace AnimateFlags
{
	enum Flags : unsigned int
	{
		POSITION = 1,
		SCALE = 2,
		OPACITY = 4,
		ALL = 0xFFFFFFFF
	};
}

enum class ExtraType : unsigned int
{
	BUILTIN = 0,
	EXTRA = 1,
	STATIC = 2,
	EXTRACHILDREN = 3
};

#define GetComponentScreenRect(tx, sz) Renderer::getScreenRect(tx, sz)

class GuiComponent
{
public:
	GuiComponent(Window* window);
	virtual ~GuiComponent();

	template<typename T>
	bool isKindOf() { return (dynamic_cast<T*>(this) != nullptr); }

	virtual std::string getThemeTypeName() { return "component"; }

	virtual void textInput(const char* text);

	//Called when input is received.
	//Return true if the input is consumed, false if it should continue to be passed to other children.
	virtual bool input(InputConfig* config, Input input);

	//Called when time passes.  Default implementation calls updateSelf(deltaTime) and updateChildren(deltaTime) - so you should probably call GuiComponent::update(deltaTime) at some point (or at least updateSelf so animations work).
	virtual void update(int deltaTime);

	//Called when it's time to render.  By default, just calls renderChildren(parentTrans * getTransform()).
	//You probably want to override this like so:
	//1. Calculate the new transform that your control will draw at with Transform4x4f t = parentTrans * getTransform().
	//2. Set the renderer to use that new transform as the model matrix - Renderer::setMatrix(t);
	//3. Draw your component.
	//4. Tell your children to render, based on your component's transform - renderChildren(t).
	virtual void render(const Transform4x4f& parentTrans);

	Vector3f getPosition() const;
	inline void setPosition(const Vector3f& offset) { setPosition(offset.x(), offset.y(), offset.z()); }
	void setPosition(float x, float y, float z = 0.0f);
	virtual void onPositionChanged();

	//Sets the origin as a percentage of this image (e.g. (0, 0) is top left, (0.5, 0.5) is the center)
	Vector2f getOrigin() const;
	void setOrigin(float originX, float originY);
	inline void setOrigin(Vector2f origin) { setOrigin(origin.x(), origin.y()); }
	virtual void onOriginChanged();

	//Sets the rotation origin as a percentage of this image (e.g. (0, 0) is top left, (0.5, 0.5) is the center)
	Vector2f getRotationOrigin() const;
	void setRotationOrigin(float originX, float originY);
	inline void setRotationOrigin(Vector2f origin) { setRotationOrigin(origin.x(), origin.y()); }
	virtual void onRotationOriginChanged();

	virtual Vector2f getSize() const;
	inline void setSize(const Vector2f& size) { setSize(size.x(), size.y()); }
	void setSize(float w, float h);
	virtual void onSizeChanged();

	virtual void setColor(unsigned int color) {};

	virtual Vector2f getRotationSize() const { return getSize(); };

	float getRotation() const;
	void setRotation(float rotation);
	virtual void onRotationChanged();

	inline void setRotationDegrees(float rotation) { setRotation((float)ES_DEG_TO_RAD(rotation)); }

	float getScale() const;
	virtual void setScale(float scale);
	virtual void onScaleChanged();

	Vector2f getScaleOrigin() const;
	void setScaleOrigin(const Vector2f& scaleOrigin);
	virtual void onScaleOriginChanged();

	Vector2f getScreenOffset() const;
	void setScreenOffset(const Vector2f& screenOffset);
	virtual void onScreenOffsetChanged();

	float getZIndex() const;
	void setZIndex(float zIndex);

	float getDefaultZIndex() const;
	void setDefaultZIndex(float zIndex);

	bool isVisible() const;
	void setVisible(bool visible);

	// Returns the center point of the image (takes origin into account).
	Vector2f getCenter() const;

	void setParent(GuiComponent* parent);
	GuiComponent* getParent() const;

	void addChild(GuiComponent* cmp);
	void removeChild(GuiComponent* cmp);
	void clearChildren();
	void sortChildren();
	unsigned int getChildCount() const;
	GuiComponent* getChild(unsigned int i) const;

	// animation will be automatically deleted when it completes or is stopped.
	bool isAnimationPlaying(unsigned char slot) const;
	bool isAnimationReversed(unsigned char slot) const;
	int getAnimationTime(unsigned char slot) const;
	void setAnimation(Animation* animation, int delay = 0, std::function<void()> finishedCallback = nullptr, bool reverse = false, unsigned char slot = 0);
	bool stopAnimation(unsigned char slot);
	bool cancelAnimation(unsigned char slot); // Like stopAnimation, but doesn't call finishedCallback - only removes the animation, leaving things in their current state.  Returns true if successful (an animation was in this slot).
	bool finishAnimation(unsigned char slot); // Calls update(1.f) and finishedCallback, then deletes the animation - basically skips to the end.  Returns true if successful (an animation was in this slot).
	bool advanceAnimation(unsigned char slot, unsigned int time); // Returns true if successful (an animation was in this slot).
	void stopAllAnimations();
	void cancelAllAnimations();

	virtual unsigned char getOpacity() const;
	virtual void setOpacity(unsigned char opacity);

	virtual const Transform4x4f& getTransform();

	virtual std::string getValue() const;
	virtual void setValue(const std::string& value);

	virtual void onFocusGained() {};
	virtual void onFocusLost() {};

	virtual void onShow();
	virtual void onHide();

	virtual void onScreenSaverActivate();
	virtual void onScreenSaverDeactivate();
	virtual void topWindow(bool isTop);

	// Default implementation just handles <pos> and <size> tags as normalized float pairs.
	// You probably want to keep this behavior for any derived classes as well as add your own.
	virtual void applyTheme(const std::shared_ptr<ThemeData>& theme, const std::string& view, const std::string& element, unsigned int properties);

	// Returns a list of help prompts.
	virtual std::vector<HelpPrompt> getHelpPrompts() { return std::vector<HelpPrompt>(); };

	// Called whenever help prompts change.
	void updateHelpPrompts();

	virtual HelpStyle getHelpStyle();

	void animateTo(Vector2f from, Vector2f to, unsigned int flags = 0xFFFFFFFF, int delay = 350);
	void animateTo(Vector2f from, unsigned int flags = AnimateFlags::OPACITY | AnimateFlags::SCALE, int delay = 350) { animateTo(from, from, flags, delay); }

	bool isChild(GuiComponent* cmp);

	std::string getTag() const { return mTag; };
	void setTag(const std::string& value) { mTag = value; };

	ExtraType getExtraType() { return mExtraType; }
	bool isStaticExtra() const { return mExtraType == ExtraType::STATIC; }
	void setExtraType(ExtraType value) { mExtraType = value; }

	virtual ThemeData::ThemeElement::Property getProperty(const std::string name);
	virtual void setProperty(const std::string name, const ThemeData::ThemeElement::Property& value);

	bool& isShowing() { return mShowing; }

	// Storyboards
	bool hasStoryBoard(const std::string& name = "", bool compareEmptyName = false);
	bool applyStoryboard(const ThemeData::ThemeElement* elem, const std::string name = "");
	bool selectStoryboard(const std::string& name = "");
	void deselectStoryboard(bool restoreinitialProperties = true);
	void startStoryboard();
	void pauseStoryboard();
	void stopStoryboard();
	void enableStoryboardProperty(const std::string& name, bool enable);
	bool currentStoryBoardHasProperty(const std::string& propertyName);

	bool storyBoardExists(const std::string& name = "", const std::string& propertyName = "");

	bool isStoryBoardRunning(const std::string& name = "");

	Vector4f& getClipRect() { return mClipRect; }
	virtual void setClipRect(const Vector4f& vec);

	// Mouse
	bool isMouseOver() { return mIsMouseOver; }

	virtual void onMouseLeave();
	virtual void onMouseEnter();
	virtual void onMouseMove(int x, int y);
	virtual void onMouseWheel(int delta);
	virtual bool onMouseClick(int button, bool pressed, int x, int y);

	virtual bool hitTest(int x, int y, Transform4x4f& parentTransform, std::vector<GuiComponent*>* pResult = nullptr);

	virtual bool onAction(const std::string& action);
	
	void setClickAction(const std::string& action) { mClickAction = action; }

	std::map<std::string, std::string> getBindingExpressions() { return mBindingExpressions; }

protected:
	void beginCustomClipRect();
	void endCustomClipRect();

	void renderChildren(const Transform4x4f& transform) const;
	void updateSelf(int deltaTime); // updates animations
	void updateChildren(int deltaTime); // updates animations

	void loadThemedChildren(const ThemeData::ThemeElement* elem);

	unsigned char mOpacity;
	Window* mWindow;

	GuiComponent* mParent;
	std::vector<GuiComponent*> mChildren;

	Vector3f mPosition;
	Vector2f mOrigin;
	Vector2f mRotationOrigin;
	Vector2f mSize;
	Vector2f mScaleOrigin;

	Vector2f mScreenOffset;

	std::string mStoryBoardSound;

	float mRotation = 0.0;
	float mScale = 1.0;
	float mDefaultZIndex = 0;
	float mZIndex = 0;

	bool mVisible;

	std::map<std::string, std::string> mBindingExpressions;

	bool mShowing;
	ExtraType mExtraType;

	bool mTransformDirty;
	bool mChildZIndexDirty;

	bool mIsMouseOver;

public:
	const static unsigned char MAX_ANIMATIONS = 4;
	static bool isLaunchTransitionRunning;

private:
	std::string		mClickAction;
	bool			mMousePressed;

	Transform4x4f mTransform; //Don't access this directly! Use getTransform()!
	Vector4f mClipRect;

	std::map<unsigned char, AnimationController*> mAnimationMap;
	//AnimationController* mAnimationMap[MAX_ANIMATIONS];
	
	StoryboardAnimator* mStoryboardAnimator;
	std::map<std::string, ThemeStoryboard*> mStoryBoards;

	std::string mTag;
};

#endif // ES_CORE_GUI_COMPONENT_H
