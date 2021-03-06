/*
 * Copyright (C) 2019 Philippe Proulx <eepp.ca>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include <QApplication>
#include <QCommandLineParser>
#include <QString>
#include <QProcess>
#include <QTimer>
#include <iostream>
#include <cstdio>
#include <cstdlib>

#include "emoji-db.hpp"
#include "emoji-images.hpp"
#include "q-jome-window.hpp"
#include "q-jome-server.hpp"

enum class Format {
    UTF8,
    CODEPOINTS_HEX,
};

struct Params
{
    Format fmt;
    bool noNewline;
    std::string serverName;
    std::string cmd;
    std::string cpPrefix;
};

static Params parseArgs(QApplication& app, int argc, char **argv)
{
    QCommandLineParser parser;

    parser.setApplicationDescription("An emoji picker desktop application");
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption formatOpt {"f", "Output format", "FORMAT", "utf-8"};
    QCommandLineOption serverNameOpt {"s", "Server name", "NAME"};
    QCommandLineOption cmdOpt {"c", "External command", "CMD"};
    QCommandLineOption cpPrefixOpt {"p", "Codepoint prefix", "CPPREFIX"};
    QCommandLineOption noNlOpt {"n", "Do not output newline"};

    parser.addOption(formatOpt);
    parser.addOption(serverNameOpt);
    parser.addOption(cmdOpt);
    parser.addOption(cpPrefixOpt);
    parser.addOption(noNlOpt);
    parser.process(app);

    Params params;

    params.noNewline = parser.isSet(noNlOpt);

    const auto fmt = parser.value(formatOpt);

    if (fmt == "utf-8") {
        params.fmt = Format::UTF8;
    } else if (fmt == "cp") {
        params.fmt = Format::CODEPOINTS_HEX;
    } else {
        std::cerr << "Command-line error: unknown format `" <<
                     fmt.toUtf8().constData() << "`." << std::endl;
        std::exit(1);
    }

    if (parser.isSet(serverNameOpt)) {
        params.serverName = parser.value(serverNameOpt).toUtf8().constData();
    }

    if (parser.isSet(cmdOpt)) {
        params.cmd = parser.value(cmdOpt).toUtf8().constData();
    }

    if (parser.isSet(cpPrefixOpt)) {
        params.cpPrefix = parser.value(cpPrefixOpt).toUtf8().constData();
    }

    return params;
}

static void execCommand(const std::string& cmd, const std::string& arg)
{
    QString fullCmd = QString::fromStdString(cmd);

    fullCmd += " ";
    fullCmd += QString::fromStdString(arg);
    static_cast<void>(QProcess::execute(fullCmd));
}

static std::string formatEmoji(const jome::Emoji& emoji,
                               const jome::Emoji::SkinTone skinTone,
                               const Format fmt, const std::string& cpPrefix,
                               const bool noNl)
{
    std::string output;

    switch (fmt) {
    case Format::UTF8:
    {
        std::string str;

        if (emoji.hasSkinToneSupport()) {
            str = emoji.strWithSkinTone(skinTone);
        } else {
            str = emoji.str();
        }

        output = emoji.str();
        break;
    }

    case Format::CODEPOINTS_HEX:
    {
        jome::Emoji::Codepoints codepoints;

        if (emoji.hasSkinToneSupport()) {
            codepoints = emoji.codepointsWithSkinTone(skinTone);
        } else {
            codepoints = emoji.codepoints();
        }

        for (const auto codepoint : codepoints) {
            std::array<char, 32> buf;

            std::sprintf(buf.data(), "%s%x ", cpPrefix.c_str(),
                         codepoint);
            output += buf.data();
        }

        // remove trailing space
        output.resize(output.size() - 1);
        break;
    }
    }

    if (!noNl) {
        output += '\n';
    }

    return output;
}

int main(int argc, char **argv)
{
    QApplication app {argc, argv};
    std::unique_ptr<jome::QJomeServer> server;

    app.setApplicationDisplayName("jome");
    app.setOrganizationName("jome");
    app.setApplicationName("jome");
    app.setApplicationVersion(JOME_VERSION);

    const auto params = parseArgs(app, argc, argv);
    jome::EmojiDb db {JOME_DATA_DIR};
    jome::QJomeWindow win {db};

    QObject::connect(&win, &jome::QJomeWindow::canceled,
                     [&params, &app, &server]() {
        if (server) {
            // reply to the client at least
            server->sendToClient("");
        }

        if (!server) {
            // TODO: make sure the message is sent before quitting
            QTimer::singleShot(0, [&app]() {
                app.exit(1);
            });
        }
    });
    QObject::connect(&win, &jome::QJomeWindow::emojiChosen,
                     [&](const auto& emoji, const auto skinTone) {
        const auto emojiStr = formatEmoji(emoji, skinTone, params.fmt,
                                          params.cpPrefix,
                                          params.noNewline || !params.cmd.empty());

        if (server) {
            // send response to client
            server->sendToClient(emojiStr);
        }

        // print result
        std::cout << emojiStr;
        std::cout.flush();

        if (!params.cmd.empty()) {
            // execute command in 20 ms
            QTimer::singleShot(20, &app, [&params, &server, &app, emojiStr]() {
                execCommand(params.cmd, emojiStr);

                if (!server) {
                    // no server: quit after executing the command
                    QTimer::singleShot(0, &app, &QApplication::quit);
                }
            });
        } else {
            if (!server) {
                // no server: quit now
                QTimer::singleShot(0, &app, &QApplication::quit);
            }
        }

        // always hide when accepting
        win.hide();

        // add emoji as recent emoji
        db.addRecentEmoji(emoji);

        if (server) {
            /*
             * Not calling directly because we're potentially within an
             * event handler which is currently using an emoji graphics
             * item, so we cannot delete it.
             */
            QTimer::singleShot(0, &win, &jome::QJomeWindow::emojiDbChanged);
        }
    });

    if (!params.serverName.empty()) {
        server = std::make_unique<jome::QJomeServer>(nullptr,
                                                     params.serverName);
        QObject::connect(server.get(), &jome::QJomeServer::clientRequested,
                         [&app, &server, &win](const jome::QJomeServer::Command cmd) {
            if (cmd == jome::QJomeServer::Command::QUIT) {
                // reply to client, then quit
                server->sendToClient("");

                // TODO: make sure the message is sent before quitting
                QTimer::singleShot(10, &QApplication::quit);
            } else {
                win.show();
            }
        });
    }

    if (!server) {
        win.show();
    }

    return app.exec();
}
