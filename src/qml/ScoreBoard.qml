
import QtQuick 2.5

Row {
    spacing: 2
    Repeater {
        id: lists
        model: game.teams.length
        delegate: Column {
            width: parent.width / lists.count
            height: parent.height
            Text {
                text: "Team " + game.teams[index].name
            }
            Repeater {
                model: game.teams[index].scores
                delegate: Text { text: modelData }
            }
        }
    }
}
