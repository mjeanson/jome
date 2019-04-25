/*
 * Copyright (C) 2019 Philippe Proulx <eepp.ca>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QTimer>
#include <QString>
#include <iostream>

#include "q-wake-up-client.hpp"

struct Params
{
    bool quit;
    std::string serverName;
};

static Params parseArgs(QCoreApplication& app, int argc, char **argv)
{
    QCommandLineParser parser;

    parser.setApplicationDescription("Wake up jome");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("SERVER-NAME", "jome server name", "NAME");

    QCommandLineOption quitOpt {"q", "Quit instead of showing window"};

    parser.addOption(quitOpt);
    parser.process(app);

    Params params;

    if (parser.positionalArguments().isEmpty()) {
        std::cerr << "Command-line error: missing server name." << std::endl;
        std::exit(1);
    }

    params.serverName = parser.positionalArguments().first().toUtf8().constData();
    params.quit = parser.isSet(quitOpt);
    return params;
}

int main(int argc, char **argv)
{
    QCoreApplication app {argc, argv};

    app.setApplicationName("jome-wake-up");
    app.setApplicationVersion(JOME_VERSION);

    const auto params = parseArgs(app, argc, argv);

    jome::QWakeUpClient client {nullptr, params.serverName};

    client.wakeUp(params.quit);
    QTimer::singleShot(0, &app, SLOT(quit()));
    return app.exec();
}
