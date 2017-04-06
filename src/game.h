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

#ifndef GAME_H
#define GAME_H

#include "rules.h"
#include "cardset.h"
#include "trick.h"

#include <QObject>
#include <QVector>
#include <QList>
#include <QMap>
#include <QQmlListProperty>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(klaverjasGame)

class Player;
class Team;

class Game : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int round READ round)
    Q_PROPERTY(Card::Suit trumpSuit READ trumpSuit NOTIFY trumpSuitChanged)
    Q_PROPERTY(QQmlListProperty<Player> players READ players NOTIFY playersChanged)
    Q_PROPERTY(QQmlListProperty<Team> teams READ teams NOTIFY teamsChanged)
    typedef QMap<Team*,int> Score;

public:
    Game(QObject* parent = 0);

    int round() const;
    Card::Suit trumpSuit() const;
    QQmlListProperty<Player> players();
    QQmlListProperty<Team> teams();
    QVariantMap scores() const;

    enum Bid { Spades, Hearts, Diamonds, Clubs, Pass };
    Q_ENUM(Bid)

signals:
    void playersChanged();
    void scoresChanged();
    void teamsChanged();
    void newTrick();
    void trumpSuitChanged(Card::Suit newSuit);

public slots:
    void advance();

private slots:
    void acceptBid(Bid bid);
    void acceptTurn(Card card);

private:
    void deal();
    void proposeBid();
    void setContract(const Card::Suit suit, const Player* player);
    Score scoreRound(const QVector<Trick> tricks) const;
    const QVector<Card> legalMoves(const Player* player, const Trick& trick) const;

    Player* nextPlayer(Player* player) const;
    void advancePlayer(Player*& player) const;

    TrumpRule m_trumpRule;
    BidRule m_bidRule;
    Card::Suit m_trumpSuit;
    CardSet m_deck;
    bool m_biddingPhase;
    int m_bidRound;
    int m_round;
    int m_trick;
    bool m_awaitingTurn;
    Trick m_currentTrick;
    QVector<QVector<Trick>> m_tricks;
    QVector<Score> m_scores;
    QMap<Team*,QList<QVariant>> m_roundScores;
    QList<Player*> m_players;
    QList<Team*> m_teams;
    Player* m_dealer;
    Player* m_eldest;
    Player* m_currentPlayer;
    Team* m_contractors;
    Team* m_defenders;

    const static QStringList s_defaultPlayerNames;
};

Q_DECLARE_METATYPE(Game::Bid)

#endif // GAME_H
