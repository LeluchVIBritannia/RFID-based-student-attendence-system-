#include "SerialManager.h"
#include <QDebug>

SerialManager::SerialManager(QObject *parent) : QObject(parent)
{
    serial.setPortName("/dev/ttyUSB0");
    serial.setBaudRate(QSerialPort::Baud9600); //dont change

    if(serial.open(QIODevice::ReadOnly))
        qDebug() << "Serial Conected";
    else
        qDebug() << "Failed to open port";

    connect(&serial , & QSerialPort::readyRead , this , &SerialManager::onDataRecived);
}


void SerialManager::onDataRecived()
{
    buffer += serial.readAll();

    while(buffer.contains('\n'))
    {
        int index = buffer.indexOf('\n');
        QString uid = buffer.left(index).trimmed();
        buffer = buffer.mid(index + 1);

        if(!uid.isEmpty())
        {
            qDebug() << "Card id: " << uid;
            emit cardScanned(uid);
        }
    }
}