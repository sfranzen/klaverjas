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
#include <QSet>

#include <algorithm>

namespace {

using Rank = Card::Rank;
using Suit = Card::Suit;
using CIter = QVector<Card>::iterator;

const GameEngine::ConstraintSet DefaultConstraints {
    {Suit::Clubs,       Rank::Ace},
    {Suit::Diamonds,    Rank::Ace},
    {Suit::Hearts,      Rank::Ace},
    {Suit::Spades,      Rank::Ace}
};

const QSet<Rank> FaceRanks { Rank::Jack, Rank::Queen, Rank::King };

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

CIter highestInPlainSuit(QVector<Card> &cards, Suit suit)
{
    CIter top = nullptr;
    for (auto c = cards.begin(), cEnd = cards.end(); c < cEnd; ++c) {
        if (c->suit() != suit)
            continue;
        if (!top || c->beats(*top, PlainOrder))
            top = c;
    }
    return top ? top : cards.end();
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
    , m_playerSignals(4)
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
    for (const auto &p : m_players) {
        m_playerConstraints.insert(p, DefaultConstraints);
        m_playerConstraints[p][m_trumpSuit] = Rank::Jack;
    }
}

// Copy all value members, but allocate a new copy of each player
GameEngine::GameEngine(const GameEngine& other)
{
    *this = other;
    m_players.clear();
    m_playerConstraints.clear();
    for (const auto &p : other.m_players) {
        const auto clone = Player(new BasePlayer(*p));
        m_players << clone;
        m_playerConstraints.insert(clone, other.m_playerConstraints.value(p));
    }
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
    QVector<Player> others;
    QVector<Card> unknowns;
    const auto oPlayer = m_players[observer];
    unknowns.reserve(24);
    for (const auto &player : m_players) {
        if (player != oPlayer) {
            others << player;
            unknowns << player->hand();
        }
    }
    // Now deal out the cards, starting with the most constrained player; "most
    // constrained" meaning having the lowest sum of constraint ranks.
    auto constraintSum = [&](const ConstraintSet &constraints) {
        int sum = 0;
        // Add 1 to each constraint present because the lowest order is 0
        for (const auto &c : constraints)
            sum += 1 + rankOrder(c.first == m_trumpSuit)[c.second];
        return sum;
    };
    std::sort(others.begin(), others.end(), [&](Player p1, Player p2) {
        return constraintSum(m_playerConstraints.value(p1)) < constraintSum(m_playerConstraints.value(p2));
    });
    constrainedDeal(others, unknowns);
}

void GameEngine::constrainedDeal(const GameEngine::PlayerList players, const QVector<Card> cards) const
{
    bool deal = false;
    int dealCounter = 1;
    QVector<Card> newHands(cards.size());
    while (!deal && dealCounter < 1000) {
        newHands.clear();
        QVector<Card> shuffled(cards);
        std::random_shuffle(shuffled.begin(), shuffled.end());
        for (const auto &player : players) {
            auto i = shuffled.size() - 1;
            auto target = newHands.begin();
            const auto end = target + player->hand().size();
            const auto &currentConstraints = m_playerConstraints[player];
            while (i > -1 && target < end) {
                if (takeCard(shuffled.at(i), currentConstraints)) {
                    *target = shuffled.takeAt(i);
                    ++target;
                }
                --i;
            }
        }
        deal = shuffled.isEmpty();
        if (!deal)
            ++dealCounter;
    }
    Q_ASSERT(deal);

    int pos = 0;
    for (auto &player : players) {
        player->setHand(newHands.mid(pos, player->hand().size()));
        pos += player->hand().size();
    }
}

// Take those cards from the unknowns that match the given player's signals
QVector<Card> GameEngine::signalCards(QVector<Card> &unknowns, uint player) const
{
    QVector<Card> cards;
    const auto sigs = m_playerSignals.at(player);
    if (sigs.isEmpty())
        return cards;
    QVector<CIter> iters;
    for (auto s = sigs.begin(); s != sigs.end(); ++s) {
        switch (s.value()) {
        case Trick::Signal::Long:
            iters << std::find(unknowns.begin(), unknowns.end(), Card(s.key(), Rank::Ten));
            iters << std::find_if(unknowns.begin(), unknowns.end(), [&](Card &c){
                return c.suit() == s.key() && FaceRanks.contains(c.rank());
            });
            break;
        case Trick::Signal::High:
            iters << highestInPlainSuit(unknowns, s.key());
            break;
        case Trick::Signal::Low:
            setConstraint(player, s.key(), Rank::King);
            break;
        default:
            break;
        }
    }
    for (const auto &i : iters) {
        if (i < unknowns.end()) {
            cards << *i;
            unknowns.erase(i);
        }
    }
    return cards;
}

inline bool GameEngine::takeCard(Card card, const ConstraintSet &constraints) const
{
    const auto &order = rankOrder(card.suit() == m_trumpSuit);
    return constraints.count(card.suit()) > 0 && order[card.rank()] <= order[constraints.at(card.suit())];
}

uint GameEngine::currentPlayer() const
{
    return uint(m_currentPlayer);
}

QVector<Card> GameEngine::validMoves() const
{
    const auto currentHand = m_players[currentPlayer()]->hand();
    const auto currentPos = currentTrick().cards().size();
    // minimumRank returns empty if all moves are valid
    const auto minRank = minimumRank(currentHand, currentPos);
    if (minRank.isEmpty())
        return currentHand;

    const auto order = rankOrder(minRank[0].suit() == m_trumpSuit);
    auto moves = higherCards(currentHand, minRank[0], order);
    if (moves.isEmpty()) {
        // If there are still no valid moves at this point, it means:
        // a) Trumps were led but the player cannot overtrump;
        // b) A trick was trumped, but the player can neither overtrump nor
        //    follow suit.
        // In case a the player must play a lower trump, in case b he must play
        // a different suit unless, he only has trumps.
        setConstraint(currentPlayer(), m_trumpSuit, minRank[0].rank());
        if (currentTrick().suitLed() == m_trumpSuit)
            moves = higherCards(currentHand, {m_trumpSuit, Rank::Seven}, TrumpOrder);
        else if (currentHand.suitSets().size() == 1 && currentHand.containsSuit(m_trumpSuit)) {
            moves = higherCards(currentHand, {m_trumpSuit, Rank::Seven}, TrumpOrder);
            for (const auto &suit : Card::Suits)
                if (suit != m_trumpSuit)
                    removeConstraint(currentPlayer(), suit);
        } else {
            for (const auto &set : currentHand.suitSets()) {
                if (set.first().suit() != m_trumpSuit)
                    moves << set;
            }
        }
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
    // Allow all moves if the player is in first position or has too few cards
    if (position == 0 || hand.size() < 2)
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
    if (currentTrick().cards().size() > 2) {
        const auto signal = currentTrick().checkSignal();
        const auto suit = std::get<0>(signal);
        if (signal != Trick::NullSignal && !m_playerSignals.value(currentPlayer()).contains(suit))
            m_playerSignals[currentPlayer()].insert(suit, std::get<1>(signal));
    }
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
    m_playerSignals.fill(SignalMap());
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
    m_playerConstraints[m_players[player]][suit] = rank;
}

void GameEngine::removeConstraint(uint player, Card::Suit suit) const
{
    m_playerConstraints[m_players[player]].erase(suit);
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
