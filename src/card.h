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

#ifndef CARD_H
#define CARD_H

#include <QObject>
#include <QVector>
#include <QMap>

class Card
{
    Q_GADGET
    Q_PROPERTY(Card::Suit suit READ suit CONSTANT)
    Q_PROPERTY(Card::Rank rank READ rank CONSTANT)
public:
    enum class Suit { Spades, Hearts, Diamonds, Clubs };
    enum class Rank { Ace, King, Queen, Jack, Ten, Nine, Eight, Seven };
    Q_ENUM(Suit)
    Q_ENUM(Rank)

    Card() = default;
    Card(Suit s, Rank r);

    bool operator==(const Card& other) const;

    QString name() const { return m_name; };
    Suit suit() const { return m_suit; };
    Rank rank() const { return m_rank; };

    /* Whether this card beats the other in the given sorting order.
     */
    bool beats(const Card& other, const QVector<Rank> order) const;

    const static QVector<Suit> Suits;

    friend QDebug operator<<(QDebug dbg, Suit s);
    friend QDebug operator<<(QDebug dbg, Rank r);

private:
    Suit m_suit;
    Rank m_rank;
    QString m_name;

    const static QMap<Suit,QString> s_suitLabels;
    const static QMap<Rank,QString> s_rankLabels;
};

QDebug operator<<(QDebug dbg, const Card& c);
QDebug operator<<(QDebug dbg, Card::Suit s);
QDebug operator<<(QDebug dbg, Card::Rank r);

#endif // CARD_H
