#ifndef SPACETESLAHUNTER_H
#define SPACETESLAHUNTER_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QMediaPlayer>

#include "gameengine.h"
#include "gamegraphicscene.h"
#include "menugraphicsscene.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class SpaceTeslaHunter;
}
QT_END_NAMESPACE

class SpaceTeslaHunter : public QMainWindow
{
    Q_OBJECT

public:
    explicit SpaceTeslaHunter(QWidget *parent = nullptr);
    ~SpaceTeslaHunter() override;

private slots:
    void startNewGame(bool);
    void startNewCoopGame(bool);
    void exitGame(bool);

    void gameFinished(GameStatus status);

private:
    Ui::SpaceTeslaHunter *ui;

    QMediaPlayer *music;

    MenuGraphicsScene *menuScene;

    GameEngine *gameEngine;
    GameGraphicScene *gameScene;

    void deleteGameObjects();

};
#endif // SPACETESLAHUNTER_H
