#ifndef RULES_H
#define RULES_H

#include "card.h"

#include <QMap>
#include <QVector>

// Game rules and definitions
enum class TrumpRule {
    //! A player who cannot follow suit is not required to trump his partner's
    //! trick.
    Amsterdams,
    //! A player who cannot follow suit must always (over)trump if he can.
    Rotterdams
};

enum class BidRule {
    Official,
    Random,
    Utrechts,
    Twents
};

enum class Bid { Spades, Hearts, Diamonds, Clubs, Pass };

// Card definitions
// enum class Card::Suit { Spades, Hearts, Diamonds, Clubs };
// enum class Rank { Ace, King, Queen, Jack, Ten, Nine, Eight, Seven };

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
static int rankDifference(const Card::Rank r1, const Card::Rank r2, const QVector<Card::Rank> order) {
    return order.indexOf(r2) - order.indexOf(r1);
};

#endif // RULES_H
