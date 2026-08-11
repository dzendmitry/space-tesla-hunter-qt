#ifndef SPACETESLAHUNTER_H
#define SPACETESLAHUNTER_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QMediaPlayer>

#include "gameenginebase.h"
#include "gamegraphicscene.h"
#include "menugraphicsscene.h"

enum class GameMode { Single, Coop };

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
    void startNewSingleGame(bool);
    void startNewCoopGame(bool);
    void exitGame(bool);

    void gameFinished(GameStatus);

private:
    Ui::SpaceTeslaHunter *ui;

    QMediaPlayer *music;

    MenuGraphicsScene *menuScene;

    GameEngineBase *gameEngine;
    GameGraphicScene *gameScene;

    void startNewGame(GameMode);
    void deleteGameObjects();

};
#endif // SPACETESLAHUNTER_H
