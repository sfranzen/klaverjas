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

#ifndef SOLVER_H
#define SOLVER_H

#include "node.h"
#include "card.h"

#include <QMutex>

class Game;

namespace ISMC
{
    class Solver
    {
    public:
        Solver(int iterMax = 10000);

    public:
        Card treeSearch(const Game* rootState);

    private:
        void search(Node* rootNode, const Game* rootState);

        Node m_root;
        int m_iterMax;
    };
};

#endif // SOLVER_H
