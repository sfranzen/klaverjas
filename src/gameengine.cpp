/*
 * This file is part of Klaverjas.
 * Copyright (C) 2018  Steven Franzen <sfranzen85@gmail.com>
 *
 * Klaverjas is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Klaverjas is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "gameengine.h"
#include "players/baseplayer.h"

#include <QMap>

#include <algorithm>

namespace {

using Rank = Card::Rank;
using Suit = Card::Suit;
using CardIter = QVector<Card>::const_iterator;
using RankOrder = const QMap<Rank,int>;

template<typename T> inline GameEngine::Position operator+(GameEngine::Position p, T t)
{
    return p += t;
};

RankOrder PlainOrder {
    {Rank::Seven, 0},
    {Rank::Eight, 1},
    {Rank::Nine,  2},
    {Rank::Jack,  3},
    {Rank::Queen, 4},
    {Rank::King,  5},
    {Rank::Ten,   6},
    {Rank::Ace,   7}
};

RankOrder TrumpOrder {
    {Rank::Seven, 0},
    {Rank::Eight, 1},
    {Rank::Queen, 2},
    {Rank::King,  3},
    {Rank::Ten,   4},
    {Rank::Ace,   5},
    {Rank::Nine,  6},
    {Rank::Jack,  7}
};

RankOrder BonusOrder {
    {Rank::Seven, 0},
    {Rank::Eight, 1},
    {Rank::Nine,  2},
    {Rank::Ten,   3},
    {Rank::Jack,  4},
    {Rank::Queen, 5},
    {Rank::King,  6},
    {Rank::Ace,   7}
};

inline RankOrder &rankOrder(bool isTrump)
{
    return isTrump ? TrumpOrder : PlainOrder;
}

inline ushort team(GameEngine::Position position)
{
    return ushort(position) % 2;
}

CardIter trickWinner(CardIter first, CardIter last, Suit trumpSuit)
{
    if (first == last)
        return first;
    auto winner = first;
    for (auto c = first + 1; c < last; ++c) {
        if (c->suit() == winner->suit()) {
            const auto order = rankOrder(c->suit() == trumpSuit);
            if (order[c->rank()] > order[winner->rank()])
                winner = c;
        } else if (c->suit() == trumpSuit)
            winner = c;
    };
    return winner;
}

/* A trick is scored by the following rules:
 *
 * The face value of each card is added, which depends on the trump suit. Bonus
 * points are scored if either three (20 points) or four (50 points) cards form
 * a consecutive run in one suit in the regular card order of numbers, face
 * cards, ace. If the King and Queen of the trump suit are among these, another
 * 20 points are given.
 *
 * If, on the other hand, all cards have different suits but the same rank,
 * 100 bonus points are awarded, or 200 if the rank is Jack.
 */
uint trickScore(QVector<Card> &trick, Suit trumpSuit)
{
    auto first = trick.begin();
    auto last = trick.end();
    // Group the cards by suit and sort each suit by bonus scoring order
    std::sort(first, last, [](Card &a, Card &b) {
        return a.suit() != b.suit() || BonusOrder[a.rank()] < BonusOrder[b.rank()];
    });
    // Award card values plus bonus points for runs
    uint score = 0;
    uint runLength = 1;
    for (auto c = first; c < last; ++c) {
        score += c->suit() == trumpSuit ? TrumpValues[c->rank()] : PlainValues[c->rank()];
        if (c == first)
            continue;
        if (c->suit() == (c-1)->suit() && BonusOrder[c->rank()] - BonusOrder[(c-1)->rank()] == 1) {
            ++runLength;
            if (runLength > 1 && c->suit() == trumpSuit &&  c->rank() == Rank::King)
                score += 20;
        } else {
            runLength = 1;
        }
        if (runLength == 3)
            score += 20;
        else if (runLength == 4) // Add 30 to make 50 and keep it simple
            score += 30;
    }
    // Extra bonus points for the rare case of 4 equal ranks
    const bool sameRank = std::all_of(first + 1, last, [&](const Card& a) { return a.rank() == first->rank(); });
    if (sameRank)
        score += first->rank() == Rank::Jack ? 200 : 100;
    return score;
}

