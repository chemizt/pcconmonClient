#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    udpSock = new QUdpSocket(this);
    tcpSock = new QTcpSocket(this);

    ui->setupUi(this);

    this->setWindowTitle("Клиент PCConMon v0.2.1");

    udpSock->bind(QHostAddress("192.168.1.10"), 9090);
    udpSock->writeDatagram("IDENTIFY", QHostAddress("192.168.1.255"), 9090);

    connect(udpSock, &QIODevice::readyRead, this, &MainWindow::udpSockReady);
    connect(tcpSock, &QIODevice::readyRead, this, &MainWindow::tcpSockReady);

    currentlySelectedCompName = "";
}

MainWindow::~MainWindow()
{
    delete ui;
}

void warnNoItemSelected()
{
    QMessageBox warnBox;

    warnBox.setWindowTitle("Внимание");
    warnBox.setText("Пожалуйста, выберите компьютер в сетевом окружении");
    warnBox.setIcon(QMessageBox::Icon::Warning);
    warnBox.setStandardButtons(QMessageBox::Ok);
    warnBox.exec();
}

void MainWindow::on_shutdownButton_clicked()
{
    QMessageBox confirmationBox;
    int resp;

    if (!ui->treeWidget->currentItem())
    {
        warnNoItemSelected();
    }
    else
    {
        confirmationBox.setWindowTitle("Подтвердите действие");
        confirmationBox.setText("Компьютер " + currentlySelectedCompName + " будет выключен. Вы уверены, что хотите продолжить?");
        confirmationBox.setIcon(QMessageBox::Icon::Question);
        confirmationBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        confirmationBox.setDefaultButton(QMessageBox::No);
        resp = confirmationBox.exec();

        switch (resp)
        {
            case QMessageBox::No:
            {
                break;
            }
            case QMessageBox::Yes:
            {
                tcpSock->write("SHUTDOWN");
                udpSock->writeDatagram("SHUTDOWN", compAddressesList.value(currentlySelectedCompName), 9090);
                break;
            }
        }
    }
}

void MainWindow::on_treeWidget_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
    if (current != NULL)
    {
        QTreeWidgetItem* topLevelParent = current;

        while (topLevelParent->parent())
        {
            topLevelParent = topLevelParent->parent();
        }

        QString compName = topLevelParent->text(0);
        ui->infoTab->findChild<QLabel*>("nameLabel")->setText(QString("Компьютер: " + compName));

        if (currentlySelectedCompName != compName)
        {
            if (tcpSock->isOpen())
            {
                tcpSock->disconnectFromHost();
                tcpSock->close();
            }

            tcpSock->connectToHost(compAddressesList.value(compName), 9090);
            tcpSock->waitForConnected(250);

            if (!compSystemsList.value(compName))
            {
                tcpSock->write("SEND");
            }
            else
            {
                checkIfEmpty(compName);
            }

            currentlySelectedCompName = compName;
        }

        if (current->parent())
        {
            if (current->parent()->text(0) == "Процессоры")
            {
                if (compSystemsList.value(currentlySelectedCompName))
                {
                   string name = string(current->text(0).toUtf8().data());
                   Processor* cpu = (Processor*)compSystemsList.value(currentlySelectedCompName)->getProcessors().at(name);

                   mapProcessorToInfoTab(cpu);
                }
            }
            else if (current->parent()->text(0) == "Видеоадаптеры")
            {
                if (compSystemsList.value(currentlySelectedCompName))
                {
                   string name = string(current->text(0).toUtf8().data());
                   VideoController* vc = (VideoController*)compSystemsList.value(currentlySelectedCompName)->getVideoControllers().at(name);

                   mapVideoControllerToInfoTab(vc);
                }
            }
            else if (current->parent()->text(0) == "Дисковые устройства")
            {
                if (compSystemsList.value(currentlySelectedCompName))
                {
                   string name = string(current->text(0).toUtf8().data());
                   DiskDrive* dd = (DiskDrive*)compSystemsList.value(currentlySelectedCompName)->getDiskDrives().at(name);

                   mapDiskDriveToInfoTab(dd);
                }
            }
            else if (current->parent()->text(0) == "Оперативная память")
            {
                if (compSystemsList.value(currentlySelectedCompName))
                {
                   string name = string(current->text(0).toUtf8().data());
                   SystemMemory* sm = (SystemMemory*)compSystemsList.value(currentlySelectedCompName)->getSystemMemory().at(name);

                   mapSysMemToInfoTab(sm);
                }
            }
            else if (current->parent()->text(0) == "Материнские платы")
            {
                if (compSystemsList.value(currentlySelectedCompName))
                {
                   string name = string(current->text(0).toUtf8().data());
                   BaseBoard* bb = (BaseBoard*)compSystemsList.value(currentlySelectedCompName)->getBaseBoards().at(name);

                   mapBaseBoardToInfoTab(bb);
                }
            }
        }
        else
        {
            mapCompSysToInfoTab(compSystemsList.value(currentlySelectedCompName));
        }
    }
}

