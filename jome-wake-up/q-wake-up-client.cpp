/*
 * Copyright (C) 2019 Philippe Proulx <eepp.ca>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include <cassert>
#include <iostream>

#include "q-wake-up-client.hpp"

namespace jome {

QWakeUpClient::QWakeUpClient(QObject * const parent, const std::string& name) :
    QObject {parent}
{
    _socket.setServerName(QString::fromStdString(name));
    QObject::connect(&_socket, SIGNAL(connected()),
                     this, SLOT(_socketConnected()));
}

void QWakeUpClient::_connectToServer()
{
    _socket.connectToServer(QIODevice::WriteOnly);
}

void QWakeUpClient::wakeUp(const bool quit)
{
    _quit = quit;
    this->_connectToServer();
}

void QWakeUpClient::_socketConnected()
{
    if (_quit) {
        _socket.write("quit");
    } else {
        _socket.write("wake");
    }

    _socket.disconnectFromServer();
}

} // namespace jome
