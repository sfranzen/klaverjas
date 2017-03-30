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
#include "player.h"

#include <QDebug>

const CardSet::SortingMap Trick::s_bonusSortingMap {
    {Card::Suit::Spades, BonusRanks},
    {Card::Suit::Hearts, BonusRanks},
    {Card::Suit::Diamonds, BonusRanks},
    {Card::Suit::Clubs, BonusRanks}
};

Trick::Trick(Card::Suit trumpSuit)
    : m_trumpSuit(trumpSuit)
    , m_order(PlainRanks)
    , m_values(PlainValues)
    , m_points(0)
    , m_winner(nullptr)
{
}

/* Each time a new card is played, we check whether it beats the previous card;
 * if so, this player becomes the trick's current winner.
 *
 * The current card beats the previous one either if it follows suit and ranks
 * higher or if it is of a different suit and that suit is the trump suit.
 */
void Trick::add(Player* player, const Card& card)
{
    m_cards << card;
    m_players << player;
    const Card::Suit suitPlayed = card.suit();
    m_points += suitPlayed == m_trumpSuit ? TrumpValues[card.rank()] : PlainValues[card.rank()];
    if (m_players.size() == 1) {
        m_suitLed = suitPlayed;
        setWinner(player, card);
    } else {
        if (suitPlayed == m_winningCard.suit()) {
            auto order = suitPlayed == m_trumpSuit ? TrumpRanks : PlainRanks;
            if (card.beats(m_winningCard, order))
                setWinner(player, card);
        } else if (suitPlayed == m_trumpSuit) {
            setWinner(player, card);
        }
    }
}

const CardSet * Trick::cards() const
{
    return &m_cards;
}

const QVector<Player*> Trick::players() const
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
        const Card::Rank rank = sets.first().first().rank();
        bool sameRank = true;
        for (const auto cards : sets) {
            if (cards.first().rank() != rank) {
                sameRank = false;
                break;
            }
        }
        if (sameRank)
            total += rank == Card::Rank::Jack ? 200 : 100;
    } else {
        const auto maxRunMap = m_cards.maxRunLengths(s_bonusSortingMap);
        for (auto set = sets.constBegin(); set != sets.constEnd(); ++set) {
            const Card::Suit suit = set.key();
            const int length = maxRunMap[suit];
            const QVector<Card>* cards = &set.value();
            if (length >= 2
                && cards->contains(Card(m_trumpSuit, Card::Rank::King))
                && cards->contains(Card(m_trumpSuit, Card::Rank::Queen))) {
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

void Trick::setWinner(Player* player, const Card& card)
{
    m_winner = player;
    m_winningCard = card;
}


Player* Trick::winner() const
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

