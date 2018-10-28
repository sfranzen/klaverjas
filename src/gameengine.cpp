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

const GameEngine::ConstraintSet DefaultConstraints {
    {Suit::Clubs,       Rank::Ace},
    {Suit::Diamonds,    Rank::Ace},
    {Suit::Hearts,      Rank::Ace},
    {Suit::Spades,      Rank::Ace}
};

inline ushort team(GameEngine::Position position)
{
    return ushort(position) % 2;
}

inline QVector<Card> higherCards(const QVector<Card> cards, Card toBeat, const Card::Order order)
{
    QVector<Card> result;
    result.reserve(cards.size());
    for (const auto &c : cards)
        if (c.suit() == toBeat.suit() && order[c.rank()] >= order[toBeat.rank()])
            result << c;
    return result;
}

template<typename T> inline GameEngine::Position operator+(GameEngine::Position p, T t)
{
    return p += t;
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
    , m_tricks {{trumpSuit}}
    , m_scores(2)
    , m_trumpSuit(trumpSuit)
    , m_trumpRule(trumpRule)
    , m_currentPlayer(firstPlayer)
    , m_contractor(contractor)
    , m_isMarch(true)
{
    m_tricks.reserve(8);
    setDefaultConstraints();

}

void GameEngine::setDefaultConstraints()
{
    m_playerConstraints.clear();
    for (uint p = 0; p < 4; ++p) {
        m_playerConstraints.insert(p, DefaultConstraints);
        m_playerConstraints[p][m_trumpSuit] = Rank::Jack;
    }
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
 */
void GameEngine::determiniseCards(uint observer) const
{
    QVector<Card> unknowns;
    const auto oPlayer = m_players[observer].get();
    unknowns.reserve(24);
    for (const auto &p : m_players)
        if (p.get() != oPlayer)
            unknowns << p->hand();
    std::random_shuffle(unknowns.begin(), unknowns.end());
    for (uint p = 0; p < 4; ++p) {
        if (p == observer)
            continue;
        const auto player = m_players.at(p).get();
        const auto constraints = m_playerConstraints.value(p);
        QVector<Card> newHand;
        for (auto i = 0; newHand.size() <= player->hand().size() && i < unknowns.size(); ++i)
            if (takeCard(unknowns.at(i), constraints))
                newHand << unknowns.takeAt(i);
        player->setHand(newHand);
    }
}

inline bool GameEngine::takeCard(Card card, const GameEngine::ConstraintSet constraints) const
{
    const auto order = rankOrder(card.suit() == m_trumpSuit);
    return constraints.contains(card.suit()) && order[card.rank()] <= order[constraints[card.suit()]];
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
    const auto currentPos = currentTrick().cards().size();
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
        setConstraint(currentPlayer(), m_trumpSuit, minRank[0].rank());
    }
    Q_ASSERT(!moves.isEmpty());
    return moves;
}

bool GameEngine::isFinished() const
{
    return m_tricks.size() == 8 && currentTrick().isComplete();
}

QVector<Card> GameEngine::minimumRank(const CardSet& hand, uint position) const
{
    QVector<Card> minRank;
    // Allow all moves if current player is first in trick
    if (position == 0 || hand.size() == 1)
        return minRank;

    // Player must always follow suit if possible, otherwise the rules and
    // state of the game determine whether he must trump if possible
    const auto leadingCard = currentTrick().cards().first();
    if (hand.containsSuit(leadingCard.suit())) {
        // Player can follow suit
        if (leadingCard.suit() != m_trumpSuit)
            minRank << Card(leadingCard.suit(), Rank::Seven);
        else { // Player must beat the highest trump played in this trick
            minRank << currentTrick().winningCard();
        }
    } else {
        removeConstraint(currentPlayer(), leadingCard.suit());
        if (hand.containsSuit(m_trumpSuit)) {
            // Player cannot follow suit but has trumps; he must generally (over-)
            // trump, but is exempt from this under Amsterdam rules if his partner
            // is the current winner of the trick
            const auto winner = currentTrick().winner();
            if (m_trumpRule != TrumpRule::Amsterdams || position - winner != 2) {
                const auto &wCard = currentTrick().winningCard();
                const auto rank = wCard.suit() == m_trumpSuit ? wCard.rank() : Rank::Seven;
                minRank << Card(m_trumpSuit, rank);
            }
        }
    }
    return minRank;
}

void GameEngine::doMove(const Card move)
{
    if (isFinished())
        return;

    currentTrick().add(move);
    m_players.at(currentPlayer())->removeCard(move);
    ++m_currentPlayer;
    if (currentTrick().isComplete())
        finishTrick();
}

// Score is kept in the first element of m_score for the first player's team
// and the last element for the second player's team, in the order of m_players
qreal GameEngine::getResult(uint player) const
{
    return isFinished() ? m_scores[player % 2].sum() / 162.0 : -1;
}

inline RoundScore &GameEngine::teamScore(Position position)
{
    return m_scores[team(position)];
}

void GameEngine::finishTrick()
{
    const auto winner = m_currentPlayer + currentTrick().winner();
    teamScore(winner) += currentTrick().score();
    m_currentPlayer = winner;
    // A "march" is scored if the contracting team wins all tricks and gives an
    // extra 100 points
    if (m_isMarch && team(winner) != team(m_contractor))
        m_isMarch = false;

    if (m_tricks.size() < 8) {
        // New trick
        m_tricks << Trick(m_trumpSuit);
    } else {
        // Game complete
        teamScore(winner).points += 10;
        finishGame();
    }
}

void GameEngine::finishGame()
{
    auto &contractorScore = teamScore(m_contractor);
    auto &defenderScore = teamScore(m_contractor + 1);
    if (contractorScore.sum() <= defenderScore.sum()) {
        defenderScore += contractorScore;
        contractorScore.setWet();
    } else if (m_isMarch) {
        contractorScore.march = true;
        contractorScore.points += 100;
    }
}

void GameEngine::reset(Position firstPlayer, Position contractor, Card::Suit trumpSuit)
{
    if (!isFinished())
        return;
    m_currentPlayer = firstPlayer;
    m_contractor = contractor;
    m_scores.fill(RoundScore());
    m_trumpSuit = trumpSuit;
    m_tricks = {{trumpSuit}};
    setDefaultConstraints();
}

const QVector<Card> GameEngine::cardsPlayed() const
{
    QVector<Card> cards;
    for (const auto &t : m_tricks)
        cards << t.cards();
    return cards;
}

const QVector<RoundScore> GameEngine::scores() const
{
    return m_scores;
}

void GameEngine::setConstraint(uint player, Card::Suit suit, Card::Rank rank) const
{
    m_playerConstraints[player][suit] = rank;
}

void GameEngine::removeConstraint(uint player, Card::Suit suit) const
{
    m_playerConstraints[player].remove(suit);
}

Trick &GameEngine::currentTrick()
{
    return m_tricks.last();
}

const Trick &GameEngine::currentTrick() const
{
    return m_tricks.last();
}

GameEngine::Position &operator++(GameEngine::Position& p)
{
    return p += 1;
}
