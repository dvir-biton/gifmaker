#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QApplication>

#include <QLabel>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QListWidgetItem>
#include <QStandardItemModel>

#include <QJsonArray>
#include <QJsonDocument>

#include <QImageWriter>
#include <QMovie>
#include <QGraphicsBlurEffect>
#include <QGraphicsPixmapItem>
#include <QGraphicsScene>
#include <QGraphicsView>

#include <QTimer>
#include <QBuffer>

#include <QDebug>
#include <QProcess>

#define WIDTH 128
#define HEIGHT WIDTH

#define MAX_ANGLE 360
#define MIN_ANGLE (MAX_ANGLE * -1)

// build the ui window
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->frame_list->setSelectionMode(QAbstractItemView::ExtendedSelection);
}

// free the linked list and clear the ui
MainWindow::~MainWindow()
{
    freeLinkedList();
    delete ui;
}

void MainWindow::on_addFrame_button_clicked()
{
    QString name = QInputDialog::getText(this, "Enter Frame Name", "Name:");
    bool ok;
    unsigned int duration = QInputDialog::getInt(this, "Enter Frame Duration", "Duration:", 1, 1, INT_MAX, 1, &ok);
    if (!ok)
    {
        return;
    }

    QString path = QFileDialog::getOpenFileName(this, "Choose Frame Image", "", "Images (*.png *.jpeg)");

    if(path.isEmpty())
    {
        QMessageBox::warning(this, tr("Warning"), tr("No file selected."));
        return;
    }

    FrameNode* currentNode = head;
    while (currentNode)
    {
        if (currentNode->frame->name == name)
        {
            QMessageBox::warning(this, tr("Warning"), tr("Frame name already exists."));
            return;
        }
        currentNode = currentNode->next;
    }

    Frame* newFrame = new Frame;
    newFrame->name = name;
    newFrame->duration = duration;
    newFrame->path = path;

    FrameNode* newNode = new FrameNode;
    newNode->frame = newFrame;
    newNode->next = nullptr;

    if (!head)
    {
        head = newNode;
    }
    else
    {
        FrameNode* currentNode = head;
        while (currentNode->next)
        {
            currentNode = currentNode->next;
        }
        currentNode->next = newNode;
    }

    // Add the frame to the list widget with the correct format -> <frameNumber>. <name> | <path> | <duration>
    frameNumber++;
    QListWidget* frameList = ui->frame_list;
    QString listItemText = QString("%1. %2 | %3 | %4").arg(frameNumber).arg(name).arg(path).arg(duration);
    QListWidgetItem* listItem = new QListWidgetItem(listItemText);
    frameList->addItem(listItem);
    newNode->frame->item = listItem;

    QPixmap image(path);
    updateImage(image);

    // add the frame to the bottom list frame view
    QListWidgetItem* item1 = new QListWidgetItem();
    QLabel* label1 = new QLabel();

    label1->setPixmap(image.scaled(WIDTH, HEIGHT));
    item1->setSizeHint(QSize(WIDTH, HEIGHT));

    ui->frames_view->addItem(item1);
    ui->frames_view->setItemWidget(item1, label1);
    newNode->frame->itemPic = item1;
}

// the play function, to play the video (oops I Didn't used opencv but it's still works)
void MainWindow::on_play_button_clicked()
{
    // check if the linked list is empty
    if (head == nullptr)
    {
        QMessageBox::warning(this, tr("Warning"), tr("No frames available."));
        return;
    }

    // disable the add and del frame buttons while playing the gif
    ui->addFrame_button->setDisabled(true);
    ui->delete_frame_button->setDisabled(true);

    playTimer = new QTimer(this);
    curr = head;
    connect(playTimer, &QTimer::timeout, this, &MainWindow::showNextFrame);
    playTimer->start(curr->frame->duration);
}

// show the next frame endlessly
void MainWindow::showNextFrame()
{
    // push to the next node, if we reached the end of the list we start again
    curr = curr->next;
    if (curr == nullptr)
    {
        curr = head;
    }

    // show each picture for it duration
    if (playTimer)
    {
        playTimer->start(curr->frame->duration);
    }

    // switch to the next image
    QPixmap image(curr->frame->path);
    updateImage(image);
}

