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

#ifndef ROUNDSCORE_H
#define ROUNDSCORE_H

#include <QtGlobal>
#include <QObject>
#include <QString>

/**
 * Base score
 *
 * Used to report the result of a single trick.
 */
struct Score
{
    ushort points = 0;
    ushort bonus = 0;

    ushort sum() const
    {
        return points + bonus;
    }

    Score &operator+=(const Score &other)
    {
        points += other.points;
        bonus += other.bonus;
        return *this;
    }

    friend Score operator+(Score s1, const Score &s2)
    {
        return s1 += s2;
    }
};

/**
 * Round score
 *
 * Adds two fields for marking a failed ("wet") contract or a march. Also
 * exposes all properties to QML.
 */
struct RoundScore : public Score
{
    bool wet = false;
    bool march = false;

    void setWet()
    {
        points = 0;
        bonus = 0;
        wet = true;
    }

    operator QString() const
    {
        return QString("Points: %1 + %2, Wet: %3, March: %4").arg(points).arg(bonus).arg(wet).arg(march);
    }

    Q_GADGET
    Q_PROPERTY(ushort   points  MEMBER points CONSTANT)
    Q_PROPERTY(ushort   bonus   MEMBER bonus  CONSTANT)
    Q_PROPERTY(bool     wet     MEMBER wet    CONSTANT)
    Q_PROPERTY(bool     march   MEMBER march  CONSTANT)
};

#endif // ROUNDSCORE_H
