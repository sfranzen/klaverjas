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
#include "gameengine.h"

#include <QString>
#include <QVector>

AiPlayer::AiPlayer(const std::unique_ptr<GameEngine> &engine, QString name, QObject *parent)
    : RandomPlayer(name, parent)
    , m_solver(2500)
    , m_engine(engine)
{
}

void AiPlayer::selectMove(const QVector<Card> legalMoves) const
{
    Q_UNUSED(legalMoves)
    Card move = m_solver(m_engine.get());
    emit moveSelected(move);
}
