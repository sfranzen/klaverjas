
import QtQuick 2.7
import org.kde.klaverjas 1.0

// The visual representation of a card
Image {
    property var card
    property bool faceUp: true
    height: 100
    fillMode: Image.PreserveAspectFit
    onCardChanged: {
        if (!card) {
            source = "";
            return;
        } else if (!faceUp) {
            source = "image://cards/back";
            return;
        }

        var name = "image://cards/";
        var r = card.rank;
        if (r == Card.Ace)
            name += "1";
        else if (r == Card.Seven)
            name += "7";
        else if (r == Card.Eight)
            name += "8";
        else if (r == Card.Nine)
            name += "9";
        else if (r == Card.Ten)
            name += "10";
        else if (r == Card.Jack)
            name += "jack";
        else if (r == Card.Queen)
            name += "queen";
        else if (r == Card.King)
            name += "king";
        name += "_";
        var s = card.suit;
        if (s == Card.Spades)
            name += "spade";
        else if (s == Card.Hearts)
            name += "heart";
        else if (s == Card.Diamonds)
            name += "diamond";
        else
            name += "club";
        source = name;
    }
}
