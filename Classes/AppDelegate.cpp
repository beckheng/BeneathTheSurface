#include "AppDelegate.h"
#include "HelloWorldScene.h"

#include "extensions/cocos-ext.h"
#include "spritebuilder/SpriteBuilder.h"

#include "SimpleAudioEngine.h"

#include "MenuLayer.h"

USING_NS_CC;
using namespace CocosDenshion;

AppDelegate::AppDelegate() {

}

AppDelegate::~AppDelegate() 
{
}

bool AppDelegate::applicationDidFinishLaunching() {
    // initialize director
    auto director = Director::getInstance();
    auto glview = director->getOpenGLView();
    if(!glview) {
        glview = GLView::create("My Game");
        director->setOpenGLView(glview);
    }

    // turn on display FPS
//    director->setDisplayStats(true);

    // set FPS. the default value is 1.0/60 if you don't call this
    director->setAnimationInterval(1.0 / 60);
	
	Size designSize  = Size(650, 390);
	Size size = director->getWinSize();
	float scaleFactor = size.height / designSize.height;
	glview->setDesignResolutionSize(designSize.width, designSize.height, ResolutionPolicy::EXACT_FIT);
	director->setContentScaleFactor(scaleFactor / (size.height / 390)); //because the current resource is phonehd's
	spritebuilder::CCBReader::setupSpriteBuilder("resources-phone", 1);

    // create a scene. it's an autorelease object
    auto scene = MenuLayer::createScene();

    // run
    director->runWithScene(scene);

    return true;
}

// This function will be called when the app is inactive. When comes a phone call,it's be invoked too
void AppDelegate::applicationDidEnterBackground() {
    Director::getInstance()->stopAnimation();

    // if you use SimpleAudioEngine, it must be pause
    SimpleAudioEngine::sharedEngine()->pauseBackgroundMusic();
}

// this function will be called when the app is active again
void AppDelegate::applicationWillEnterForeground() {
    Director::getInstance()->startAnimation();

    // if you use SimpleAudioEngine, it must resume here
    SimpleAudioEngine::sharedEngine()->resumeBackgroundMusic();
}
