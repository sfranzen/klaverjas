
import QtQuick 2.7
import QtQuick.Controls 2.4
import org.kde.klaverjas 1.0

// TODO move player labels somewhere else
Item {
    implicitHeight: names.height
    implicitWidth: names.width
    CircleView {
        id: names
        radius: 170
        model: game.players
        delegate: Label {
            text: modelData.name
        }
    }
    CircleView {
        id: cards
        radius: 100
        model: game.players
        anchors.centerIn: names
        delegate: CardImage {
            id: img
            Connections {
                target: game
                onCardPlayed: {
                    if (index == player)
                        img.card = card;
                }
                onNewTrick: img.card = undefined;
                onNewRound: img.card = undefined;
            }
        }
    }
}
