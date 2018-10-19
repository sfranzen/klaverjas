
import QtQuick 2.7
import QtQuick.Controls 2.0
import org.kde.klaverjas 1.0

Popup {
    id: root
    default property HumanPlayer player: game.humanPlayer
    x: (parent.width - width) / 2
    y: (parent.height - height) / 2
    modal: true
    focus: true
    closePolicy: Popup.NoAutoClose
    background: Rectangle {
        color: "green"
        border.width: 1
        border.color: "white"
    }
    Column {
        Label {
            width: options.width
            wrapMode: Text.WordWrap
            bottomPadding: 5
            text: "Declare your bid:"
        }
        Row {
            id: options
            spacing: 2
            Repeater {
                id: optionList
                delegate: Button {
                    width: 50; height: 50
                    SuitLabel {
                        suit: modelData
                        anchors.fill: parent
                        anchors.margins: 5
                    }
                    onClicked: {
                        player.bidSelected(modelData);
                        root.close();
                    }
                }
            }
            Connections {
                target: game
                onBidRequested: {
                    if (player == root.player) {
                        optionList.model = options;
                        root.open();
                    }
                }
            }
        }
    }
}
