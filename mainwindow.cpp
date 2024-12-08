#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "mapwidget.h"
#include <QJsonDocument>  // Для работы с JSON
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QMessageBox>
#include <QFileInfo>
#include <QImage>
#include <QBuffer>
#include <QImageWriter>
#include <QByteArray>

    MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setupSideMenu();  // Настраиваем боковое меню
    setupTreeView();  // Настраиваем файловое дерево

    connect(ui->cart_treeView, &QTreeView::doubleClicked, this, &MainWindow::openMap);

}

MainWindow::~MainWindow()
{
    delete ui;
}

// Настройка бокового меню
void MainWindow::setupSideMenu()
{
    // Создаем группу кнопок и добавляем кнопки
    QButtonGroup *buttonGroup = new QButtonGroup(this);
    buttonGroup->addButton(ui->monsters_pushButton);  // Кнопка "Бестиарий"
    buttonGroup->addButton(ui->spells_pushButton);    // Кнопка "Заклинания"
    buttonGroup->addButton(ui->item_pushButton);      // Кнопка "Предметы"
    buttonGroup->addButton(ui->cart_pushButton);      // Кнопка "Карты"
    buttonGroup->setExclusive(true);                 // Группа делает кнопки эксклюзивными
}

// Настройка файлового дерева
void MainWindow::setupTreeView()
{
    // Путь к папке map
    QString mapsPath = QDir::currentPath() + "/map";

    // Проверка и создание папки map, если её нет
    QDir dir;
    if (!dir.exists(mapsPath)) {
        dir.mkpath(mapsPath);  // Создать папку, если её нет
    }

    // Создаем модель файловой системы
    fileSystemModel = new QFileSystemModel(this);
    fileSystemModel->setRootPath(mapsPath);  // Устанавливаем корень как папку "map"

    ui->cart_treeView->setModel(fileSystemModel);       // Привязываем модель к TreeView
    ui->cart_treeView->setRootIndex(fileSystemModel->index(mapsPath));  // Устанавливаем корень TreeView на "map"

    // Скрываем ненужные столбцы
    ui->cart_treeView->hideColumn(1); // Скрываем "Size"
    ui->cart_treeView->hideColumn(2); // Скрываем "Type"
    ui->cart_treeView->hideColumn(3); // Скрываем "Date"
    ui->cart_treeView->setHeaderHidden(true);  // Отключить заголовки
}


// Функция переключения на "Бестиарий"
void MainWindow::on_monsters_pushButton_released()
{
    if (activeButton == ui->monsters_pushButton) {
        ui->extended_side_menu->setVisible(!ui->extended_side_menu->isVisible());
    } else {
        activeButton = ui->monsters_pushButton;
        ui->extended_side_menu->setCurrentIndex(0); // Устанавливаем страницу "Бестиарий"
        ui->extended_side_menu->setVisible(true);
    }
}

// Функция переключения на "Заклинания"
void MainWindow::on_spells_pushButton_released()
{
    if (activeButton == ui->spells_pushButton) {
        ui->extended_side_menu->setVisible(!ui->extended_side_menu->isVisible());
    } else {
        activeButton = ui->spells_pushButton;
        ui->extended_side_menu->setCurrentIndex(1); // Устанавливаем страницу "Заклинания"
        ui->extended_side_menu->setVisible(true);
    }
}

// Функция переключения на "Предметы"
void MainWindow::on_item_pushButton_released()
{
    if (activeButton == ui->item_pushButton) {
        ui->extended_side_menu->setVisible(!ui->extended_side_menu->isVisible());
    } else {
        activeButton = ui->item_pushButton;
        ui->extended_side_menu->setCurrentIndex(2); // Устанавливаем страницу "Предметы"
        ui->extended_side_menu->setVisible(true);
    }
}

// Функция переключения на "Карты"
void MainWindow::on_cart_pushButton_released()
{
    if (activeButton == ui->cart_pushButton) {
        ui->extended_side_menu->setVisible(!ui->extended_side_menu->isVisible());
    } else {
        activeButton = ui->cart_pushButton;
        ui->extended_side_menu->setCurrentIndex(3); // Устанавливаем страницу "Карты"
        ui->extended_side_menu->setVisible(true);
    }
}

