
import QtQuick 2.7
import QtQuick.Controls 1.4

Row {
    spacing: 2
    Repeater {
        id: lists
        model: game.teams
        delegate: Column {
            width: parent.width / lists.count
            Label { text: "Team " + modelData.name }
            Repeater {
                model: modelData.scores
                delegate: Label { text: modelData }
            }
        }
    }
}
