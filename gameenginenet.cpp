#include "gameenginenet.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QNetworkDatagram>

ShipType fromShipRecordType(ShipRecordType srt) {
    switch (srt) {
    default:
        throw std::runtime_error("Unknown ship type");
    case ShipRecordType::Player:
        return ShipType::Player;
    case ShipRecordType::Enemy:
        return ShipType::Enemy;
    }
}

BulletType fromBulletRecordType(BulletRecordType brt) {
    switch (brt) {
    default:
        throw std::runtime_error("Unknown bullet type");
    case BulletRecordType::Player:
        return BulletType::Player;
    case BulletRecordType::Enemy:
        return BulletType::Enemy;
    }
}

GameStatus fromGameRecordStatus(GameRecordStatus grs) {
    switch (grs) {
    default:
        throw std::runtime_error("Unknown game status");
    case GameRecordStatus::InProgress:
        return GameStatus::InProgress;
    case GameRecordStatus::Victory:
        return GameStatus::Victory;
    case GameRecordStatus::Defeat:
        return GameStatus::Defeat;
    }
}

GameEngineNet::GameEngineNet(QGraphicsScene* gameScene, QObject* parent)
    : GameEngineBase(gameScene, parent)
{}

GameEngineNet::~GameEngineNet() {
    for (auto it = ships.begin(); it != ships.end();) {
        Ship* ship = it->second;
        it = ships.erase(it);
        delete ship;
    }
    for (auto it = bullets.begin(); it != bullets.end();) {
        Bullet* bullet = it->second;
        it = bullets.erase(it);
        delete bullet;
    }
}