inline QVector<Card> higherCards(const QVector<Card> cards, Card toBeat, RankOrder order)
{
    QVector<Card> result;
    result.reserve(cards.size());
    for (const auto &c : cards)
        if (c.suit() == toBeat.suit() && order[c.rank()] >= order[toBeat.rank()])
            result << c;
    return result;
}

} // namespace

std::unique_ptr<GameEngine> GameEngine::create(const PlayerList players, Position firstPlayer, Position contractor, TrumpRule trumpRule, Card::Suit trumpSuit)
{
    if (players.size() != 4)
        return nullptr;
    if (std::any_of(players.begin(), players.end(), [](const Player p){ return !p || p->hand().size() != 8; }))
        return nullptr;
    auto game = new GameEngine(players, firstPlayer, contractor, trumpRule, trumpSuit);
    return std::unique_ptr<GameEngine>(game);
}

GameEngine::GameEngine(const PlayerList players, Position firstPlayer, Position contractor, TrumpRule trumpRule, Card::Suit trumpSuit)
    : m_players(players)
    , m_scores{0,0}
    , m_trumpSuit(trumpSuit)
    , m_trumpRule(trumpRule)
    , m_currentPlayer(firstPlayer)
    , m_contractor(contractor)
    , m_isMarch(true)
{
    m_cardsPlayed.reserve(32);
}

// Copy all value members, but allocate a new copy of each player
GameEngine::GameEngine(const GameEngine& other)
{
    *this = other;
    m_players.clear();
    for (const auto &p : other.m_players)
        m_players << Player(new BasePlayer(*p));
}

std::unique_ptr<ISMC::Game<Card>> GameEngine::cloneAndRandomise(uint observer) const
{
    auto clone = new GameEngine(*this);
    clone->determiniseCards(observer);
    return std::unique_ptr<GameEngine>(clone);
}

/* Observer has seen his own cards as well as all cards played up to this
 * point, so collect the other players' hands and randomly deal them the same
 * number of new cards
 *
 * TODO Improve determinisation. As a round progresses, the players can observe
 * information about the others' hands, such as when a player fails to follow
 * suit, fails to overtrump when required or signals a strong or weak hand to
 * his partner. Try to gather and use this information to constrain the cards
 * dealt to the non-observers.
 */
void GameEngine::determiniseCards(uint observer) const
{
    QVector<Card> unknowns;
    const auto oPlayer = m_players[observer];
    unknowns.reserve(32 - m_cardsPlayed.size() - oPlayer->hand().size());
    for (const auto &p : m_players)
        if (p != oPlayer)
            unknowns << p->hand();
    std::random_shuffle(unknowns.begin(), unknowns.end());
    for (const auto &p : m_players) {
        if (p == oPlayer)
            continue;
        const auto cardIndex = unknowns.size() - p->hand().size();
        p->setHand(unknowns.mid(cardIndex));
        unknowns.resize(cardIndex);
    }
}

uint GameEngine::currentPlayer() const
{
    return uint(m_currentPlayer);
}

QVector<Card> GameEngine::validMoves() const
{
    if (isFinished())
        return QVector<Card>();

    const auto currentHand = m_players[currentPlayer()]->hand();
    const auto currentPos = m_cardsPlayed.size() % 4;
    // minimumRank returns empty if all moves are valid
    const auto minRank = minimumRank(currentHand, currentPos);
    if (minRank.isEmpty())
        return currentHand;

    const auto order = rankOrder(minRank[0].suit() == m_trumpSuit);
    auto moves = higherCards(currentHand, minRank[0], order);
    if (moves.isEmpty()) {
        // If there are still no valid moves at this point, it means the player
        // had to beat a trump but cannot; he may then play a lower trump
        moves = higherCards(currentHand, {m_trumpSuit, Rank::Seven}, TrumpOrder);
    }
    Q_ASSERT(!moves.isEmpty());
    return moves;
}

