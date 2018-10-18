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

class Card
{
    Q_GADGET
    Q_PROPERTY(QVariant suit READ suitVariant CONSTANT)
    Q_PROPERTY(QVariant rank READ rankVariant CONSTANT)
public:
    enum class Suit : uchar { Clubs = 0x00, Diamonds = 0x10, Hearts = 0x20, Spades = 0x30};
    enum class Rank : uchar { Seven = 7, Eight, Nine, Ten, King, Queen, Jack, Ace };
    Q_ENUM(Suit)
    Q_ENUM(Rank)
    using Order = QMap<Rank,int>;

    Card() = default;
    Card(Suit s, Rank r);

    QString name() const;
    Suit suit() const;
    Rank rank() const;

    bool operator==(const Card& other) const;

    /* Whether this card beats the other in the given sorting order.
     */
    bool beats(const Card& other, const Order order) const;
    friend uint qHash(const Card& card, uint seed);

    static const QVector<Suit> Suits;
    static const QVector<Rank> Ranks;

private:
    uchar m_value;

    // QML properties
    QVariant suitVariant() const;
    QVariant rankVariant() const;
};

inline uint qHash(const Card& card, uint seed = 0) {
    return qHash(card.m_value, seed);
}

inline uint qHash(Card::Suit suit, uint seed = 0) {
    return qHash(uint(suit), seed);
}

inline uint qHash(Card::Rank rank, uint seed = 0) {
    return qHash(uint(rank), seed);
}

QDebug operator<<(QDebug dbg, const Card& c);
QDebug operator<<(QDebug dbg, const Card::Suit s);
QDebug operator<<(QDebug dbg, const Card::Rank r);

#endif // CARD_H
