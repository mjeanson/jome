/*
 * Copyright (C) 2019 Philippe Proulx <eepp.ca>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include <cassert>
#include <iostream>

#include "q-wake-up-server.hpp"

namespace jome {

QWakeUpServer::QWakeUpServer(QObject * const parent, const std::string& name) :
    QObject {parent}
{
    QObject::connect(&_server, SIGNAL(newConnection()),
                     this, SLOT(_newConnection()));
    _server.listen(QString::fromStdString(name));
}

void QWakeUpServer::_newConnection()
{
    auto socket = _server.nextPendingConnection();

    if (!socket) {
        return;
    }

    QObject::connect(socket, SIGNAL(disconnected()),
                     socket, SLOT(deleteLater()));

    if (_socket) {
        // we should probably be able to handle more than one
        socket->close();
        return;
    }

    _socket = socket;
    QObject::connect(socket, SIGNAL(disconnected()),
                     this, SLOT(_socketDisconnected()));
    QObject::connect(socket, SIGNAL(readyRead()),
                     this, SLOT(_socketReadyRead()));
    _tmpData.clear();
}

void QWakeUpServer::_socketDisconnected()
{
    _socket = nullptr;
}

bool QWakeUpServer::_checkTmpData()
{
    if (_tmpData == "wake") {
        emit this->wakeUp();
    } else if (_tmpData == "quit") {
        emit this->quit();
        return false;
    }

    if (_tmpData.size() >= 4) {
        _tmpData.clear();
    }

    return true;
}

void QWakeUpServer::_socketReadyRead()
{
    assert(_socket);

    while (_socket->bytesAvailable() > 0) {
        if (!this->_checkTmpData()) {
            _socket->close();
            return;
        }

        char byte;
        const auto count = _socket->read(&byte, 1);

        assert(count == 1);
        _tmpData += byte;
    }

    this->_checkTmpData();
}

} // namespace jome
