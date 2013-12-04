#include "HelloWorldScene.h"
#include "GameOverScene.h"
#include "SimpleAudioEngine.h"
#include <algorithm>

using namespace cocos2d;

bool MenuLayer::init()
{
	bool bRet = false;

	do {
		// 1. Add a menu item with "X" image, which is clicked to quit the program.

		// Create a "close" menu item with close icon, it's an auto release object.
		CCMenuItemImage *pCloseItem = CCMenuItemImage::create(
			"CloseNormal.png",
			"CloseSelected.png",
			this,
			menu_selector(MenuLayer::menuCloseCallback));
		CC_BREAK_IF(! pCloseItem);

		// Place the menu item bottom-right conner.
		CCSize visibleSize = CCDirector::sharedDirector()->getVisibleSize();
		CCPoint origin = CCDirector::sharedDirector()->getVisibleOrigin();

		pCloseItem->setPosition(ccp(
			origin.x + visibleSize.width - pCloseItem->getContentSize().width/2,
			origin.y + pCloseItem->getContentSize().height/2));

		// Create a menu with the "close" menu item, it's an auto release object.
		CCMenu* pMenu = CCMenu::create(pCloseItem, NULL);
		pMenu->setPosition(CCPointZero);
		CC_BREAK_IF(! pMenu);

		// Add the menu to HelloWorld layer as a child layer.
		this->addChild(pMenu, 1);

		bRet = true;
	} while (0);

	return bRet;
}

void MenuLayer::menuCloseCallback(CCObject* pSender)
{
	// "close" menu item clicked
	CCDirector::sharedDirector()->end();
}



HelloWorld::~HelloWorld()
{
	if (m_enemies) {
		m_enemies->release();
		m_enemies = NULL;
	}
	if (m_projectiles) {
		m_projectiles->release();
		m_projectiles = NULL;
	}
	if (m_visibleEnemies) {
		m_visibleEnemies->release();
		m_visibleEnemies = NULL;
	}
	if (m_visibleProjectiles) {
		m_visibleProjectiles->release();
		m_visibleProjectiles = NULL;
	}
	this->unscheduleUpdate();
	CocosDenshion::SimpleAudioEngine::sharedEngine()->stopBackgroundMusic(true);

	// cpp don't need to call super dealloc
	// virtual destructor will do this
}

HelloWorld::HelloWorld()
{
}

CCScene* HelloWorld::scene()
{
	CCScene * scene = NULL;
	do 
	{
		// 'scene' is an autorelease object
		scene = CCScene::create();
		CC_BREAK_IF(! scene);

		// 'layer' is an autorelease object
		HelloWorld *game = HelloWorld::create();
		CC_BREAK_IF(! game);
		scene->addChild(game);

		HudLayer *hud = HudLayer::create();
		CC_BREAK_IF(! hud);
		scene->addChild(hud);

		game->setHud(hud);
		hud->getDPad()->setDelegate(game);

		MenuLayer* menu = MenuLayer::create();
		CC_BREAK_IF(!menu);
		scene->addChild(menu);
	} while (0);

	// return the scene
	return scene;
}

// on "init" you need to initialize your instance
bool HelloWorld::init()
{
	bool bRet = false;
	do 
	{
		CC_BREAK_IF(! CCLayer::init() );

		m_tileMap = CCTMXTiledMap::create("TileMap.tmx");
		CC_BREAK_IF(! m_tileMap);
		this->addChild(m_tileMap);

		CCTMXLayer* layer = m_tileMap->layerNamed("Background");

		CCTMXObjectGroup *objects = m_tileMap->objectGroupNamed("Objects");
		CCAssert(objects != NULL, "Objects' object group not found");
		CCDictionary *spawnPoint = objects->objectNamed("SpawnPoint");
		CCAssert(spawnPoint != NULL, "SpawnPoint object not found");
		int x = spawnPoint->valueForKey("x")->intValue();
		int y = spawnPoint->valueForKey("y")->intValue();

		m_player = CCSprite::create("Player.png");
		m_player->setPosition( ccp(x, y) );

		this->addChild(m_player);
		setViewpointCenter(m_player->getPosition());
		//pMenu->setPosition(this->convertToNodeSpace(CCPointZero));

		m_meta = m_tileMap->layerNamed("Meta");
		m_meta->setVisible(false);

		m_foreground = m_tileMap->layerNamed("Foreground");

		m_projectiles = new CCArray;
		m_enemies = new CCArray;
		m_visibleEnemies = new CCArray;
		m_visibleProjectiles = new CCArray;

		CCArray* arr = objects->getObjects();
		CCObject* obj;
		CCARRAY_FOREACH(arr, obj) {
			CCDictionary* enemyPoint = (CCDictionary *)obj;
			const CCString* name = enemyPoint->valueForKey("name");
			if (name != NULL && name->compare("EnemySpawn") == 0) {
				int x = enemyPoint->valueForKey("x")->intValue();
				int y = enemyPoint->valueForKey("y")->intValue();
				addEnemy(x, y);
			}
		}

		this->setTouchEnabled(true);
		this->scheduleUpdate();

		CocosDenshion::SimpleAudioEngine::sharedEngine()->preloadEffect("pickup.wav");
		CocosDenshion::SimpleAudioEngine::sharedEngine()->preloadEffect("hit.wav");
		CocosDenshion::SimpleAudioEngine::sharedEngine()->preloadEffect("move.wav");
		CocosDenshion::SimpleAudioEngine::sharedEngine()->preloadEffect("pew-pew-lei.wav");
		CocosDenshion::SimpleAudioEngine::sharedEngine()->playBackgroundMusic("TileMap.wav", true);

		bRet = true;
	} while (0);

	return bRet;
}

