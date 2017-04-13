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

#ifndef TEAM_H
#define TEAM_H

#include <QObject>
#include <QVector>
#include <QVariantList>

class Player;

class Team : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantList scores READ scores NOTIFY scoreChanged)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)

public:
    Team(QObject* parent = 0) : QObject(parent) {};
    Team(QString name, QObject* parent = 0);

    QString name() const;
    void setName(QString name);

    const QVector<Player*>& players() const;
    void addPlayer(Player* p);
    void removePlayer(Player* p);

    QVariantList scores();
    int score() const;
    void addPoints(const uint points);
    void resetScore();

signals:
    void nameChanged(QString name);
    void scoreChanged(int newScore);

private:
    QString m_name;
    QVector<Player*> m_players;
    QVector<int> m_score;

    friend QDebug operator<<(QDebug dbg, const Team* team);
};

#endif // TEAM_H
