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

#ifndef KLAVERJASWINDOW_H
#define KLAVERJASWINDOW_H

#include "ui_settingsBase.h"
#include "klaverjasSettings.h"
#include "game.h"

#include <QLoggingCategory>
#include <QQuickWidget>
#include <KXmlGuiWindow>

Q_DECLARE_LOGGING_CATEGORY(klaverjas)

/**
 * This class serves as the main window for klaverjas.  It handles the
 * menus, toolbars and status bars.
 *
 * @short Main window class
 * @author %{AUTHOR} <%{EMAIL}>
 * @version %{VERSION}
 */
class KlaverjasWindow : public KXmlGuiWindow
{
    Q_OBJECT
public:
    /**
     * Default Constructor
     */
    KlaverjasWindow();

    /**
     * Default Destructor
     */
    virtual ~KlaverjasWindow();

private slots:
    /**
     * Create a new window
     */
    void fileNew();

    /**
     * Open the settings dialog
     */
    void settingsConfigure();

private:
    Game m_game;
    Ui::settingsBase settingsBase;
    QAction *m_switchAction;
    QQuickWidget* m_klaverjasView;
};

#endif // KLAVERJASWINDOW_H
