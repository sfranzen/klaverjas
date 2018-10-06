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

#ifndef ISMC_GAME_H
#define ISMC_GAME_H

#include <QtGlobal>

#include <memory>

namespace ISMC {

/** Required interface for a card game playable by ISMC AI.
 *
 * For a game to be solvable by ISMCTS, it needs to be able to clone its state
 * and randomise information unknown to a given observer.
 * Furthermore, it must be able to indicate the active player, provide a list
 * of valid moves for this player, as well as any player's score at the end of
 * the game. The game should advance to the next state when given a move from
 * the list of valid moves.
 */
template<class Move> class Game
{
public:
    virtual ~Game() {};
    /**
     * Create randomised copy of game state.
     *
     * Constructs a copy of the game state that has the information unknown to
     * the observer randomised.
     *
     * @param observer The player whose information remains constant.
     * @return The cloned state.
     */
    virtual std::unique_ptr<Game> cloneAndRandomise(uint observer) const = 0;

    /**
     * The player making a move from this state.
     *
     * @return The integer representing the current player.
     */
    virtual uint currentPlayer() const = 0;

    /**
     * Valid moves for the current state.
     *
     * @return A list of valid moves or an empty list if the state is terminal.
     */
    virtual QVector<Move> validMoves() const = 0;

    /**
     * Apply the given move to the state, and update current player to specify
     * whose turn is next.
     *
     * @param move The move to play.
     */
    virtual void doMove(const Move move) = 0;

    /**
     * Return result for the given player.
     *
     * @param player Player for whom to compute result.
     * @return If the state is terminal, return 1 if the given player has won
     * or 0 if not. For other games, this function could return 0.5 for a draw,
     * or some other score between 0 and 1 for other outcomes. If the state is
     * not terminal, the result is undefined.
     */
    virtual qreal getResult(uint player) const = 0;
};

} // ISMC

#endif // ISMC_GAME_H
