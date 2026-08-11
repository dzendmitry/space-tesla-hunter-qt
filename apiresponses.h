#ifndef APIRESPONSES_H
#define APIRESPONSES_H

#include <QJsonObject>

class ApiError
{
public:
    static ApiError fromJson(const QJsonObject &json, QJsonParseError *error = nullptr);
public:
    struct Error {
        QString code;    // json:"code"
        QString message; // json:"message"
    } error;             // json:"error"
};

class JoinResponse
{
public:
    static JoinResponse fromJson(const QJsonObject &json, QJsonParseError *error = nullptr);
    //QJsonObject toJson() const;
public:
    uint32_t roomId;      // json:"roomId"
    uint8_t playerSlot;  // json:"playerSlot"
    uint64_t sessionToken; // json:"sessionToken"
    struct GameConfig {
        int mapWidth;     // json:"mapWidth"
        int mapHeight;    // json:"mapHeight"
        int shipWidth;    // json:"shipWidth"
        int shipHeight;   // json:"shipHeight"
        int bulletWidth;  // json:"bulletWidth"
        int bulletHeight; // json:"bulletHeight"
        int tickRateHz;   // json:"tickRateHz"
        int maxPlayers;   // json:"maxPlayers"
    } gameConfig;         // json:"gameConfig"
};

#endif // APIRESPONSES_H
