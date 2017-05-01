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

AiTest::AiTest(QObject* parent, int maxIterations)
    : QObject(parent)
    , m_maxIterations(maxIterations)
{
    // Construct the game with two AI players in the first team
    m_game = new Game(this, true, false);
    for (int i = 0; i < 4; ++i) {
        Player* player = i % 2 ? new AiPlayer("", m_game) : new RandomPlayer("", m_game);
        m_game->addPlayer(player);
    }
    m_game->start();
    m_score = m_game->score();
    connect(m_game, &Game::statusChanged, this, &AiTest::proceed);
}

void AiTest::run()
{

    if (m_iteration < m_maxIterations)
        m_game->advance();
    else {
        result();
        m_iteration = 0;
        for (auto score : m_score)
            score = 0;
        m_game->restart();
    }

}

void AiTest::proceed(Game::Status status)
{
    if (status == Game::Ready) {
        run();
    } else if (status == Game::Finished) {
        m_iteration++;
        auto score = m_game->score();
        for (auto s = score.cbegin(); s != score.cend(); ++s)
            m_score[s.key()] += s.value();
        m_game->restart();
    }
}

void AiTest::result() const
{
    const auto teams = m_score.keys();

    for (auto team : teams) {
        int total = m_score.value(team);
        QString teamName = team == teams.first() ? "Ai" : "Random";
        qCInfo(klaverjasTest) << teamName + " Team:";
        qCInfo(klaverjasTest) << "Total " << total
            << " Game average " << qreal(total) / m_maxIterations
            << " Round average " << qreal(total) / m_maxIterations / 16;
    }
}
