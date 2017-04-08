
import QtQuick 2.7
import org.kde.klaverjas 1.0

ListView {
    id: view
    property Player player
    height: 100
    width: parent.width
    anchors.margins: 5
    orientation: ListView.Horizontal
    interactive: false
    spacing: 2
    model: player ? player.hand.cards : undefined
    delegate: CardImage {
        card: modelData
        rotation: orientation == ListView.Horizontal ? 0 : 90
        MouseArea {
            id: mArea
            enabled: false
            anchors.fill: parent
            hoverEnabled: true
            onPositionChanged: {
                currentIndex = index;
            }
            onClicked: {
                player.cardPlayed(modelData);
                enabled = false;
                view.highlightItem.visible = false;
            }
            Connections {
                target: player
                onMoveRequested: {
                    mArea.enabled = true;
                    view.highlightItem.visible = true;
                }
            }
        }
    }
    highlight: Rectangle {
        visible: false
        color: "steelblue"
        z: 1
        opacity: 0.3
    }
}
