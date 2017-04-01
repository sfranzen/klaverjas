
import QtQuick 2.7
import QtQuick.Controls 1.4

Item {
    height: childrenRect.height
    width: childrenRect.width
    Item {
        id: rect
        height: 300
        width: 300
    }
    Label {
        text: game.players[0].name
        anchors.bottom: rect.top
        anchors.horizontalCenter: rect.horizontalCenter
    }
    Label {
        text: game.players[1].name
        anchors.left: rect.right
        anchors.verticalCenter: rect.verticalCenter
    }
    Label {
        text: game.players[2].name
        anchors.top: rect.bottom
        anchors.horizontalCenter: rect.horizontalCenter
    }
    Label {
        text: game.players[3].name
        anchors.right: rect.left
        anchors.verticalCenter: rect.verticalCenter
    }
    PathView {
        id: path
        model: game.players
        delegate: CardImage {
            id: image
            Connections {
                target: modelData
                onCardPlayed: {
                    image.card = card;
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
