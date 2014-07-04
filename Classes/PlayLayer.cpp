#include "PlayLayer.h"

#include "extensions/cocos-ext.h"
#include "spritebuilder/SpriteBuilder.h"
#include "SimpleAudioEngine.h"

USING_NS_CC;
USING_NS_CC_EXT;
using namespace CocosDenshion;

#include "cocos2dx/ScrollBg.h"
#include "cocos2dx/util.h"

#include "util/util.h"
#include "global.h"

#define BEN_CATEGORY 0x1
#define FISH_CATEGORY 0x2

#define BEN_ANIMATE_ACTION 0x100

#define BEN_BODY_NORMAL 1
#define BEN_BODY_DOWN 2

#define BEN_ANIMATE_NORMAL 1
#define BEN_ANIMATE_JUMP 2

#define BEN_STATUS_ALIVE 1
#define BEN_STATUS_DEAD 2

Scene* PlayLayer::createScene()
{
	// 'scene' is an autorelease object
    auto scene = Scene::createWithPhysics();
	scene->getPhysicsWorld()->setGravity(Vect(0, 0));
//	scene->getPhysicsWorld()->setDebugDrawMask(PhysicsWorld::DEBUGDRAW_ALL);
	
    // 'layer' is an autorelease object
//    auto layer = PlayLayer::create();
	auto layer = PlayLayer::createFromUI();
	
	// add layer as a child to scene
    scene->addChild(layer);
	
    // return the scene
	return scene;
}

Node* PlayLayer::createFromUI()
{
	spritebuilder::NodeLoaderLibrary * ccNodeLoaderLibrary = spritebuilder::NodeLoaderLibrary::getInstance();
	ccNodeLoaderLibrary->registerNodeLoader("PlayLayer", PlayLayerLoader::loader());
	spritebuilder::CCBReader * ccbReader = new spritebuilder::CCBReader(ccNodeLoaderLibrary);
	auto node = ccbReader->readNodeGraphFromFile("PlayLayer.ccbi");
	return node;
}

