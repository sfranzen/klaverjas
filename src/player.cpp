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

#include "player.h"
#include "game.h"

#include <QQueue>
#include <QTimer>
#include <QDebug>
#include <QLoggingCategory>
#include <numeric>

Q_DECLARE_LOGGING_CATEGORY(klaverjasPlayer)

Player::Player(QString name, QObject* parent)
    : QObject(parent)
    , m_name(name)
    , m_team(nullptr)
{
    connect(this, &Player::cardPlayed, &Player::removeCard);
}

const QString& Player::name() const
{
    return m_name;
}

void Player::setName(const QString& name)
{
    m_name = name;
}

const CardSet& Player::hand() const
{
    return m_hand;
}

void Player::setHand(CardSet cards)
{
    m_hand = cards;
    emit handChanged();
}

void Player::setTeam(Team* team)
{
    m_team = team;
}

Team* Player::team() const
{
    return m_team;
}

void Player::removeCard(Card card)
{
    m_hand.removeOne(card);
    emit handChanged();
}

bool Player::canBeat(const Card& card, const QVector<Card::Rank> order) const
{
    if (!m_hand.containsSuit(card.suit())) {
        return false;
    } else {
        for (const auto myCard : m_hand.suitSets()[card.suit()]) {
            if (myCard.beats(card, order))
                return true;
        }
        return false;
    }
}

QDebug operator<<(QDebug dbg, const Player* player) {
    return dbg << player->name();
};
