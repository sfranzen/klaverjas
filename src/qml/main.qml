/*
 * This file is part of Klaverjas.
 * Copyright (C) 2018  Steven Franzen <sfranzen85@gmail.com>
 *
 * Klaverjas is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Klaverjas is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

import QtQuick 2.11
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import org.kde.klaverjas 1.0

ApplicationWindow {
    id: window
    visible: true
    minimumWidth: view.minimumWidth
    minimumHeight: view.minimumHeight + menuBar.__contentItem.height + statusBar.contentItem.height
    menuBar: MenuBar {
        id: menuBar
        Menu {
            title: "&File"
            MenuItem {
                text: "&Quit"
                shortcut: StandardKey.Quit
                onTriggered: Qt.quit()
            }
        }
        style: MenuBarStyle {
            background: barStyle
        }
    }
    KlaverjasView {
        id: view
        anchors.fill: parent
        Component.onCompleted: game.start();
    }
    Component.onCompleted: console.log(height,width)
    statusBar: StatusBar {
        id: statusBar
        Label { text: "Read Only" }
        style: StatusBarStyle {
            background: barStyle
        }
    }
    Component {
        id: barStyle
        Rectangle {
            SystemPalette { id: palette }
            color: palette.window
        }
    }
}
