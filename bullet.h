#ifndef BULLET_H
#define BULLET_H

#include <map>
#include <QString>
#include <QGraphicsPixmapItem>

#include "resources.h"

enum class BulletType { Player, Enemy };

const int DEFAULT_BULLET_SPEED = 5;

static std::map<BulletType, int> bulletTypeDamageMap = {
    {BulletType::Player, 35},
    {BulletType::Enemy, 5},
};

static std::map<BulletType, ResourceId> bulletTypeResourcesMap = {
    {BulletType::Player, ResourceId::PlayerBullet},
    {BulletType::Enemy, ResourceId::EnemyBullet},
};

class Bullet : public QGraphicsPixmapItem
{
public:
    explicit Bullet(BulletType type, QGraphicsItem *parent = nullptr);

    const unsigned getDamage() const { return damage; }
    const BulletType getType() const { return type; }

    static const int DEFAULT_WIDTH = 32;
    static const int DEFAULT_HEIGHT = 32;
private:
    BulletType type;
    QPixmap pixmap;
    unsigned damage;
};

#endif // BULLET_H
