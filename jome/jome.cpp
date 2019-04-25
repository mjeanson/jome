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
#include "q-wake-up-server.hpp"

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
    QCommandLineOption serverNameOpt {"s", "Wake up server name", "NAME"};
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

int main(int argc, char **argv)
{
    QApplication app {argc, argv};
    std::unique_ptr<jome::QWakeUpServer> server;

    app.setApplicationDisplayName("jome");
    app.setApplicationName("jome");
    app.setApplicationVersion(JOME_VERSION);

    const auto params = parseArgs(app, argc, argv);
    const jome::EmojiDb db {JOME_DATA_DIR};
    jome::QJomeWindow win {db, !params.serverName.empty(),
                           [&params, &app, &win](const auto& emoji,
                                                 const auto skinTone) {
        std::string output;

        switch (params.fmt) {
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

                std::sprintf(buf.data(), "%s%x ", params.cpPrefix.c_str(),
                             codepoint);
                output += buf.data();
            }

            // remove trailing space
            output.resize(output.size() - 1);
            break;
        }

        default:
            std::abort();
        }

        if (params.cmd.empty() && !params.noNewline) {
            output += '\n';
        }

        if (params.cmd.empty()) {
            std::printf("%s", output.c_str());
            std::fflush(stdout);
        } else {
            if (params.serverName.empty()) {
                execCommand(params.cmd, output);
            } else {
                win.hide();
                QTimer::singleShot(20, &app, [&params, output]() {
                    execCommand(params.cmd, output);
                });
            }
        }
    }};

    if (!params.serverName.empty()) {
        server = std::make_unique<jome::QWakeUpServer>(nullptr,
                                                       params.serverName);
        QObject::connect(server.get(), SIGNAL(wakeUp()),
                         &win, SLOT(show()));
        QObject::connect(server.get(), SIGNAL(quit()),
                         &app, SLOT(quit()));
    }

    if (params.serverName.empty()) {
        win.show();
    }

    return app.exec();
}
