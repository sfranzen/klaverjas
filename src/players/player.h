/*
 * This file is part of Klaverjas.
 * Copyright (C) 2018  Steven Franzen <sfranzen85@gmail.com>
 *
 * Klaverjas is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Klaverjas is distributed in the hope that it will be useful,
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

#include "baseplayer.h"
#include "game.h"

#include <QObject>
#include <QString>
#include <QVariantList>

class Card;
class CardSet;
class Team;

/**
 * Abstract base class for interactive players.
 *
 * The Player class provides the common QObject interface for players in an
 * interactive game of klaverjas. It leaves the slots selectBid and selectMove
 * to be implemented by the derived player types.
 */
class Player : public QObject, public BasePlayer
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(CardSet hand READ hand NOTIFY handChanged)

public:
    explicit Player(QString name = "", QObject *parent = nullptr);
    virtual ~Player() = default;

    const QString &name() const;
    void setName(const QString &name);

    virtual void setHand(const CardSet &cards) override;

    Team *team() const;
    virtual void setTeam(Team *team);
    bool canBeat(Card card, const QVector<Card::Rank> order) const;

signals:
    void bidSelected(Game::Bid bid);
    void moveSelected(Card card);
    void handChanged();

public slots:
    virtual void selectBid(QVariantList options) = 0;
    virtual void selectMove(const QVector<Card> legalMoves) = 0;
    virtual void removeCard(Card card) override;

protected:
    QString m_name;
    Team *m_team;
};

QDebug operator<<(QDebug dbg, const Player* player);

#endif // PLAYER_H