// stopping the gif, freeing the timer and stopping the play of the video.
void MainWindow::on_stop_button_clicked()
{
    if (head == nullptr)
    {
        QMessageBox::warning(this, tr("Warning"), tr("No frames available."));
        return;
    }

    // enable the add and del frame buttons after stopping the gif
    ui->addFrame_button->setEnabled(true);
    ui->delete_frame_button->setEnabled(true);

    if (playTimer)
    {
        disconnect(playTimer, &QTimer::timeout, this, &MainWindow::showNextFrame);
        playTimer->stop();
        playTimer->deleteLater();
        playTimer = nullptr;
    }
}


// making a new project, free the linked list and clear the ui
void MainWindow::on_actionnew_triggered()
{
    freeLinkedList();
    updateFrameList();
    ui->picture_show->clear();
}


void MainWindow::on_actionsave_triggered()
{
   saveProject();
}


// when opening a new project, we delete the old one and then loading the new one
void MainWindow::on_actionopen_triggered()
{
    on_actionnew_triggered();
    loadProject();
}


// just closing the program on exit (frees the ui and the taken memory)
void MainWindow::on_actionexit_triggered()
{
    QApplication::quit();
}

// the about, not much to explain just a few words about me
void MainWindow::on_actioninfo_triggered()
{
    QMessageBox::information(this, tr("about"), tr("this program created by bidbid\nfor magshimim"));
}


void MainWindow::on_frame_list_itemClicked(QListWidgetItem *item)
{
    FrameNode* currentNode = head;

    while (currentNode->frame->item != item)
        currentNode = currentNode->next;

    QPixmap image(currentNode->frame->path);
    if (image.isNull())
    {
        QMessageBox::warning(this, tr("Warning"), tr("Failed to load image."));
        return;
    }

    updateImage(image);
}


void MainWindow::on_delete_frame_button_clicked()
{
    QList<QListWidgetItem*> selectedItems = ui->frame_list->selectedItems();

    if (selectedItems.isEmpty())
    {
        QMessageBox::warning(this, tr("Warning"), tr("No item selected."));
        return;
    }

    QListWidgetItem* selectedItem = selectedItems.first();
    int row = ui->frame_list->row(selectedItem);

    FrameNode* currentNode = head;
    FrameNode* previousNode = nullptr;

    while (currentNode != nullptr && currentNode->frame->item != selectedItem)
    {
        previousNode = currentNode;
        currentNode = currentNode->next;
    }

    // If the node was found, remove it
    if (currentNode != nullptr)
    {
        // Delete the associated QListWidgetItem from frame_list
        delete ui->frame_list->takeItem(row);

        // Delete the associated QListWidgetItem from frames_view
        delete ui->frames_view->takeItem(row);

        if (previousNode != nullptr)
        {
            previousNode->next = currentNode->next;
        }
        else
        {
            head = currentNode->next;
        }

        delete currentNode->frame;
        delete currentNode;
    }
    ui->picture_show->clear();
    updateFrameList();
}


void MainWindow::updateFrameList()
{
    ui->frame_list->clear();
    ui->frames_view->clear();

    FrameNode* currentNode = head;
    frameNumber = 0;
    int row = 0;
    while (currentNode)
    {
        Frame* frame = currentNode->frame;
        QString listItemText = QString("%1. %2 | %3 | %4").arg(++frameNumber).arg(frame->name).arg(frame->path).arg(frame->duration);
        QListWidgetItem* listItem = new QListWidgetItem(listItemText);
        ui->frame_list->addItem(listItem);

        // Update the associated QListWidgetItem row
        listItem->setData(Qt::UserRole, row);

        // Assign the QListWidgetItem object to the item member of the Frame struct
        frame->item = listItem;

        QListWidgetItem* item1 = new QListWidgetItem();
        QLabel* label1 = new QLabel();
        QPixmap image(currentNode->frame->path);
        label1->setPixmap(image.scaled(WIDTH, HEIGHT));
        item1->setSizeHint(QSize(WIDTH, HEIGHT));
        ui->frames_view->addItem(item1);
        ui->frames_view->setItemWidget(item1, label1);

        // Assign the QListWidgetItem object to the itemPic member of the Frame struct
        frame->itemPic = item1;

        currentNode = currentNode->next;
        ++row;
    }
}



