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

#include "card.h"
#include "rules.h"

#include <QDebug>

// Suit collection for ease of use
const QVector<Card::Suit> Card::Suits {
    Card::Suit::Spades,
    Card::Suit::Hearts,
    Card::Suit::Diamonds,
    Card::Suit::Clubs
};

// Textual labels
const QMap<Card::Suit,QString> Card::s_suitLabels {
    {Card::Suit::Spades, "\u2660"},
    {Card::Suit::Hearts, "\u2665"},
    {Card::Suit::Diamonds, "\u2666"},
    {Card::Suit::Clubs, "\u2663"}
};

const QMap<Card::Rank,QString> Card::s_rankLabels {
    {Card::Rank::Seven, "7"},
    {Card::Rank::Eight, "8"},
    {Card::Rank::Nine, "9"},
    {Card::Rank::Jack, "J"},
    {Card::Rank::Queen, "Q"},
    {Card::Rank::King, "K"},
    {Card::Rank::Ten, "10"},
    {Card::Rank::Ace, "A"}
};

Card::Card(Suit s, Rank r)
    : m_suit(s)
    , m_rank(r)
    , m_name(s_suitLabels[s] + s_rankLabels[r])
{
}

bool Card::beats(const Card& other, const QVector<Rank> order) const
{
    return rankDifference(rank(), other.rank(), order) > 0;
}

bool Card::operator==(const Card& other) const
{
    return m_rank == other.m_rank && m_suit == other.m_suit;
}

QDebug operator<<(QDebug dbg, const Card& c)
{
    return dbg << c.name();
}

QDebug operator<<(QDebug dbg, Card::Suit s)
{
    return dbg << Card::s_suitLabels[s];
}

QDebug operator<<(QDebug dbg, Card::Rank r)
{
    return dbg << Card::s_rankLabels[r];
}
