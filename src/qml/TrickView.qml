
import QtQuick 2.7
import QtQuick.Controls 1.4

Item {
    height: rect.height + top.height + bottom.height
    width: rect.width + left.width + right.width
    Item {
        // Invisible rectangle that encloses the cards and is used to position
        // the player's names around them.
        id: rect
        anchors.centerIn: parent
        height: 300
        width: 300
    }
    Label {
        id: top
        text: game.players[0].name
        anchors.bottom: rect.top
        anchors.horizontalCenter: rect.horizontalCenter
    }
    Label {
        id: right
        text: game.players[1].name
        anchors.left: rect.right
        anchors.verticalCenter: rect.verticalCenter
    }
    Label {
        id: bottom
        text: game.players[2].name
        anchors.top: rect.bottom
        anchors.horizontalCenter: rect.horizontalCenter
    }
    Label {
        id: left
        text: game.players[3].name
        anchors.right: rect.left
        anchors.verticalCenter: rect.verticalCenter
    }
    PathView {
        id: path
        model: game.players
        delegate: CardImage {
            id: img
            Connections {
                target: modelData
                onCardPlayed: {
                    img.card = card;
                }
            }
            Connections {
                target: game
                onNewTrick: {
                    img.card = undefined;
                }
            }
        }
        path: Path {
            id: circle
            property var radius: 100
            startX: rect.width/2; startY: rect.y + rect.height/2 - radius
            PathArc {
                relativeX: 0; relativeY: 2*circle.radius
                radiusX: circle.radius
                radiusY: circle.radius
            }
            PathArc {
                relativeX: 0; relativeY: -2*circle.radius
                radiusX: circle.radius
                radiusY: circle.radius
            }
        }
    }
}
