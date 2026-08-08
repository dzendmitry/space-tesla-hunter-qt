#include "gamegraphicscene.h"

#include "resources.h"

GameGraphicScene::GameGraphicScene(const QRectF &sceneRect, QObject *parent)
    : QGraphicsScene(sceneRect, parent)
{
    setBackgroundBrush(backgroundColor);
}

void GameGraphicScene::keyPressEvent(QKeyEvent *event) {
    emit keyPressed(event->key());
}

void GameGraphicScene::keyReleaseEvent(QKeyEvent *event) {
    emit keyReleased(event->key());
}

void GameGraphicScene::drawBackground(QPainter *painter, const QRectF &rect) {
    painter->drawPixmap(
        this->sceneRect().toRect(),
        Resources::GetInstance()->getPixmapById(ResourceId::GameSceneBackground)
    );
}

GameGraphicScene::~GameGraphicScene() {}