void MainWindow::on_actionchange_all_dur_triggered()
{
    bool ok;
    unsigned int newDuration = QInputDialog::getInt(this, "Change All Durations", "New Duration:", 1, 1, INT_MAX, 1, &ok);
    if (!ok)
    {
        return;
    }

    // Update the durations in the linked list
    FrameNode* currentNode = head;
    while (currentNode)
    {
        currentNode->frame->duration = newDuration;
        currentNode = currentNode->next;
    }

    // Clear the list widget and update it with the modified durations
    ui->frame_list->clear();
    updateFrameList();
}

void MainWindow::on_frame_list_itemDoubleClicked(QListWidgetItem* item)
{
    bool ok;
    unsigned int newDuration = QInputDialog::getInt(this, "Change Duration", "New Duration:", 1, 1, INT_MAX, 1, &ok);
    if (!ok)
    {
        return;
    }

    FrameNode* currentNode = head;
    while (currentNode)
    {
        if (currentNode->frame->item == item)
        {
            currentNode->frame->duration = newDuration;

            QString listItemText = item->text();
            QStringList parts = listItemText.split('|');
            if (parts.size() >= 3)
            {
                parts[2] = QString(" %1").arg(newDuration);
                QString updatedText = parts.join('|').trimmed();
                item->setText(updatedText);
            }
            break;
        }
        currentNode = currentNode->next;
    }
}

void MainWindow::on_actionexport_to_gif_triggered()
{
    QString filePath = QFileDialog::getSaveFileName(this, "Export to GIF", "", "GIF files (*.gif)");
    if (filePath.isEmpty())
        return;

    QString pythonPath = "gifmaker.py";

    FrameNode* currentNode = head;
    QString frameListFilePath = "frame_list.txt";  // Temporary file to store the paths of the frames

    // Create a temporary file to store the list of frames
    QFile frameListFile(frameListFilePath);
    if (!frameListFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        return;
    }

    QTextStream frameListStream(&frameListFile);

    // Write the paths of the frames to the temporary file
    while (currentNode)
    {
        QString framePath = currentNode->frame->path;
        frameListStream << framePath << "\n";

        currentNode = currentNode->next;
    }

    frameListFile.close();

    // Run the Python script to create the GIF file
    QStringList pythonArgs;
    pythonArgs << frameListFilePath << filePath;

    // Create a QProcess to execute the Python script
    QProcess process;
    process.setProgram(pythonPath);
    process.setArguments(pythonArgs);

    qDebug() << "Python Command: " << process.program() << process.arguments().join(" ");

    // Start the Python script process
    process.start();
    process.waitForFinished(-1);  // Wait for the process to finish

    // Print the exit code and output of the Python script process
    qDebug() << "Exit Code: " << process.exitCode();
    qDebug() << "Standard Output: " << process.readAllStandardOutput();
    qDebug() << "Error Output: " << process.readAllStandardError();

    // Delete the temporary frame list file
    QFile::remove(frameListFilePath);
}


// the function to save the project to a json file in the location the user chose
void MainWindow::saveProject()
{
    printLinkedList();
    QString filePath = QFileDialog::getSaveFileName(this, "Save Project", "", "JSON files (*.json)");
    if (filePath.isEmpty())
    {
        return;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly))
    {
        // Error handling for file open failure
        return;
    }

    QJsonObject json;
    QJsonArray framesArray;

    FrameNode* currentNode = head;
    while (currentNode)
    {
        QJsonObject frameObject;
        frameObject["name"] = currentNode->frame->name;
        frameObject["path"] = currentNode->frame->path;
        frameObject["duration"] = static_cast<int>(currentNode->frame->duration);

        framesArray.append(frameObject);

        currentNode = currentNode->next;
    }


    json["frames"] = framesArray;

    QJsonDocument jsonDoc(json);
    file.write(jsonDoc.toJson());
    file.close();
}


void MainWindow::loadProject()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Load Project", "", "JSON files (*.json)");
    if (filePath.isEmpty())
    {
        return;
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
    {
        return;
    }

    QByteArray jsonData = file.readAll();
    file.close();

    QJsonDocument jsonDoc(QJsonDocument::fromJson(jsonData));
    QJsonObject json = jsonDoc.object();

    if (json.contains("frames") && json["frames"].isArray())
    {
        QJsonArray framesArray = json["frames"].toArray();

        for (int i = 0; i < framesArray.size(); ++i)
        {
            QJsonObject frameObject = framesArray[i].toObject();
            QString name = frameObject["name"].toString();
            QString path = frameObject["path"].toString();
            int duration = frameObject["duration"].toInt();

            Frame* newFrame = new Frame;
            newFrame->name = name;
            newFrame->duration = duration;
            newFrame->path = path;

            FrameNode* newNode = new FrameNode;
            newNode->frame = newFrame;
            newNode->next = nullptr;

            if (!head)
            {
                head = newNode;
            }
            else
            {
                FrameNode* currentNode = head;
                while (currentNode->next)
                {
                    currentNode = currentNode->next;
                }
                currentNode->next = newNode;
            }
        }

        updateFrameList();
    }
    printLinkedList();
}


