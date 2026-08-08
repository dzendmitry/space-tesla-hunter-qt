#ifndef RESOURCES_H
#define RESOURCES_H

#include <map>
#include <QPixmap>

enum class ResourceId {
    // music
    MainMusicTrack,
    //
    Victory,
    Defeat,
    // menu_scene
    MenuSceneBackground,
    MenuSceneButton1,
    MenuSceneButton2,
    MenuSceneButton3,
    // game_scene
    GameSceneBackground,
    // bullets
    PlayerBullet,
    EnemyBullet,
    // player
    PlayerLevel100,
    PlayerLevel70,
    PlayerLevel40,
    PlayerLevel10,
    PlayerLevel0,
    // enemy_1
    Enemy1Level100,
    Enemy1Level70,
    Enemy1Level40,
    Enemy1Level10,
    Enemy1Level0,
    // enemy_2
    Enemy2Level100,
    Enemy2Level70,
    Enemy2Level40,
    Enemy2Level10,
    Enemy2Level0,
    // enemy_3
    Enemy3Level100,
    Enemy3Level70,
    Enemy3Level40,
    Enemy3Level10,
    Enemy3Level0,
};

static std::map<ResourceId, QString> utilsResourcePaths = {
    {ResourceId::MainMusicTrack, ":/resources/music/main.mp3"},
};

static std::map<ResourceId, QString> resultsResourcePaths = {
    {ResourceId::Victory, ":/resources/images/victory.png"},
    {ResourceId::Defeat, ":/resources/images/defeat.png"},
};

static std::map<ResourceId, QString> sceneResourcePaths = {
    // menu_scene
    {ResourceId::MenuSceneBackground, ":/resources/images/menu/menu_background_2.png"},
    {ResourceId::MenuSceneButton1, ":/resources/images/menu/menu_button_1.png"},
    {ResourceId::MenuSceneButton2, ":/resources/images/menu/menu_button_2.png"},
    {ResourceId::MenuSceneButton3, ":/resources/images/menu/menu_button_3.png"},
    // game_scene
    {ResourceId::GameSceneBackground, ":/resources/images/background.png"},
};

static std::map<ResourceId, QString> bulletResourcePaths = {
    // bullets
    {ResourceId::PlayerBullet, ":/resources/images/bullet_player_1.png"},
    {ResourceId::EnemyBullet, ":/resources/images/bullet_enemy_1.png"},
};

static std::map<ResourceId, QString> shipResourcePaths = {
    // player
    {ResourceId::PlayerLevel100, ":/resources/images/player/player.png"},
    {ResourceId::PlayerLevel70, ":/resources/images/player/player_damaged_70.png"},
    {ResourceId::PlayerLevel40, ":/resources/images/player/player_damaged_40.png"},
    {ResourceId::PlayerLevel10, ":/resources/images/player/player_damaged_10.png"},
    {ResourceId::PlayerLevel0, ":/resources/images/player/player_damaged_0.png"},
    // enemy_1
    {ResourceId::Enemy1Level100, ":/resources/images/enemy_1/enemy_1.png"},
    {ResourceId::Enemy1Level70, ":/resources/images/enemy_1/enemy_1_damaged_70.png"},
    {ResourceId::Enemy1Level40, ":/resources/images/enemy_1/enemy_1_damaged_40.png"},
    {ResourceId::Enemy1Level10, ":/resources/images/enemy_1/enemy_1_damaged_10.png"},
    {ResourceId::Enemy1Level0, ":/resources/images/enemy_1/enemy_1_damaged_0.png"},
    // enemy_2
    {ResourceId::Enemy2Level100, ":/resources/images/enemy_2/enemy_2.png"},
    {ResourceId::Enemy2Level70, ":/resources/images/enemy_2/enemy_2_damaged_70.png"},
    {ResourceId::Enemy2Level40, ":/resources/images/enemy_2/enemy_2_damaged_40.png"},
    {ResourceId::Enemy2Level10, ":/resources/images/enemy_2/enemy_2_damaged_10.png"},
    {ResourceId::Enemy2Level0, ":/resources/images/enemy_2/enemy_2_damaged_0.png"},
    // enemy_3
    {ResourceId::Enemy3Level100, ":/resources/images/enemy_3/enemy_3.png"},
    {ResourceId::Enemy3Level70, ":/resources/images/enemy_3/enemy_3_damaged_70.png"},
    {ResourceId::Enemy3Level40, ":/resources/images/enemy_3/enemy_3_damaged_40.png"},
    {ResourceId::Enemy3Level10, ":/resources/images/enemy_3/enemy_3_damaged_10.png"},
    {ResourceId::Enemy3Level0, ":/resources/images/enemy_3/enemy_3_damaged_0.png"},
};

struct ResourcesParams {
    // ship
    unsigned ShipWidth;
    unsigned ShipHeight;
    // bullet
    unsigned BulletWidth;
    unsigned BulletHeight;
};

class Resources
{
public:
    Resources() = delete;
    Resources(Resources &other) = delete;
    void operator=(const Resources &) = delete;
    void operator=(Resources &) = delete;

    static Resources* GetInstance() {
        if (resources == nullptr) {
            throw std::runtime_error("Resources must be initialized first via GetInstance with ResourcesParams!");
        }
        return resources;
    };
    static Resources* GetInstance(const ResourcesParams& params) {
        if (resources == nullptr) {
            resources = new Resources(params);
        }
        return resources;
    };

    const QPixmap& getPixmapById(ResourceId id);
protected:
    Resources(const ResourcesParams& params);
private:
    std::map<ResourceId, QPixmap> resourcesMap;
    static Resources *resources;
};

#endif // RESOURCES_H