QString GameEngineNet::joinGame() {
    const QUrl httpUrl(HTTP_SERVER + "/api/v1/rooms/join");
    QNetworkRequest joinRequest(httpUrl);

    QNetworkAccessManager qnam;
    QScopedPointer<QNetworkReply, QScopedPointerDeleteLater> joinReply;
    joinReply.reset(qnam.post(joinRequest, nullptr));

    QEventLoop loop;
    QObject::connect(joinReply.get(), &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    QNetworkReply::NetworkError error = joinReply->error();
    const QString &errorString = joinReply->errorString();
    if (error != QNetworkReply::NoError) {
        return QString("join reply error: ") + errorString;
    }

    QByteArray joinReplyData = joinReply->readAll();
    joinReply.reset();

    qDebug() << joinReplyData;

    QJsonParseError docParseError;
    QJsonDocument jd = QJsonDocument::fromJson(joinReplyData, &docParseError);
    if (docParseError.error != QJsonParseError::NoError) {
        return QString("join reply doc parse error: ") + docParseError.errorString();
    }

    QJsonParseError objParseError;
    joinResponse = JoinResponse::fromJson(jd.object(), &objParseError);
    if (objParseError.error != QJsonParseError::NoError) {
        return QString("join reply obj parse error: ") + objParseError.errorString();
    }

    return "";
}

void GameEngineNet::StartNewGame() {

    if (QString err = joinGame(); err != "") {
        emit gameError(err);
        return;
    }

    if(!udpSocket.bind(QHostAddress::Any)) {
        QString err = "unable to bind udp socket";
        emit gameError(err);
        return;
    }
    udpSocketReceiverConnection = QObject::connect(
        &udpSocket, &QUdpSocket::readyRead, this, &GameEngineNet::processPendingDatagrams);

    udpSocketSender.setInterval(60); // 60 ms
    udpSocketSenderConnection = QObject::connect(
        &udpSocketSender, &QTimer::timeout, this, &GameEngineNet::sendUdpHandler);
    udpSocketSender.start();

    countdown = DELAY_BEFORE_START_SEC;
    emit playerHealthChanged(countdown);
    startGameSec.setInterval(1000);
    QObject::connect(
        &startGameSec, &QTimer::timeout, this, &GameEngineNet::countDownHandler);
    startGameSec.start();

}

void GameEngineNet::processPendingDatagrams() {
    while (udpSocket.hasPendingDatagrams()) {
        startGameSec.stop();

        QNetworkDatagram dgram = udpSocket.receiveDatagram();
        Snapshot s = Snapshot::unpack(dgram.data());

        // gg
        GameStatus gameStatus = fromGameRecordStatus((GameRecordStatus)s.status);
        if (gameStatus != GameStatus::InProgress) {
            QObject::disconnect(udpSocketReceiverConnection);
            QObject::disconnect(udpSocketSenderConnection);
            udpSocketSender.stop();
            udpSocket.close();
            emit gameFinished(gameStatus);
            return;
        }

        std::map<uint32_t, nullptr_t> shipIds;
        std::map<uint32_t, nullptr_t> bulletIds;

        // put ships on the ground
        for (int i = 0; i < s.ships.size(); i++) {
            ShipRecord sr = s.ships.at(i);
            shipIds[sr.id] = nullptr;

            Ship* ship;
            try {
                ship = ships.at(sr.id);
            } catch(const std::out_of_range& ex) {

                ShipType stype = ShipType::Player;
                if (sr.isEnemy()) {
                    stype = ShipType::Enemy;
                }

                ShipSide sside = ShipSide::Local;
                if (stype == ShipType::Player && !sr.isPlayer(joinResponse.playerSlot)) {
                    sside = ShipSide::Remote;
                }

                ship = new Ship(stype, sside);
                ships[sr.id]= ship;
                gameScene->addItem(ship);

                if (stype == ShipType::Player && sside == ShipSide::Local) {
                    QObject::connect(ship, &Ship::healthChanged, this, &GameEngineBase::playerHealthChanged);
                    ship->decHealth(0);
                    gameScene->setFocusItem(ship->focusItem());
                    ship->grabKeyboard();
                }
            }

            ship->setHealth(sr.health);
            ship->setPos(sr.x, sr.y);
        }

        // put bullets on the ground
        for (int i = 0; i < s.bullets.size(); i++) {
            BulletRecord br = s.bullets.at(i);
            bulletIds[br.id] = nullptr;

            Bullet *bullet;
            try {
                bullet = bullets.at(br.id);

            } catch (const std::out_of_range& ex) {
                bullet = new Bullet(fromBulletRecordType((BulletRecordType)br.type));
                bullets[br.id] = bullet;
                gameScene->addItem(bullet);
            }

            bullet->setPos(br.x, br.y);
        }

        // ships gc
        for (auto it = ships.begin(); it != ships.end();) {
            uint32_t shipId = it->first;
            Ship* ship = it->second;
            try {
                shipIds.at(shipId);
                it++;
            } catch (const std::out_of_range& ex) {
                ship->setHealth(0);
                gameScene->removeItem(ship);
                it = ships.erase(it);
                delete ship;
            }
        }

        // bullets gc
        for (auto it = bullets.begin(); it != bullets.end();) {
            uint32_t bulletId = it->first;
            Bullet* bullet = it->second;
            try {
                bulletIds.at(bulletId);
                it++;
            } catch (const std::out_of_range& ex) {
                gameScene->removeItem(bullet);
                it = bullets.erase(it);
                delete bullet;
            }
        }
    }
}

void GameEngineNet::sendUdpHandler() {
    seq++;
    QByteArray datagram = Input{
        .h = {
            .msgType = (uint8_t)MsgType::MsgInput,
            .version = 1,
            .roomId = (uint8_t)joinResponse.roomId,
            .playerSlot = joinResponse.playerSlot,
            .sessionToken = joinResponse.sessionToken,
            .sequence = seq,
        },
        .actions = (uint8_t)actions,
    }.pack();
    if (udpSocket.writeDatagram(datagram, QHostAddress(UDP_SERVER), UDP_PORT) == -1) {
        qDebug() << "unable to send datagram";
    }
}

void GameEngineNet::countDownHandler() {
    countdown--;
    emit playerHealthChanged(countdown);
}

QByteArray Input::pack() {
    QByteArray r;
    QDataStream stream(&r, QIODevice::Append);
    stream.setByteOrder(QDataStream::BigEndian);
    stream << h.msgType << h.version;
    stream << h.roomId << h.playerSlot;
    stream << h.sessionToken << h.sequence;
    stream << actions;
    return r;
}

Snapshot Snapshot::unpack(QByteArray data) {
    Snapshot sn;
    QDataStream s(data);
    s.setByteOrder(QDataStream::BigEndian);
    // header
    s >> sn.h.msgType >> sn.h.version;
    s >> sn.h.roomId >> sn.h.playerSlot;
    s >> sn.h.sessionToken >> sn.h.sequence;
    // snapshot static
    s >> sn.status;
    s >> sn.roster[0] >> sn.roster[1] >> sn.roster[2];
    s >> sn.shipsCount;
    s >> sn.bulletsCount;
    // snapshot dynamic
    for (int i = 0; i < sn.shipsCount; i++) {
        sn.ships.push_back(ShipRecord::unpack(s));
    }
    for (int i = 0; i < sn.bulletsCount; i++) {
        sn.bullets.push_back(BulletRecord::unpack(s));
    }
    return sn;
}

ShipRecord ShipRecord::unpack(QDataStream &s) {
    ShipRecord r;
    s >> r.id >> r.owner;
    s >> r.x >> r.y;
    s >> r.health >> r.spriteVariant;
    return r;
}

BulletRecord BulletRecord::unpack(QDataStream &s) {
    BulletRecord r;
    s >> r.id >> r.type;
    s >> r.x >> r.y;
    return r;
}