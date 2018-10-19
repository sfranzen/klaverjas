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
#include "card.h"
#include "cardset.h"
#include "trick.h"
#include "gameengine.h"

#include <random>
#include <memory>

#include <QObject>
#include <QVector>
#include <QList>
#include <QMap>
#include <QVariantList>
#include <QQmlListProperty>
#include <QLoggingCategory>

Q_DECLARE_LOGGING_CATEGORY(klaverjasGame)

class GameEngine;
class Player;
class HumanPlayer;
class Team;

class Game : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int round READ round NOTIFY newRound)
    Q_PROPERTY(Card::Suit trumpSuit READ trumpSuit NOTIFY newContract)
    Q_PROPERTY(QQmlListProperty<Player> players READ players NOTIFY playersChanged)
    Q_PROPERTY(HumanPlayer* humanPlayer READ humanPlayer CONSTANT)
    Q_PROPERTY(QQmlListProperty<Team> teams READ teams NOTIFY teamsChanged)

public:
    explicit Game(QObject *parent = 0, int numRounds = 16);

    typedef QMap<Team*,int> Score;

    enum Status { Ready, Waiting, Finished };
    Q_ENUM(Status)

    void addPlayer(Player *player);
    void removePlayer(Player *player);
    int currentPlayer() const;
    int playerIndex(const Player* player) const;
    Player *playerAt(int index) const;
    HumanPlayer *humanPlayer() const;
    QQmlListProperty<Player> players();
    QQmlListProperty<Team> teams();
    const Team *contractors() const;
    const Team *defenders() const;
    int round() const;
    Card::Suit trumpSuit() const;
    const QVector<Card> cardsPlayed() const;
    const Score &score() const;
    Status status() const;
    const Trick& currentTrick() const;
    void start();
    void restart();

signals:
    void playersChanged();
    void scoresChanged();
    void teamsChanged();
    void biddingStarted();
    void bidRequested(QVariantList options, Player *player);
    void newContract(Card::Suit suit, Team *contractors);
    void moveRequested(QVector<Card> legalMoves);
    void cardPlayed(int player, Card card);
    void newTrick();
    void newRound();
    void statusChanged(Status status);

public slots:
    void advance();
    void acceptBid(QVariant bid);
    void acceptMove(Card card);

private slots:

private:
    void deal();
    void proposeBid();
    void setContract(const Card::Suit suit, const Player *player);
    void setStatus(Status newStatus);
    QVariantList initialBidOptions() const;
    void refineBidOptions();

    Player *nextPlayer(Player *player) const;
    void advancePlayer(Player *&player) const;

    TrumpRule m_trumpRule;
    BidRule m_bidRule;
    Card::Suit m_trumpSuit;
    QVector<Card> m_deck;
    bool m_biddingPhase;
    QVariantList m_bidOptions;
    int m_bidCounter;
    int m_numRounds;
    int m_round;
    int m_turn;
    Trick m_currentTrick;
    QVector<QVector<Card>> m_roundCards;
    Score m_scores;
    GameEngine::PlayerList m_players;
    QList<Team*> m_teams;
    Player *m_dealer;
    Player *m_eldest;
    Player *m_currentPlayer;
    Team *m_contractors;
    Team *m_defenders;
    HumanPlayer *m_human;
    Status m_status;

    std::unique_ptr<GameEngine> m_engine;
};

#endif // GAME_H