void HelloWorld::addEnemy(int x,int y)
{
	CCSprite* enemy = CCSprite::create("enemy1.png");
	enemy->setPosition(ccp(x,y));
	addChild(enemy);
	m_enemies->addObject(enemy);
}

void HelloWorld::setPlayerPosition(CCPoint position)
{
	CCPoint tileCoord = this->tileCoordForPosition(position);
	int tileGid = m_meta->tileGIDAt(tileCoord);
	if (tileGid) {
		CCDictionary *properties = m_tileMap->propertiesForGID(tileGid);
		if (properties) {
			const CCString *collision = properties->valueForKey("Collidable");
			if (collision && collision->boolValue()) {
				CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect("hit.wav");
				return;
			}
			const CCString *collectable = properties->valueForKey("Collectable");
			if (collectable && collectable->boolValue()) {
				m_foreground->removeTileAt(tileCoord);
				m_meta->removeTileAt(tileCoord);
				CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect("pickup.wav");
			}
		}
	}

	if (m_player->getTag() != kCCNodeTagInvalid)
		return;
	m_player->setTag(3);
	CCFiniteTimeAction* actionMove = CCMoveTo::create( 0.2f, position );
	CCFiniteTimeAction* actionMoveDone = CCCallFuncN::create( this,
		callfuncN_selector(HelloWorld::spriteMoveFinished));
	m_player->runAction( CCSequence::create(actionMove, actionMoveDone, NULL) );

	CCSize s1 = m_tileMap->getContentSize();
	CCSize s2 = m_tileMap->getTileSize();
	CCFollow* follow = CCFollow::create(m_player, CCRectMake(0, 0, 
		m_tileMap->getMapSize().width * m_tileMap->getTileSize().width,
		m_tileMap->getMapSize().height * m_tileMap->getTileSize().height));
	this->runAction(follow);

//  	m_player->setPosition(position);
//  	setViewpointCenter(position);

	CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect("move.wav");
}

void HelloWorld::setViewpointCenter(CCPoint position)
{
	CCSize winSize = CCDirector::sharedDirector()->getWinSize();

	int x = MAX(position.x, winSize.width / 2);
	int y = MAX(position.y, winSize.height / 2);
	x = MIN(x, (m_tileMap->getMapSize().width * m_tileMap->getTileSize().width) - winSize.width / 2);
	y = MIN(y, (m_tileMap->getMapSize().height * m_tileMap->getTileSize().height) - winSize.height / 2);
	CCPoint actualPosition = ccp(x, y);

	CCPoint centerOfView = ccp(winSize.width / 2, winSize.height / 2);
	CCPoint viewPoint = ccpSub(centerOfView, actualPosition);
	this->setPosition(viewPoint);
}

CCPoint HelloWorld::tileCoordForPosition(CCPoint position)
{
	int x = position.x / m_tileMap->getTileSize().width;
	int y = ((m_tileMap->getMapSize().height * m_tileMap->getTileSize().height) - position.y) / m_tileMap->getTileSize().height;
	return ccp(x, y);
}

void HelloWorld::gameOver()
{
	GameOverScene *gameOverScene = GameOverScene::create();
	gameOverScene->getLayer()->getLabel()->setString("You Lose:[");
	CCDirector::sharedDirector()->replaceScene(gameOverScene);
}

