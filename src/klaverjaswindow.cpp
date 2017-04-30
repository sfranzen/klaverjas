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

// application headers
#include "klaverjaswindow.h"
#include "cardimageprovider.h"
#include "players/humanplayer.h"

// KDE headers
#include <KActionCollection>
#include <KConfigDialog>

#include <QQmlContext>
#include <QTimer>

KlaverjasWindow::KlaverjasWindow()
    : KXmlGuiWindow()
    , m_game(this)
{
    m_game.addPlayer(new HumanPlayer("You"));
    m_game.start();
    m_klaverjasView = new QQuickWidget(this);
    m_klaverjasView->engine()->addImageProvider("cards", new CardImageProvider);
    m_klaverjasView->rootContext()->setContextProperty("game", &m_game);
    m_klaverjasView->setResizeMode(QQuickWidget::SizeRootObjectToView);
    m_klaverjasView->setSource(QUrl("qrc:/KlaverjasView.qml"));
    setCentralWidget(m_klaverjasView);

//     m_switchAction = actionCollection()->addAction(QStringLiteral("switch_action"), this, SLOT(slotSwitchColors()));
//     m_switchAction->setText(i18n("Switch Colors"));
//     m_switchAction->setIcon(QIcon::fromTheme(QStringLiteral("fill-color")));
//     connect(m_switchAction, SIGNAL(triggered(bool)), m_klaverjasView, SLOT(slotSwitchColors()));
    KStandardAction::openNew(this, SLOT(fileNew()), actionCollection());
    KStandardAction::quit(qApp, SLOT(closeAllWindows()), actionCollection());
    KStandardAction::preferences(this, SLOT(settingsConfigure()), actionCollection());
    setupGUI();
}

KlaverjasWindow::~KlaverjasWindow()
{
}

void KlaverjasWindow::fileNew()
{
    qCDebug(klaverjas) << "klaverjas::fileNew()";
    (new KlaverjasWindow)->show();
}

void KlaverjasWindow::settingsConfigure()
{
    qCDebug(klaverjas) << "klaverjas::settingsConfigure()";
    // The preference dialog is derived from prefs_base.ui
    //
    // compare the names of the widgets in the .ui file
    // to the names of the variables in the .kcfg file
    //avoid to have 2 dialogs shown
    if (KConfigDialog::showDialog(QStringLiteral("settings"))) {
        return;
    }
    KConfigDialog *dialog = new KConfigDialog(this, QStringLiteral("settings"), klaverjasSettings::self());
    QWidget *generalSettingsDialog = new QWidget;
    settingsBase.setupUi(generalSettingsDialog);
    dialog->addPage(generalSettingsDialog, i18n("General"), QStringLiteral("package_setting"));
    connect(dialog, SIGNAL(settingsChanged(QString)), m_klaverjasView, SLOT(slotSettingsChanged()));
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}

