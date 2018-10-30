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
#include "rules.h"

#include <QVariantList>
#include <QList>

#include <algorithm>

namespace {

using Suit = Card::Suit;

inline bool isBlack(const Suit s)
{
    return s == Suit::Spades || s == Suit::Clubs;
}

inline bool areSameColour(const Suit s1, const Suit s2)
{
    return isBlack(s1) == isBlack(s2);
}

inline void alternateColourSort(QList<Suit> &suits)
{
    for (auto s = suits.begin() + 1, sEnd = suits.end(); s < sEnd; ++s) {
        if (areSameColour(*(s - 1), *s) && s + 1 != sEnd)
            std::swap(*s, *(s + 1));
    }
}

inline void sort(QVector<Card> &cards, const Card::Order order)
{
    const auto compare = [&](const Card& c1, const Card& c2) { return c1.beats(c2, order); };
    return std::sort(cards.begin(), cards.end(), compare);
}

}

QVariantList CardSet::cards() const
{
    QVariantList list;
    for (const auto &card : *this)
        list << QVariant::fromValue(card);
    return list;
}

CardSet::CardSet(QVector<Card> cards)
{
    for (const auto &c : cards)
        append(c);
}

void CardSet::append(const Card &card)
{
    QVector::append(card);
    m_suitSets[card.suit()] << card;
    ++m_suitCounts[card.suit()];
}

void CardSet::append(const CardSet &set)
{
    for (const Card& c : set)
        append(c);
}

CardSet &CardSet::operator<<(const Card &card)
{
    append(card);
    return *this;
}

CardSet &CardSet::operator<<(const CardSet &set)
{
    append(set);
    return *this;
}

void CardSet::remove(const Card &card)
{
    m_suitSets[card.suit()].removeOne(card);
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
    return m_suitSets.contains(suit) && !m_suitSets.value(suit).isEmpty();
}

const QMap<Card::Suit, QVector<Card>> &CardSet::suitSets() const
{
    return m_suitSets;
}

QMap<Card::Suit, int> CardSet::cardsPerSuit(const QVector<Card::Suit> suits) const
{
    Q_UNUSED(suits)
    return m_suitCounts;
}

/* Compute the run lengths for each suit of cards.
 *
 * The run length is defined as the number of successive high cards. It is an
 * important characteristic in determining the strength of one's hand.
 */
CardSet::RunMap CardSet::runs(const SortingMap sortingMap) const
{
    RunMap runs;
    for (auto set = m_suitSets.begin(); set != m_suitSets.end(); ++set) {
        auto suit = set.key();
        auto cards = set.value();
        const auto order = sortingMap.value(suit);
        sort(cards, order);
        const auto firstRank = cards.first().rank();
        if (order[firstRank] != 7)
            continue;
        runs.insert(suit, {firstRank});
        for (auto c = cards.cbegin() + 1; c < cards.cend() && order[(c - 1)->rank()] - order[c->rank()] == 1; ++c)
            runs[suit].append(c->rank());
    }
    return runs;
}

QMap<Card::Suit,int> CardSet::maxRunLengths(const SortingMap sortingMap) const
{
    QMap<Card::Suit,int> runLengths;

    for (auto set = m_suitSets.begin(); set != m_suitSets.end(); ++set) {
        auto suit = set.key();
        auto cards = set.value();

        auto order = sortingMap[suit];
        sort(cards, order);
        int currentLength = 1;

        for (auto c = cards.cbegin(); c < cards.cend() - 1; ++c) {
            if (order[c->rank()] - order[(c + 1)->rank()] == 1)
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
    for (const Card c : *this)
        score += cardValues(c.suit() == trumpSuit)[c.rank()];
    return score;
}

void CardSet::sortAll()
{
    auto suits = m_suitCounts.keys();
    alternateColourSort(suits);
    QVector<Card> sorted;
    for (const auto &s : suits) {
        auto set = m_suitSets.value(s);
        sort(set, PlainOrder);
        sorted.append(set);
    }
    swap(sorted);
}

void CardSet::sortAll(SuitOrder suitOrder, Suit trumpSuit)
{
    // Prepare the suit order
    auto suits = m_suitCounts.keys();
    if (suitOrder == CardSet::SuitOrder::TrumpFirst) {
        const auto trumpIt = std::find(suits.begin(), suits.end(), trumpSuit);
        if (trumpIt != suits.end())
            std::rotate(suits.begin(), trumpIt, suits.end());
    }
    alternateColourSort(suits);
    QVector<Card> sorted;
    for (const auto &s : suits) {
        auto set = m_suitSets.value(s);
        sort(set, rankOrder(s == trumpSuit));
        sorted.append(set);
    }
    swap(sorted);
}
