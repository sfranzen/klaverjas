/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2017  Steven Franzen <sfranzen85@gmail.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */

import QtQuick 2.7
import QtQuick.Controls 1.4
import QtQuick.Layouts 1.4
import org.kde.klaverjas 1.0

Rectangle {
    id: root
    color: "green"
    property real minimumHeight: table.Layout.minimumHeight
    property real minimumWidth: table.Layout.minimumWidth + infoArea.Layout.minimumWidth
    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton
        onClicked: game.advance();
    }
    RowLayout {
        id: row
        anchors.fill: parent
        Item {
            id: table
            Layout.minimumWidth: Math.max(p.width, trick.width)
            Layout.minimumHeight: trick.height + p.height + 30
            Layout.fillWidth: true
            Layout.fillHeight: true
            MouseArea {
                z: 1
                anchors.fill: parent
                acceptedButtons: Qt.LeftButton
                onClicked: game.advance();
            }
            TrickView {
                id: trick
                anchors.top: parent.top
                anchors.horizontalCenter: parent.horizontalCenter
            }
            BidView {
                id: bidView
                anchors.centerIn: trick
            }
            PlayerView {
                id: p
                height: 100
                z: 2
                player: game.humanPlayer
                anchors.top: trick.bottom
                anchors.horizontalCenter: trick.horizontalCenter
                anchors.topMargin: 20
            }
            BidDialog {/* hidden by default */}
        }
        Column {
            id: infoArea
            Layout.minimumWidth: 200
            Layout.minimumHeight: childrenRect.height
            Layout.alignment: Qt.AlignTop
            ScoreBoard {
                width: parent.width
            }
            RowLayout {
                height: childrenRect.height
                Label {
                    id: label
                    text: "Trump suit:"
                }
                SuitLabel {
                    suit: game.trumpSuit
                    Layout.minimumHeight: 14
                    Layout.minimumWidth: 14
                }
            }
        }
    }
}
