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

#ifndef NODE_H
#define NODE_H

#include "card.h"

#include <QLinkedList>
#include <QVector>

class Player;
class Game;

/* For a game to be playable by ISMCTS, only one data field and four functions
 * need to be defined:
 *
 * - playerToMove: The player making a move from this state.
 * - CloneAndRandomize(observer): Create a copy of the state, then determinize
 *      the hidden information from observer's point of view.
 * - GetMoves(): Get a list of the legal moves from this state, or return an
 *      empty list if the state is terminal.
 * - DoMove(move): Apply the given move to the state, and update playerToMove
 *      to specify whose turn is next.
 * - GetResult(player): If the state is terminal, return 1 if the given player
 *      has won or 0 if not. For other games, this function could return 0.5
 *      for a draw, or some other score between 0 and 1 for other outcomes. If
 *      the state is not terminal, the result is undefined.
 */
class Node
{
public:
    Node(const Card& move, Node* parent = nullptr, int playerJustMoved = -1);

    const Card& move() const;
    Node* parent() const;
    QLinkedList<Node*> children() const;
    int visits() const;

    Node* addChild(const Card& move, int playerJustMoved);
    void update(Game* terminalState);
    QVector<Card> untriedMoves(const QVector<Card> legalMoves) const;
    Node* ucbSelectChild(const QVector<Card> legalMoves, qreal exploration = 0.7);

private:
    qreal ucbScore(qreal exploration) const;

    Node* m_parent;
    QLinkedList<Node*> m_children;

    int m_score;
    int m_visits;
    int m_available;

    Card m_move;
    int m_playerJustMoved;
};

#endif // NODE_H
