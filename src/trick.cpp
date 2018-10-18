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

#include "trick.h"

#include <QDebug>

const CardSet::SortingMap Trick::s_bonusSortingMap {
    {Card::Suit::Spades, BonusOrder},
    {Card::Suit::Hearts, BonusOrder},
    {Card::Suit::Diamonds, BonusOrder},
    {Card::Suit::Clubs, BonusOrder}
};

Trick::Trick(Card::Suit trumpSuit)
    : m_trumpSuit(trumpSuit)
    , m_points(0)
    , m_winner(-1)
{
}

/* Each time a new card is played, we check whether it beats the previous card;
 * if so, this player becomes the trick's current winner.
 *
 * The current card beats the previous one either if it follows suit and ranks
 * higher or if it is of a different suit and that suit is the trump suit.
 */
void Trick::add(int player, const Card& card)
{
    m_cards << card;
    m_players << player;
    const Card::Suit suitPlayed = card.suit();
    m_points += cardValues(suitPlayed == m_trumpSuit)[card.rank()];
    if (m_players.size() == 1) {
        m_suitLed = suitPlayed;
        setWinner(player, card);
    } else {
        if (suitPlayed == m_winningCard.suit()) {
            const auto order = rankOrder(suitPlayed == m_trumpSuit);
            if (card.beats(m_winningCard, order))
                setWinner(player, card);
        } else if (suitPlayed == m_trumpSuit) {
            setWinner(player, card);
        }
    }
    return;

    // Detect player signaling, which can only be done by the third or fourth
    // player
    const int count = m_players.size();
    if (count > 2
        && suitPlayed != m_suitLed
        && suitPlayed != m_trumpSuit
        && m_players.at(count - 3) == m_winner
    ) {
        Signal signal = Signal::None;
        const auto rankPlayed = card.rank();
        if (rankPlayed >= Card::Rank::Nine && rankPlayed <= Card::Rank::Seven)
            signal = Signal::High;
        else if (rankPlayed >= Card::Rank::King && rankPlayed <= Card::Rank::Jack)
            signal = Signal::Low;
        else if (rankPlayed == Card::Rank::Ace)
            signal = Signal::Long;

        if (signal != Signal::None) {
            qCDebug(klaverjasTrick) << player << "potential signal" << signal << "in suit" << suitPlayed;
//             emit playerSignal(player, signal, suitPlayed);
        }
    }
}

const CardSet* Trick::cards() const
{
    return &m_cards;
}

const QVector<int> Trick::players() const
{
    return m_players;
}

/* Return total points for current trick including any bonus points.
 *
 * Bonus combinations and points are:
 * 20 points: K+Q of trump suit
 * 20 points: a run of 3 cards in suit
 * 50 points: a run of 4 cards in suit
 * 100 points: 4 cards of the same rank
 * 200 points: 4 jacks
 */
int Trick::points() const
{
    int total = m_points;
    const auto sets = m_cards.suitSets();
    const auto suitLengthMap = m_cards.cardsPerSuit();
    const int maxLength = *std::max_element(suitLengthMap.constBegin(), suitLengthMap.constEnd());

    if (maxLength == 1) {
        auto iSet = sets.cbegin();
        const Card::Rank rank = iSet.value().first().rank();
        bool sameRank = true;
        while (iSet != sets.cend()) {
            if (iSet.value().first().rank() != rank) {
                sameRank = false;
                break;
            }
            ++iSet;
        }
        if (sameRank)
            total += rank == Card::Rank::Jack ? 200 : 100;
    } else {
        const auto maxRunMap = m_cards.maxRunLengths(s_bonusSortingMap);
        for (auto set = sets.constBegin(); set != sets.constEnd(); ++set) {
            const Card::Suit suit = set.key();
            const int length = maxRunMap[suit];
            if (length >= 2
                && m_cards.contains(Card(m_trumpSuit, Card::Rank::King))
                && m_cards.contains(Card(m_trumpSuit, Card::Rank::Queen))) {
                total += 20;
            }
            if (length == 3)
                total += 20;
            if (length == 4)
                total += 50;
        }
    }
    return total;
}

Card::Suit Trick::suitLed() const
{
    return m_suitLed;
}

void Trick::setWinner(int player, const Card& card)
{
    m_winner = player;
    m_winningCard = card;
}


int Trick::winner() const
{
    return m_winner;
}

const Card* Trick::winningCard() const
{
    return &m_winningCard;
}

QDebug operator<<(QDebug dbg, const Trick& trick)
{
    for (int i = 0; i < trick.m_players.size(); ++i)
        dbg.nospace() << "(" << trick.m_players.at(i) << ": " << trick.m_cards.at(i) << ")";
    return dbg.maybeSpace();
}

