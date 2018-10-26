/*
Copyright (C) %{CURRENT_YEAR} by %{AUTHOR} <%{EMAIL}>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of
the License or (at your option) version 3 or any later version
accepted by the membership of KDE e.V. (or its successor approved
by the membership of KDE e.V.), which shall act as a proxy 
defined in Section 14 of version 3 of the license.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// application header
#include "game.h"
#include "card.h"
#include "cardset.h"
#include "scores.h"
#include "team.h"
#include "players/player.h"
#include "players/humanplayer.h"
#include "cardimageprovider.h"

// Qt headers
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QCommandLineParser>
#include <QIcon>
#include <QLoggingCategory>
#include <QTime>

Q_DECLARE_LOGGING_CATEGORY(klaverjas)
Q_LOGGING_CATEGORY(klaverjas, "klaverjas")
Q_LOGGING_CATEGORY(klaverjasGame, "klaverjas.game")
Q_LOGGING_CATEGORY(klaverjasPlayer, "klaverjas.player")
Q_LOGGING_CATEGORY(klaverjasAi, "klaverjas.ai")
Q_LOGGING_CATEGORY(klaverjasTrick, "klaverjas.trick")
Q_LOGGING_CATEGORY(klaverjasTest, "klaverjas.aitest")

int main(int argc, char **argv)
{
    QGuiApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("klaverjas"));
    app.setApplicationDisplayName("Klaverjas");
    app.setApplicationVersion(QStringLiteral("%{VERSION}"));
    app.setDesktopFileName(QStringLiteral("org.example.klaverjas.desktop"));
    app.setWindowIcon(QIcon::fromTheme("klaverjas"));
    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    parser.process(app);

    qmlRegisterUncreatableType<Game>("org.kde.klaverjas", 1, 0, "Game", "Only available as context object \"game\".");
    qmlRegisterUncreatableType<Card>("org.kde.klaverjas", 1, 0, "Card", "Enum/property access only.");
    qmlRegisterUncreatableType<RoundScore>("org.kde.klaverjas", 1, 0, "Score", "Property access only.");
    qmlRegisterUncreatableType<Player>("org.kde.klaverjas", 1, 0, "Player", "Abstract class.");
    qmlRegisterType<HumanPlayer>("org.kde.klaverjas", 1, 0, "HumanPlayer");
    qmlRegisterType<Team>("org.kde.klaverjas", 1, 0, "Team");
    qRegisterMetaType<CardSet>("CardSet");
    qRegisterMetaType<Card::Suit>("Suit");
    qRegisterMetaType<Card::Rank>("Rank");

    QLoggingCategory::setFilterRules("debug=true\n"
        "klaverjas.*.debug=true\n"
        "qml=true"
    );

    // Initialise PRNG
    std::srand(QTime::currentTime().msec());

    // Set up game and engine
    auto *game = new Game();
    game->addPlayer(new HumanPlayer("You", game));
    QQmlApplicationEngine engine;
    engine.addImageProvider("cards", new CardImageProvider());
    engine.rootContext()->setContextProperty("game", game);
    engine.load(QUrl("qrc:/main.qml"));

    if (engine.rootObjects().isEmpty())
        return -1;
    return app.exec();
}
