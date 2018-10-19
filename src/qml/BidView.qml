/*
 * This file is part of Klaverjas.
 * Copyright (C) 2018  Steven Franzen <sfranzen85@gmail.com>
 *
 * Klaverjas is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Klaverjas is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

import QtQuick 2.11
import org.kde.klaverjas 1.0

/**
 * Bid View type.
 *
 * The bid view shows the progress of the bidding phase in each round.
 */
CircleView {
    radius: 120
    opacity: 0
    model: {}
    delegate: SuitLabel {
        Connections {
            target: modelData
            onBidSelected: {
                suit = bid;
            }
        }
    }
    Connections {
        target: game
        onBiddingStarted: {
            model = game.players;
            opacity = 1;
        }
        onNewTrick: {
            opacity = 0;
            model = {};
        }
    }
}
