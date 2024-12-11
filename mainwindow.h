#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    // I changed the struct a bit, the item and itemPic values are the item value of the ui in the ui lists
    // in this way I can arrange the list in the ui the same way as the linked list in the memory (for switching places or for removing and adding Nodes)
    typedef struct Frame
    {
        QString         name;
        unsigned int	duration;
        QString         path;
        QListWidgetItem *item;
        QListWidgetItem *itemPic;
    } Frame;
    typedef struct FrameNode
    {
        Frame* frame;
        struct FrameNode* next;
    } FrameNode;

    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void showNextFrame();

    void on_addFrame_button_clicked();

    void on_play_button_clicked();

    void on_stop_button_clicked();

    void on_actionnew_triggered();

    void on_actionsave_triggered();

    void on_actionopen_triggered();

    void on_actionexit_triggered();

    void on_actioninfo_triggered();

    void on_frame_list_itemClicked(QListWidgetItem *item);

    void on_delete_frame_button_clicked();

    void on_actionchange_all_dur_triggered();

    void updateFrameList();

    void on_frame_list_itemDoubleClicked(QListWidgetItem *item);

    void on_actionexport_to_gif_triggered();

    void saveProject();

    void loadProject();

    void printLinkedList();

    void on_actionblack_and_white_triggered();

    void on_actionback_to_normal_triggered();

    void on_actionrotate_triggered();

    void updateImage(QPixmap image);

    void freeLinkedList();

private:

    FrameNode* head = nullptr;
    int frameNumber = 0;
    QTimer* playTimer;
    FrameNode* curr;

    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
