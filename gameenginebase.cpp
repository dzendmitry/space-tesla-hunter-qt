#include "gameenginebase.h"

GameEngineBase::GameEngineBase(QGraphicsScene* gameScene, QObject *parent)
    : QObject(parent)
{
    if (!gameScene) {
        throw std::runtime_error("Game Scene for Game Engine MUST exist!");
    }
    this->gameScene = gameScene;
}

GameEngineBase::~GameEngineBase() {}

void GameEngineBase::keyPressed(int key) {
    switch(key) {
    case Qt::Key_Up:
        actions |= 1;
        break;
    case Qt::Key_Right:
        actions |= (1 << 1);
        break;
    case Qt::Key_Down:
        actions |= (1 << 2);
        break;
    case Qt::Key_Left:
        actions |= (1 << 3);
        break;
    case Qt::Key_Space:
        actions |= (1 << 4);
        break;
    }
}

void GameEngineBase::keyReleased(int key) {
    switch(key) {
    case Qt::Key_Up:
        actions ^= 1;
        break;
    case Qt::Key_Right:
        actions ^= (1 << 1);
        break;
    case Qt::Key_Down:
        actions ^= (1 << 2);
        break;
    case Qt::Key_Left:
        actions ^= (1 << 3);
        break;
    case Qt::Key_Space:
        actions ^= (1 << 4);
        break;
    }
}