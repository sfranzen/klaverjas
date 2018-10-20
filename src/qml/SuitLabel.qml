
import QtQuick 2.7
import org.kde.klaverjas 1.0

Text {
    default property var suit: {}
    height: 20
    width: 20
    text: suitLabel()
    color: suitColor()
    font.pixelSize: height
    fontSizeMode: Text.Fit
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
        if (suit == undefined)
            return "Pass";
        return "";
    }
    function suitColor() {
        if (suit == Card.Hearts || suit == Card.Diamonds)
            return "red";
        else
            return "black";
    }
}
