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
#include <QVector>
#include <QMap>
#include <QString>
#include <QVariant>

namespace {

// Unicode points of textual symbols
const static QMap<Card::Suit,QString> SuitLabels {
    {Card::Suit::Spades,    "\u2660"},
    {Card::Suit::Hearts,    "\u2665"},
    {Card::Suit::Diamonds,  "\u2666"},
    {Card::Suit::Clubs,     "\u2663"}
};

const QMap<Card::Rank,QString> RankLabels {
    {Card::Rank::Seven, "7"},
    {Card::Rank::Eight, "8"},
    {Card::Rank::Nine,  "9"},
    {Card::Rank::Ten,   "10"},
    {Card::Rank::Jack,  "J"},
    {Card::Rank::Queen, "Q"},
    {Card::Rank::King,  "K"},
    {Card::Rank::Ace,   "A"}
};

} // namespace

// Collections for ease of use
const QVector<Card::Suit> Card::Suits = SuitLabels.keys().toVector();
const QVector<Card::Rank> Card::Ranks = RankLabels.keys().toVector();

Card::Card(Suit s, Rank r)
    : m_value(uchar(s) | uchar(r))
{
}

QString Card::name() const
{
    return SuitLabels[suit()] + RankLabels[rank()];
}

Card::Suit Card::suit() const
{
    return Suit(m_value & ~15);
}

QVariant Card::suitVariant() const
{
    return QVariant::fromValue(suit());
}

Card::Rank Card::rank() const
{
    return Rank(m_value & 15);
}

QVariant Card::rankVariant() const
{
    return QVariant::fromValue(rank());
}

bool Card::beats(const Card& other, const Order order) const
{
    return suit() == other.suit() && order[rank()] > order[other.rank()];
}

bool Card::operator==(const Card& other) const
{
    return m_value == other.m_value;
}

QDebug operator<<(QDebug dbg, const Card& c)
{
    return dbg << c.name();
}

QDebug operator<<(QDebug dbg, const Card::Suit s)
{
    return dbg << SuitLabels[s];
}

QDebug operator<<(QDebug dbg, const Card::Rank r)
{
    return dbg << RankLabels[r];
}