void PlayLayer::onNodeLoaded(cocos2d::Node * pNode, spritebuilder::NodeLoader * pNodeLoader) {
	visibleSize = Director::getInstance()->getVisibleSize();
	
	loadSetting();
	loadScore();
	
	soundOn = atoi(drSetting["music_on"].c_str());
	
	score = 0;
	bestScore = atoi(drScore["best_score"].c_str());
	
	SimpleAudioEngine::getInstance()->preloadEffect("hit.wav");
	SimpleAudioEngine::getInstance()->preloadEffect("jump3.wav");
	SimpleAudioEngine::getInstance()->preloadEffect("dive.wav");
	SimpleAudioEngine::getInstance()->preloadBackgroundMusic("music.wav");
	
	if (soundOn == 1)
	{
		SimpleAudioEngine::getInstance()->playBackgroundMusic("music.wav", true);
	}
	
	benStatus = BEN_STATUS_ALIVE;
	
	benSpriteFrames = getSpriteFrames("ben6.png", 60, 71);
	fishs = getSpriteFrames("fish3.png", 40, 35);
	birdSpriteFrames = getSpriteFrames("bird.png", 35, 15);
	
	ben = Sprite::createWithSpriteFrame(benSpriteFrames.at(0));
	ben->setPosition(Point(100, 225));
	setBenAnimateNormal();
	this->addChild(ben);
	
	setBenBody(BEN_BODY_NORMAL);
	
	MoveTo *benMoveTo = getAvgMoveToAction(ben->getPosition(), Point(ben->getPositionX(), 135.5), 100);
	ben->runAction(benMoveTo);
	
	Vector<Sprite*> cloudBgs;
	cloudBgs.pushBack(cloudBg);
	ScrollBg *scrollBg = ScrollBg::create();
	scrollBg->addBg(cloudBgs, BG_HORIZONTAL);
	scrollBg->setPosition(0, 290);
	bgNode->addChild(scrollBg);
	scrollBg->scroll(-0.6, 0);
	
	Vector<Sprite*> middleBgs;
	middleBgs.pushBack(waveBg);
	ScrollBg *scrollMiddleBg = ScrollBg::create();
	scrollMiddleBg->addBg(middleBgs, BG_HORIZONTAL);
	scrollMiddleBg->setPosition(0, 100);
	bgNode->addChild(scrollMiddleBg);
	scrollMiddleBg->scroll(-1.2, 0);
	
	Vector<Sprite*> bottomBgs;
	bottomBgs.pushBack(seaBg);
	ScrollBg *scrollBottomBg = ScrollBg::create();
	scrollBottomBg->addBg(bottomBgs, BG_HORIZONTAL);
	scrollBottomBg->setPosition(0, 0);
	bgNode->addChild(scrollBottomBg);
	scrollBottomBg->scroll(-1.2, 0);
		
	setScoreString();
	bestLabel->setString(StringUtils::format("best: %d", bestScore));
	
	schedule(schedule_selector(PlayLayer::calcScore), 1);
	schedule(schedule_selector(PlayLayer::play), 0.5);
	
	auto touchListener = EventListenerTouchOneByOne::create();
	touchListener->onTouchBegan = CC_CALLBACK_2(PlayLayer::onTouchBegan, this);
	touchListener->onTouchMoved = CC_CALLBACK_2(PlayLayer::onTouchMoved, this);
	touchListener->onTouchEnded = CC_CALLBACK_2(PlayLayer::onTouchEnded, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(touchListener, this);
	
	auto contactListener = EventListenerPhysicsContact::create();
	contactListener->onContactBegin = CC_CALLBACK_1(PlayLayer::onContactBegin, this);
	_eventDispatcher->addEventListenerWithSceneGraphPriority(contactListener, this);
}

bool PlayLayer::onAssignCCBMemberVariable(cocos2d::Ref* pTarget, const char* pMemberVariableName, cocos2d::Node* pNode) {
	SB_MEMBERVARIABLEASSIGNER_GLUE(this, "cloudBg", Sprite*, cloudBg);
	SB_MEMBERVARIABLEASSIGNER_GLUE(this, "waveBg", Sprite*, waveBg);
	SB_MEMBERVARIABLEASSIGNER_GLUE(this, "seaBg", Sprite*, seaBg);
	
	SB_MEMBERVARIABLEASSIGNER_GLUE(this, "bgNode", Node*, bgNode);
	
	SB_MEMBERVARIABLEASSIGNER_GLUE(this, "scoreLabel", Label*, scoreLabel);
	SB_MEMBERVARIABLEASSIGNER_GLUE(this, "bestLabel", Label*, bestLabel);
	
	return false;
}

bool PlayLayer::onAssignCCBCustomProperty(cocos2d::Ref* target, const char* memberVariableName, const cocos2d::Value& value) {
	return false;
}

cocos2d::SEL_MenuHandler PlayLayer::onResolveCCBCCMenuItemSelector(cocos2d::Ref * pTarget, const char* pSelectorName) {
	return NULL;
}

cocos2d::SEL_CallFuncN PlayLayer::onResolveCCBCCCallFuncSelector(cocos2d::Ref * pTarget, const char* pSelectorName) {
	return NULL;
}

cocos2d::extension::Control::Handler PlayLayer::onResolveCCBCCControlSelector(cocos2d::Ref * pTarget, const char* pSelectorName) {
	CCB_SELECTORRESOLVER_CCCONTROL_GLUE(this, "selector", PlayLayer::selector);
	return NULL;
}

void PlayLayer::selector(cocos2d::Ref * sender, cocos2d::extension::Control::EventType pControlEvent) {
	CCLOG("on play %d", rand());
}

void PlayLayer::calcScore(float dt)
{
	if (benStatus == BEN_STATUS_DEAD)
	{
		return;
	}
	
	score += 1;
	
	setScoreString();
	
//	spritebuilder::NodeLoaderLibrary * ccNodeLoaderLibrary = spritebuilder::NodeLoaderLibrary::getInstance();
//	ccNodeLoaderLibrary->registerNodeLoader("PlayLayer", PlayLayerLoader::loader());
//	spritebuilder::CCBReader * ccbReader = new spritebuilder::CCBReader(ccNodeLoaderLibrary);
//	Node *particle = ccbReader->readNodeGraphFromFile("Pixel.ccbi");
//	ben->addChild(particle);
}

void PlayLayer::resetBen(float dt)
{
	ben->setRotation(0);
	ben->setPosition(Point(100, 225));
	setBenAnimateNormal();
	
	ben->setVisible(true);
	setBenBody(BEN_BODY_NORMAL);
	
	benStatus = BEN_STATUS_ALIVE;
		
	MoveTo *benMoveTo = getAvgMoveToAction(ben->getPosition(), Point(ben->getPositionX(), 135.5), 100);
	ben->runAction(benMoveTo);
}

void PlayLayer::play(float dt)
{
	int rno = rand() % 12;
	if (rno == 3)
	{
//		Vector<SpriteFrame*> vectorFrames;
//		vectorFrames.pushBack(fishs.at(0));
//		vectorFrames.pushBack(fishs.at(1));
//		vectorFrames.pushBack(fishs.at(2));
//		
//		Animate *fishAnimate = getFrameAnimation(vectorFrames, 0.3f);
//		
//		Sprite *fish = Sprite::createWithSpriteFrame(vectorFrames.at(0));
//		setFishBody(fish);
//		fish->setPosition(Point(visibleSize.width, 165));
//		fish->runAction(RepeatForever::create(fishAnimate));
//		this->addChild(fish);
//		
//		setBenBody(BEN_BODY_DOWN);
//		
//		MoveTo *moveTo = getAvgMoveToAction(fish->getPosition(), Point(-fish->getContentSize().width, fish->getPositionY()), 150);
//		Sequence *seq = Sequence::create(moveTo, CallFuncN::create(CC_CALLBACK_1(PlayLayer::cleanNode, this)), NULL);
//		fish->runAction(seq);
	}
	else if (rno == 5)
	{
		if (!ben->isVisible())
		{
			return;
		}
		
		Vector<SpriteFrame*> vectorFrames;
		vectorFrames.pushBack(fishs.at(3));
		vectorFrames.pushBack(fishs.at(4));
		
		Animate *fishAnimate = getFrameAnimation(vectorFrames, 0.3f);
		
		Sprite *fish = Sprite::createWithSpriteFrame(vectorFrames.at(0));
		setFishBody(fish);
		fish->setPosition(Point(visibleSize.width, 117.5));
		fish->runAction(RepeatForever::create(fishAnimate));
		this->addChild(fish);
		
		setBenBody(BEN_BODY_NORMAL);
		
		MoveTo *moveTo = getAvgMoveToAction(fish->getPosition(), Point(-fish->getContentSize().width, fish->getPositionY()), 200);
		Sequence *seq = Sequence::create(moveTo, CallFuncN::create(CC_CALLBACK_1(PlayLayer::cleanNode, this)), NULL);
		fish->runAction(seq);
	}
	else if (rno == 7)
	{
		Vector<SpriteFrame*> birdAnimateFrames;
		birdAnimateFrames.pushBack(birdSpriteFrames.at(0));
		birdAnimateFrames.pushBack(birdSpriteFrames.at(1));
		birdAnimateFrames.pushBack(birdSpriteFrames.at(2));
		Animate *birdAnimate = getFrameAnimation(birdAnimateFrames, 0.3f);
		
		Sprite *bird = Sprite::createWithSpriteFrame(birdAnimateFrames.at(0));
		bird->setPosition(Point(200, 150));
		bird->runAction(RepeatForever::create(birdAnimate));
		this->addChild(bird);
		
		int moveToX = genRandom(0, visibleSize.width);
		MoveTo *moveTo = getAvgMoveToAction(bird->getPosition(), Point(moveToX, visibleSize.height), 200);
		Sequence *seq = Sequence::create(moveTo, CallFuncN::create(CC_CALLBACK_1(PlayLayer::cleanNode, this)), NULL);
		bird->runAction(seq);
	}
}

void PlayLayer::setFishBody(Sprite *fish)
{
	PhysicsBody *fishBody = PhysicsBody::createBox(Size(40, 35));
	fishBody->setCategoryBitmask(FISH_CATEGORY);
	fishBody->setContactTestBitmask(BEN_CATEGORY);
	fishBody->setCollisionBitmask(0);
	fish->setPhysicsBody(fishBody);
}

void PlayLayer::setScoreString()
{
	scoreLabel->setString(StringUtils::format("score: %d", score));
}

void PlayLayer::cleanNode(cocos2d::Node *node)
{
	node->removeFromParentAndCleanup(true);
}

void PlayLayer::setBenBody(int shapeType)
{
	PhysicsShape *shape;
	
	if (shapeType == BEN_BODY_NORMAL)
	{
		shape = PhysicsShapeBox::create(Size(65, 71));
	}
	else if (shapeType == BEN_BODY_DOWN)
	{
		Point points[] = {Point(-32.5, -35.5), Point(-32.5, 0), Point(32.5, 0), Point(32.5, -35.5)};
		shape = PhysicsShapePolygon::create(points, 4);
	}
	
	shape->setCategoryBitmask(BEN_CATEGORY);
	shape->setContactTestBitmask(FISH_CATEGORY);
	shape->setCollisionBitmask(0);
	shape->setMass(0);
	shape->setFriction(0);
	shape->setRestitution(0);
	
	resetPhysicsBody(ben, shape);
}

void PlayLayer::setBenAnimateNormal()
{
	setBenAnimate(BEN_ANIMATE_NORMAL);
}

void PlayLayer::setBenAnimate(int animateType)
{
	ben->stopAllActions();
	
	Animate *benAnimate;
	
	Vector<SpriteFrame*> benAnimateFrames;
	
	if (animateType == BEN_ANIMATE_JUMP)
	{
		benAnimateFrames.pushBack(benSpriteFrames.at(10));
		benAnimateFrames.pushBack(benSpriteFrames.at(11));
		
	}
	else if (animateType == BEN_ANIMATE_NORMAL)
	{
		benAnimateFrames.pushBack(benSpriteFrames.at(0));
		benAnimateFrames.pushBack(benSpriteFrames.at(1));
		benAnimateFrames.pushBack(benSpriteFrames.at(2));
		benAnimateFrames.pushBack(benSpriteFrames.at(3));
		benAnimateFrames.pushBack(benSpriteFrames.at(4));
	}
	
	benAnimate = getFrameAnimation(benAnimateFrames, 0.2);
	benAnimate->setTag(BEN_ANIMATE_ACTION);
	ben->runAction(RepeatForever::create(benAnimate));
}

void PlayLayer::hideBen()
{
	ben->setVisible(false);
	
	scheduleOnce(schedule_selector(PlayLayer::resetBen), 1.5);
}

// on "init" you need to initialize your instance
//bool PlayLayer::init()
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

//void PlayLayer::menuCloseCallback(Ref* pSender)
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

bool PlayLayer::onTouchBegan(cocos2d::Touch *touch, cocos2d::Event *event)
{
	touchBeginPoint = touch->getLocation();
	return true;
}

void PlayLayer::onTouchMoved(cocos2d::Touch *touch, cocos2d::Event *event)
{
}

void PlayLayer::onTouchEnded(cocos2d::Touch *touch, cocos2d::Event *event)
{
	if (benStatus == BEN_STATUS_DEAD)
	{
		return;
	}
	
	Point touchPoint = touch->getLocation();
	
	float distince = touchPoint.y - touchBeginPoint.y;
	
	if (distince > 40)
	{
		if (ben->getPositionY() == 135.5)
		{
			if (soundOn == 1)
			{
				SimpleAudioEngine::getInstance()->playEffect("jump3.wav");
			}
			
			setBenAnimate(BEN_ANIMATE_JUMP);
			
			MoveTo *benJumpUp = getAvgMoveToAction(ben->getPosition(), Point(ben->getPositionX(), 225), 100);
			MoveTo *benJumpDown = getAvgMoveToAction(Point(ben->getPositionX(), 225), Point(ben->getPositionX(), 135.5), 100);
			ben->runAction(Sequence::create(benJumpUp, benJumpDown, CallFunc::create(CC_CALLBACK_0(PlayLayer::setBenAnimateNormal, this)), NULL));
		}
	}
}

bool PlayLayer::onContactBegin(const cocos2d::PhysicsContact &contact)
{
	if (benStatus == BEN_STATUS_DEAD)
	{
		return false;
	}
	
	benStatus = BEN_STATUS_DEAD;
	
	auto shapea = contact.getShapeA();
	auto shapeb = contact.getShapeB();
	
	if ((shapea->getCategoryBitmask() == BEN_CATEGORY))
	{
		shapeb->getBody()->setEnable(false);
		
	}
	else
	{
		shapea->getBody()->setEnable(false);
	}
	
	if (score > bestScore)
	{
		bestScore = score;
	}
	saveScore(bestScore);
	bestLabel->setString(StringUtils::format("best: %d", bestScore));
	
	score = 0;
	setScoreString();
	
	if (soundOn == 1)
	{
		SimpleAudioEngine::getInstance()->playEffect("hit.wav");
	}
	ben->setRotation(-180);
//	RotateTo *rotateTo = RotateTo::create(1, 30);
	ben->stopAllActions();
	MoveTo *moveDown = getAvgMoveToAction(ben->getPosition(), Point(ben->getPositionX(), 0), 200);
	ben->runAction(Sequence::create(moveDown, CallFunc::create(CC_CALLBACK_0(PlayLayer::hideBen, this)), NULL));
	
	return false;
}
