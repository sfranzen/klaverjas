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

#ifndef RANDOMPLAYER_H
#define RANDOMPLAYER_H

#include "aiplayer.h"
#include "card.h"

#include <QString>
#include <QVector>
#include <QVariantList>

class RandomPlayer : public AiPlayer
{
public:
    using AiPlayer::AiPlayer;

public slots:
    virtual void selectMove(QVector<Card> legalMoves) override;
};

#endif // RANDOMPLAYER_H