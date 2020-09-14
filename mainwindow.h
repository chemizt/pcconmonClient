#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeWidget>
#include <QMessageBox>
#include <QTcpSocket>
#include <QUdpSocket>

#include "hardwareDataClasses.hpp"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    QTcpSocket* tcpSock;
    QUdpSocket* udpSock;

public slots:
    void udpSockReady();
    void tcpSockReady();
    void tcpSockDisconnected();

private slots:
    void on_shutdownButton_clicked();
    void on_treeWidget_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
    void on_rebootButton_clicked();
    void on_pushButton_clicked();
    void on_refreshButton_clicked();

private:
    Ui::MainWindow *ui;
    QString currentlySelectedCompName;
    QMap<QString, QHostAddress> compAddressesList;
    QMap<QString, ComputerSystem*> compSystemsList;
    void mapProcessorToInfoTab(Processor* proc);
    void mapVideoControllerToInfoTab(VideoController* vc);
    void mapDiskDriveToInfoTab(DiskDrive* dd);
    void mapSysMemToInfoTab(SystemMemory* sm);
    void mapBaseBoardToInfoTab(BaseBoard* bb);
    void mapCompSysToInfoTab(ComputerSystem* cs);
    void mapCompSysToTree(ComputerSystem* cs, QTreeWidgetItem *parentItem);
    void checkIfEmpty(QString name);
    void deleteListItemChildren(QString name);
    void addRowIfNotNull(QString firstCol, QString secondCol);
};
#endif // MAINWINDOW_H
