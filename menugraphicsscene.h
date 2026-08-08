#ifndef MENUGRAPHICSSCENE_H
#define MENUGRAPHICSSCENE_H

#include <QGraphicsScene>
#include <QPainter>
#include <QPushButton>

class MenuGraphicsScene : public QGraphicsScene
{
    Q_OBJECT

public:
    explicit MenuGraphicsScene(const QRectF &sceneRect, QObject *parent = nullptr);
    ~MenuGraphicsScene() override;

    const int DEFAULT_BUTTON_WIDTH = 400;
    const int DEFAULT_BUTTON_HEIGHT = 100;

    const QString EMPTY_BACKGROUND_STYPE = "background: rgba(255, 0, 0, 0);";
    // background-color: none;
    // background: transparent;

signals:
    void startGameButtonClicked(bool clicked = false);
    void startCoopGameButtonClicked(bool clicked = false);
    void exitGameButtonClicked(bool clicked = false);

protected:
    virtual void drawBackground(QPainter *painter, const QRectF &rect) override;

private:
    QPushButton* startGameButton;
    void addStartGameButton(const QRectF &sceneRect);
    QPushButton* startCoopGameButton;
    void addStartCoopGameButton(const QRectF &sceneRect);
    QPushButton* exitGameButton;
    void addExitGameButton(const QRectF &sceneRect);
};

#endif // MENUGRAPHICSSCENE_H