bool GameEngine::isFinished() const
{
    return m_cardsPlayed.size() == 32;
}

QVector<Card> GameEngine::minimumRank(const CardSet& hand, uint position) const
{
    QVector<Card> minRank;
    // Allow all moves if current player is first in trick
    if (position == 0)
        return minRank;

    // Player must always follow suit if possible, otherwise the rules and
    // state of the game determine whether he must trump if possible
    const auto leadingCard = m_cardsPlayed.end() - position;
    if (hand.containsSuit(leadingCard->suit())) {
        // Player can follow suit
        if (leadingCard->suit() != m_trumpSuit)
            minRank << Card(leadingCard->suit(), PlainOrder.firstKey());
        else { // Player must beat the highest trump played in this trick
            auto top = std::max_element(leadingCard, m_cardsPlayed.end(), [&](const Card &c1, const Card &c2) {
                return c2.suit() != m_trumpSuit || TrumpOrder[c1.rank()] < TrumpOrder[c2.rank()];
            });
            minRank << Card(m_trumpSuit, top->rank());
        }
    } else if (hand.containsSuit(m_trumpSuit)) {
        // Player cannot follow suit but has trumps; he must generally (over-)
        // trump, but is exempt from this under Amsterdam rules if his partner
        // is the current winner of the trick
        const auto winner = trickWinner(leadingCard, m_cardsPlayed.end(), m_trumpSuit);
        const auto winnerPos = winner - leadingCard;
        if (m_trumpRule != TrumpRule::Amsterdams || position - winnerPos != 2) {
            const auto rank = winner->suit() == m_trumpSuit ? winner->rank() : TrumpOrder.firstKey();
            minRank << Card(m_trumpSuit, rank);
        }
    }
    return minRank;
}

void GameEngine::doMove(const Card move)
{
    if (isFinished())
        return;

    m_cardsPlayed << move;
    m_players.at(int(m_currentPlayer))->removeCard(move);

    // Update score at end of each trick
    const auto numCards = m_cardsPlayed.size();
    if (numCards % 4 != 0) {
        ++m_currentPlayer;
    } else { // Trick complete
        auto trick = m_cardsPlayed.mid(numCards - 4, 4);
        const auto winnerCard = trickWinner(trick.begin(), trick.end(), m_trumpSuit);
        const auto winner = m_currentPlayer + (winnerCard - trick.cbegin() + 1);
        teamScore(winner) += trickScore(trick, m_trumpSuit);
        m_currentPlayer = winner;
        // A "march" is scored if the contracting team wins all tricks and
        // awards an extra 100 points
        if (m_isMarch && team(winner) != team(m_contractor))
            m_isMarch = false;
        if (numCards == 32) { // Game complete, tally final score
            teamScore(winner) += 10;
            auto &contractorScore = teamScore(m_contractor);
            auto &defenderScore = teamScore(m_contractor + 1);
            if (contractorScore <= defenderScore) {
                defenderScore += contractorScore;
                contractorScore = 0;
            }
            if (m_isMarch)
                contractorScore += 100;
        }
    }
}

inline ushort &GameEngine::teamScore(Position position)
{
    return m_scores[team(position)];
}

// Score is kept in the first element of m_score for the first player's team
// and the last element for the second player's team, in the order of m_players
qreal GameEngine::getResult(uint player) const
{
    return isFinished() ? m_scores[player % 2] / 162.0 : -1;
}

void GameEngine::reset(Position firstPlayer, Position contractor, Card::Suit trumpSuit)
{
    if (!isFinished())
        return;
    m_currentPlayer = firstPlayer;
    m_contractor = contractor;
    m_scores[0] = m_scores[1] = 0;
    m_trumpSuit = trumpSuit;
    m_cardsPlayed.clear();
}

const QVector<Card> GameEngine::cardsPlayed() const
{
    return m_cardsPlayed;
}

GameEngine::Position &operator++(GameEngine::Position& p)
{
    return p += 1;
}