bool HelloWorld::isSpriteInViewPort(CCSprite* sprite)
{
	CCPoint winPos = ccpSub(CCPointZero, this->getPosition());
	CCSize winSize = CCDirector::sharedDirector()->getVisibleSize();
	CCRect winRect = CCRectMake(winPos.x, winPos.y, winSize.width, winSize.height);

	CCRect spriteRect = CCRectMake(
		sprite->getPosition().x - sprite->getContentSize().width/2,
		sprite->getPosition().y - sprite->getContentSize().height/2,
		sprite->getContentSize().width,
		sprite->getContentSize().height);

	return winRect.intersectsRect(spriteRect);
}

void HelloWorld::filterVisibleSprites()
{
	CCPoint winPos = ccpSub(CCPointZero, this->getPosition());
	CCSize winSize = CCDirector::sharedDirector()->getVisibleSize();
	CCRect winRect = CCRectMake(winPos.x, winPos.y, winSize.width, winSize.height);

	CCArray* container[2] = { m_visibleEnemies, m_visibleProjectiles };
	CCArray** pContainer = &container[0];
	CCArray* arr[2] = { m_enemies, m_projectiles };

	std::for_each(&arr[0], arr + sizeof(arr) / sizeof(arr[0]), [&winRect, &pContainer](CCArray* arr2){
		(*pContainer)->removeAllObjects();
		CCObject *iter = NULL;
		CCARRAY_FOREACH(arr2, iter) {
			CCSprite* sprite = dynamic_cast<CCSprite*>(iter);
			CCRect spriteRect = CCRectMake(
				sprite->getPosition().x - sprite->getContentSize().width/2,
				sprite->getPosition().y - sprite->getContentSize().height/2,
				sprite->getContentSize().width,
				sprite->getContentSize().height);
			if (winRect.intersectsRect(spriteRect)) {
				(*pContainer)->addObject(sprite);
			}
		}
		if ((*pContainer)->count() > 0) {
			CCLOG("visible sprites: %d", (*pContainer)->count());
		}
		++pContainer;
	});
}

void HelloWorld::spriteMoveFinished(CCNode* sender)
{
	CCSprite *sprite = (CCSprite *)sender;

	int tag = sprite->getTag();
	if (tag == 1) { // enemy
		if (isSpriteInViewPort(sprite)) {
			moveEnemy(sprite);
		} else {
			sprite->setTag(kCCNodeTagInvalid);
		}
	} else if (tag == 2) { // projectile
		m_visibleProjectiles->removeObject(sprite);
		m_projectiles->removeObject(sprite);
		this->removeChild(sprite, true);
	} else if (tag == 3) {
		sprite->setTag(kCCNodeTagInvalid);
	}
}

void HelloWorld::moveEnemy(CCSprite* enemy)
{
	CCPoint diff = ccpSub(m_player->getPosition(), enemy->getPosition());
	enemy->setRotation(-CC_RADIANS_TO_DEGREES(diff.getAngle()));

	float speed = 320 / 3.0f;
	float duration = ccpLength(diff) / speed;

	CCFiniteTimeAction* actionMove = CCMoveBy::create( duration, diff );
	CCFiniteTimeAction* actionMoveDone = CCCallFuncN::create( this,
		callfuncN_selector(HelloWorld::spriteMoveFinished));
	enemy->runAction( CCSequence::create(actionMove, actionMoveDone, NULL) );
	enemy->setTag(1);
}

void HelloWorld::checkMoveEnemy()
{
	CCObject* iter = NULL;
	CCARRAY_FOREACH(m_visibleEnemies, iter) {
		CCSprite *sprite = dynamic_cast<CCSprite*>(iter);
		int tag = sprite->getTag();
		if (sprite->getTag() == kCCNodeTagInvalid) {
			moveEnemy(sprite);
		}
	}
}

