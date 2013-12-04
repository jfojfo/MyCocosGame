#ifndef __HELLOWORLD_SCENE_H__
#define __HELLOWORLD_SCENE_H__

#include "cocos2d.h"
#include "SimpleAudioEngine.h"
#include "SimpleDPad.h"
#include "HudLayer.h"

#define COCOS2D_DEBUG 1

class HelloWorld : public cocos2d::CCLayer, public SimpleDPadDelegate
{
public:
	HelloWorld();
	~HelloWorld();

	// Here's a difference. Method 'init' in cocos2d-x returns bool, 
    // instead of returning 'id' in cocos2d-iphone
	virtual bool init();  

	// there's no 'id' in cpp, so we recommand to return the exactly class pointer
	static cocos2d::CCScene* scene();

	// implement the "static node()" method manually
	CREATE_FUNC(HelloWorld);

	void update(float dt);

	void registerWithTouchDispatcher();
	void ccTouchesEnded(cocos2d::CCSet* touches, cocos2d::CCEvent* event);
    
	virtual void didChangeDirectionTo(SimpleDPad *simpleDPad, cocos2d::CCPoint direction);
	virtual void isHoldingDirection(SimpleDPad *simpleDPad, cocos2d::CCPoint direction);
	virtual void simpleDPadTouchEnded(SimpleDPad *simpleDPad);

private:
	void spriteMoveFinished(cocos2d::CCNode* sender);
	void setViewpointCenter(cocos2d::CCPoint position);
	void setPlayerPosition(cocos2d::CCPoint position);
	cocos2d::CCPoint tileCoordForPosition(cocos2d::CCPoint position);
	void addEnemy(int x, int y);
	void filterVisibleSprites();
	bool isSpriteInViewPort(cocos2d::CCSprite* sprite);
	void moveEnemy(cocos2d::CCSprite* enemy);
	void checkHit();
	void checkMoveEnemy();
	void gameOver();
	void updatePlayerPosition(cocos2d::CCPoint dir);
	void sendProjectile(cocos2d::CCPoint dir);

	cocos2d::CCTMXTiledMap* m_tileMap;
	cocos2d::CCTMXLayer* m_meta;
	cocos2d::CCTMXLayer* m_foreground;
	cocos2d::CCSprite* m_player;
	cocos2d::CCArray *m_enemies, *m_visibleEnemies;
	cocos2d::CCArray *m_projectiles, *m_visibleProjectiles;
	CC_SYNTHESIZE(HudLayer*, m_hud, Hud);
};


class MenuLayer : public cocos2d::CCLayer
{
public:
	MenuLayer(){};
	virtual ~MenuLayer(){};
	bool init();
	CREATE_FUNC(MenuLayer);
protected:
	void menuCloseCallback(CCObject* pSender);
};

#endif  // __HELLOWORLD_SCENE_H__