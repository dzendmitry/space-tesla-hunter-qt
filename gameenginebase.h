#ifndef GAMEENGINEBASE_H
#define GAMEENGINEBASE_H

#include <QGraphicsScene>

enum class GameStatus { InProgress, Victory, Defeat,  };

class GameEngineBase : public QObject
{
    Q_OBJECT

public:
    explicit GameEngineBase(QGraphicsScene* gameScene, QObject *parent = nullptr);
    virtual ~GameEngineBase();

    virtual void StartNewGame() = 0;

signals:
    void gameError(QString);
    void playerHealthChanged(int);
    void gameFinished(GameStatus);

public slots:
    void keyPressed(int);
    void keyReleased(int);

protected:
    QGraphicsScene* gameScene;
    uint32_t actions;
};

#endif // GAMEENGINEBASE_H
