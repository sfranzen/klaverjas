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

#ifndef CARDSET_H
#define CARDSET_H

#include "card.h"

#include <QObject>
#include <QVector>
#include <QMap>

class CardSet : public QVector<Card>
{
    Q_GADGET
    Q_PROPERTY(QVariantList cards READ cards)

public:
    using SortingMap = QMap<Card::Suit,Card::Order>;
    using RunMap = QMap<Card::Suit,QVector<Card::Rank>>;
    enum class SuitOrder : char {
        Alternating,
        TrumpFirst
    };

    CardSet() = default;
    CardSet(QVector<Card> cards);

    QVariantList cards() const;

    // Implement some nice QVector-like methods
    void append(const Card &card);
    void append(const CardSet &set);
    CardSet &operator<<(const Card &card);
    CardSet &operator<<(const CardSet &set);

    // Custom methods
    bool containsSuit(const Card::Suit suit) const;
    const QMap<Card::Suit, QVector<Card>> &suitSets() const;
    QMap<Card::Suit,int> cardsPerSuit(const QVector<Card::Suit> suits = Card::Suits) const;
    RunMap runs(const SortingMap sortingMap) const;
    QMap<Card::Suit,int> maxRunLengths(const SortingMap sortingMap) const;
    int score(Card::Suit trumpSuit) const;

    /// Sort all cards in plain ranks
    void sortAll();
    /// Sort with trump ranks
    void sortAll(SuitOrder suitOrder, Card::Suit trumpSuit);
    void remove(const Card &card);
    void remove(int i, int count);
    void clear();

private:
    QMap<Card::Suit,QVector<Card>> m_suitSets;
    QMap<Card::Suit,int> m_suitCounts;
};

#endif // CARDSET_H
