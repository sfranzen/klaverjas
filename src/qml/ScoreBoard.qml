
import QtQuick 2.7
import QtQuick.Controls 1.4

/**
 * ScoreBoard QML type
 *
 * The score board consists of a black rectangle onto which green rectangles
 * are placed, giving a tabular appearance. Two smaller rectangles at the top
 * show the team names, while the columns underneath display their scores.
 */
Rectangle {
    id: root
    property int scoreSize: 16
    height: childrenRect.height
    width: 200
    color: "black"
    Row {
        spacing: 2
        Repeater {
            id: columns
            model: game.teams
            Rectangle {
                width: (root.width - 2) / columns.count
                height: header.height + 4 + 16 * scoreSize + total.height
                color: "green"
                Column {
                    width: parent.width
                    Label {
                        id: header
                        padding: 2
                        text: "Team " + name
                        Connections {
                            target: game
                            onNewContract: {
                                if (modelData == contractors)
                                    header.text = "Team " + name + " *";
                                else
                                    header.text = "Team " + name;
                            }
                        }
                    }
                    Rectangle {
                        width: parent.width
                        height: 2
                        color: "black"
                    }
                    Repeater {
                        width: parent.width
                        model: scores
                        delegate: scoreElement
                    }
                }
                Rectangle {
                    width: parent.width
                    height: 2
                    anchors.bottom: total.top
                    color: "black"
                }
                Label {
                    id: total
                    anchors.bottom: parent.bottom
                    leftPadding: 2
                    text: totalScore
                }
            }
        }
    } // Row
    Component {
        id: scoreElement
        Item {
            width: parent.width
            height: scoreSize
            Label {
                id: points
                text: modelData.points
                fontSizeMode: Text.VerticalFit
                leftPadding: 2
            }
            Label {
                anchors.left: points.right
                font: points.font
                text: modelData.bonus == 0 ? "" : " + " + modelData.bonus
            }
            Label {
                anchors.right: parent.right
                font: points.font
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
