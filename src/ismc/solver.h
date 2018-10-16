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

#ifndef ISMC_SOLVER_H
#define ISMC_SOLVER_H

#include "game.h"
#include "node.h"

#include <QtGlobal>
#include <QtConcurrent>
#include <QFutureSynchronizer>

#include <memory>
#include <algorithm>

namespace ISMC {

/**
 * ISMC Solver class.
 *
 * Solver implements the Information Set Monte Carlo search algorithm. The
 * algorithm performs a tree search from a given root game state, exploring
 * sequences of available moves against random variations of opponents' moves.
 * The resulting data structure is examined to determine the most promising
 * move from this state.
 */
template<class Move> class Solver
{
public:
    /**
     * Solver constructor.
     *
     * @param iterMax Sets the number of simulations performed for each search.
     * @param exploration Sets the algorithm's bias towards unexplored moves.
     *      Must be between 0 (never explore) and 1 (always explore).
     */
    explicit Solver(uint iterMax = 1000, qreal exploration = 0.7)
        : m_iterMax(iterMax)
        , m_exploration(std::min(std::max(exploration, 0.), 1.))
    {}

    /**
     * Search operator.
     *
     * Performs the tree search on multiple threads.
     *
     * @param rootState Pointer to the root state for the search.
     * @return The most promising move after iterMax iterations.
     */
    Move operator()(const Game<Move> *rootState) const
    {
        using NodePtr = typename std::shared_ptr<Node<Move>>;
        QFutureSynchronizer<void> sync;
        auto *root = new Node<Move>();

        for (uint i = 0; i < m_iterMax; ++i) {
//             sync.addFuture(QtConcurrent::run(&*this, &Solver::search, root, rootState));
            search(root, rootState);
        }
//         sync.waitForFinished();

        // Return move of most-visited child node
        auto compareVisits = [](const NodePtr a, const NodePtr b){ return a->visits() < b->visits(); };
        auto rootList = root->children();
        const auto mostVisited = *std::max_element(rootList.cbegin(), rootList.cend(), compareVisits);
        delete root;
        return mostVisited->move();
    }

private:
    const uint m_iterMax;
    const qreal m_exploration;

    /**
     * Information Set Monte Carlo tree search algorithm.
     *
     * The algorithm uses the given starting node and game state to explore a
     * single sequence of moves through to the final state, where information
     * in the initial state hidden from the current player is randomised.
     *
     * The moves selected are random at first, but become shaped by the game
     * rewards as more search iterations are performed and more information is
     * added to the tree.
     */
    void search(Node<Move> *rootNode, const Game<Move> *rootState) const
    {
        auto *node = rootNode;
        auto randomState = rootState->cloneAndRandomise(rootState->currentPlayer());
        auto statePtr = randomState.get();
        select(node, statePtr);
        expand(node, statePtr);
        simulate(statePtr);
        backPropagate(node, statePtr);
    }

    /**
     * Selection stage.
     *
     * Descend the tree until a node is reached that has unexplored moves, or
     * is a terminal node (no more moves available).
     */
    void select(Node<Move> *node, Game<Move> *state) const
    {
        bool selected = false;
        while (!selected) {
            const auto validMoves = state->validMoves();
            selected = validMoves.isEmpty() || !node->untriedMoves(validMoves).isEmpty();
            if (!selected) {
                node->addVirtualLoss();
                node = node->ucbSelectChild(validMoves, m_exploration);
                state->doMove(node->move());
            }
        }
    }

    /**
     * Expansion stage.
     *
     * Choose a random unexplored move, add it to the children of the current
     * node and select this new node.
     */
    static void expand(Node<Move> *node, Game<Move> *state)
    {
        const auto untriedMoves = node->untriedMoves(state->validMoves());
        if (!untriedMoves.isEmpty()) {
            const auto move = untriedMoves[std::rand() % untriedMoves.size()];
            const auto player = state->currentPlayer();
            state->doMove(move);
            node = node->addChild(move, player);
        }
    }

    /**
     * Simulation stage.
     *
     * Continue performing random available moves from this state until the
     * end of the game.
     */
    static void simulate(Game<Move> *state)
    {
        bool terminal = false;
        while (!terminal) {
            const auto moves = state->validMoves();
            terminal = moves.isEmpty();
            if (!terminal) {
                const auto move = moves[std::rand() % moves.size()];
                state->doMove(move);
            }
        }
    }

    /**
     * Backpropagation stage.
     *
     * Update the node statistics using the rewards from the terminal state.
     */
    static void backPropagate(Node<Move> *node, Game<Move> *state)
    {
        node->removeVirtualLoss();
        while (node) {
            node->update(state);
            node = node->parent();
        }
    }
};

} // ISMC

#endif // ISMC_SOLVER_H
