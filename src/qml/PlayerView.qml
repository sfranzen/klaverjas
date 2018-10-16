
import QtQuick 2.7
import org.kde.klaverjas 1.0

ListView {
    id: list
    property HumanPlayer player: game.humanPlayer
    height: 100
    implicitWidth: contentItem.childrenRect.width
    anchors.margins: 5
    orientation: ListView.Horizontal
    interactive: false
    spacing: 2
    model: player.hand.cards
    delegate: CardImage {
        card: modelData
        rotation: orientation == ListView.Horizontal ? 0 : 90
    }
    highlight: Rectangle {
        visible: false
        color: "steelblue"
        z: 0
        opacity: 0.3
    }
    MouseArea {
        id: mArea
        anchors.fill: parent
        enabled: false
        hoverEnabled: true
        onPositionChanged: {
            if (itemAt(mouse.x, mouse.y))
                parent.currentIndex = indexAt(mouse.x, mouse.y);
        }
        onClicked: {
            var choice = itemAt(mouse.x, mouse.y);
            if (choice) {
                game.acceptMove(choice.card);
                enabled = false;
                list.highlightItem.visible = false;
            }
        }
    }
    Connections {
        target: game
        onMoveRequested: {
            mArea.enabled = true;
            highlightItem.visible = true;
        }
    }
//     Connections {
//         target: player
//         onHandChanged: {
//             model = player.hand.cards;
//         }
//     }
}
