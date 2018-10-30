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
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(klaverjasTrick)

const Trick::PlayerSignal Trick::NullSignal {Card::Suit::Spades, Signal::None};

namespace {

using Rank = Card::Rank;

}

Trick::Trick(Card::Suit trumpSuit)
    : m_winner(0)
    , m_trumpSuit(trumpSuit)
{
}

const QVector<Card> &Trick::cards() const
{
    return m_cards;
}

Score Trick::score() const
{
    return m_score;
}

Card::Suit Trick::suitLed() const
{
    return m_cards[0].suit();
}

ushort Trick::winner() const
{
    return m_winner;
}

const Card &Trick::winningCard() const
{
    return m_cards[m_winner];
}

bool Trick::isComplete() const
{
    return m_cards.size() == 4;
}

void Trick::add(const Card card)
{
    m_cards << card;
    m_score.points += cardValues(card.suit() == m_trumpSuit)[card.rank()];
    checkWinner();
    if (isComplete())
        checkBonus();
}

/* Each time a new card is played, we check whether it beats the previous card;
 * if so, this player becomes the trick's current winner.
 *
 * The current card beats the previous one either if it follows suit and ranks
 * higher or if it is of a different suit and that suit is the trump suit.
 */
void Trick::checkWinner()
{
    const auto card = m_cards.last();
    if (m_cards.size() == 1) {
        m_winner = 0;
        return;
    }
    const auto suitPlayed = card.suit();
    if (suitPlayed == winningCard().suit()) {
        const auto order = rankOrder(suitPlayed == m_trumpSuit);
        if (card.beats(winningCard(), order))
            m_winner = m_cards.size() - 1;
    } else if (suitPlayed == m_trumpSuit) {
        m_winner = m_cards.size() - 1;
    }
}

/* Bonuses are scored by the following rules:
 *
 * A sequence of three cards of the same suit in the regular card order of
 * numbers, face cards, ace scores 20 points, and four cards scores 50.
 *
 * The King and Queen of the trump suit in the same trick scores 20 points.
 *
 * If all cards in the trick have the same rank, this scores either 100 points
 * or 200 if the rank is Jack.
 */
void Trick::checkBonus()
{
    auto trick = m_cards;
    auto first = trick.begin();
    auto last = trick.end();
    // Group the cards by suit and sort each suit by bonus scoring order
    std::sort(first, last, [](Card &a, Card &b) {
        return a.suit() < b.suit() || BonusOrder[a.rank()] < BonusOrder[b.rank()];
    });
    // Award bonus points for runs
    uint runLength = 1;
    for (auto c = first + 1; c < last; ++c) {
        if (c->suit() == (c-1)->suit() && BonusOrder[c->rank()] - BonusOrder[(c-1)->rank()] == 1) {
            ++runLength;
            if (runLength > 1 && c->suit() == m_trumpSuit &&  c->rank() == Rank::King)
                m_score.bonus += 20;
        } else {
            runLength = 1;
        }
        if (runLength == 3)
            m_score.bonus += 20;
        else if (runLength == 4) // Add 30 to make 50 and keep it simple
            m_score.bonus += 30;
    }
    if (m_score.bonus != 0)
        return;
    // Extra bonus points for the rare case of 4 equal ranks
    const bool sameRank = std::all_of(first + 1, last, [&](const Card& a) { return a.rank() == first->rank(); });
    if (sameRank)
        m_score.bonus += first->rank() == Rank::Jack ? 200 : 100;
}

Trick::PlayerSignal Trick::checkSignal() const
{
    if (m_cards.size() < 3)
        return NullSignal;

    const auto lead = m_cards[0];
    for (ushort p = 2; p < m_cards.size(); ++p) {
        const auto card = m_cards[p];
        if (m_winner == p - 2 &&  card.suit() != lead.suit() && card.suit() != m_trumpSuit) {
            switch(card.rank()) {
            case Rank::Seven: Q_FALLTHROUGH();
            case Rank::Eight: Q_FALLTHROUGH();
            case Rank::Nine:
                return PlayerSignal(card.suit(), Signal::High);
            case Rank::Jack: Q_FALLTHROUGH();
            case Rank::Queen: Q_FALLTHROUGH();
            case Rank::King:
                return PlayerSignal(card.suit(), Signal::Low);
            case Rank::Ace:
                return PlayerSignal(card.suit(), Signal::Long);
            default:
                break;
            };
            break;
        }
    }
    return NullSignal;
}

QDebug operator<<(QDebug dbg, const Trick& trick)
{
    for (int i = 0; i < trick.m_cards.size(); ++i)
        dbg.nospace() << "(" << i << ": " << trick.m_cards[i] << ")";
    return dbg.maybeSpace();
}
