/*
 * Copyright (C) 2019 Philippe Proulx <eepp.ca>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#ifndef _JOME_Q_WAKE_UP_SERVER_HPP
#define _JOME_Q_WAKE_UP_SERVER_HPP

#include <QObject>
#include <QLocalServer>
#include <QLocalSocket>

namespace jome {

class QWakeUpServer :
    public QObject
{
    Q_OBJECT

public:
    explicit QWakeUpServer(QObject *parent, const std::string& name);

signals:
    void wakeUp();
    void quit();

private slots:
    void _newConnection();
    void _socketDisconnected();
    void _socketReadyRead();

private:
    bool _checkTmpData();

private:
    QLocalServer _server;
    QLocalSocket *_socket = nullptr;
    std::string _tmpData;
};

} // namespace jome

#endif // _JOME_Q_WAKE_UP_SERVER_HPP
