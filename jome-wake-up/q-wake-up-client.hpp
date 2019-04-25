/*
 * Copyright (C) 2019 Philippe Proulx <eepp.ca>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#ifndef _JOME_WAKE_UP_Q_WAKE_UP_CLIENT_HPP
#define _JOME_WAKE_UP_Q_WAKE_UP_CLIENT_HPP

#include <QObject>
#include <QLocalSocket>

namespace jome {

class QWakeUpClient :
    public QObject
{
    Q_OBJECT

public:
    explicit QWakeUpClient(QObject *parent, const std::string& name);
    void wakeUp(bool quit);

private slots:
    void _socketConnected();

private:
    void _connectToServer();

private:
    QLocalSocket _socket;
    bool _quit = false;
};

} // namespace jome

#endif // _JOME_WAKE_UP_Q_WAKE_UP_CLIENT_HPP
