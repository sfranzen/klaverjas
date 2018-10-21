
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
                width: (root.width - 2) / headers.count
                text: "Team " + modelData.name
                padding: 2
                background: Rectangle {
                    color: "green"
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
                width: (root.width - 2) / headers.count
                color: "green"
                Column {
                    anchors.fill: parent
                    Repeater {
                        model: modelData.scores
                        delegate: scoreElement
                    }
                }
            }
        }
        Component {
            id: scoreElement
            Item {
                width: parent.width
                implicitHeight: childrenRect.height
                Label {
                    id: points
                    text: modelData.points
                    leftPadding: 2
                }
                Label {
                    anchors.left: points.right
                    text: modelData.bonus == 0 ? "" : " + " + modelData.bonus
                }
                Label {
                    anchors.right: parent.right
                    rightPadding: 2
                    text: {
                        if (modelData.wet)
                            return "NAT";
                        else if (modelData.march)
                            return "PIT";
                        else
                            return "";
                    }
                }
            }
        }
    }
}
