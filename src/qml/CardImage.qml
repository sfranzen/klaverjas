
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
        }
        if (!faceUp) {
            source = "image://cards/back";
            return;
        }
        var name = "image://cards/";
        switch (card.rank) {
            case Card.Ace:
                name += "1";
                break;
            case Card.Seven:
                name += "7";
                break;
            case Card.Eight:
                name += "8";
                break;
            case Card.Nine:
                name += "9";
                break;
            case Card.Ten:
                name += "10";
                break;
            case Card.Jack:
                name += "jack";
                break;
            case Card.Queen:
                name += "queen";
                break;
            case Card.King:
                name += "king";
        }
        name += "_";
        switch (card.suit) {
            case Card.Spades:
                name += "spade";
                break;
            case Card.Hearts:
                name += "heart";
                break;
            case Card.Diamonds:
                name += "diamond";
                break;
            case Card.Clubs:
                name += "club";
        }
        source = name;
    }
}
