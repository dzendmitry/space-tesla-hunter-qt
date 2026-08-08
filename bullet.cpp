#include "bullet.h"

Bullet::Bullet(BulletType type, QGraphicsItem *parent)
    : QGraphicsPixmapItem(parent)
{
    this->type = type;
    this->damage = bulletTypeDamageMap.at(type);

    setPixmap(Resources::GetInstance()->getPixmapById(bulletTypeResourcesMap.at(type)));
}
