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

#include "randomplayer.h"

#include <QLoggingCategory>
#include <QMap>
#include <QVector>
#include <QVariantList>

Q_DECLARE_LOGGING_CATEGORY(klaverjasAi);

void RandomPlayer::selectMove(QVector<Card> legalMoves) const
{
    const int idx = std::rand() % legalMoves.size();
    emit moveSelected(legalMoves.at(idx));
}

/* Choose a bid from the options presented.
 *
 * The bid options are scored according to the run lengths, which give an
 * indication of how many tricks might be secured by the player.
 */
void RandomPlayer::selectBid(QVariantList options) const
{
    qCDebug(klaverjasAi) << m_name + "'s hand:" << m_hand;
    CardSet::SortingMap map;
    for (const Card::Suit s : Card::Suits)
        map[s] = PlainRanks;

    QVector<Card::Suit> bidOptions;
    for (const auto &b : options) {
        if (!b.isNull())
            bidOptions << b.value<Card::Suit>();
    }
    const QMap<Card::Suit,int> strengthMap = handStrength(bidOptions);

    // If we can pass, we only choose one of the options if, with that suit as
    // trumps, our hand matches one of the following conditions:
    //  * The strengths computed above add up to more than 40 points;
    //  * We have the J and at least three more trump cards;
    // otherwise choose the strongest suit.
    const auto strengthList = strengthMap.values();
    const auto suitCounts = m_hand.cardsPerSuit(strengthMap.keys().toVector());
    QMap<Card::Suit,int> tempCounts;

    auto maxStrength = std::max_element(strengthMap.constBegin(), strengthMap.constEnd());
    QVector<Card::Suit> shortList;

    if (*maxStrength > 40) {
        shortList << maxStrength.key();
        // If the top strength is greater than 40 and is not unique, pick the
        // suit with the most cards
        if (strengthList.count(*maxStrength) > 1) {
            for (auto s = maxStrength + 1; s != strengthMap.constEnd(); ++s) {
                if (*s == *maxStrength)
                    shortList << s.key();
            }

            for (const Card::Suit s : shortList)
                tempCounts[s] = suitCounts.value(s);

            shortList.clear();
            shortList << std::max_element(tempCounts.constBegin(), tempCounts.constEnd()).key();
        }
    } else {
        for (auto count = suitCounts.constBegin(); count != suitCounts.constEnd(); ++count) {
            if (strengthMap[count.key()] >= 20 && *count > 3)
                tempCounts[count.key()] = *count;
        }

        if (!tempCounts.isEmpty())
            shortList << std::max_element(tempCounts.constBegin(), tempCounts.constEnd()).key();
    }

    // Decide
    QVariant choice;
    if (shortList.isEmpty())
        choice = options.last().isNull() ? options.last() : QVariant::fromValue(maxStrength.key());
    else
        choice = QVariant::fromValue(shortList.first());

    emit bidSelected(choice);
}

// The strength is the estimated number of points that could be scored with
// a given trump option.
QMap<Card::Suit,int> RandomPlayer::handStrength(const QVector<Card::Suit> bidOptions) const
{
    QMap<Card::Suit,int> strengthMap;
    CardSet::SortingMap sortingMap;
    for (const Card::Suit option : bidOptions) {
        strengthMap[option] = 0;
        for (const Card::Suit s : Card::Suits) {
            sortingMap[s] = s == option ? TrumpRanks : PlainRanks;
        }
        const auto runLengthMap = m_hand.runLengths(sortingMap);
        for (const Card::Suit s : Card::Suits) {
            auto sortOrder = sortingMap[s].constBegin();
            auto values = s == option ? TrumpValues : PlainValues;
            for (auto i = sortOrder; i < sortOrder + runLengthMap.value(s); ++i)
                strengthMap[option] += values[*i];
        }
        qCDebug(klaverjasAi) << "Suit " << option << " run lengths " << runLengthMap;
    }
    qCDebug(klaverjasAi) << "Strengths" << strengthMap;
    return strengthMap;
}