void MainWindow::mapProcessorToInfoTab(Processor* proc)
{
    if (proc)
    {
        ui->tableWidget->setRowCount(0);

        addRowIfNotNull("ID", proc->getId().c_str());
        addRowIfNotNull("Название", proc->getName().c_str());
        addRowIfNotNull("Частота шины", QString(proc->getBusClock().c_str()) + " МГц");
        addRowIfNotNull("Максимальная частота", QString(proc->getMaxClock().c_str()) + " МГц");
        addRowIfNotNull("Текущая частота", QString(proc->getCurrentClock().c_str()) + " МГц");
        addRowIfNotNull("Семейство", proc->getFamily().c_str());
        addRowIfNotNull("Сокет", proc->getSocket().c_str());
        addRowIfNotNull("Разрядность", QString(proc->getWidth().c_str()) + " бита");
        addRowIfNotNull("Количество ядер", proc->getCoreCount().c_str());
        addRowIfNotNull("Количество потоков", proc->getThreadCount().c_str());
    }
}

void MainWindow::mapVideoControllerToInfoTab(VideoController* vc)
{
    if (vc)
    {
        ui->tableWidget->setRowCount(0);

        addRowIfNotNull("Название", vc->getName().c_str());
        addRowIfNotNull("Модель", vc->getModel().c_str());
        addRowIfNotNull("Видеопроцессор", vc->getVideoProcessor().c_str());
        addRowIfNotNull("Частота обновления", QString(vc->getCurrentRefreshRate().c_str()) == "" ? "" : QString(vc->getCurrentRefreshRate().c_str()) + " Гц");
        addRowIfNotNull("Минимальная частота обновления", QString(vc->getMinRefreshRate().c_str()) + " Гц");
        addRowIfNotNull("Максимальная частота обновления", QString(vc->getMaxRefreshRate().c_str()) + " Гц");
        addRowIfNotNull("Горизонтальное разрешение", vc->getCurrentHorizontalResolution().c_str());
        addRowIfNotNull("Вертикальное разрешение", vc->getCurrentVerticalResolution().c_str());
    }
}

void MainWindow::mapDiskDriveToInfoTab(DiskDrive* dd)
{
    if (dd)
    {
        ui->tableWidget->setRowCount(0);

        addRowIfNotNull("Название", dd->getName().c_str());
        addRowIfNotNull("Производитель", dd->getManufacturer().c_str());
        addRowIfNotNull("Серийный номер", dd->getSerialNumber().c_str());
        addRowIfNotNull("Логическое имя", dd->getLogicalName().c_str());
        addRowIfNotNull("Размер сектора", QString(dd->getBytesPerSector().c_str()) + " Б");
        addRowIfNotNull("Ёмкость", QString::number(std::stoull(dd->getSize()) / 1024 / 1024) + " МБ");
        addRowIfNotNull("Версия прошивки", dd->getFirmwareRevision().c_str());
        addRowIfNotNull("Версия оборудования", dd->getVersion().c_str());
        addRowIfNotNull("Интерфейс", dd->getInterfaceType().c_str());
    }
}

