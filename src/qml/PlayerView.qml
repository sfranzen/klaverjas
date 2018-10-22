
import QtQuick 2.11
import org.kde.klaverjas 1.0

ListView {
    id: list
    property HumanPlayer player: game.humanPlayer
    height: 100
    implicitWidth: contentItem.childrenRect.width
    orientation: ListView.Horizontal
    interactive: false
    spacing: 2
    model: player.hand.cards
    delegate: CardImage {
        card: modelData
        rotation: orientation == ListView.Horizontal ? 0 : 90
        MouseArea {
            id: mArea
            anchors.fill: parent
            enabled: false
            hoverEnabled: true
            propagateComposedEvents: true
            onClicked: {
                mArea.enabled = false;
                player.moveSelected(modelData);
            }
            onEntered: list.currentIndex = index
            Connections {
                target: game
                onMoveRequested: if (game.currentPlayer == player) {
                    list.highlightItem.visible = true;
                    mArea.enabled = true;
                }
            }
        }
    }
    highlight: Rectangle {
        visible: false
        color: "steelblue"
        z: 1.1
        opacity: 0.3
    }
}
