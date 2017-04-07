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

#ifndef AIPLAYER_H
#define AIPLAYER_H

#include "player.h"
#include "trick.h"

#include <QObject>
#include <QMap>
#include <QVector>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(klaverjasAi)

/** AI Player class.
 *
 * This player takes note of the gameplay and is intended to make decisions
 * similar to a human player.
 */
class AiPlayer : public Player
{
    Q_OBJECT
    typedef QMap<Player*,QMap<Card::Suit,Trick::Signal>> SignalMap;

public:
    AiPlayer(QString name, QObject* parent = 0);

    virtual void setHand(CardSet cards) override;

public slots:
    virtual void onSignal(Player* player, const Trick::Signal signal, const Card::Suit suit) override;

private slots:
    void selectBid(QVariantList options);
    virtual void requestTurn(const QVector<Card> legalMoves);

private:
    QMap<Card::Suit,int> handStrength(const QVector<Card::Suit> bidOptions) const;

    SignalMap m_signals;
};

#endif // AIPLAYER_H