void MainWindow::mapSysMemToInfoTab(SystemMemory* sm)
{
    if (sm)
    {
        ui->tableWidget->setRowCount(0);

        addRowIfNotNull("Название", sm->getName().c_str());
        addRowIfNotNull("Производитель", sm->getManufacturer().c_str());
        addRowIfNotNull("Серийный номер", sm->getSerialNumber().c_str());
        addRowIfNotNull("Номер части", sm->getPartNumber().c_str());
        addRowIfNotNull("Форм-фактор", sm->getFormFactor().c_str());
        addRowIfNotNull("Тип", sm->getType().c_str());
        addRowIfNotNull("Модуль", sm->getDimmName().c_str());
        addRowIfNotNull("Канал", sm->getChannel().c_str());
        addRowIfNotNull("Ёмкость", QString::number(std::stoull(sm->getSize()) / 1024 / 1024) + " МБ");
        addRowIfNotNull("Частота", QString(sm->getCurrentClock().c_str()) == "" ? "" : QString(sm->getCurrentClock().c_str()) + " МГц");
        addRowIfNotNull("Напряжение", QString(sm->getVoltage().c_str()) == "" ? "" : QString(sm->getVoltage().c_str()) + " В");
        addRowIfNotNull("Разрядность", QString(sm->getWidth().c_str()) + " бита");
    }
}

void MainWindow::mapBaseBoardToInfoTab(BaseBoard* bb)
{
    if (bb)
    {
        ui->tableWidget->setRowCount(0);

        addRowIfNotNull("Название", bb->getName().c_str());
        addRowIfNotNull("Производитель", bb->getId().c_str());
        addRowIfNotNull("Серийный номер", bb->getSerialNumber().c_str());
        addRowIfNotNull("Версия оборудования", bb->getVersion().c_str());
        addRowIfNotNull("Главная", bb->isHosting() ? "Да" : "Нет");
        addRowIfNotNull("Горячая замена", bb->isHotswappable() ? "Поддерживается" : "Не поддерживается");
        addRowIfNotNull("Замена", bb->isReplaceable() ? "Возможна" : "Невозможна");
        addRowIfNotNull("Извлечение", bb->isRemovable() ? "Возможно" : "Невозможно");
    }
}

void MainWindow::mapCompSysToInfoTab(ComputerSystem* cs)
{
    if (cs)
    {
        ui->tableWidget->setRowCount(0);

        addRowIfNotNull("Название", cs->getName().c_str());
        addRowIfNotNull("Архитектура", cs->getArchitecture().c_str());
        addRowIfNotNull("Операционная система", cs->getOperatingSystemName().c_str());
    }
}

void MainWindow::on_rebootButton_clicked()
{
    QMessageBox confirmationBox;
    int resp;

    if (!ui->treeWidget->currentItem())
    {
        warnNoItemSelected();
    }
    else
    {
        confirmationBox.setWindowTitle("Подтвердите действие");
        confirmationBox.setText("Компьютер " + currentlySelectedCompName + " будет перезагружен. Вы уверены, что хотите продолжить?");
        confirmationBox.setIcon(QMessageBox::Icon::Question);
        confirmationBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        confirmationBox.setDefaultButton(QMessageBox::No);
        resp = confirmationBox.exec();

        switch (resp)
        {
            case QMessageBox::No:
            {
                break;
            }
            case QMessageBox::Yes:
            {
                tcpSock->write("REBOOT");
                udpSock->writeDatagram("REBOOT", compAddressesList.value(currentlySelectedCompName), 9090);
                break;
            }
        }
    }
}

void MainWindow::on_pushButton_clicked()
{
    if (!ui->treeWidget->currentItem())
    {
        warnNoItemSelected();
    }
    else
    {
        tcpSock->write("REFRESH");
        delete compSystemsList.take(currentlySelectedCompName);
        deleteListItemChildren(currentlySelectedCompName);
    }
}

void MainWindow::on_refreshButton_clicked()
{
    udpSock->writeDatagram("IDENTIFY", QHostAddress("192.168.1.255"), 9090);
    ui->treeWidget->clear();
}

