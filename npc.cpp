#include "npc.h"

NPC::NPC(const QString &name, const QString &characteristics, const QString &imagePath)
    : name(name), characteristics(characteristics), imagePath(imagePath)
{
}

QString NPC::getName() const {
    return name;
}

void NPC::setName(const QString &name) {
    this->name = name;
}

QString NPC::getCharacteristics() const {
    return characteristics;
}

void NPC::setCharacteristics(const QString &characteristics) {
    this->characteristics = characteristics;
}

QString NPC::getImagePath() const {
    return imagePath;
}

void NPC::setImagePath(const QString &imagePath) {
    this->imagePath = imagePath;
}

QJsonObject NPC::toJson() const {
    QJsonObject json;
    json["name"] = name;
    json["characteristics"] = characteristics;
    json["image"] = imagePath;
    return json;
}
