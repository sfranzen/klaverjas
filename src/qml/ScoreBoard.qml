
import QtQuick 2.7
import QtQuick.Controls 2.4

/**
 * ScoreBoard QML type
 *
 * The score board consists of a black rectangle onto which green rectangles
 * are placed, giving a tabular appearance. Two smaller rectangles at the top
 * show the team names, while the columns underneath display their scores.
 */
Rectangle {
    id: root
    color: "black"
    implicitHeight: childrenRect.height
    Grid {
        spacing: 2
        columns: headers.count
        Repeater {
            id: headers
            model: game.teams
            Label {
                id: names
                text: "Team " + modelData.name
                background: Rectangle {
                    color: "green"
                    width: root.width / headers.count
                }
                Connections {
                    target: game
                    onNewContract: {
                        if (modelData == contractors)
                            text = "Team " + modelData.name + " *";
                    }
                    onNewRound: {
                        if (text.endsWith(" *"))
                            text = "Team " + modelData.name;
                    }
                }
            }
        }
        Repeater {
            id: scores
            model: game.teams
            Rectangle {
                height: 192
                width: root.width / headers.count
                color: "green"
                Column {
                    spacing: 2
                    Repeater {
                        model: modelData.scores
                        Label {
                            text: modelData
                            height: 10
                        }
                    }
                }
            }
        }
    }
}
