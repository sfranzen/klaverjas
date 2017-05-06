/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2017  Steven Franzen <sfranzen85@gmail.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */

#include "solver.h"
#include "game.h"

#include <QDebug>
#include <QVector>
#include <QtConcurrent>
#include <QFutureSynchronizer>

ISMC::Solver::Solver(int iterMax)
    : m_iterMax(iterMax)
    , m_root()
{
}

Card ISMC::Solver::treeSearch(const Game* rootState)
{
    QFutureSynchronizer<void> sync;
    Node* root = new Node();

    for (int i = 0; i < m_iterMax; ++i) {
        sync.addFuture(QtConcurrent::run(&*this, &Solver::search, root, rootState));
    }
    sync.waitForFinished();

    // Return move of most-visited child node
    auto compareVisits = [](const std::shared_ptr<Node> a, const std::shared_ptr<Node> b){ return a->visits() < b->visits(); };
    auto rootList = root->children();

    const auto mostVisited = *std::max_element(rootList.cbegin(), rootList.cend(), compareVisits);
    return mostVisited->move();
}

void ISMC::Solver::search(Node* rootNode, const Game* rootState)
{
    Node* node = rootNode;

    // Determinize
    Game* state = rootState->cloneAndRandomize(rootState->currentPlayer());

    // Select
    bool selected = false;
    while (!selected) {
        auto legalMoves = state->legalMoves();
        if (!legalMoves.isEmpty() && node->untriedMoves(legalMoves).isEmpty()) {
            node = node->ucbSelectChild( legalMoves);
            state->acceptMove(node->move());
        } else {
            selected = true;
        }
    }

    // Expand
    const auto untriedMoves = node->untriedMoves(state->legalMoves());
    if (!untriedMoves.isEmpty()) {
        Card move = untriedMoves.at(std::rand() % untriedMoves.size());
        int player = state->currentPlayer();
        state->acceptMove(move);
        node = node->addChild(move, player);
    }

    // Simulate
    bool terminal = false;
    while (!terminal) {
        const auto moves = state->legalMoves();
        if (!moves.isEmpty()) {
            Card move = moves.at(std::rand() % moves.size());
            state->acceptMove(move);
        } else {
            terminal = true;
        }
    }

    // Backpropagate
    while (node) {
        node->update(state);
        node = node->parent();
    }
    delete state;
}