void MainWindow::on_actionblack_and_white_triggered()
{
    // Get the selected item from the frame_list
    QListWidgetItem* selectedItem = ui->frame_list->currentItem();
    if (!selectedItem)
    {
        return;
    }

    // Get the associated Frame object
    int row = selectedItem->data(Qt::UserRole).toInt();
    FrameNode* currentNode = head;
    for (int i = 0; i < row && currentNode; ++i)
    {
        currentNode = currentNode->next;
    }
    if (!currentNode)
    {
        return;
    }

    // Load the image from the selected Frame
    QPixmap originalImage(currentNode->frame->path);
    if (originalImage.isNull())
    {
        QMessageBox::warning(this, tr("Warning"), tr("Failed to load image."));
        return;
    }

    QImage image = originalImage.toImage();
    image = image.convertToFormat(QImage::Format_Grayscale8);
    QPixmap blackAndWhiteImage = QPixmap::fromImage(image);
    ui->picture_show->setPixmap(blackAndWhiteImage);
}

void MainWindow::on_actionback_to_normal_triggered()
{
    // Get the selected item from the frame_list
    QListWidgetItem* selectedItem = ui->frame_list->currentItem();
    if (!selectedItem)
    {
        QMessageBox::warning(this, tr("Warning"), tr("no Image selected."));
        return;
    }

    // Get the associated Frame object
    int row = selectedItem->data(Qt::UserRole).toInt();
    FrameNode* currentNode = head;
    for (int i = 0; i < row && currentNode; ++i)
    {
        currentNode = currentNode->next;
    }
    if (!currentNode)
    {
        return;
    }

    // Load the image from the selected Frame
    QPixmap originalImage(currentNode->frame->path);
    if (originalImage.isNull())
    {
        QMessageBox::warning(this, tr("Warning"), tr("Failed to load image."));
        return;
    }

    // Convert the QPixmap to QImage
    QImage image = originalImage.toImage();

    QPixmap normal = QPixmap::fromImage(image);

    // Update the displayed image
    ui->picture_show->setPixmap(normal);
}


void MainWindow::on_actionrotate_triggered()
{
    // Get the selected item from the frame_list
    QListWidgetItem* selectedItem = ui->frame_list->currentItem();
    if (!selectedItem)
    {
        return;
    }

    // Get the associated Frame object
    int row = selectedItem->data(Qt::UserRole).toInt();
    FrameNode* currentNode = head;
    for (int i = 0; i < row && currentNode; ++i)
    {
        currentNode = currentNode->next;
    }
    if (!currentNode)
    {
       return;
    }

    // Load the image from the selected Frame
    QPixmap originalImage(currentNode->frame->path);
    if (originalImage.isNull())
    {
        QMessageBox::warning(this, tr("Warning"), tr("Failed to load image."));
        return;
    }

    // Prompt the user for the rotation angle
    bool ok;
    int angle = QInputDialog::getInt(this, tr("Rotate Image"), tr("Enter rotation angle in degrees:"), 0, MIN_ANGLE, MAX_ANGLE, 1, &ok);

    if (!ok)
    {
        return;
    }

    // Rotate the image by the specified angle
    QPixmap rotatedImage = originalImage.transformed(QTransform().rotate(angle));
    ui->picture_show->setPixmap(rotatedImage);
}

// update the image in the picture_show ui label element
void MainWindow::updateImage(QPixmap image)
{
    // check if the image is null and show the warning for the user
    if (image.isNull())
    {
        QMessageBox::warning(this, tr("Warning"), tr("Failed to load image."));
        return;
    }

    // resize the image to the size of the picture_show label and then replace it with the current image (if it's not null)
    ui->picture_show->setScaledContents(true);
    ui->picture_show->setMinimumSize(image.size());
    ui->picture_show->setPixmap(image);
}





