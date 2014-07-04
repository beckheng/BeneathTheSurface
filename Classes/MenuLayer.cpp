#include "MenuLayer.h"

#include "extensions/cocos-ext.h"
#include "spritebuilder/SpriteBuilder.h"

USING_NS_CC;
USING_NS_CC_EXT;

#include "PlayLayer.h"
#include "cocos2dx/util.h"

#include "global.h"

Scene* MenuLayer::createScene()
{
	// 'scene' is an autorelease object
    auto scene = Scene::create();
	
    // 'layer' is an autorelease object
//    auto layer = MenuLayer::create();
	auto layer = MenuLayer::createFromUI();
	
	// add layer as a child to scene
    scene->addChild(layer);
	
    // return the scene
	return scene;
}

Node* MenuLayer::createFromUI()
{
	spritebuilder::NodeLoaderLibrary * ccNodeLoaderLibrary = spritebuilder::NodeLoaderLibrary::getInstance();
	ccNodeLoaderLibrary->registerNodeLoader("MenuLayer", MenuLayerLoader::loader());
	spritebuilder::CCBReader * ccbReader = new spritebuilder::CCBReader(ccNodeLoaderLibrary);
	auto node = ccbReader->readNodeGraphFromFile("MenuLayer.ccbi");
	return node;
}

void MenuLayer::onNodeLoaded(cocos2d::Node * pNode, spritebuilder::NodeLoader * pNodeLoader) {
	soundOn = 1;
	
	initDb();
	loadSetting();
	
	soundOn = atoi(drSetting["music_on"].c_str());
	
	soundSpriteFrames = getSpriteFrames("sound.png", 28, 22);
	
	setSoundSwitchSprite();
	
	auto touchListener = EventListenerTouchOneByOne::create();
	touchListener->onTouchBegan = CC_CALLBACK_2(MenuLayer::onTouchBegan, this);
	touchListener->onTouchMoved = CC_CALLBACK_2(MenuLayer::onTouchMoved, this);
	touchListener->onTouchEnded = CC_CALLBACK_2(MenuLayer::onTouchEnded, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(touchListener, this);
}

bool MenuLayer::onAssignCCBMemberVariable(cocos2d::Ref* pTarget, const char* pMemberVariableName, cocos2d::Node* pNode) {
	SB_MEMBERVARIABLEASSIGNER_GLUE(this, "soundSwitchButton", ControlButton*, soundSwitchButton);
	return false;
}

bool MenuLayer::onAssignCCBCustomProperty(cocos2d::Ref* target, const char* memberVariableName, const cocos2d::Value& value) {
	return false;
}

cocos2d::SEL_MenuHandler MenuLayer::onResolveCCBCCMenuItemSelector(cocos2d::Ref * pTarget, const char* pSelectorName) {
	return NULL;
}

cocos2d::SEL_CallFuncN MenuLayer::onResolveCCBCCCallFuncSelector(cocos2d::Ref * pTarget, const char* pSelectorName) {
	return NULL;
}

cocos2d::extension::Control::Handler MenuLayer::onResolveCCBCCControlSelector(cocos2d::Ref * pTarget, const char* pSelectorName) {
	CCB_SELECTORRESOLVER_CCCONTROL_GLUE(this, "soundSwitch", MenuLayer::soundSwitch);
	return NULL;
}

void MenuLayer::soundSwitch(cocos2d::Ref * sender, cocos2d::extension::Control::EventType pControlEvent) {
	if (soundOn == 1)
	{
		soundOn = 0;
	}
	else
	{
		soundOn = 1;
	}
	
	saveSetting(soundOn);
	setSoundSwitchSprite();
}

void MenuLayer::setSoundSwitchSprite()
{
	if (soundOn)
	{
		soundSwitchButton->setBackgroundSpriteFrameForState(soundSpriteFrames.at(0), Control::State::NORMAL);
		soundSwitchButton->setBackgroundSpriteFrameForState(soundSpriteFrames.at(0), Control::State::HIGH_LIGHTED);
		soundSwitchButton->setBackgroundSpriteFrameForState(soundSpriteFrames.at(0), Control::State::SELECTED);
		soundSwitchButton->setBackgroundSpriteFrameForState(soundSpriteFrames.at(0), Control::State::DISABLED);
	}
	else
	{
		soundSwitchButton->setBackgroundSpriteFrameForState(soundSpriteFrames.at(1), Control::State::NORMAL);
		soundSwitchButton->setBackgroundSpriteFrameForState(soundSpriteFrames.at(1), Control::State::HIGH_LIGHTED);
		soundSwitchButton->setBackgroundSpriteFrameForState(soundSpriteFrames.at(1), Control::State::SELECTED);
		soundSwitchButton->setBackgroundSpriteFrameForState(soundSpriteFrames.at(1), Control::State::DISABLED);
	}
}

// on "init" you need to initialize your instance
//bool MenuLayer::init()
//{
//    //////////////////////////////
//    // 1. super init first
//    if ( !Layer::init() )
//    {
//        return false;
//    }
//
//    Size visibleSize = Director::getInstance()->getVisibleSize();
//    Point origin = Director::getInstance()->getVisibleOrigin();
//
//    return true;
//}

//void MenuLayer::menuCloseCallback(Ref* pSender)
//{
//#if (CC_TARGET_PLATFORM == CC_PLATFORM_WP8) || (CC_TARGET_PLATFORM == CC_PLATFORM_WINRT)
//	MessageBox("You pressed the close button. Windows Store Apps do not implement a close button.","Alert");
//    return;
//#endif
//
//    Director::getInstance()->end();
//
//#if (CC_TARGET_PLATFORM == CC_PLATFORM_IOS)
//    exit(0);
//#endif
//}

bool MenuLayer::onTouchBegan(cocos2d::Touch *touch, cocos2d::Event *event)
{
	Point convertedLocation = touch->getLocation();
	if (!soundSwitchButton->getBoundingBox().containsPoint(convertedLocation))
	{
		auto scene = PlayLayer::createScene();
		Director::getInstance()->replaceScene(scene);
	}
	
	return true;
}

void MenuLayer::onTouchMoved(cocos2d::Touch *touch, cocos2d::Event *event)
{
}

void MenuLayer::onTouchEnded(cocos2d::Touch *touch, cocos2d::Event *event)
{
}
