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

#ifndef AITEST_H
#define AITEST_H

#include "game.h"
#include "ismc/solver.h"

#include <QObject>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(klaverjasTest)

class Team;

class AiTest : public QObject
{
    Q_OBJECT
public:
    AiTest(QObject* parent = 0, int numRounds = 1000);

    void run();

private slots:
    void proceed(Game::Status status);

private:
    void showResult() const;

    Game* m_game;
    Game::Score m_score;
    int m_numRounds;
};

#endif // AITEST_H
