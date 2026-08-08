#include <QKeyEvent>
#include "ship.h"

ResourceId resolveResource(ShipType type, int shipIndex, int health) {
    std::map<ShipHealt, ResourceId> resourcesMap = shipTypeResourcesMap.at(type).at(shipIndex);
    if (health > (int)ShipHealt::Level70 && health <= (int)ShipHealt::Level100) {
        return resourcesMap.at(ShipHealt::Level100);
    } else if (health > (int)ShipHealt::Level40 && health <= (int)ShipHealt::Level70) {
        return resourcesMap.at(ShipHealt::Level70);
    } else if (health > (int)ShipHealt::Level10 && health <= (int)ShipHealt::Level40) {
        return resourcesMap.at(ShipHealt::Level40);
    } else if (health > (int)ShipHealt::Level0 && health <= (int)ShipHealt::Level10) {
        return resourcesMap.at(ShipHealt::Level10);
    } else {
        return resourcesMap.at(ShipHealt::Level0);
    }
}

int resolveShipIndex(ShipType type) {
    switch (type) {
    default:
        break;
    case ShipType::Player:
        return 0;
        break;
    case ShipType::Enemy:
        return arc4random_uniform(shipTypeResourcesMap.at(type).size());
        break;
    }
}

void Ship::setPixmap(ShipType type, int shipIndex, int health) {
    ResourceId r = resolveResource(type, shipIndex, health);
    QPixmap newPixmap = QPixmap(Resources::GetInstance()->getPixmapById(r));
    if (pixmap.cacheKey() == newPixmap.cacheKey()) {
        return;
    }
    pixmap = newPixmap;
    QGraphicsPixmapItem::setPixmap(pixmap);
}

Ship::Ship(ShipType type, ShipSide side, int maxHealth, QGraphicsItem *parent)
    : QObject(nullptr), QGraphicsPixmapItem(parent)
{
    this->type = type;
    this->shipIndex = resolveShipIndex(this->type);
    this->state = ShipState::Alive;
    this->side = side;
    this->maxHealth = maxHealth;
    this->health = maxHealth;
    setPixmap(type, shipIndex, health);
    emit healthChanged(health);
}

void Ship::decHealth(unsigned damage) {
    health -= damage;
    if (health < maxHealth) {
        state = ShipState::Damaged;
    }
    if (health <= 0) {
        health = 0;
        state = ShipState::Dead;
    }
    setPixmap(type, shipIndex, health);
    emit healthChanged(health);
}