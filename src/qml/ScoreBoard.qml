
import QtQuick 2.7
import QtQuick.Controls 1.4

Rectangle {
    id: root
    color: "black"
    height: childrenRect.height
    Grid {
        spacing: 2
        columns: headers.count
        Repeater {
            id: headers
            model: game.teams
            Rectangle {
                color: "green"
                width: root.width / headers.count
                height: names.height
                Label {
                    id: names
                    text: "Team " + modelData.name
                    anchors.centerIn: parent
                }
            }
        }
        Repeater {
            id: scores
            model: game.teams
            Rectangle {
                id: rect
                color: "green"
                width: root.width / headers.count
                height: 16 * 10
                Column {
                    anchors.fill: parent
                    Repeater {
                        model: modelData.scores
                        Label {
                            text: modelData
                            height: 10
                            anchors.horizontalCenter: parent.horizontalCenter
                        }
                    }
                }
            }
        }
    }
}
