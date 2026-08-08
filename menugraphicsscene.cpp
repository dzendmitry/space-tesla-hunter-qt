#include "menugraphicsscene.h"
#include "resources.h"

#include <QGraphicsProxyWidget>

void MenuGraphicsScene::addStartGameButton(const QRectF &sceneRect) {
    startGameButton = new QPushButton(QIcon(Resources::GetInstance()->getPixmapById(ResourceId::MenuSceneButton1)), "");
    startGameButton->setIconSize(QSize(DEFAULT_BUTTON_WIDTH, DEFAULT_BUTTON_HEIGHT));
    startGameButton->setFlat(true);
    startGameButton->setStyleSheet(EMPTY_BACKGROUND_STYPE);

    addWidget(startGameButton)->setPos(
        sceneRect.width() / 2 - startGameButton->width() / 2, 280
    );

    QObject::connect(startGameButton, &QPushButton::clicked, this, &MenuGraphicsScene::startGameButtonClicked);
}

void MenuGraphicsScene::addStartCoopGameButton(const QRectF &sceneRect) {
    startCoopGameButton = new QPushButton(QIcon(Resources::GetInstance()->getPixmapById(ResourceId::MenuSceneButton2)), "");
    startCoopGameButton->setIconSize(QSize(DEFAULT_BUTTON_WIDTH, DEFAULT_BUTTON_HEIGHT));
    startCoopGameButton->setFlat(true);
    startCoopGameButton->setStyleSheet(EMPTY_BACKGROUND_STYPE);

    addWidget(startCoopGameButton)->setPos(
        sceneRect.width() / 2 - startCoopGameButton->width() / 2, 390
    );

    QObject::connect(startCoopGameButton, &QPushButton::clicked, this, &MenuGraphicsScene::startCoopGameButtonClicked);
}

void MenuGraphicsScene::addExitGameButton(const QRectF &sceneRect) {
    exitGameButton = new QPushButton(QIcon(Resources::GetInstance()->getPixmapById(ResourceId::MenuSceneButton3)), "");
    exitGameButton->setIconSize(QSize(DEFAULT_BUTTON_WIDTH, DEFAULT_BUTTON_HEIGHT));
    exitGameButton->setFlat(true);
    exitGameButton->setStyleSheet(EMPTY_BACKGROUND_STYPE);

    addWidget(exitGameButton)->setPos(
        sceneRect.width() / 2 - exitGameButton->width() / 2, 500
    );

    QObject::connect(exitGameButton, &QPushButton::clicked, this, &MenuGraphicsScene::exitGameButtonClicked);
}

MenuGraphicsScene::MenuGraphicsScene(const QRectF &sceneRect, QObject *parent)
    : QGraphicsScene(sceneRect, parent)
{
    addStartGameButton(sceneRect);
    addStartCoopGameButton(sceneRect);
    addExitGameButton(sceneRect);
}

void MenuGraphicsScene::drawBackground(QPainter *painter, const QRectF &rect) {
    painter->drawPixmap(
        this->sceneRect().toRect(),
        Resources::GetInstance()->getPixmapById(ResourceId::MenuSceneBackground)
    );
}

MenuGraphicsScene::~MenuGraphicsScene() {
    delete startGameButton;
    delete startCoopGameButton;
    delete exitGameButton;
}