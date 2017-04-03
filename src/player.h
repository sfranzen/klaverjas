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

#ifndef PLAYER_H
#define PLAYER_H

#include "rules.h"
#include "cardset.h"
#include "trick.h"

#include <QObject>
#include <QVector>
#include <QMap>
#include <QVariantList>

class Team;

class Player : public QObject
{
    Q_OBJECT
    Q_PROPERTY(CardSet hand READ hand NOTIFY handChanged)
    Q_PROPERTY(QString name READ name CONSTANT)

public:
    Player(QString name = "", QObject* parent = 0);

    const QString& name() const;
    void setName(const QString& name);

    const CardSet& hand() const;
    void setHand(CardSet cards);

    Team* team() const;
    void setTeam(Team* team);

    virtual Bid bid(const QVector<Bid> options) const;
    bool canBeat(const Card& card, const QVector<Card::Rank> order) const;

signals:
    void handChanged();
    void cardPlayed(Card card);

public slots:
    virtual void requestTurn(const QVector<Card> legalMoves);

protected:
    CardSet m_hand;
    QString m_name;
    Team* m_team;
};

QDebug operator<<(QDebug dbg, const Player* player);

#endif // PLAYER_H
