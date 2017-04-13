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

#include "card.h"
#include "cardset.h"
#include "rules.h"

#include <QObject>
#include <QVector>
#include <QMap>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(klaverjasTrick)

class Player;

class Trick
{
    Q_GADGET
public:
    Trick() = default;
    Trick(Card::Suit trumpSuit);

    /** Signal definitions
     *
     * Klaverjas players can inform each other about the strength of their hand
     * if their partner is heading the current trick and they cannot follow
     * suit. This is done by playing a particular rank of a different suit.
     * The meanings and associated ranks are as follows:
     *
     * - High: the partner has played a low card (7, 8 or 9) to signal that he
     *      has the current high card (usually the ace) in that suit.
     *
     * - Long: the partner has played the ace to signal that he also has at least
     *      the 10 and K in that suit.
     *
     * - Low: the partner has played a K, Q or J to signal that he has no
     *      valuable cards in that suit.
     */
    enum class Signal { High, Long, Low, None };
    Q_ENUM(Signal)

    void add(int player, const Card& card);
    const CardSet* cards() const;
    const QVector<int> players() const;
    int points() const;
    Card::Suit suitLed() const;
    int winner() const;
    const Card* winningCard() const;

// signals:
//     void playerSignal(Player* player, Signal signal, Card::Suit suit);

private:
    void setWinner(int player, const Card& card);

    CardSet m_cards;
    QVector<int> m_players;
    Card::Suit m_trumpSuit;
    Card::Suit m_suitLed;
    int m_points;
    int m_winner;
    Card m_winningCard;

    const static CardSet::SortingMap s_bonusSortingMap;
    friend QDebug operator<<(QDebug dbg, const Trick& trick);
};


#endif // TRICK_H
