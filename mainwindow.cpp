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
#include <QStandardItemModel>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setupSideMenu();  // Настраиваем боковое меню
    setupTreeView();  // Настраиваем файловое дерево
    setupMonstersTreeView();  // Настраиваем отображение монстров

    connect(ui->cart_treeView, &QTreeView::doubleClicked, this, &MainWindow::openMap);
    connect(ui->monsters_treeView, &QTreeView::customContextMenuRequested, this, &MainWindow::on_monsters_treeView_customContextMenuRequested);
    ui->monsters_treeView->setContextMenuPolicy(Qt::CustomContextMenu);

    npcModel = new QStandardItemModel(this);
    ui->monsters_treeView->setModel(npcModel);

    loadMonstersFromFiles();  // Загружаем данные NPC при запуске

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
    QString mapsPath = QDir::currentPath() + "/map";
    QDir dir;
    if (!dir.exists(mapsPath)) {
        dir.mkpath(mapsPath);
    }

    fileSystemModel = new QFileSystemModel(this);
    fileSystemModel->setRootPath(mapsPath);

    ui->cart_treeView->setModel(fileSystemModel);
    ui->cart_treeView->setRootIndex(fileSystemModel->index(mapsPath));

    ui->cart_treeView->hideColumn(1);
    ui->cart_treeView->hideColumn(2);
    ui->cart_treeView->hideColumn(3);
    ui->cart_treeView->setHeaderHidden(true);
}

void MainWindow::setupMonstersTreeView()
{
    QString monstersPath = QDir::currentPath() + "/monsters";
    QDir dir(monstersPath);
    if (!dir.exists()) {
        dir.mkpath(monstersPath);
    }
}


void MainWindow::loadMonstersFromFiles()
{
    QString monstersPath = QDir::currentPath() + "/monsters";
    QDir dir(monstersPath);

    // Получаем список JSON файлов
    QStringList filters = {"*.json"};
    QStringList monsterFiles = dir.entryList(filters, QDir::Files);

    for (const QString &fileName : monsterFiles) {
        QString filePath = monstersPath + "/" + fileName;
        QFile file(filePath);
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QByteArray fileData = file.readAll();
            file.close();

            QJsonDocument doc = QJsonDocument::fromJson(fileData);
            if (!doc.isNull()) {
                QJsonObject npcObject = doc.object();
                QString name = npcObject["name"].toString();
                QString characteristics = npcObject["characteristics"].toString();
                QString imagePath = npcObject["image"].toString();

                // Добавляем NPC в модель
                QStandardItem *npcItem = new QStandardItem(name);
                npcItem->setData(characteristics, Qt::UserRole + 1);
                npcItem->setData(imagePath, Qt::UserRole + 2);
                npcModel->appendRow(npcItem);
            }
        }
    }
}

