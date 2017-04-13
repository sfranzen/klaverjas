
import QtQuick 2.7
import QtQuick.Controls 1.4
import org.kde.klaverjas 1.0

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
        id: bottom
        text: game.players[0].name
        anchors.top: rect.bottom
        anchors.horizontalCenter: rect.horizontalCenter
    }
    Label {
        id: left
        text: game.players[1].name
        anchors.right: rect.left
        anchors.verticalCenter: rect.verticalCenter
    }
    Label {
        id: top
        text: game.players[2].name
        anchors.bottom: rect.top
        anchors.horizontalCenter: rect.horizontalCenter
    }
    Label {
        id: right
        text: game.players[3].name
        anchors.left: rect.right
        anchors.verticalCenter: rect.verticalCenter
    }
    PathView {
        id: path
        model: game.players
        delegate: CardImage {
            id: img
            Connections {
                target: game
                onCardPlayed: {
                    if (index == player)
                        img.card = card;
                }
                onNewTrick: {
                    img.card = undefined;
                }
            }
        }
        path: Path {
            id: circle
            property var radius: 100
            startX: rect.x + rect.width/2; startY: rect.y + rect.height/2 + radius
            PathArc {
                relativeX: 0; relativeY: -2*circle.radius
                radiusX: circle.radius
                radiusY: circle.radius
            }
            PathArc {
                relativeX: 0; relativeY: 2*circle.radius
                radiusX: circle.radius
                radiusY: circle.radius
            }
        }
    }
}
