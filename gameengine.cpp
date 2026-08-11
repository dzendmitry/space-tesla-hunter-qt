#include "gameengine.h"

const std::set<Ship*> GameEngine::getShipsByFilters(std::set<ShipState> states, std::set<ShipSide> sides, std::set<ShipType> types) const {
    std::set<Ship*> r;
    for (auto it = ships.begin(); it != ships.end(); it++) {
        Ship *ship = *it;
        if ( ( states.empty() || states.find(ship->getState()) != states.end() ) &&
            ( sides.empty() || sides.find(ship->getSide()) != sides.end() ) &&
            ( types.empty() || types.find(ship->getType()) != types.end()) ) {

            r.insert(ship);

        }
    }
    return r;
}

Ship* const GameEngine::getPlayerLocalShip() const {
    std::set<Ship*> localPlayerShips = getShipsByFilters({},{ShipSide::Local},{ShipType::Player});
    auto pIt = localPlayerShips.begin();
    if (pIt == localPlayerShips.end()) {

        return nullptr;
    }
    return *pIt;
}

void GameEngine::stopTheWorld() {
    enemiesGenTimer.stop();
    clashTimer.stop();
    enemyShotTimer.stop();
    playerShotTimer.stop();
    worldTimer.stop();
}

GameEngine::GameEngine(QGraphicsScene* gameScene, QObject* parent)
    : GameEngineBase(gameScene, parent)
{}

void GameEngine::StartNewGame() {
    clashAllowed = true;
    clashTimer.setInterval(1000); // 1 clash/s
    QObject::connect(&clashTimer, &QTimer::timeout, this, &GameEngine::clashTimerHandler);
    clashTimer.start();

    playerShotTimer.setInterval(100); // max(10 bullets/s)
    QObject::connect(&playerShotTimer, &QTimer::timeout, this, &GameEngine::playerShotTimerHandler);
    playerShotTimer.start();

    enemyShotTimer.setInterval(333); // 3 bullets/s
    QObject::connect(&enemyShotTimer, &QTimer::timeout, this, &GameEngine::enemyShotTimerHandler);
    enemyShotTimer.start();

    worldTimer.setInterval(16); // 60 FPS
    QObject::connect(&worldTimer, &QTimer::timeout, this, &GameEngine::worldTimerHandler);
    worldTimer.start();

    enemiesGenTimer.setInterval(1000);
    QObject::connect(&enemiesGenTimer, &QTimer::timeout, this, &GameEngine::enemiesGenTimerHandler);
    enemiesGenTimer.start();

    addMoarEnemies(5);

    Ship *player = new Ship(ShipType::Player);
    ships.insert(player);
    player->setPos(
        gameScene->sceneRect().width() / 2 - player->getHalfWidth(),
        gameScene->sceneRect().height() - player->DEFAULT_HEIGHT
    );

    QObject::connect(player, &Ship::healthChanged, this, &GameEngine::playerHealthChanged);
    player->decHealth(0);

    gameScene->addItem(player);
    gameScene->setFocusItem(player->focusItem());
    player->grabKeyboard();
}

GameStatus GameEngine::checkGameStatus() {
    auto alivePlayers = getShipsByFilters({ShipState::Damaged, ShipState::Alive},{},{ShipType::Player});
    auto aliveEnemies = getShipsByFilters({ShipState::Damaged, ShipState::Alive},{},{ShipType::Enemy});
    if (alivePlayers.begin() == alivePlayers.end()) {
        return GameStatus::Defeat;
    }
    if (aliveEnemies.begin() == aliveEnemies.end()) {
        return GameStatus::Victory;
    }

    return GameStatus::InProgress;
}

void GameEngine::addEnemy() {
    auto aliveEnemies = getShipsByFilters({ShipState::Damaged, ShipState::Alive},{},{ShipType::Enemy});
    QPointF point;

    int i = 0;
    for (; i < MAX_ENEMIES_SPAWN_TRIES; i++) {
        point = QPointF(
            arc4random_uniform(gameScene->sceneRect().width() - Ship::DEFAULT_WIDTH),
            arc4random_uniform(gameScene->sceneRect().height() / 3)
            );
        QRegion candidateRegion = QRegion(point.x(), point.y(), Ship::DEFAULT_WIDTH, Ship::DEFAULT_HEIGHT);
        auto eIt = aliveEnemies.begin();
        for (; eIt != aliveEnemies.end(); eIt++) {
            auto enemy = *eIt;
            QRegion enemyRegion = enemy->boundingRegion(enemy->sceneTransform());
            if (enemyRegion.intersects(candidateRegion)) {
                break;
            }
        }
        if (eIt == aliveEnemies.end()) {
            break;
        }
    }
    if (i >= MAX_ENEMIES_SPAWN_TRIES) {
        return;
    }

    Ship *enemy = new Ship(ShipType::Enemy);
    ships.insert(enemy);
    enemy->setPos(point);
    gameScene->addItem(enemy);
}

