#ifndef RULES_H
#define RULES_H

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
enum class Suit { Spades, Hearts, Diamonds, Clubs };
enum class Rank { Ace, King, Queen, Jack, Ten, Nine, Eight, Seven };

// The ranks and values of the playing cards
const QVector<Rank> PlainRanks {
    Rank::Ace,
    Rank::Ten,
    Rank::King,
    Rank::Queen,
    Rank::Jack,
    Rank::Nine,
    Rank::Eight,
    Rank::Seven
};

const QVector<Rank> TrumpRanks {
    Rank::Jack,
    Rank::Nine,
    Rank::Ace,
    Rank::Ten,
    Rank::King,
    Rank::Queen,
    Rank::Eight,
    Rank::Seven
};

const QVector<Rank> BonusRanks {
    Rank::Ace,
    Rank::King,
    Rank::Queen,
    Rank::Jack,
    Rank::Ten,
    Rank::Nine,
    Rank::Eight,
    Rank::Seven
};

const QMap<Rank,int> PlainValues {
    {Rank::Ace, 11},
    {Rank::Ten, 10},
    {Rank::King, 4},
    {Rank::Queen, 3},
    {Rank::Jack, 2},
    {Rank::Nine, 0},
    {Rank::Eight, 0},
    {Rank::Seven, 0}
};

const QMap<Rank,int> TrumpValues {
    {Rank::Jack, 20},
    {Rank::Nine, 14},
    {Rank::Ace, 11},
    {Rank::Ten, 10},
    {Rank::King, 4},
    {Rank::Queen, 3},
    {Rank::Eight, 0},
    {Rank::Seven, 0}
};

/*! Returns the difference between r1 and r2 in the given sorting order.
 *
 * The sorting order must specify the ranks from high to low. The result is
 * positive if r1 ranks above r2.
 */
static int rankDifference(const Rank r1, const Rank r2, const QVector<Rank> order) {
    return order.indexOf(r2) - order.indexOf(r1);
};

#endif // RULES_H
