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

#include "cardset.h"
#include "card.h"
#include "rules.h"

#include <QQueue>
#include <algorithm>
#include <QDebug>

QVariantList CardSet::cards()
{
    QVariantList list;
    for (Card& card : *this)
        list << QVariant::fromValue(card);
    return list;
}


CardSet::CardSet(QVector<Card> cards)
{
    for (const Card& c : cards)
        append(c);
}

void CardSet::append(const Card& card)
{
    QVector::append(card);
    m_suitSets[card.suit()].append(card);
    ++m_suitCounts[card.suit()];
}

void CardSet::append(const CardSet& set)
{
    for (const Card& c : set)
        append(c);
}

CardSet& CardSet::operator<<(const Card& card)
{
    append(card);
    return *this;
}

CardSet& CardSet::operator<<(const CardSet& set)
{
    append(set);
    return *this;
}

void CardSet::remove(const Card& card)
{
    m_suitSets[card.suit()].removeAll(card);
    --m_suitCounts[card.suit()];
    const bool removed = removeOne(card);
    Q_ASSERT(removed);
}

void CardSet::clear()
{
    m_suitSets.clear();
    m_suitCounts.clear();
    QVector::clear();
}

bool CardSet::containsSuit(const Card::Suit suit) const
{
    return m_suitSets.contains(suit) && !m_suitSets[suit].isEmpty();
}

const QMap<Card::Suit, QVector<Card>> CardSet::suitSets() const
{
    return m_suitSets;
}

QMap<Card::Suit, int> CardSet::cardsPerSuit(const QVector<Card::Suit> suits) const
{
    return m_suitCounts;
}

/* Compute the run lengths for each suit of cards.
 *
 * The run length is defined as the number of successive high cards. It is an
 * important characteristic in determining the strength of one's hand.
 */
QMap<Card::Suit,int> CardSet::runLengths(const SortingMap sortingMap) const
{
    QMap<Card::Suit,int> runLengths;

    for (auto set = m_suitSets.constBegin(); set != m_suitSets.constEnd(); ++set) {
        Card::Suit suit = set.key();
        CardSet cards = set.value();
        runLengths[suit] = 0;

        const auto order = sortingMap.value(suit);
        auto rank = order.constBegin();
        cards.suitSort(order);

        for (auto card = cards.constBegin(); card != cards.constEnd(); ++card, ++rank) {
            if ((*card).rank() == *rank)
                ++runLengths[suit];
            else
                break;
        }
    }
    return runLengths;
}

QMap<Card::Suit,int> CardSet::maxRunLengths(const SortingMap sortingMap) const
{
    QMap<Card::Suit,int> runLengths;

    for (auto set = m_suitSets.constBegin(); set != m_suitSets.constEnd(); ++set) {
        Card::Suit suit = set.key();
        CardSet cards = set.value();

        auto order = sortingMap[suit];
        cards.suitSort(order);
        int currentLength = 1;

        for (auto c = cards.constBegin(); c < cards.constEnd() - 1; ++c) {
            if (rankDifference(c->rank(), (c + 1)->rank(), order) == 1)
                ++currentLength;
            else
                currentLength = 1;
        }
        runLengths[suit] = currentLength;
    }
    return runLengths;
}

int CardSet::score(const Card::Suit trumpSuit) const
{
    int score = 0;
    for (const Card c : *this) {
        if (c.suit() == trumpSuit)
            score += TrumpValues[c.rank()];
        else
            score += PlainValues[c.rank()];
    }
    return score;
}

Card::Rank CardSet::highestRank(const Card::Suit suit, const QVector< Card::Rank > order) const
{
    CardSet suitSet = m_suitSets.value(suit);
    Card::Rank rank;
    switch (suitSet.size()) {
        case 0:
            rank = order.last();
            break;
        default:
            suitSet.suitSort(order);
        case 1:
            rank = suitSet.first().rank();
            break;
    }
    return rank;
}

void CardSet::shuffle()
{
    std::random_shuffle(begin(), end());
}

void CardSet::sort(const QVector<Card::Rank> order, Card::Suit trumpSuit, bool trumpFirst)
{
    CardSet sorted, set;
    QQueue<Card::Suit> suitOrder;

    suitOrder.enqueue(trumpFirst ? trumpSuit : Card::Suits.first());
    for (Card::Suit s : Card::Suits)
        if (s != suitOrder.first())
            suitOrder.enqueue(s);

    while (!suitOrder.isEmpty()) {
        Card::Suit current = suitOrder.dequeue();
        set = m_suitSets.value(current);
        set.suitSort(order);
        sorted.append(set);
    }
    swap(sorted);
}

void CardSet::sortAll(const SortingMap sortingMap, const QVector<Card::Suit> suitOrder)
{
    CardSet sorted, set;
    for (const Card::Suit s : suitOrder) {
        set = m_suitSets.value(s);
        set.suitSort(sortingMap[s]);
        sorted.append(set);
    }
    swap(sorted);
}

void CardSet::suitSort(const QVector<Card::Rank> order)
{
    const auto compare = [&] (const Card& card1, const Card& card2) { return card1.beats(card2, order); };
    std::sort(begin(), end(), compare);
}
