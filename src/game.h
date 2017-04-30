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

#include <QObject>
#include <QStringList>
#include <QVector>
#include <QList>
#include <QMap>
#include <QVariantMap>
#include <QQmlListProperty>
#include <QLoggingCategory>
#include <QThread>

Q_DECLARE_LOGGING_CATEGORY(klaverjasGame)

class Player;
class HumanPlayer;
class Team;

class Game : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int round READ round)
    Q_PROPERTY(Card::Suit trumpSuit READ trumpSuit NOTIFY trumpSuitChanged)
    Q_PROPERTY(QQmlListProperty<Player> players READ players NOTIFY playersChanged)
    Q_PROPERTY(HumanPlayer* humanPlayer READ humanPlayer CONSTANT)
    Q_PROPERTY(QQmlListProperty<Team> teams READ teams NOTIFY teamsChanged)
    typedef QMap<Team*,int> Score;

public:
    Game(bool interactive = true, QObject* parent = 0);

    int round() const;
    Card::Suit trumpSuit() const;
    QQmlListProperty<Player> players();
    HumanPlayer* humanPlayer() const;
    QQmlListProperty<Team> teams();
    QVariantMap scores() const;
    int currentPlayer() const;
    int playerIndex(Player* player) const;
    Player* playerAt(int index) const;
    const Team* contractors() const;
    const Team* defenders() const;
    const QVector<Card> cardsPlayed() const;
    const QVector<Card> legalMoves() const;

    // ISMCTS
    // Return a copy of the game's state, but with the information that is
    // hidden from the observer determinised.
    Game* cloneAndRandomize(int observer) const;
    // Return the outcome if the current game has not terminated, otherwise
    // return -1.
    qreal getResult(int playerIndex) const;

    enum Bid { Spades, Hearts, Diamonds, Clubs, Pass };
    Q_ENUM(Bid)

signals:
    void bidRequested(QVariantList options, Player* player);
    void moveRequested(QVector<Card> legalMoves);
    void playersChanged();
    void scoresChanged();
    void teamsChanged();
    void newTrick();
    void cardPlayed(int player, Card card);
    void trumpSuitChanged(Card::Suit newSuit);

public slots:
    void advance();
    void acceptBid(Bid bid);
    void acceptMove(Card card);

private slots:

private:
    void deal();
    void proposeBid();
    void setContract(const Card::Suit suit, const Player* player);
    Score scoreRound(const QVector<Trick> tricks) const;

    Player* nextPlayer(Player* player) const;
    void advancePlayer(Player*& player) const;

    TrumpRule m_trumpRule;
    BidRule m_bidRule;
    Card::Suit m_trumpSuit;
    QVector<Card> m_deck;
    QVector<Card> m_cardsPlayed;
    bool m_interactive;
    bool m_biddingPhase;
    bool m_waiting;
    bool m_paused;
    int m_bidCounter;
    int m_round;
    int m_trick;
    int m_turn;
    Trick m_currentTrick;
    QVector<Trick> m_roundTricks;
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
    HumanPlayer* m_human;
    QThread m_solverThread;

    const static QStringList s_defaultPlayerNames;
};

Q_DECLARE_METATYPE(Game::Bid)

#endif // GAME_H
