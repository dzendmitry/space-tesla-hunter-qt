#ifndef SHIP_H
#define SHIP_H

#include <QGraphicsPixmapItem>

#include "resources.h"

enum class ShipState { Dead, Damaged, Alive };
enum class ShipSide { Local, Remote };
enum class ShipType { Player, Enemy };

enum class ShipHealt {
    Level100 = 100,
    Level70  = 70,
    Level40  = 40,
    Level10  = 10,
    Level0   = 0,
};

const int PLAYER_SHIP_SPEED = 10;
const int ENEMY_SHIP_SPEED = 1;

const int PLAYER_SHIP_CLASH_DAMAGE = 10;
const int ENEMY_SHIP_CLASH_DAMAEGE = 35;

const int ENEMY_SHIP_SHOT_PROB_PERCENT = 30;

class Ship : public QObject, public QGraphicsPixmapItem
{
    Q_OBJECT

public:
    explicit Ship(ShipType type, ShipSide side = ShipSide::Local, int maxHealth = 100, QGraphicsItem *parent = nullptr);

    ShipSide getSide() { return side; }
    ShipType getType() { return type; }
    ShipState getState() { return state; }
    int getHealth() { return health; }
    bool isDead() { return state == ShipState::Dead; }

    void setState(ShipState state) { this->state = state; }
    void decHealth(unsigned damage);
    void setHealth(unsigned health);

    static int getHalfWidth() { return DEFAULT_WIDTH / 2; }
    static int getHalfHeight() { return DEFAULT_HEIGHT / 2; }

    static const int DEFAULT_WIDTH = 128;
    static const int DEFAULT_HEIGHT = 128;

signals:
    void healthChanged(int);

private:
    void setPixmap(ShipType type, int shipIndex, int health);
    void healthChecks();

    QPixmap pixmap;
    ShipState state;
    ShipType type;
    int shipIndex;
    ShipSide side;
    int maxHealth;
    int health;
};

static std::map<ShipType, std::vector<std::map<ShipHealt, ResourceId>>> shipTypeResourcesMap = {
    {ShipType::Player,
        {{
            {ShipHealt::Level100, ResourceId::PlayerLevel100},
            {ShipHealt::Level70, ResourceId::PlayerLevel70},
            {ShipHealt::Level40, ResourceId::PlayerLevel40},
            {ShipHealt::Level10, ResourceId::PlayerLevel10},
            {ShipHealt::Level0, ResourceId::PlayerLevel0},
        }}
    },
    {ShipType::Enemy,
        {
          {
            {ShipHealt::Level100, ResourceId::Enemy1Level100},
            {ShipHealt::Level70, ResourceId::Enemy1Level70},
            {ShipHealt::Level40, ResourceId::Enemy1Level40},
            {ShipHealt::Level10, ResourceId::Enemy1Level10},
            {ShipHealt::Level0, ResourceId::Enemy1Level0},
            },
          {
            {ShipHealt::Level100, ResourceId::Enemy2Level100},
            {ShipHealt::Level70, ResourceId::Enemy2Level70},
            {ShipHealt::Level40, ResourceId::Enemy2Level40},
            {ShipHealt::Level10, ResourceId::Enemy2Level10},
            {ShipHealt::Level0, ResourceId::Enemy2Level0},
            },
          {
            {ShipHealt::Level100, ResourceId::Enemy3Level100},
            {ShipHealt::Level70, ResourceId::Enemy3Level70},
            {ShipHealt::Level40, ResourceId::Enemy3Level40},
            {ShipHealt::Level10, ResourceId::Enemy3Level10},
            {ShipHealt::Level0, ResourceId::Enemy3Level0},
          },
        }
    },
};

#endif // SHIP_H
