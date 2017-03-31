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
#include "klaverjas.h"
#include "cardset.h"
#include "game.h"
#include "team.h"
#include "player.h"

// KDE headers
#include <KAboutData>
#include <KLocalizedString>

// Qt headers
#include <QApplication>
#include <QCommandLineParser>
#include <QIcon>
#include <QLoggingCategory>
#include <QDebug>

Q_DECLARE_LOGGING_CATEGORY(klaverjas)
Q_LOGGING_CATEGORY(klaverjas, "klaverjas")
Q_LOGGING_CATEGORY(klaverjasGame, "klaverjas.game")
Q_LOGGING_CATEGORY(klaverjasPlayer, "klaverjas.player")

int main(int argc, char **argv)
{
    QApplication application(argc, argv);

    KLocalizedString::setApplicationDomain("klaverjas");
    KAboutData aboutData( QStringLiteral("klaverjas"),
                          i18n("Klaverjas"),
                          QStringLiteral("%{VERSION}"),
                          i18n("Simple Klaverjas game using KDE Frameworks and QML"),
                          KAboutLicense::GPL,
                          i18n("(c) 2017, Steven Franzen <sfranzen85@gmail.com>"));

    aboutData.addAuthor(i18n("%{AUTHOR}"),i18n("Author"), QStringLiteral("%{EMAIL}"));
    application.setWindowIcon(QIcon::fromTheme("klaverjas"));
    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();
    aboutData.setupCommandLine(&parser);
    parser.process(application);
    aboutData.processCommandLine(&parser);
    KAboutData::setApplicationData(aboutData);

    qRegisterMetaType<CardSet>();
    qmlRegisterUncreatableType<Card>("org.kde.klaverjas", 1, 0, "Card", "Enum/property access only.");
    qmlRegisterType<Team>("org.kde.klaverjas", 1, 0, "Team");
    qmlRegisterType<Player>("org.kde.klaverjas", 1, 0, "Player");

    Klaverjas *appwindow = new Klaverjas;
    appwindow->show();
    return application.exec();
}
