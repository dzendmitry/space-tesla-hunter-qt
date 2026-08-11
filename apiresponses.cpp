#include "apiresponses.h"

ApiError ApiError::fromJson(const QJsonObject &json, QJsonParseError *error) {
    ApiError r;

    if (const QJsonValue errorOjb = json["error"]; errorOjb.isObject()) {
        if (const QJsonValue v = errorOjb["code"]; v.isString()) {
            r.error.code = v.toString();
        }
        if (const QJsonValue v = errorOjb["message"]; v.isString()) {
            r.error.message = v.toString();
        }
    }

    return r;
}

JoinResponse JoinResponse::fromJson(const QJsonObject &json, QJsonParseError *error) {
    JoinResponse r;

    if (const QJsonValue v = json["roomId"]; v.isDouble()) {
        r.roomId = v.toInt();
    }
    if (const QJsonValue v = json["playerSlot"]; v.isDouble()) {
        r.playerSlot = v.toInt();
    }
    if (const QJsonValue v = json["sessionToken"]; v.isString()) {
        bool ok;
        r.sessionToken = v.toString().toULongLong(&ok, 10);
        if (!ok && error != nullptr) {
            error->error = QJsonParseError::IllegalValue;
        }
    }
    if (const QJsonValue gameConfig = json["gameConfig"]; gameConfig.isObject()) {
        if (const QJsonValue v = gameConfig["mapWidth"]; v.isDouble()) {
            r.gameConfig.mapWidth = v.toInt();
        }
        if (const QJsonValue v = gameConfig["mapHeight"]; v.isDouble()) {
            r.gameConfig.mapHeight = v.toInt();
        }
        if (const QJsonValue v = gameConfig["shipWidth"]; v.isDouble()) {
            r.gameConfig.shipWidth = v.toInt();
        }
        if (const QJsonValue v = gameConfig["shipHeight"]; v.isDouble()) {
            r.gameConfig.shipHeight = v.toInt();
        }
        if (const QJsonValue v = gameConfig["bulletWidth"]; v.isDouble()) {
            r.gameConfig.bulletWidth = v.toInt();
        }
        if (const QJsonValue v = gameConfig["bulletHeight"]; v.isDouble()) {
            r.gameConfig.bulletHeight = v.toInt();
        }
        if (const QJsonValue v = gameConfig["tickRateHz"]; v.isDouble()) {
            r.gameConfig.tickRateHz = v.toInt();
        }
        if (const QJsonValue v = gameConfig["maxPlayers"]; v.isDouble()) {
            r.gameConfig.maxPlayers = v.toInt();
        }
    }

    return r;
}
