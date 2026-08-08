#ifndef GAMEENGINE_H
#define GAMEENGINE_H

#include <QObject>
#include <QTimer>
#include <QGraphicsScene>

#include "ship.h"
#include "bullet.h"

enum class GameStatus { Victory, Defeat, InProgress };

const int MAX_ENEMIES_SPAWN_TRIES = 5;
const int MIN_ALIVE_ENEMIES_AMOUNT = 5;
const int MAX_ALIVE_ENEMIES_AMOUNT = 5;

class GameEngine : public QObject
{
    Q_OBJECT

public:
    explicit GameEngine(QGraphicsScene* gameScene, QObject *parent = nullptr);
    ~GameEngine();

    void StartNewGame();

signals:
    void playerHealthChanged(int);
    void gameFinished(GameStatus);

public slots:
    void keyPressed(int);
    void keyReleased(int);

private slots:
    // timer handlers
    void worldTimerHandler();
    void playerShotTimerHandler();
    void enemyShotTimerHandler();
    void clashTimerHandler();
    void enemiesGenTimerHandler();

private:
    QGraphicsScene* gameScene;

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
    unsigned direction;

    QTimer playerShotTimer;
    unsigned playerShot;

    QTimer clashTimer;
    bool clashAllowed;

    QTimer enemyShotTimer;
    QTimer enemiesGenTimer;
};

#endif // GAMEENGINE_H
