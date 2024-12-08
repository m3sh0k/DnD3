#ifndef MAPWIDGET_H
#define MAPWIDGET_H

#include <QWidget>
#include <QImage>
#include <QPoint>
#include <QSize>
#include <QJsonObject>
#include <QPainter>

class MapWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MapWidget(QWidget *parent = nullptr);
    ~MapWidget();

    // Метод для установки изображения
    void setImage(const QImage &image);

    // Метод для сохранения изображения в формате JSON
    void saveImageAsJson(const QString &filePath);
    // Преобразование изображения в JSON формат
    QJsonObject imageToJson(const QImage &image);

protected:
    // Переопределенные методы для работы с отображением карты
    void paintEvent(QPaintEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    // Основные функции для работы с изображением
    void drawGrid(QPainter &painter);  // Рисование сетки
    void updateScale(double scale);    // Обновление масштаба
    void zoomIn();                    // Увеличение масштаба
    void zoomOut();                   // Уменьшение масштаба



    // Переменные для масштаба и смещения
    double scaleFactor;               // Масштаб
    bool isDragging;                  // Флаг перетаскивания
    QPoint lastMousePos;              // Последняя позиция мыши
    QPoint offset;                    // Смещение для перетаскивания
    QImage mapImage;                  // Изображение карты
};

#endif // MAPWIDGET_H
