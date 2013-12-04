#ifndef _TEST_H_
#define _TEST_H_

#include "cocos2d.h"
#include <algorithm>
using namespace cocos2d;

class TestLayer : public cocos2d::CCLayer
{
public:
	TestLayer(){};
	virtual ~TestLayer(){};
	bool init();
	CREATE_FUNC(TestLayer);
	static cocos2d::CCScene* scene();
private:
	void test();
};

bool TestLayer::init()
{
	bool bRet = false;
	do 
	{
		CC_BREAK_IF(! CCLayer::init());

		CCTMXTiledMap* map = CCTMXTiledMap::create("TileMap.tmx");
		CC_BREAK_IF(! map);
		this->addChild(map);
		CCTMXLayer* mapBg = map->layerNamed("Background");

		CCSize winSize = CCDirector::sharedDirector()->getWinSize();
		CCSprite* player = CCSprite::create("Player.png");
		player->setPosition( ccp(30, winSize.height/2) );
		this->addChild(player);

		CCSprite* enimy = CCSprite::create("Target.png");
		enimy->setPosition(ccp(winSize.width/2, winSize.height/2));
		this->addChild(enimy);

		CCActionInterval* move = CCMoveBy::create(6, ccp(
			mapBg->getLayerSize().width * mapBg->getMapTileSize().width + 300, 
			mapBg->getLayerSize().height * mapBg->getMapTileSize().height + 200) );
		CCActionInterval* move_back = move->reverse();
		CCSequence* seq = CCSequence::create(move, move_back, NULL);
		CCAction* repeat = CCRepeatForever::create(seq);
		player->runAction(repeat);

		CCFollow* follow = CCFollow::create(player, CCRectMake(
			0, 0, 
			mapBg->getLayerSize().width * mapBg->getMapTileSize().width,
			mapBg->getLayerSize().height * mapBg->getMapTileSize().height));
		this->runAction(follow);

		bRet = true;
	} while (0);
	return bRet;
}

CCScene* TestLayer::scene()
{
	CCScene * scene = NULL;
	do 
	{
		// 'scene' is an autorelease object
		scene = CCScene::create();
		CC_BREAK_IF(! scene);

		// 'layer' is an autorelease object
		TestLayer *layer = TestLayer::create();
		CC_BREAK_IF(! layer);
		scene->addChild(layer);

		MenuLayer* menu = MenuLayer::create();
		CC_BREAK_IF(!menu);
		scene->addChild(menu);
	} while (0);

	// return the scene
	return scene;
}

void TestLayer::test()
{
}

#endif // _TEST_H_