// Контекстное меню
void MainWindow::on_cart_treeView_customContextMenuRequested(const QPoint &pos)
{
    QMenu contextMenu(this);  // Создаем контекстное меню
    QAction *addAction = contextMenu.addAction("Добавить папку");
    QAction *addMapAction = contextMenu.addAction("Добавить карту");
    QAction *removeAction = contextMenu.addAction("Удалить");

    // Показываем меню в точке клика
    QAction *selectedAction = contextMenu.exec(ui->cart_treeView->mapToGlobal(pos));

    if (selectedAction == addAction) {
        QModelIndex index = ui->cart_treeView->indexAt(pos);

        QDir dir;
        QString parentDirPath;

        if (index.isValid()) {
            parentDirPath = fileSystemModel->filePath(index);
        } else {
            parentDirPath = QDir::currentPath() + "/map";
        }

        dir.setPath(parentDirPath);

        if (!dir.exists() && !parentDirPath.endsWith("/map")) {
            dir.mkdir(parentDirPath);
        }

        bool ok;
        QString folderName = QInputDialog::getText(this, tr("Введите имя папки"), tr("Имя папки:"), QLineEdit::Normal, "", &ok);

        if (ok && !folderName.isEmpty()) {
            if (dir.mkdir(folderName)) {
                fileSystemModel->setRootPath(QDir::currentPath());
            } else {
                QMessageBox::warning(this, "Ошибка", "Не удалось создать папку.");
            }
        }
    } else if (selectedAction == addMapAction) {
        // Получаем индекс выбранного элемента
        QModelIndex index = ui->cart_treeView->indexAt(pos);
        QString parentDirPath;

        if (index.isValid()) {
            parentDirPath = fileSystemModel->filePath(index);
        } else {
            parentDirPath = QDir::currentPath() + "/map";
        }

        // Открываем диалог выбора файла
        QString filePath = QFileDialog::getOpenFileName(this, tr("Выберите карту"), QDir::homePath(), tr("Images (*.png *.jpg *.jpeg);;All Files (*)"));
        if (filePath.isEmpty())
            return;

        QFileInfo fileInfo(filePath);
        QString destinationPath = parentDirPath + "/" + fileInfo.fileName();

        if (QFile::exists(destinationPath)) {
            QMessageBox::warning(this, tr("Ошибка"), tr("Файл с таким именем уже существует!"));
            return;
        }

        if (QFile::copy(filePath, destinationPath)) {
            // Создаем JSON файл с изображением
            if (fileInfo.suffix().toLower() == "png" || fileInfo.suffix().toLower() == "jpg" || fileInfo.suffix().toLower() == "jpeg") {
                // Загружаем изображение
                QImage image(filePath);
                if (!image.isNull()) {
                    // Преобразуем изображение в JSON
                    MapWidget mapWidget;
                    QJsonObject json = mapWidget.imageToJson(image);

                    // Формируем путь для сохранения JSON
                    QString jsonFilePath = destinationPath + ".json";

                    // Сохраняем JSON в файл
                    QFile jsonFile(jsonFilePath);
                    if (jsonFile.open(QIODevice::WriteOnly)) {
                        QJsonDocument doc(json);
                        jsonFile.write(doc.toJson());
                        jsonFile.close();
                    } else {
                        QMessageBox::warning(this, tr("Ошибка"), tr("Не удалось сохранить JSON файл."));
                    }
                } else {
                    QMessageBox::warning(this, tr("Ошибка"), tr("Не удалось загрузить изображение."));
                }
            }

            QMessageBox::information(this, tr("Успех"), tr("Карта добавлена."));
            fileSystemModel->setRootPath(QDir::currentPath());  // Перезагружаем модель
        } else {
            QMessageBox::warning(this, tr("Ошибка"), tr("Не удалось добавить карту."));
        }
    } else if (selectedAction == removeAction) {
        QModelIndex index = ui->cart_treeView->indexAt(pos);
        if (index.isValid()) {
            QString filePath = fileSystemModel->filePath(index);
            QFileInfo fileInfo(filePath);
            if (fileInfo.isDir()) {
                QDir dir(filePath);
                if (!dir.removeRecursively()) {
                    QMessageBox::warning(this, "Ошибка", "Не удалось удалить папку.");
                }
            } else {
                QFile file(filePath);
                if (!file.remove()) {
                    QMessageBox::warning(this, "Ошибка", "Не удалось удалить файл.");
                }
            }
        }
    }
}


