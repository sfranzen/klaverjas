
import QtQuick 2.7
import org.kde.klaverjas 1.0

Item {
    default property var suit
    height: 20
    width: 20
    Text {
        anchors.fill: parent
        anchors.margins: 5
        text: suitLabel(suit)
        color: suitColor(suit)
        font.pixelSize: parent.height
        fontSizeMode: Text.HorizontalFit
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter

        // Game.Pass is not technically a suit but we get away with this because
        // its integer value exceeds those of the suits.
        function suitLabel(suit) {
            switch (suit) {
                case Card.Spades:
                    return "\u2660";
                case Card.Hearts:
                    return "\u2665";
                case Card.Diamonds:
                    return "\u2666";
                case Card.Clubs:
                    return "\u2663";
                case Game.Pass:
                    return "Pass";
                default:
                    return "";
            }
        }
        function suitColor(suit) {
            if (suit == Card.Hearts || suit == Card.Diamonds)
                return "red";
            else
                return "black";
        }
    }
}
