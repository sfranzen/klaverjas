#ifndef RULES_H
#define RULES_H

#include "card.h"

#include <QObject>
#include <QMap>
#include <QVector>


// Game rules and definitions
/// Rules for bidding (electing a trump suit)
enum class BidRule {
    /// Each player may elect a suit or pass. If all pass, first player must
    /// elect.
    Official,
    /// Pick a random suit followed by bidding. If all pass, first player must
    /// player must elect from the remaining suits.
    Random,
    /// Like Random, but if all players pass, the trump suit will be picked at
    /// random from all suits.
    Twents,
    /// No bidding; first player must elect a trump suit right away.
    Utrechts
};

// The ranks and values of the playing cards
const QVector<Card::Rank> PlainRanks {
    Card::Rank::Ace,
    Card::Rank::Ten,
    Card::Rank::King,
    Card::Rank::Queen,
    Card::Rank::Jack,
    Card::Rank::Nine,
    Card::Rank::Eight,
    Card::Rank::Seven
};

const QVector<Card::Rank> TrumpRanks {
    Card::Rank::Jack,
    Card::Rank::Nine,
    Card::Rank::Ace,
    Card::Rank::Ten,
    Card::Rank::King,
    Card::Rank::Queen,
    Card::Rank::Eight,
    Card::Rank::Seven
};

const QVector<Card::Rank> BonusRanks {
    Card::Rank::Ace,
    Card::Rank::King,
    Card::Rank::Queen,
    Card::Rank::Jack,
    Card::Rank::Ten,
    Card::Rank::Nine,
    Card::Rank::Eight,
    Card::Rank::Seven
};

const QMap<Card::Rank,int> PlainValues {
    {Card::Rank::Ace, 11},
    {Card::Rank::Ten, 10},
    {Card::Rank::King, 4},
    {Card::Rank::Queen, 3},
    {Card::Rank::Jack, 2},
    {Card::Rank::Nine, 0},
    {Card::Rank::Eight, 0},
    {Card::Rank::Seven, 0}
};

const QMap<Card::Rank,int> TrumpValues {
    {Card::Rank::Jack, 20},
    {Card::Rank::Nine, 14},
    {Card::Rank::Ace, 11},
    {Card::Rank::Ten, 10},
    {Card::Rank::King, 4},
    {Card::Rank::Queen, 3},
    {Card::Rank::Eight, 0},
    {Card::Rank::Seven, 0}
};

/*! Returns the difference between r1 and r2 in the given sorting order.
 *
 * The sorting order must specify the ranks from high to low. The result is
 * positive if r1 ranks above r2.
 */
int rankDifference(const Card::Rank r1, const Card::Rank r2, const QVector<Card::Rank> order);

#endif // RULES_H