void HelloWorld::checkHit()
{
	CCObject* iter = NULL;

	// check enemies and player
	CCRect playerRect = CCRectMake(
		m_player->getPosition().x - m_player->getContentSize().width/2,
		m_player->getPosition().y - m_player->getContentSize().height/2,
		m_player->getContentSize().width,
		m_player->getContentSize().height);

	CCArray *projectilesToDelete = new CCArray;

	CCARRAY_FOREACH(m_visibleEnemies, iter) {
		CCSprite *enemy = dynamic_cast<CCSprite*>(iter);
		CCRect enemyRect = CCRectMake(
			enemy->getPosition().x - enemy->getContentSize().width/2,
			enemy->getPosition().y - enemy->getContentSize().height/2,
			enemy->getContentSize().width,
			enemy->getContentSize().height);
		if (enemyRect.intersectsRect(playerRect)) {
			gameOver();
		}
	}

	CCObject* iter2 = NULL;
	CCARRAY_FOREACH(m_visibleProjectiles, iter2) {
		CCSprite *projectile = dynamic_cast<CCSprite*>(iter2);
		CCRect projectileRect = CCRectMake(
			projectile->getPosition().x - projectile->getContentSize().width/2,
			projectile->getPosition().y - projectile->getContentSize().height/2,
			projectile->getContentSize().width,
			projectile->getContentSize().height);

		CCArray *enemiesToDelete = new CCArray;
		CCARRAY_FOREACH(m_visibleEnemies, iter) {
			CCSprite *enemy = dynamic_cast<CCSprite*>(iter);
			CCRect enemyRect = CCRectMake(
				enemy->getPosition().x - enemy->getContentSize().width/2,
				enemy->getPosition().y - enemy->getContentSize().height/2,
				enemy->getContentSize().width,
				enemy->getContentSize().height);
			if (projectileRect.intersectsRect(enemyRect)) {
				enemiesToDelete->addObject(enemy);
			}
		}
		if (enemiesToDelete->count() > 0) {
			projectilesToDelete->addObject(projectile);
			CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect("hit.wav");
		}

		CCARRAY_FOREACH(enemiesToDelete, iter) {
			CCSprite *enemy = dynamic_cast<CCSprite*>(iter);
			m_visibleEnemies->removeObject(enemy);
			m_enemies->removeObject(enemy);
			this->removeChild(enemy, true);
		}
		enemiesToDelete->release();
	}

	CCARRAY_FOREACH(projectilesToDelete, iter) {
		CCSprite *projectile = dynamic_cast<CCSprite*>(iter);
		m_visibleProjectiles->removeObject(projectile);
		m_projectiles->removeObject(projectile);
		this->removeChild(projectile, true);
	}
	projectilesToDelete->release();
}

void HelloWorld::sendProjectile(CCPoint dir)
{
	CCSprite* projectile = CCSprite::create("Projectile.png");
	projectile->setPosition(m_player->getPosition());
	this->addChild(projectile);
	projectile->setTag(2);


	const float LEN = 240;
	float speed = 1200 / 3.0f;
	float duration = LEN / speed;
	CCPoint amount = dir.normalize() * LEN;

	CCFiniteTimeAction* actionMove = CCMoveBy::create( duration, amount );
	CCFiniteTimeAction* actionMoveDone = CCCallFuncN::create( this,
		callfuncN_selector(HelloWorld::spriteMoveFinished));
	projectile->runAction( CCSequence::create(actionMove, actionMoveDone, NULL) );

	m_projectiles->addObject(projectile);

	CocosDenshion::SimpleAudioEngine::sharedEngine()->playEffect("pew-pew-lei.wav");
}

void HelloWorld::update(float dt)
{
	filterVisibleSprites();
	checkMoveEnemy();
	checkHit();
}

void HelloWorld::updatePlayerPosition(CCPoint dir)
{
	if (m_player->getTag() != kCCNodeTagInvalid)
		return;

	CCPoint playerPos = m_player->getPosition();
	CCPoint step = ccpCompMult(dir, CCPoint(m_tileMap->getTileSize()));
	playerPos = ccpAdd(playerPos, step);

	if (playerPos.x <= (m_tileMap->getMapSize().width * m_tileMap->getTileSize().width) &&
		playerPos.y <= (m_tileMap->getMapSize().height * m_tileMap->getTileSize().height) &&
		playerPos.y >= 0 && playerPos.x >= 0)
	{
		setPlayerPosition(playerPos);
	}
}

void HelloWorld::ccTouchesEnded(CCSet* touches, CCEvent* event)
{
	CCTouch* touch = (CCTouch*)( touches->anyObject() );
	//CCPoint location = touch->getLocation();
	CCPoint touchLocation = this->convertTouchToNodeSpace(touch);
    
	CCLog("++++++++after  x:%f, y:%f", touchLocation.x, touchLocation.y);

	CCPoint playerPos = m_player->getPosition();
	CCPoint diff = ccpSub(touchLocation, playerPos);
	sendProjectile(diff);
}

void HelloWorld::registerWithTouchDispatcher()
{
	// CCTouchDispatcher::sharedDispatcher()->addTargetedDelegate(this,0,true);
    CCDirector::sharedDirector()->getTouchDispatcher()->addStandardDelegate(this,0);
}

void HelloWorld::didChangeDirectionTo( SimpleDPad *simpleDPad, cocos2d::CCPoint direction )
{
	updatePlayerPosition(direction);
}

void HelloWorld::isHoldingDirection( SimpleDPad *simpleDPad, cocos2d::CCPoint direction )
{
	updatePlayerPosition(direction);
}

void HelloWorld::simpleDPadTouchEnded( SimpleDPad *simpleDPad )
{

}
