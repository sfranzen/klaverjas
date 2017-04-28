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

#include <QObject>
#include <QVector>
#include <QList>
#include <QMutex>

class Game;

namespace ISMC
{
    class Solver : public QObject
    {
        Q_OBJECT
    public:
        Solver(int iterMax = 10000, QObject* parent = 0);

    public slots:
        Card treeSearch(Game* rootState);

    signals:
        void searchComplete(Card card);

    private:
        void search(Node* rootNode, Game* rootState);

        int m_iterMax;
        Node m_root;
        QMutex m_mutex;
    };
};

#endif // SOLVER_H
