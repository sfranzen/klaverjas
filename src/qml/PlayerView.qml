
import QtQuick 2.7

Grid {
    property bool horizontal: true
    property alias model: cards.model
    columns: horizontal ? 8 : 1
    spacing: 2

    Repeater {
        id: cards
        delegate: CardImage {
            card: modelData
            rotation: horizontal ? 0 : 90
        }
    }
}
