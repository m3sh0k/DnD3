#ifndef NPC_H
#define NPC_H

#include <QString>
#include <QJsonObject>

class NPC
{
public:
    NPC(const QString &name, const QString &characteristics, const QString &imagePath);

    QString getName() const;
    void setName(const QString &name);

    QString getCharacteristics() const;
    void setCharacteristics(const QString &characteristics);

    QString getImagePath() const;
    void setImagePath(const QString &imagePath);

    QJsonObject toJson() const;  // Преобразование NPC в JSON объект



private:
    QString name;
    QString characteristics;
    QString imagePath;
};

#endif // NPC_H
