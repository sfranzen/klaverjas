
import QtQuick 2.7
import QtQuick.Controls 2.0
import org.kde.klaverjas 1.0

Popup {
    id: root
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
                    Text {
                        anchors.fill: parent
                        anchors.margins: 2
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        fontSizeMode: Text.Fit
                        font.pixelSize: parent.height
                        text: label(modelData)
                        color: textColor(modelData)
                    }
                    onClicked: {
                        game.players[0].bidSelected(modelData);
                        root.close();
                    }
                }
            }
            Connections {
                target: game.players[0]
                onBidRequested: {
                    optionList.model = options;
                    root.open();
                }
            }
        }
    }
    // Output prettification functions
    function label(bid) {
        switch (bid) {
            case Game.Spades:
                return "\u2260";
            case Game.Hearts:
                return "\u2665";
            case Game.Diamonds:
                return "\u2666";
            case Game.Clubs:
                return "\u2663";
            default:
                return "Pass";
        }
    }
    function textColor(bid) {
        if (bid == Game.Hearts || bid == Game.Diamonds)
            return "red";
        else
            return "black";
    }
}
