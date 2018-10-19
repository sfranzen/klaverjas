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
    CardSet() = default;
    CardSet(QVector<Card> cards);

    QVariantList cards();

    using SortingMap = QMap<Card::Suit,Card::Order>;
    using RunMap = QMap<Card::Suit,QVector<Card::Rank>>;

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
    Card::Rank highestRank(const Card::Suit suit, const Card::Order order) const;

    void shuffle();
    void sortAll(const SortingMap sortingMap, const QVector<Card::Suit> suitOrder = Card::Suits);
    void remove(const Card &card);
    void remove(int i, int count);
    void clear();

    // In-place sort of a single suit vector of cards
    static void sort(QVector<Card> &cards, const Card::Order order);

private:
    void suitSort(const Card::Order order);

    QMap<Card::Suit,QVector<Card>> m_suitSets;
    QMap<Card::Suit,int> m_suitCounts;
};

Q_DECLARE_METATYPE(CardSet)
#endif // CARDSET_H
