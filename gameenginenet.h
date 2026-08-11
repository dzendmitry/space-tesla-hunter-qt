#ifndef GAMEENGINENET_H
#define GAMEENGINENET_H

#include <QString>
#include <QUdpSocket>
#include <QTimer>

#include "gameenginebase.h"
#include "apiresponses.h"

#include "ship.h"
#include "bullet.h"

const QString HTTP_SERVER = "http://127.0.0.1:8080";
const QString UDP_SERVER = "127.0.0.1";
const quint16 UDP_PORT = 8081;
const int DELAY_BEFORE_START_SEC = 30;

enum class MsgType {
    MsgInput = 1,
    MsgSnapshot = 2,
};

const uint8_t SHIP_RECORD_ENEMY = 0xFF;

enum class ShipRecordType {
    Player = 0,
    Enemy  = 1,
};
ShipType fromShipRecordType(ShipRecordType);

enum class BulletRecordType {
    Player = 0,
    Enemy  = 1,
};
BulletType fromBulletRecordType(BulletRecordType);

enum class GameRecordStatus {
    InProgress = 0,
    Victory    = 1,
    Defeat     = 2,
};
GameStatus fromGameRecordStatus(GameRecordStatus);

struct Header {
    uint8_t  msgType;
    uint8_t  version;
    uint32_t roomId;
    uint8_t  playerSlot;
    uint64_t sessionToken;
    uint32_t sequence;
};

struct Input {
    Header h;
    uint8_t actions;

    QByteArray pack();
};

struct ShipRecord {
    uint8_t id;
    uint8_t owner;
    int16_t x;
    int16_t y;
    uint8_t health;
    uint8_t spriteVariant;

    static ShipRecord unpack(QDataStream&);
    bool isEnemy() {
        return owner == SHIP_RECORD_ENEMY;
    }
    bool isPlayer(uint8_t playerSlot) {
        return owner == playerSlot;
    }
};

struct BulletRecord {
    uint8_t id;
    uint8_t type;
    int16_t x;
    int16_t y;

    static BulletRecord unpack(QDataStream&);
};

struct Snapshot {
    Header h;

    uint8_t status;
    uint8_t roster[3];
    uint8_t shipsCount;
    uint8_t bulletsCount;

    std::vector<ShipRecord> ships;
    std::vector<BulletRecord> bullets;

    static Snapshot unpack(QByteArray);
};

class GameEngineNet : public GameEngineBase
{
public:
    explicit GameEngineNet(QGraphicsScene* gameScene, QObject *parent = nullptr);
    ~GameEngineNet();

    void StartNewGame();

private slots:
    void processPendingDatagrams();
    void sendUdpHandler();
    void countDownHandler();

private:
    std::map<uint32_t, Ship*> ships;
    std::map<uint32_t, Bullet*> bullets;

    QString joinGame();
    JoinResponse joinResponse;

    uint32_t seq;
    QUdpSocket udpSocket;
    QTimer udpSocketSender;
    QMetaObject::Connection udpSocketReceiverConnection;
    QMetaObject::Connection udpSocketSenderConnection;

    QTimer startGameSec;
    int countdown;
};

#endif // GAMEENGINENET_H
