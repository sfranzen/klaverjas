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

#include <QVector>
#include <QMap>

#include "card.h"

class CardSet : public QVector<Card>
{
public:
    CardSet() = default;
    CardSet(QVector<Card> cards);

    typedef QMap<Suit,QVector<Rank>> SortingMap;

    // Implement some nice QVector-like methods
    void append(const Card& card);
    void append(const CardSet& set);
    CardSet& operator<<(const Card& card);
    CardSet& operator<<(const CardSet& set);
    Card takeAt(int i);

    // Custom methods
    bool containsSuit(const Suit suit) const;
    const QMap<Suit,QVector<Card>> suitSets() const;
    QMap<Suit,int> cardsPerSuit(const QVector<Suit> suits = Card::Suits) const;
    QMap<Suit,int> runLengths(const SortingMap sortingMap) const;
    QMap<Suit,int> maxRunLengths(const SortingMap sortingMap) const;
    int score(Suit trumpSuit) const;
    void shuffle();
    void sort(const QVector<Rank> order, Suit trumpSuit, bool trumpFirst = true);
    void sortAll(const SortingMap sortingMap, const QVector<Suit> suitOrder = Card::Suits);

private:
    void suitSort(const QVector<Rank> order);

    QMap<Suit,QVector<Card>> m_suitSets;
    QMap<Suit,int> m_suitCounts;

};

#endif // CARDSET_H