void MainWindow::openMap(const QModelIndex &index)
{
    // Получаем путь к файлу
    QString filePath = fileSystemModel->filePath(index);
    QFileInfo fileInfo(filePath);

    // Проверяем, является ли выбранный элемент файлом
    if (fileInfo.isFile()) {
        // Создаем новую вкладку
        QWidget *mapTab = new QWidget(ui->tabWidget);
        QVBoxLayout *layout = new QVBoxLayout(mapTab);

        // Если файл - это изображение
        if (fileInfo.suffix().toLower() == "png" || fileInfo.suffix().toLower() == "jpg" || fileInfo.suffix().toLower() == "jpeg" || fileInfo.suffix().toLower() == "bmp") {
            // Создаем MapWidget для отображения карты
            MapWidget *mapWidget = new MapWidget(mapTab);

            // Загружаем изображение
            QImage mapImage(filePath);
            if (!mapImage.isNull()) {
                mapWidget->setImage(mapImage);  // Устанавливаем изображение в MapWidget
                layout->addWidget(mapWidget);  // Добавляем MapWidget в layout

                // Добавляем вкладку в tabWidget
                ui->tabWidget->addTab(mapTab, fileInfo.fileName());
                ui->tabWidget->setCurrentWidget(mapTab);
            } else {
                QMessageBox::warning(this, "Ошибка", "Не удалось загрузить карту. Проверьте формат файла.");
                delete mapTab; // Удаляем вкладку, если загрузка не удалась
            }
        }
        // Если файл - это JSON
        else if (fileInfo.suffix().toLower() == "json") {
            // Читаем JSON файл
            QFile jsonFile(filePath);
            if (jsonFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QByteArray jsonData = jsonFile.readAll();
                jsonFile.close();

                // Парсим JSON
                QJsonDocument doc = QJsonDocument::fromJson(jsonData);
                if (!doc.isNull()) {
                    QJsonObject jsonObj = doc.object();

                    // Проверяем, есть ли в JSON нужные данные
                    if (jsonObj.contains("image") && jsonObj.contains("width") && jsonObj.contains("height")) {
                        // Декодируем Base64 изображение
                        QString base64Image = jsonObj["image"].toString();
                        QByteArray byteArray = QByteArray::fromBase64(base64Image.toUtf8());

                        // Создаем QImage из декодированных данных
                        QImage mapImage;
                        if (mapImage.loadFromData(byteArray)) {
                            // Создаем MapWidget для отображения карты
                            MapWidget *mapWidget = new MapWidget(mapTab);
                            mapWidget->setImage(mapImage);  // Устанавливаем изображение в MapWidget
                            layout->addWidget(mapWidget);  // Добавляем MapWidget в layout

                            // Добавляем вкладку в tabWidget
                            ui->tabWidget->addTab(mapTab, fileInfo.fileName());
                            ui->tabWidget->setCurrentWidget(mapTab);
                        } else {
                            QMessageBox::warning(this, "Ошибка", "Не удалось загрузить изображение из JSON.");
                            delete mapTab;
                        }
                    } else {
                        QMessageBox::warning(this, "Ошибка", "Некорректный формат JSON. Отсутствуют данные изображения.");
                        delete mapTab;
                    }
                } else {
                    QMessageBox::warning(this, "Ошибка", "Не удалось распарсить JSON.");
                    delete mapTab;
                }
            } else {
                QMessageBox::warning(this, "Ошибка", "Не удалось открыть JSON файл.");
                delete mapTab;
            }
        }
    }
}

