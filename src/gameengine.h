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

#ifndef GAMEENGINE_H
#define GAMEENGINE_H

#include "ismc/game.h"
#include "card.h"
#include "rules.h"

#include <QtGlobal>
#include <QVector>

#include <memory>

class BasePlayer;
class CardSet;

/**
 * Klaverjas game engine.
 *
 * The GameEngine controls the trick-taking rounds of the game, which take
 * place after the players have received their cards and elected a trump suit.
 * The number of such rounds in a game is up to the players, but is typically 8
 * or 16.
 *
 * A round proceeds by calling doMove for each player in turn with their
 * selected move. The GameEngine removes this card from the player's hand and
 * waits for the next move. After each trick, it determines the winner, awards
 * the trick score to their team and makes them the current player.
 *
 * After a full round, it determines whether the contracting team have made
 * their bid and assigns the final scores accordingly.
 *
 * The GameEngine implements the ISMC::Game interface to allow the game to be
 * simulated repeatedly from a given starting state by a Monte Carlo tree
 * search algorithm.
 */
class GameEngine : public ISMC::Game<Card>
{
public:
    using Player = std::shared_ptr<BasePlayer>;
    using PlayerList = QVector<Player>;
    enum class Position : uchar { North = 0, East, South, West };
    enum class TrumpRule : uchar {
        /// A player who cannot follow suit is not required to trump if the
        /// winning card in the current trick is his partner's.
        Amsterdams,
        /// A player who cannot follow suit must always (over)trump if he can.
        Rotterdams
    };
    /**
     * Create a new game with the given settings.
     *
     * @param players The (4) participants, clockwise from North. The players
     *      must hold 8 cards each.
     * @param firstPlayer The first player to move.
     * @param contractor The player who made the trump bid for this game.
     * @param trumpRule The rule for trumping.
     * @param trumpSuit The trump suit.
     * @return A std::unique_ptr pointing to a new GameEngine instance if the
     *      players were in a valid state, otherwise a nullptr.
     */
    static std::unique_ptr<GameEngine> create(const PlayerList players, Position firstPlayer, Position contractor, TrumpRule trumpRule, Card::Suit trumpSuit);
    GameEngine() = delete;

    std::unique_ptr<Game<Card>> cloneAndRandomise(uint observer) const override;
    uint currentPlayer() const override;
    QVector<Card> validMoves() const override;
    void doMove(const Card move) override;
    qreal getResult(uint player) const override;

    /// Whether the game is finished, i.e. all 32 cards have been played
    bool isFinished() const;
    /// Start a game with the same rules and players, but a new trump bid.
    void reset(Position firstPlayer, Position contractor, Card::Suit trumpSuit);

private:
    PlayerList m_players;
    QVector<Card> m_cardsPlayed;
    ushort m_scores[2];
    Card::Suit m_trumpSuit;
    TrumpRule m_trumpRule;
    Position m_currentPlayer;
    Position m_contractor;
    bool m_isMarch;

    // Only BaseGame may construct itself
    GameEngine(const PlayerList players, Position firstPlayer, Position contractor, TrumpRule trumpRule, Card::Suit trumpSuit);
    GameEngine(const GameEngine &other);
    GameEngine &operator=(const GameEngine &other) = default;
    ushort &teamScore(Position position);

    /**
    * Collect the cards held by each player other than the observer and give
    * them back randomly from this stack.
    *
    * @param observer The player observing this game.
    */
    void determiniseCards(uint observer) const;

    /**
    * Find the card rank and suit the player should beat in the current state.
    *
    * @param hand The player's cards.
    * @param position The position (0, 1, 2 or 3) the player is moving from.
    * @return A vector that contains a single card if the player should beat a
    *       given rank and suit, or an empty vector if any move is valid.
    */
    QVector<Card> minimumRank(const CardSet& hand, uint position) const;
};

GameEngine::Position &operator++(GameEngine::Position &p);
template<typename T> inline GameEngine::Position &operator+=(GameEngine::Position &p, T t)
{
    return p = GameEngine::Position((uint(p) + uint(t)) % 4);
}

#endif // GAMEENGINE_H