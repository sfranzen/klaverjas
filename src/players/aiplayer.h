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

#ifndef AIPLAYER_H
#define AIPLAYER_H

#include "randomplayer.h"
#include "card.h"
#include "ismc/solver.h"

#include <memory>

class Game;
class GameEngine;

class AiPlayer : public RandomPlayer
{
public:
    explicit AiPlayer(const std::unique_ptr<GameEngine> &engine, QString name = "", Game *parent = 0);

public slots:
    void selectMove(const QVector<Card> legalMoves) const override;

private:
    ISMC::Solver<Card> m_solver;
    const std::unique_ptr<GameEngine> &m_engine;
};

#endif // AIPLAYER_H
