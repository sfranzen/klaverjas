
import QtQuick 2.7

Grid {
    property bool horizontal: true
    property var model
    columns: horizontal ? 8 : 1
    spacing: 2

    Repeater {
        model: game.players[0].hand.cards
        delegate: CardImage { card: modelData }
    }
}
