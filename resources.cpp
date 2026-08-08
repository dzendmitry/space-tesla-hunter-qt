#include "resources.h"

Resources* Resources::resources = nullptr;

Resources::Resources(const ResourcesParams& params) {
    for (const auto& [k, v] : sceneResourcePaths) {
        resourcesMap[k] = QPixmap(v);
    }
    for (const auto& [k, v] : bulletResourcePaths) {
        resourcesMap[k] = QPixmap(v).scaled(
            params.BulletWidth,
            params.BulletHeight,
            Qt::IgnoreAspectRatio,
            Qt::SmoothTransformation
        );
    }
    for (const auto& [k, v] : shipResourcePaths) {
        resourcesMap[k] = QPixmap(v).scaled(
            params.ShipWidth,
            params.ShipHeight,
            Qt::IgnoreAspectRatio,
            Qt::SmoothTransformation
        );
    }
}

const QPixmap& Resources::getPixmapById(ResourceId id) {
    return resourcesMap.at(id);
}

