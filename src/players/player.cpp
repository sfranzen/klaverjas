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

#include "player.h"
#include "card.h"
#include "cardset.h"

#include <QVector>
#include <QDebug>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(klaverjasPlayer)

Player::Player(QString name, QObject *parent)
    : QObject(parent)
    , m_name(name)
    , m_team(nullptr)
{
    m_hand.reserve(8);
}

const QString &Player::name() const
{
    return m_name;
}

void Player::setName(const QString &name)
{
    m_name = name;
}

void Player::setHand(const CardSet &cards)
{
    BasePlayer::setHand(cards);
    emit handChanged();
}

void Player::setTeam(Team* team)
{
    m_team = team;
}

Team *Player::team() const
{
    return m_team;
}

bool Player::canBeat(Card card, const Card::Order order) const
{
    if (!m_hand.containsSuit(card.suit())) {
        return false;
    } else {
        auto sets = m_hand.suitSets();
        for (const auto myCard : sets.value(card.suit())) {
            if (myCard.beats(card, order))
                return true;
        }
        return false;
    }
}

void Player::removeCard(Card card)
{
    BasePlayer::removeCard(card);
    emit handChanged();
}

QDebug operator<<(QDebug dbg, const Player* player) {
    return dbg << player->name();
}