void GameEngine::addMoarEnemies() {
    auto aliveEnemies = getShipsByFilters({ShipState::Damaged, ShipState::Alive},{},{ShipType::Enemy});
    if (aliveEnemies.size() < MIN_ALIVE_ENEMIES_AMOUNT) {
        int maybeAddEnemiesAmount = MAX_ALIVE_ENEMIES_AMOUNT - aliveEnemies.size();
        int addEnemiesAmount = arc4random_uniform(maybeAddEnemiesAmount);
        for (int i = 0; i < addEnemiesAmount; i++) {
            addEnemy();
        }
    }
}

void GameEngine::addMoarEnemies(int amount) {
    if (amount <= 0) {
        return;
    }
    for (int i = 0; i < amount; i++) {
        addEnemy();
    }
}


std::pair<std::set<Ship*>, std::set<Bullet*>> GameEngine::damageLogic() {

    std::set<Ship*> deadShips;
    std::set<Bullet*> usedBullets;

    std::set<Ship*> playerNonDeadShips = getShipsByFilters({ShipState::Damaged, ShipState::Alive},{},{ShipType::Player});
    std::set<Ship*> enemyNonDeadShips = getShipsByFilters({ShipState::Damaged, ShipState::Alive},{},{ShipType::Enemy});

    // out of scene logic

    for (auto enemy : enemyNonDeadShips) {
        if (enemy->pos().y() > gameScene->sceneRect().height()) {
            enemy->decHealth(enemy->getHealth());
            deadShips.insert(enemy);
        }
    }

    // damage logic

    for (auto bIt = bullets.begin(); bIt != bullets.end(); bIt++) {
        Bullet* bullet = *bIt;
        std::set <Ship*> shipsToCheck;
        switch (bullet->getType()) {
        default:
            break;
        case BulletType::Player:
            shipsToCheck = enemyNonDeadShips;
            break;
        case BulletType::Enemy:
            shipsToCheck = playerNonDeadShips;
            break;
        }
        for (auto ship : shipsToCheck) {
            if (ship->isDead()) {
                continue;
            }
            QRegion shipRegion = ship->boundingRegion(ship->sceneTransform());
            if (shipRegion.contains(bullet->pos().toPoint())) {
                ship->decHealth(bullet->getDamage());
                // collect dead enemy ships
                if (ship->isDead()) {
                    deadShips.insert(ship);
                }
                // collect used bullets
                usedBullets.insert(bullet);
            }
        }
    }

    // clash logic

    if (clashAllowed) {
        clashAllowed = false;

        for (auto eIt = enemyNonDeadShips.begin(); eIt != enemyNonDeadShips.end(); eIt++) {
            Ship* enemy = *eIt;
            if (enemy->isDead()) {
                continue;
            }
            QRegion enemyRegion = enemy->boundingRegion(enemy->sceneTransform());
            for (auto pIt = playerNonDeadShips.begin(); pIt != playerNonDeadShips.end(); pIt++) {
                Ship* player = *pIt;
                if (player->isDead()) {
                    continue;
                }
                QRegion playerRegion = player->boundingRegion(player->sceneTransform());
                if (enemyRegion.intersects(playerRegion)) {
                    enemy->decHealth(ENEMY_SHIP_CLASH_DAMAEGE);
                    player->decHealth(PLAYER_SHIP_CLASH_DAMAGE);
                    // collect dead enemy ships
                    if (enemy->isDead()) {
                        deadShips.insert(enemy);
                    }
                    // collect dead player ships
                    if (player->isDead()) {
                        deadShips.insert(player);
                    }
                }
            }
        }

    }

    // gc

    for (auto bIt = usedBullets.begin(); bIt != usedBullets.end(); bIt++) {
        bullets.erase(*bIt);
    }
    for (auto shipIt = deadShips.begin(); shipIt != deadShips.end(); shipIt++) {
        ships.erase(*shipIt);
    }

    return std::pair(deadShips, usedBullets);
}

