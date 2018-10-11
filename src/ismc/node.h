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

#ifndef ISMC_NODE_H
#define ISMC_NODE_H

#include "game.h"

#include <QVector>
#include <QMutex>
#include <QMutexLocker>

#include <memory>
#include <math.h>

namespace ISMC {

/**
 * ISMC node class.
 *
 * Nodes are used by ISMC::Solver to create an information tree about a
 * sequence of simulated games.
 */
template<class Move> class Node
{
public:
    explicit Node(const Move move = Move(), Node *parent = nullptr, int playerJustMoved = -1)
        : m_parent(parent)
        , m_score(0)
        , m_visits(0)
        , m_available(1)
        , m_move(move)
        , m_playerJustMoved(playerJustMoved)
    {}

    const Move &move() const { return m_move; }
    Node *parent() const { return m_parent; }
    int visits() const { return m_visits; }
    const QVector<std::shared_ptr<Node>> &children() const { return m_children; }

    Node *addChild(Move move, int player)
    {
        QMutexLocker lock(&m_mutex);
        m_children << std::make_shared<Node>(move, this, player);
        return m_children.last().get();
    }

    void update(const Game<Move> *terminalState)
    {
        QMutexLocker lock(&m_mutex);
        ++m_visits;
        if (m_playerJustMoved != -1)
            m_score += terminalState->getResult(m_playerJustMoved);
    }

    QVector<Move> untriedMoves(const QVector<Move> legalMoves) const
    {
        QVector<Move> tried, untried;
        QMutexLocker lock(&m_mutex);
        for (const auto &node : m_children)
            tried << node->m_move;
        for (const auto &move : legalMoves)
            if (!tried.contains(move))
                untried << move;
        return untried;
    }

    Node *ucbSelectChild(const QVector<Move> legalMoves, qreal exploration = 0.7)
    {
        static auto compare = [=](const Node *a, const Node *b){ return a->ucbScore(exploration) < b->ucbScore(exploration); };

        QMutexLocker lock(&m_mutex);
        QVector<Node*> legalChildren;

        for (const auto &node : m_children) {
            if (legalMoves.contains(node->m_move)) {
                legalChildren.append(&*node);
                Q_ASSERT(&*node == legalChildren.last());
                ++(node->m_available);
            }
        }
        return *std::max_element(legalChildren.cbegin(), legalChildren.cend(), compare);
    }

    void addVirtualLoss()
    {
        QMutexLocker lock(&m_mutex);
        ++m_visits;
    }

    void removeVirtualLoss()
    {
        QMutexLocker lock(&m_mutex);
        --m_visits;
    }

private:
    Node *m_parent;
    QVector<std::shared_ptr<Node>> m_children;

    qreal m_score;
    int m_visits;
    int m_available;

    Move m_move;
    int m_playerJustMoved;
    mutable QMutex m_mutex;

    qreal ucbScore(qreal exploration) const
    {
        QMutexLocker lock(&m_mutex);
        return m_score / qreal(m_visits) + exploration * std::sqrt(std::log(m_available) / m_visits);
    }
};

template<class Move> inline uint qHash(const Node<Move> &key, uint seed = 0) {
    return qHash(key.parent(), seed);
}

} // ISMC

#endif // ISMC_NODE_H
