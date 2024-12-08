#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QButtonGroup>
#include <QPushButton>
#include <QTreeView>
#include <QFileSystemModel>
#include <QMenu>
#include <QMessageBox>
#include <QInputDialog>
#include <QFileDialog>
#include "npc.h"
#include <QStandardItemModel>


QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr); // Конструктор
    ~MainWindow();                                  // Деструктор

private slots:
    void on_monsters_pushButton_released();         // Переключение на страницу "Бестиарий"
    void on_spells_pushButton_released();           // Переключение на страницу "Заклинания"
    void on_item_pushButton_released();             // Переключение на страницу "Предметы"
    void on_cart_pushButton_released();             // Переключение на страницу "Карты"

    void on_cart_treeView_customContextMenuRequested(const QPoint &pos);
    void on_monsters_treeView_customContextMenuRequested(const QPoint &pos);
    void openMap(const QModelIndex &index);

private:
    void setupSideMenu();                           // Настройка бокового меню
    void setupTreeView();                           // Настройка файлового дерева
    void setupMonstersTreeView();
    void addNpcToTreeView(const NPC &npc);

    // Модель для дерева
    QStandardItemModel *npcModel;

    Ui::MainWindow *ui;                             // Интерфейс пользователя
    QPushButton *activeButton = nullptr;            // Текущая активная кнопка
    QFileSystemModel *fileSystemModel;              // Модель файловой системы
};

#endif // MAINWINDOW_H
