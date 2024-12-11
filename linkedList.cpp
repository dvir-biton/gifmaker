#include "mainwindow.h"

// free the linked list
void MainWindow::freeLinkedList()
{
    FrameNode* currentNode = head;

    while (currentNode)
    {
        FrameNode* nextNode = currentNode->next;
        delete currentNode->frame;
        delete currentNode;
        currentNode = nextNode;
    }

    head = nullptr;
}

void MainWindow::printLinkedList()
{
    FrameNode* currentNode = head;
    qDebug() << "struct: ";
    while (currentNode)
    {
        Frame* frame = currentNode->frame;
        qDebug() << "Name: " << frame->name;
        qDebug() << "Duration: " << frame->duration;
        qDebug() << "Path: " << frame->path;
        qDebug() << "mem: " << currentNode;

        currentNode = currentNode->next;
    }
}