void MainWindow::addNpcToTreeView(const NPC &npc)
{
    QStandardItem *npcItem = new QStandardItem(npc.getName());
    npcItem->setData(npc.getCharacteristics(), Qt::UserRole + 1);
    npcItem->setData(npc.getImagePath(), Qt::UserRole + 2);
    npcModel->appendRow(npcItem);
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

void MainWindow::on_monsters_treeView_customContextMenuRequested(const QPoint &pos)
{
    QMenu contextMenu(this);  // Создаем контекстное меню

    QAction *addNpcAction = contextMenu.addAction("Добавить NPC");
    QAction *editNpcAction = contextMenu.addAction("Изменить NPC");
    QAction *removeNpcAction = contextMenu.addAction("Удалить NPC");

    // Показываем меню в точке клика
    QAction *selectedAction = contextMenu.exec(ui->monsters_treeView->mapToGlobal(pos));

    QModelIndex index = ui->monsters_treeView->indexAt(pos);
    QString npcFilePath;

    if (index.isValid()) {
        npcFilePath = fileSystemModel->filePath(index);
    }

    // Если выбрано добавление NPC
    if (selectedAction == addNpcAction) {
        // Открываем диалог для ввода имени NPC
        bool ok;
        QString name = QInputDialog::getText(this, "Введите имя NPC", "Имя NPC:", QLineEdit::Normal, "", &ok);
        if (!ok || name.isEmpty()) return;

        // Открываем диалог для ввода характеристик NPC
        QString characteristics = QInputDialog::getText(this, "Введите характеристики NPC", "Характеристики NPC:", QLineEdit::Normal, "", &ok);
        if (!ok || characteristics.isEmpty()) return;

        // Открываем диалог для выбора изображения NPC
        QString imagePath = QFileDialog::getOpenFileName(this, "Выберите изображение NPC", "", "Images (*.png *.jpg *.jpeg)");
        if (imagePath.isEmpty()) return;

        // Создаем папку для NPC в директории monsters (если её нет)
        QString npcDirectory = QDir::currentPath() + "/monsters";
        QDir dir;
        if (!dir.exists(npcDirectory)) {
            dir.mkpath(npcDirectory);
        }

        // Сохраняем NPC как JSON
        QString npcFilePath = npcDirectory + "/" + name + ".json";
        QJsonObject npcObject;
        npcObject["name"] = name;
        npcObject["characteristics"] = characteristics;
        npcObject["image"] = imagePath;  // Путь к изображению NPC

        QJsonDocument doc(npcObject);
        QFile npcFile(npcFilePath);
        if (npcFile.open(QIODevice::WriteOnly)) {
            npcFile.write(doc.toJson());
            npcFile.close();
            QMessageBox::information(this, "Успех", "NPC добавлен.");
        } else {
            QMessageBox::warning(this, "Ошибка", "Не удалось сохранить NPC.");
        }
    }
    // Если выбрано редактирование NPC
    else if (selectedAction == editNpcAction) {
        if (!npcFilePath.isEmpty()) {
            QFile npcFile(npcFilePath);
            if (!npcFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
                QMessageBox::warning(this, "Ошибка", "Не удалось открыть файл NPC.");
                return;
            }

            QByteArray npcData = npcFile.readAll();
            npcFile.close();

            QJsonDocument doc = QJsonDocument::fromJson(npcData);
            if (doc.isNull()) {
                QMessageBox::warning(this, "Ошибка", "Не удалось прочитать данные NPC.");
                return;
            }

            QJsonObject npcObject = doc.object();
            QString name = npcObject["name"].toString();
            QString characteristics = npcObject["characteristics"].toString();
            QString imagePath = npcObject["image"].toString();

            // Открываем диалог для редактирования имени NPC
            bool ok;
            QString newName = QInputDialog::getText(this, "Редактировать имя NPC", "Имя NPC:", QLineEdit::Normal, name, &ok);
            if (!ok || newName.isEmpty()) return;

            // Открываем диалог для редактирования характеристик NPC
            QString newCharacteristics = QInputDialog::getText(this, "Редактировать характеристики NPC", "Характеристики NPC:", QLineEdit::Normal, characteristics, &ok);
            if (!ok || newCharacteristics.isEmpty()) return;

            // Открываем диалог для выбора нового изображения NPC
            QString newImagePath = QFileDialog::getOpenFileName(this, "Выберите изображение NPC", imagePath, "Images (*.png *.jpg *.jpeg)");
            if (newImagePath.isEmpty()) newImagePath = imagePath;  // Если изображение не изменилось

            // Обновляем данные NPC
            npcObject["name"] = newName;
            npcObject["characteristics"] = newCharacteristics;
            npcObject["image"] = newImagePath;

            // Сохраняем изменения в файл NPC
            QFile editedNpcFile(npcFilePath);
            if (editedNpcFile.open(QIODevice::WriteOnly)) {
                QJsonDocument newDoc(npcObject);
                editedNpcFile.write(newDoc.toJson());
                editedNpcFile.close();
                QMessageBox::information(this, "Успех", "NPC обновлен.");
            } else {
                QMessageBox::warning(this, "Ошибка", "Не удалось сохранить изменения NPC.");
            }
        }
    }
    // Если выбрано удаление NPC
    else if (selectedAction == removeNpcAction) {
        if (!npcFilePath.isEmpty()) {
            if (QFile::remove(npcFilePath)) {
                QMessageBox::information(this, "Успех", "NPC удален.");
            } else {
                QMessageBox::warning(this, "Ошибка", "Не удалось удалить NPC.");
            }
        }
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
