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
#include "gameengine.h"

#include <QObject>
#include <QVector>
#include <QMap>
#include <QQmlListProperty>
#include <QVariantList>

#include <random>
#include <memory>
#include <vector>

class Player;
class HumanPlayer;
class Team;

class Game : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int round READ round NOTIFY newRound)
    Q_PROPERTY(Card::Suit trumpSuit READ trumpSuit NOTIFY newContract)
    Q_PROPERTY(QQmlListProperty<Player> players READ players NOTIFY playersChanged)
    Q_PROPERTY(Player* currentPlayer READ currentPlayerPtr)
    Q_PROPERTY(HumanPlayer* humanPlayer READ humanPlayer CONSTANT)
    Q_PROPERTY(QQmlListProperty<Team> teams READ teams NOTIFY teamsChanged)

public:
    enum Status { Ready, Waiting, Finished };
    Q_ENUM(Status)

    explicit Game(QObject *parent = 0, int numRounds = 16);
    virtual ~Game() = default;

    void addPlayer(Player *player);
    void removePlayer(Player *player);
    int currentPlayer() const;
    Player *currentPlayerPtr() const;
    int playerIndex(const Player *player) const;
    Player *playerAt(int index) const;
    HumanPlayer *humanPlayer() const;
    QQmlListProperty<Player> players();
    QQmlListProperty<Team> teams();
    const QVector<Team*> &teams(int) const;
    const Team *contractors() const;
    const Team *defenders() const;
    int round() const;
    Card::Suit trumpSuit() const;
    const QVector<Card> cardsPlayed() const;
    Status status() const;
    const GameEngine *engine() const;
    Q_INVOKABLE void start();
    void restart();

signals:
    void playersChanged();
    void scoresChanged();
    void teamsChanged();
    void biddingStarted();
    void bidRequested(QVariantList options, Player *player);
    void newContract(Card::Suit suit, Team *contractors);
    void moveRequested(std::vector<Card> legalMoves);
    void cardPlayed(int player, Card card);
    void newTrick();
    void newRound();
    void statusChanged(Status status);

public slots:
    void advance();

private slots:
    void acceptBid(QVariant bid);
    void acceptMove(Card card);

private:
    void deal();
    void proposeBid();
    QVariantList initialBidOptions() const;
    void refineBidOptions();
    void setContract(const Card::Suit suit, const Player *player);
    void handleTrick();
    void handleRound();
    void setStatus(Status newStatus);

    Player *nextPlayer(Player *player) const;
    void advancePlayer(Player *&player) const;

    std::unique_ptr<GameEngine> m_engine;
    QVariantList m_bidOptions;
    QVector<Card> m_deck;
    QVector<QVector<Card>> m_roundCards;
    GameEngine::PlayerList m_players;
    QVector<Team*> m_teams;
    Player *m_dealer;
    Player *m_eldest;
    Player *m_currentPlayer;
    Team *m_contractors;
    Team *m_defenders;
    HumanPlayer *m_human;
    int m_bidCounter;
    int m_turn;
    int m_round;
    int m_numRounds;
    TrumpRule m_trumpRule;
    BidRule m_bidRule;
    Card::Suit m_trumpSuit;
    bool m_biddingPhase;
    Status m_status;
};

#endif // GAME_H
