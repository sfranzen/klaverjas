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
#include "card.h"
#include "cardset.h"
#include "game.h"

#include <QObject>
#include <QVector>
#include <QMap>

class Team;

class Player
{
    Q_GADGET
    Q_PROPERTY(QString name READ name CONSTANT)

public:
    Player(QString name = "", Game* game = nullptr);
    virtual ~Player() = default;

    virtual Game::Bid selectBid(QVariantList options);

    const QString& name() const;
    virtual void setName(const QString& name);

    CardSet hand();
    virtual void setHand(CardSet cards);

    Team* team() const;
    virtual void setTeam(Team* team);
    bool canBeat(const Card& card, const QVector<Card::Rank> order) const;

    virtual void removeCard(const Card& card);

protected:
    QString m_name;
    Game* m_game;
    CardSet m_hand;
    Team* m_team;

private:
    QMap<Card::Suit,int> handStrength(const QVector<Card::Suit> bidOptions) const;
};

QDebug operator<<(QDebug dbg, const Player* player);

Q_DECLARE_METATYPE(Player)

#endif // PLAYER_H
