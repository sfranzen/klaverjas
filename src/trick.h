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
#include "rules.h"
#include "scores.h"

#include <QObject>
#include <QVector>

#include <tuple>

class Trick
{
    Q_GADGET
public:
    /**
     * Signal definitions
     *
     * Klaverjas players can inform each other about the strength of their hand
     * if their partner is heading the current trick and they cannot follow
     * suit. This is done by playing a particular rank of a different suit.
     * The meanings and associated ranks are as follows:
     */
    enum class Signal : uchar {
        None,
        /// The player has played a 7, 8 or 9 to signal possession of the
        /// current high card in that suit.
        High,
        /// The player has played the ace to signal possession of the 10 and at
        /// least another face card in that suit.
        Long,
        /// The player has played a K, Q or J to signal that he has no valuable
        /// cards in that suit.
        Low
    };
    using PlayerSignal = std::tuple<Card::Suit,Signal>;
    static const PlayerSignal NullSignal;
    Q_ENUM(Signal)

    Trick() = default;
    Trick(Card::Suit trumpSuit);

    void add(const Card card);
    const QVector<Card> &cards() const;
    Score score() const;
    Card::Suit suitLed() const;
    ushort winner() const;
    const Card &winningCard() const;
    bool isComplete() const;
    /// Returns a valid signal if the last card played was a signal, otherwise
    /// a NullSignal.
    PlayerSignal checkSignal() const;

private:
    QVector<Card> m_cards;
    Score m_score;
    ushort m_winner;
    Card::Suit m_trumpSuit;

    void checkWinner();
    void checkBonus();

    friend QDebug operator<<(QDebug dbg, const Trick &trick);
};

#endif // TRICK_H
