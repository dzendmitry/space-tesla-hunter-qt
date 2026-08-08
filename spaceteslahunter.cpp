#include "spaceteslahunter.h"
#include "ui_spaceteslahunter.h"

#include <QMessageBox>
#include <QAudioOutput>

void showMessage(QWidget *parent, QString image) {
    QMessageBox b(parent);
    b.setStyleSheet(
        QString("QLabel{min-width:500 px; min-height:500 px; background-image: url(") + image + ");}"
        +
        QString("QPushButton{}")
    );
    b.exec();
}

SpaceTeslaHunter::SpaceTeslaHunter(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::SpaceTeslaHunter)
    , gameScene(nullptr)
    , gameEngine(nullptr)
{

    Resources::GetInstance(ResourcesParams{
        .ShipWidth = Ship::DEFAULT_WIDTH,
        .ShipHeight = Ship::DEFAULT_HEIGHT,
        .BulletWidth = Bullet::DEFAULT_WIDTH,
        .BulletHeight = Bullet::DEFAULT_HEIGHT,
    });

    music = new QMediaPlayer(this);
    QAudioOutput *audioOutput = new QAudioOutput(music);
    audioOutput->setVolume(0.5);
    music->setAudioOutput(audioOutput);
    music->setSource(QUrl(QString("qrc") + utilsResourcePaths.at(ResourceId::MainMusicTrack)));
    music->setLoops(QMediaPlayer::Infinite);
    music->play();

    ui->setupUi(this);
    ui->playerHealthDisplay->hide();

    menuScene = new MenuGraphicsScene(
        ui->gameGraphicsView->rect(),
        ui->gameGraphicsView
    );

    QObject::connect(menuScene, &MenuGraphicsScene::startGameButtonClicked, this, &SpaceTeslaHunter::startNewGame);
    QObject::connect(menuScene, &MenuGraphicsScene::startCoopGameButtonClicked, this, &SpaceTeslaHunter::startNewCoopGame);
    QObject::connect(menuScene, &MenuGraphicsScene::exitGameButtonClicked, this, &SpaceTeslaHunter::exitGame);

    ui->gameGraphicsView->setScene(menuScene);
}

void SpaceTeslaHunter::startNewGame(bool) {
    gameScene = new GameGraphicScene(
        ui->gameGraphicsView->rect(),
        ui->gameGraphicsView
    );
    gameEngine = new GameEngine(gameScene);

    QObject::connect(gameScene, &GameGraphicScene::keyPressed, gameEngine, &GameEngine::keyPressed);
    QObject::connect(gameScene, &GameGraphicScene::keyReleased, gameEngine, &GameEngine::keyReleased);
    QObject::connect(gameEngine, &GameEngine::gameFinished, this, &SpaceTeslaHunter::gameFinished);
    QObject::connect(gameEngine, &GameEngine::playerHealthChanged, ui->playerHealthDisplay, qOverload<int>(&QLCDNumber::display));

    gameEngine->StartNewGame();

    ui->gameGraphicsView->setScene(gameScene);
    ui->playerHealthDisplay->show();
}

void SpaceTeslaHunter::startNewCoopGame(bool) {

}

void SpaceTeslaHunter::exitGame(bool) {
    QApplication::quit();
}

void SpaceTeslaHunter::gameFinished(GameStatus status) {
    switch (status) {
    case GameStatus::Victory:
        showMessage(this, resultsResourcePaths.at(ResourceId::Victory));
        break;
    case GameStatus::Defeat:
        showMessage(this, resultsResourcePaths.at(ResourceId::Defeat));
        break;
    default:
        break;
    }
    deleteGameObjects();
    ui->gameGraphicsView->setScene(menuScene);
    ui->playerHealthDisplay->hide();
}

SpaceTeslaHunter::~SpaceTeslaHunter()
{
    music->stop();
    delete music;

    deleteGameObjects();

    if (menuScene) {
        delete menuScene;
        menuScene = nullptr;
    }

    delete ui;
}

void SpaceTeslaHunter::deleteGameObjects() {
    if (gameEngine) {
        delete gameEngine;
        gameEngine = nullptr;
    }
    if (gameScene) {
        delete gameScene;
        gameScene = nullptr;
    }
}


