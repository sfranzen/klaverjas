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

#ifndef BASEPLAYER_H
#define BASEPLAYER_H

#include "card.h"
#include "cardset.h"

/**
 * Player class with minimum required state for the base game.
 */
class BasePlayer
{
public:
    virtual ~BasePlayer() = default;

    const CardSet &hand() const { return m_hand; }
    virtual void setHand(const CardSet &cards) { m_hand = cards; }
    virtual void removeCard(Card card) { m_hand.remove(card); }

protected:
    CardSet m_hand;
};

Q_DECLARE_TYPEINFO(BasePlayer, Q_MOVABLE_TYPE);

#endif // BASEPLAYER_H