void MainWindow::udpSockReady()
{
    udpSock->waitForReadyRead(125);
    QByteArray data;
    QTreeWidgetItem* item = new QTreeWidgetItem();
    QHostAddress sender;

    data.resize(udpSock->pendingDatagramSize());
    udpSock->readDatagram(data.data(), data.size(), &sender);
    item->setText(0, QString(data));
    item->takeChildren();
    ui->treeWidget->addTopLevelItem(item);
    compAddressesList.insert(QString(data), sender);
    checkIfEmpty(QString(data));
}

void MainWindow::tcpSockReady()
{
    if (tcpSock->waitForConnected(250))
    {
        tcpSock->waitForReadyRead(125);
        QByteArray rcvData;
        ComputerSystem* newCS = new ComputerSystem();

        rcvData.resize(tcpSock->size());
        rcvData = tcpSock->readAll();

        json j = json::from_msgpack(rcvData);

        newCS->deSerialize(j);

        compSystemsList.insert(QString(newCS->getName().c_str()), newCS);

        checkIfEmpty(QString(newCS->getName().c_str()));
    }
}

void MainWindow::tcpSockDisconnected()
{
    tcpSock->deleteLater();
}

void MainWindow::checkIfEmpty(QString name)
{
    QList<QTreeWidgetItem*> clist = ui->treeWidget->findItems(name, Qt::MatchExactly, 0);

    if (!clist.at(0)->childCount() && compSystemsList.value(clist.at(0)->text(0)))
    {
        mapCompSysToTree(compSystemsList.value(clist.at(0)->text(0)), clist.at(0));
        mapCompSysToInfoTab(compSystemsList.value(currentlySelectedCompName));
    }
}

void MainWindow::deleteListItemChildren(QString name)
{
    QList<QTreeWidgetItem*> clist = ui->treeWidget->findItems(name, Qt::MatchExactly, 0);

    clist.at(0)->takeChildren();
}

void MainWindow::addRowIfNotNull(QString firstCol, QString secondCol)
{
    if (secondCol != "")
    {
        QTableWidgetItem* item;

        ui->tableWidget->setRowCount(ui->tableWidget->rowCount() + 1);

        item = new QTableWidgetItem(firstCol);
        ui->tableWidget->setItem(ui->tableWidget->rowCount() - 1, 0, item);
        item = new QTableWidgetItem(secondCol);
        ui->tableWidget->setItem(ui->tableWidget->rowCount() - 1, 1, item);
    }

}

void MainWindow::mapCompSysToTree(ComputerSystem* cs, QTreeWidgetItem* parentItem)
{
    QTreeWidgetItem* item;
    QTreeWidgetItem* childItem;

    item = new QTreeWidgetItem();
    item->setText(0, "Видеоадаптеры");
    parentItem->addChild(item);

    for (const auto& [key, value] : cs->getVideoControllers())
    {
        childItem = new QTreeWidgetItem();
        childItem->setText(0, value->getName().c_str());
        item->addChild(childItem);
    }

    item = new QTreeWidgetItem();
    item->setText(0, "Дисковые устройства");
    parentItem->addChild(item);

    for (const auto& [key, value] : cs->getDiskDrives())
    {
        childItem = new QTreeWidgetItem();
        childItem->setText(0, value->getName().c_str());
        item->addChild(childItem);
    }

    item = new QTreeWidgetItem();
    item->setText(0, "Процессоры");
    parentItem->addChild(item);

    for (const auto& [key, value] : cs->getProcessors())
    {
        childItem = new QTreeWidgetItem();
        childItem->setText(0, value->getName().c_str());
        item->addChild(childItem);
    }

    item = new QTreeWidgetItem();
    item->setText(0, "Оперативная память");
    parentItem->addChild(item);

    for (const auto& [key, value] : cs->getSystemMemory())
    {
        childItem = new QTreeWidgetItem();
        childItem->setText(0, value->getName().c_str());
        item->addChild(childItem);
    }

    item = new QTreeWidgetItem();
    item->setText(0, "Материнские платы");
    parentItem->addChild(item);

    for (const auto& [key, value] : cs->getBaseBoards())
    {
        childItem = new QTreeWidgetItem();
        childItem->setText(0, value->getName().c_str());
        item->addChild(childItem);
    }
}
