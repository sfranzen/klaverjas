/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2017  Steven Franzen <sfranzen85@gmail.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */

#ifndef TRICK_H
#define TRICK_H

#include <QVector>
#include <QMap>

#include "card.h"
#include "cardset.h"
#include "rules.h"

class Player;

class Trick
{
public:
    Trick() = default;
    Trick(Suit trumpSuit);

    void add(Player* player, const Card& card);
    const CardSet* cards() const;
    const QVector<Player*> players() const;
    int points() const;
    Suit suitLed() const;
    Player* winner() const;
    const Card* winningCard() const;

private:
    void setWinner(Player* player, const Card& card);

    CardSet m_cards;
    QVector<Player*> m_players;
    Suit m_trumpSuit;
    Suit m_suitLed;
    QVector<Rank> m_order;
    QMap<Rank,int> m_values;
    int m_points;
    Player* m_winner;
    Card m_winningCard;

    const static CardSet::SortingMap s_bonusSortingMap;
    friend QDebug operator<<(QDebug dbg, const Trick& trick);
};


#endif // TRICK_H
