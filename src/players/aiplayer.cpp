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

#include "aiplayer.h"
#include "game.h"
#include "cardset.h"

AiPlayer::AiPlayer(QString name, Game* parent)
    : RandomPlayer(name, parent)
    , m_solver(2500)
{
}

void AiPlayer::selectMove(QVector<Card> legalMoves)
{
    Q_UNUSED(legalMoves)
    Card move = m_solver.treeSearch(m_game);
    emit moveSelected(move);
}
