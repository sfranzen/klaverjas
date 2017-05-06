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

#include "node.h"
#include "game.h"

#include <math.h>
#include <QMutexLocker>

Node::Node(const Card move, Node* parent, int playerJustMoved)
    : m_parent(parent)
    , m_score(0)
    , m_visits(0)
    , m_available(1)
    , m_move(move)
    , m_playerJustMoved(playerJustMoved)
{
}

const Card& Node::move() const
{
    return m_move;
}

Node* Node::parent() const
{
    return m_parent;
}

const QVector<std::shared_ptr<Node>>& Node::children() const
{
    return m_children;
}

int Node::visits() const
{
    return m_visits;
}

Node* Node::addChild(Card move, int player)
{
    QMutexLocker lock(&m_mutex);
    m_children << std::make_shared<Node>(move, this, player);
    return m_children.last().get();
}

void Node::update(Game* terminalState)
{
    QMutexLocker lock(&m_mutex);
    m_visits++;
    if (m_playerJustMoved != -1)
        m_score += terminalState->getResult(m_playerJustMoved);
}

QVector<Card> Node::untriedMoves(const QVector<Card> legalMoves) const
{
    QVector<Card> tried, untried;
    QMutexLocker lock(&m_mutex);
    for (auto node = m_children.cbegin(); node != m_children.cend(); ++node)
        tried << (*node)->m_move;
    for (auto move = legalMoves.cbegin(); move != legalMoves.cend(); ++move)
        if (!tried.contains(*move))
            untried << *move;
    return untried;
}

Node* Node::ucbSelectChild(const QVector<Card> legalMoves, qreal exploration)
{
    static auto compare = [exploration](const Node* a, const Node* b){ return a->ucbScore(exploration) < b->ucbScore(exploration); };

    QMutexLocker lock(&m_mutex);
    QVector<Node*> legalChildren;
    for (auto node = m_children.begin(); node != m_children.end(); ++node) {
        if (legalMoves.contains((*node)->m_move)) {
            legalChildren << node->get();
            (*node)->m_available++;
        }
    }
    return *std::max_element(legalChildren.cbegin(), legalChildren.cend(), compare);
}

qreal Node::ucbScore(qreal exploration) const
{
    QMutexLocker lock(&m_mutex);
    return qreal(m_score)/qreal(m_visits) + exploration * std::sqrt(std::log(m_available) / qreal(m_visits));
}

bool Node::operator==(const Node& other) const
{
    return m_parent == other.m_parent
        && m_move == other.m_move
        && m_playerJustMoved == other.m_playerJustMoved;
}
