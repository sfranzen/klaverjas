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

#include "aitest.h"
#include "players/aiplayer.h"
#include "players/randomplayer.h"
#include "team.h"

#include <QString>

AiTest::AiTest(QObject* parent, int numRounds)
    : QObject(parent)
    , m_numRounds(numRounds)
{
    // Construct the game with two AI players in the first team
    m_game = new Game(this, true, false, numRounds);
    for (int i = 0; i < 4; ++i) {
        Player* player = i % 2 ? new RandomPlayer("", m_game) : new AiPlayer("", m_game);
        m_game->addPlayer(player);
    }
    m_score = m_game->score();
    connect(m_game, &Game::statusChanged, this, &AiTest::proceed);
    m_game->start();
}

void AiTest::run()
{
        m_game->advance();
}

void AiTest::proceed(Game::Status status)
{
    if (status == Game::Ready) {
        run();
    } else if (status == Game::Finished) {
        disconnect(m_game, &Game::statusChanged, this, &AiTest::proceed);
        m_score = m_game->score();
        showResult();
    }
}

void AiTest::showResult() const
{
    const auto teams = m_score.keys();

    for (auto team : teams) {
        int total = m_score.value(team);
        QString teamName = team == teams.first() ? "Ai" : "Random";
        qCInfo(klaverjasTest) << teamName + " Team:";
        qCInfo(klaverjasTest) << "Total " << total
            << " Round average " << qreal(total) / m_numRounds;
    }
}
