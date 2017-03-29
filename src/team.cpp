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

#include "team.h"
#include "player.h"

#include <QDebug>

Team::Team(QString name, QObject* parent)
    : QObject(parent)
    , m_name(name)
{
}

QString Team::name() const
{
    return m_name;
}

void Team::setName(QString name)
{
    if (m_name == name)
        return;
    m_name = name;
    emit nameChanged(name);
}

const QVector<Player*>& Team::players() const
{
    return m_players;
}

void Team::addPlayer(Player* p)
{
    m_players.append(p);
    p->setTeam(this);
}

void Team::removePlayer(Player* p)
{
    m_players.removeAll(p);
    p->setTeam(nullptr);
}

QVariantList Team::scores()
{
    QVariantList newList;
    foreach(const auto item, m_score)
        newList << item;
    return newList;
}

void Team::addPoints(const uint points)
{
    m_score << points;
    emit scoreChanged(points);
}

void Team::resetScore()
{
    m_score.clear();
    emit scoreChanged(0);
}

QDebug operator<<(QDebug dbg, const Team* team) {
    dbg.noquote() << team->m_name << ": " << team->m_players;
    return dbg.maybeSpace();
}