void GameEngine::worldTimerHandler() {

    GameStatus gameStatus = checkGameStatus();
    if (gameStatus != GameStatus::InProgress) {
        stopTheWorld();
        emit gameFinished(gameStatus);
        return;
    }

    std::pair<std::set<Ship*>, std::set<Bullet*>> damagedSets = damageLogic();
    std::set<Ship*> deadShips = damagedSets.first;
    std::set<Bullet*> usedBullets = damagedSets.second;

    for (auto shipIt = deadShips.begin(); shipIt != deadShips.end(); shipIt++) {
        gameScene->removeItem(*shipIt);
    }
    for (auto bIt = usedBullets.begin(); bIt != usedBullets.end(); bIt++) {
        gameScene->removeItem(*bIt);
    }

    std::set<Ship*> enemyShips = getShipsByFilters({},{},{ShipType::Enemy});
    for (auto shipIt = enemyShips.begin(); shipIt != enemyShips.end(); shipIt++) {
        Ship* ship = *shipIt;
        QPointF curPos = ship->pos();
        curPos.ry() += ENEMY_SHIP_SPEED;
        ship->setPos(curPos);
    }

    Ship *player = getPlayerLocalShip();
    if (player != nullptr) {
        QPointF curPos = player->pos();
        if (actions & 1 && (curPos.y() + Ship::getHalfHeight()) > gameScene->sceneRect().top()) {
            curPos.ry() -= PLAYER_SHIP_SPEED;
        }
        if (actions & (1 << 1) && (curPos.x() + Ship::getHalfWidth()) < gameScene->sceneRect().right() ) {
            curPos.rx() += PLAYER_SHIP_SPEED;
        }
        if (actions & (1 << 2) && (curPos.y() + Ship::getHalfHeight()) < gameScene->sceneRect().bottom()) {
            curPos.ry() += PLAYER_SHIP_SPEED;
        }
        if (actions & (1 << 3) && (curPos.x() + Ship::getHalfWidth()) > gameScene->sceneRect().left()) {
            curPos.rx() -= PLAYER_SHIP_SPEED;
        }
        player->setPos(curPos);
    }

    for (auto bIt = bullets.begin(); bIt != bullets.end(); bIt++) {
        Bullet *bullet = *bIt;
        QPointF curPos = bullet->pos();
        switch (bullet->getType()) {
        default:
            break;
        case BulletType::Player:
            curPos.ry() -= DEFAULT_BULLET_SPEED;
            break;
        case BulletType::Enemy:
            curPos.ry() += DEFAULT_BULLET_SPEED;
            break;
        }
        bullet->setPos(curPos);
    }

}

void GameEngine::playerShotTimerHandler() {

    // player shots
    if (actions & (1 << 4)) {
        const Ship *playerShip = getPlayerLocalShip();
        if (playerShip) {
            QPointF p = playerShip->pos();
            Bullet *bullet = new Bullet(BulletType::Player);
            bullets.insert(bullet);
            bullet->setPos(QPointF(p.x() + Ship::getHalfWidth(), p.y()));
            gameScene->addItem(bullet);
        }
    }

}

void GameEngine::enemyShotTimerHandler() {

    // enemy shots
    auto enemyShips = getShipsByFilters({ShipState::Damaged, ShipState::Alive}, {}, {ShipType::Enemy});
    for (auto enemyShip : enemyShips) {
        // shot or not
        uint shotProb = arc4random_uniform(100);
        if (shotProb < ENEMY_SHIP_SHOT_PROB_PERCENT) {
            QPointF p = enemyShip->pos();
            Bullet *bullet = new Bullet(BulletType::Enemy);
            bullets.insert(bullet);
            bullet->setPos(QPointF(p.x() + Ship::getHalfWidth(), p.y() + Ship::DEFAULT_HEIGHT));
            gameScene->addItem(bullet);
        }
    }

}

void GameEngine::clashTimerHandler() {
    clashAllowed = true;
}

void GameEngine::enemiesGenTimerHandler() {
    addMoarEnemies();
}

GameEngine::~GameEngine() {
    for (auto ship = ships.begin(); ship != ships.end(); ship++) {
        delete *ship;
    }
    for (auto bullet = bullets.begin(); bullet != bullets.end(); bullet++) {
        delete *bullet;
    }
}