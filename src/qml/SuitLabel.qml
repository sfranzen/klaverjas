
import QtQuick 2.7
import org.kde.klaverjas 1.0

Item {
    default property var suit: {}
    height: 20
    width: 20
    Text {
        anchors.fill: parent
        anchors.margins: 5
        text: suitLabel()
        color: suitColor()
        font.pixelSize: parent.height
        fontSizeMode: Text.HorizontalFit
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter

        function suitLabel() {
            if (suit == Card.Spades)
                return "\u2660";
            if (suit == Card.Hearts)
                return "\u2665";
            if (suit == Card.Diamonds)
                return "\u2666";
            if (suit == Card.Clubs)
                return "\u2663";
            return "Pass";
        }
        function suitColor() {
            if (suit == Card.Hearts || suit == Card.Diamonds)
                return "red";
            else
                return "black";
        }
    }
}
