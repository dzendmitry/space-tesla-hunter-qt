#ifndef GAMEENGINE_H
#define GAMEENGINE_H

#include <QTimer>
#include <QGraphicsScene>

#include "gameenginebase.h"
#include "ship.h"
#include "bullet.h"

const int MAX_ENEMIES_SPAWN_TRIES = 5;
const int MIN_ALIVE_ENEMIES_AMOUNT = 5;
const int MAX_ALIVE_ENEMIES_AMOUNT = 5;

class GameEngine : public GameEngineBase
{
public:
    explicit GameEngine(QGraphicsScene* gameScene, QObject *parent = nullptr);
    ~GameEngine();

    void StartNewGame();

private slots:
    // timer handlers
    void worldTimerHandler();
    void playerShotTimerHandler();
    void enemyShotTimerHandler();
    void clashTimerHandler();
    void enemiesGenTimerHandler();

private:
    const std::set<Ship*> getShipsByFilters(std::set<ShipState> states, std::set<ShipSide> sides, std::set<ShipType> types) const;
    Ship* const getPlayerLocalShip() const;

    void addEnemy();
    void addMoarEnemies();
    void addMoarEnemies(int amount);

    std::pair<std::set<Ship*>, std::set<Bullet*>> damageLogic();
    GameStatus checkGameStatus();
    void stopTheWorld(); // stop all timers

    std::set<Ship*> ships;
    std::set<Bullet*> bullets;
    QTimer worldTimer;

    QTimer playerShotTimer;

    QTimer clashTimer;
    bool clashAllowed;

    QTimer enemyShotTimer;
    QTimer enemiesGenTimer;
};

#endif // GAMEENGINE_H
