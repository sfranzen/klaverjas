#ifndef RULES_H
#define RULES_H

#include "card.h"

#include <QMap>

/// General game rules and definitions

/// Rules for bidding (electing a trump suit)
enum class BidRule : uchar {
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

/// Rules governing when players are required to play trumps
enum class TrumpRule : uchar {
    /// A player who cannot follow suit is not required to trump if the
    /// winning card in the current trick is his partner's.
    Amsterdams,
    /// A player who cannot follow suit must always (over)trump if he can.
    Rotterdams
};

/// The order of the plain (non-trump) suits
const Card::Order PlainOrder {
    {Card::Rank::Seven, 0},
    {Card::Rank::Eight, 1},
    {Card::Rank::Nine,  2},
    {Card::Rank::Jack,  3},
    {Card::Rank::Queen, 4},
    {Card::Rank::King,  5},
    {Card::Rank::Ten,   6},
    {Card::Rank::Ace,   7}
};

/// The order of the trump suits
const Card::Order TrumpOrder {
    {Card::Rank::Seven, 0},
    {Card::Rank::Eight, 1},
    {Card::Rank::Queen, 2},
    {Card::Rank::King,  3},
    {Card::Rank::Ten,   4},
    {Card::Rank::Ace,   5},
    {Card::Rank::Nine,  6},
    {Card::Rank::Jack,  7}
};

/// The standard card order, used for counting straight runs in tricks
const Card::Order BonusOrder {
    {Card::Rank::Seven, 0},
    {Card::Rank::Eight, 1},
    {Card::Rank::Nine,  2},
    {Card::Rank::Ten,   3},
    {Card::Rank::Jack,  4},
    {Card::Rank::Queen, 5},
    {Card::Rank::King,  6},
    {Card::Rank::Ace,   7}
};

/// Convenience function returning the appropriate order
inline const Card::Order &rankOrder(bool isTrump)
{
    return isTrump ? TrumpOrder : PlainOrder;
}

/// The values of the plain (non-trump) suits
const QMap<Card::Rank,int> PlainValues {
    {Card::Rank::Ace,   11},
    {Card::Rank::Ten,   10},
    {Card::Rank::King,  4},
    {Card::Rank::Queen, 3},
    {Card::Rank::Jack,  2},
    {Card::Rank::Nine,  0},
    {Card::Rank::Eight, 0},
    {Card::Rank::Seven, 0}
};

/// The values of the trump suit
const QMap<Card::Rank,int> TrumpValues {
    {Card::Rank::Jack,  20},
    {Card::Rank::Nine,  14},
    {Card::Rank::Ace,   11},
    {Card::Rank::Ten,   10},
    {Card::Rank::King,  4},
    {Card::Rank::Queen, 3},
    {Card::Rank::Eight, 0},
    {Card::Rank::Seven, 0}
};

/// Convenience function returning the appropriate set of values
inline const QMap<Card::Rank,int> &cardValues(bool isTrump)
{
    return isTrump ? TrumpValues : PlainValues;
}

#endif // RULES_H
