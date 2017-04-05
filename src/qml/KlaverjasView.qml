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
import org.kde.klaverjas 1.0

Rectangle {
    id: root
    color: "green"
    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton
        onClicked: {
            game.advance()
        }
    }
    Row {
        anchors.fill: parent
        Item {
            id: table
            height: parent.height
            width: parent.width - infoArea.width
            TrickView {
                id: trick
                anchors.centerIn: parent
            }
            PlayerView {
                width: parent.width
                player: game.players[1]
                anchors.top: trick.bottom
                anchors.left: parent.left
            }
            BidDialog {/* hidden by default */}
        }
        Column {
            id: infoArea
            width: 200
            height: parent.height
            ScoreBoard {
                width: parent.width
            }
            Text {
                id: trump
                Connections {
                    target: game
                    onTrumpSuitChanged: {
                        trump.text = "Trump suit: " + newSuit;
                    }
                }
            }
        }
    }
}
