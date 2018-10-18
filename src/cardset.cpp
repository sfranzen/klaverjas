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
    QVector::append(cards);
    for (auto c = cards.cbegin(); c != cards.cend(); ++c) {
        m_suitSets[c->suit()] << *c;
        m_suitCounts[c->suit()]++;
    }
}

void CardSet::append(const Card& card)
{
    QVector::append(card);
    m_suitSets[card.suit()] << card;
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

void CardSet::remove(int i, int count)
{
    QVector::remove(i, count);
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

const QMap<Card::Suit, QVector<Card>>& CardSet::suitSets() const
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
        QVector<Card> cards = set.value();
        runLengths[suit] = 0;

        const auto order = sortingMap.value(suit);
        auto rank = order.constBegin();
        sort(cards, order);

        for (auto card = cards.constBegin(); card != cards.constEnd(); ++card, ++rank) {
            if (card->rank() == rank.key())
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
        QVector<Card> cards = set.value();

        auto order = sortingMap[suit];
        sort(cards, order);
        int currentLength = 1;

        for (auto c = cards.constBegin(); c < cards.constEnd() - 1; ++c) {
            if (order[(c + 1)->rank()] - order[c->rank()] == 1)
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

Card::Rank CardSet::highestRank(const Card::Suit suit, const Card::Order order) const
{
    QVector<Card> suitSet = m_suitSets.value(suit);
    Card::Rank rank;
    switch (suitSet.size()) {
        case 0:
            rank = order.lastKey();
            break;
        default:
            sort(suitSet, order);
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

void CardSet::suitSort(const Card::Order order)
{
    const auto compare = [&] (const Card& card1, const Card& card2) { return card1.beats(card2, order); };
    std::sort(begin(), end(), compare);
}

void CardSet::sort(QVector< Card >& cards, const Card::Order order)
{
    const auto compare = [&order] (const Card& card1, const Card& card2) { return card1.beats(card2, order); };
    return std::sort(cards.begin(), cards.end(), compare);
}
