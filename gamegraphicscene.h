#ifndef GAMEGRAPHICSCENE_H
#define GAMEGRAPHICSCENE_H

#include <QKeyEvent>
#include <QPainter>
#include <QGraphicsScene>

class GameGraphicScene : public QGraphicsScene
{
    Q_OBJECT

public:
    explicit GameGraphicScene(const QRectF &sceneRect, QObject *parent = nullptr);
    ~GameGraphicScene() override;

signals:
    void keyPressed(int);
    void keyReleased(int);

protected:
    virtual void keyPressEvent(QKeyEvent *event) override;
    virtual void keyReleaseEvent(QKeyEvent *event) override;
    virtual void drawBackground(QPainter *painter, const QRectF &rect) override;

private:
    static const Qt::GlobalColor backgroundColor = Qt::gray;
};

#endif // GAMEGRAPHICSCENE_H