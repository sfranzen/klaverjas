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

#include "game.h"
#include "card.h"
#include "player.h"
#include "aiplayer.h"
#include "team.h"

#include <algorithm>
#include <QTime>
#include <QTimer>

const QStringList Game::s_defaultPlayerNames {"South", "West", "North", "East"};

Game::Game(QObject* parent)
    : QObject(parent)
    , m_trumpRule(TrumpRule::Amsterdams)
    , m_bidRule(BidRule::Random)
    , m_biddingPhase(true)
    , m_bidRound(0)
    , m_round(0)
    , m_trick(0)
    , m_awaitingTurn(false)
    , m_contractors(nullptr)
    , m_defenders(nullptr)
{
    for (int  i = 0; i < 32; ++i)
        m_deck.append(Card(Card::Suit(i/8), Card::Rank(i%8)));

    for (int i = 1; i <= 2; ++i)
        m_teams.append(new Team(QString::number(i), this));

    for (int p = 0, t = 0; p < s_defaultPlayerNames.size(); ++p, t = p % 2) {
        Player* newPlayer = p == 0 ? new Player("You", this) : new AiPlayer(s_defaultPlayerNames[p], this);
        m_teams[t]->addPlayer(newPlayer);
        m_players.append(newPlayer);
    }
    emit teamsChanged();

    qCDebug(klaverjasGame) << "Teams: " << m_teams;

    m_dealer = m_players.last();
    m_eldest = nextPlayer(m_dealer);

    // Initialise PRNG
    std::srand(QTime::currentTime().msec());
}

int Game::round() const
{
    return m_round;
}

QVariantMap Game::scores() const
{
    QVariantMap result;
    for (const auto team : m_teams)
        result[team->name()] = m_roundScores[team];
    return result;
}

QQmlListProperty<Player> Game::players()
{
    return QQmlListProperty<Player>(this, m_players);
}


QQmlListProperty<Team> Game::teams()
{
    return QQmlListProperty<Team>(this, m_teams);
}

void Game::advance()
{
    static QVector<Trick> roundTricks;
    static bool dealCards = true;
    static int turn = 0;
    if (m_awaitingTurn)
        return;
    if (m_round == 16) {
        qCDebug(klaverjasGame) << "Game finished";
        return;
    }
    qCDebug(klaverjasGame) << "Proceeding";
    if (dealCards) {
        deal();
        dealCards = false;
    }
    if (m_trick == 0 && turn == 0) {
        m_currentPlayer = m_eldest;
    }
    if (m_biddingPhase) {
        proposeBid();
        return;
    }
    if (m_trick < 8) {
        if (turn == 0) {
            m_currentTrick = Trick(m_trumpSuit);
            emit newTrick();
        }
        if (turn < 4) {
            auto legalMoves = this->legalMoves(m_currentPlayer, m_currentTrick);
            m_awaitingTurn = true;
            connect(m_currentPlayer, &Player::cardPlayed, this, &Game::acceptTurn);
            emit m_currentPlayer->playRequested(legalMoves);
            ++turn;
        } else {
            turn = 0;
            ++m_trick;
            roundTricks << m_currentTrick;
            m_currentPlayer = m_currentTrick.winner();
            qCDebug(klaverjasGame) << "Current trick:" << m_currentTrick;
            qCDebug(klaverjasGame) << "Trick winner:" << m_currentPlayer << "Points:" << m_currentTrick.points();
        }
    } else {
        m_tricks << roundTricks;
        const auto score = scoreRound(roundTricks);
        qCDebug(klaverjasGame) << "Round scores: " << score;
        for (Team* team : m_teams) {
            team->addPoints(score[team]);
        }
        m_trick = 0;
        turn = 0;
        ++m_round;
        m_biddingPhase = true;
        advancePlayer(m_dealer);
        advancePlayer(m_eldest);
        roundTricks.clear();
        dealCards = true;
    }
}

void Game::deal()
{
    m_deck.shuffle();
    for (int i = 0; i < m_players.size(); ++i)
        m_players[i]->setHand(m_deck.mid(i*8, 8));
}

void Game::proposeBid()
{
    if (m_awaitingTurn)
        return;
    m_awaitingTurn = true;
    static QVariantList options;
    if (m_bidRound == 0) {
        options.clear();
        switch (m_bidRule) {
            case BidRule::Official:
                // Each player may elect a suit or pass. If all pass, first player
                // must elect.
                options = { Bid::Spades, Bid::Hearts, Bid::Diamonds, Bid::Clubs, Bid::Pass };
                break;
            case BidRule::Utrechts:
                // No bidding; first player is forced to choose.
                options = { Bid::Spades, Bid::Hearts, Bid::Diamonds, Bid::Clubs };
                break;
            case BidRule::Random:
                // Choose random suit followed by bidding. If all pass, first
                // player must elect a different suit.
            case BidRule::Twents:
                // Like Random, but if all players pass, the trump suit will also
                // be random.
                if (m_round == 0)
                    options = { Bid::Clubs, Bid::Pass };
                else
                    options = { Bid(std::rand() % 4), Bid::Pass };
                break;
        }
    } else {
        // All players have passed in the first round of bidding.
        if (m_bidRule == BidRule::Twents) {
            acceptBid(Bid(std::rand() % 4));
            return;
        }
        if (m_bidRule == BidRule::Official) {
            options.removeLast();
        }
        if (m_bidRule == BidRule::Random) {
            Bid forbidden = options.takeFirst().value<Bid>();
            options.clear();
            for (int i = 0; i < 4; ++i)
                if (Bid(i) != forbidden)
                    options.append(Bid(i));
        }
    }
    ++m_bidRound;
    connect(m_currentPlayer, &Player::bidSelected, this, &Game::acceptBid);
    qDebug() << m_currentPlayer;
    emit m_currentPlayer->bidRequested(options);
}

void Game::acceptBid(Game::Bid bid)
{
    disconnect(m_currentPlayer, &Player::bidSelected, this, &Game::acceptBid);
    m_awaitingTurn = false;
    if (bid == Bid::Pass) {
        qCDebug(klaverjasGame) << "Player" << m_currentPlayer << "passed";
        advancePlayer(m_currentPlayer);
        proposeBid();
    } else {
        setContract((Card::Suit) bid, m_currentPlayer);
        qCDebug(klaverjasGame) << "Player" << m_currentPlayer << "elected" << m_trumpSuit;
        m_currentPlayer = m_eldest;
        m_biddingPhase = false;
        m_bidRound = 0;
    }
}

void Game::setContract(const Card::Suit suit, const Player* player)
{
    m_trumpSuit = suit;
    m_contractors = player->team();
    m_defenders = m_teams.first() == m_contractors ? m_teams.last() : m_teams.first();
    emit trumpSuitChanged(suit);
}

void Game::acceptTurn(Card card)
{
    m_currentTrick.add(m_currentPlayer, card);
    disconnect(m_currentPlayer, &Player::cardPlayed, this, &Game::acceptTurn);
    advancePlayer(m_currentPlayer);
    m_awaitingTurn = false;
    advance();
}

Game::Score Game::scoreRound(const QVector<Trick> tricks) const
{
    Score newScore;
    int total = 10;
    for (Team* t : m_teams)
        newScore[t] = 0;
    for (const auto t : tricks) {
        newScore[t.winner()->team()] += t.points();
        total += t.points();
    }
    newScore[tricks.last().winner()->team()] += 10;
    if (newScore[m_contractors] < total / 2 + 1) {
        newScore[m_contractors] = 0;
        newScore[m_defenders] = total;
    }
    return newScore;
}

const QVector<Card> Game::legalMoves(const Player* player, const Trick& trick) const
{
    QVector<Card> moves;
    if (trick.players().isEmpty()) {
        for (const auto card : player->hand())
            moves << card;
        return moves;
    }

    const Card::Suit suitLed = trick.suitLed();
    QMap<Card::Suit,Card::Rank> minRanks;
    if (player->hand().containsSuit(suitLed)) {
        // Following suit has the highest priority. If the suit led is trumps,
        // players must always overtrump if they can.
        if (trick.suitLed() != m_trumpSuit) {
            minRanks[suitLed] = PlainRanks.last();
        } else if (player->canBeat(*trick.winningCard(), TrumpRanks)) {
            minRanks[suitLed] = trick.winningCard()->rank();
        } else {
            minRanks[suitLed] = TrumpRanks.last();
        }
    } else {
        // The player cannot follow suit.
        if (player->hand().containsSuit(m_trumpSuit)) {
            // A player must generally (over)trump, but is exempt from this
            // under Amsterdam rules if his partner is the current winner of
            // the trick.
            if (m_trumpRule == TrumpRule::Amsterdams
                && player->team()->players().contains(trick.winner()))
            {
                for (const Card::Suit suit : Card::Suits)
                    minRanks[suit] = suit == m_trumpSuit ? TrumpRanks.last() : PlainRanks.last();
            } else if (trick.winningCard()->suit() == m_trumpSuit
                && player->canBeat(*trick.winningCard(), TrumpRanks))
            {
                // Player must overtrump his opponent
                minRanks[m_trumpSuit] = trick.winningCard()->rank();
            } else {
                // Player may play any trump
                minRanks[m_trumpSuit] = TrumpRanks.last();
            }
        } else {
            // Player has no trump cards.
            for (const Card::Suit suit : Card::Suits)
                minRanks[suit] = suit == m_trumpSuit ? TrumpRanks.last() : PlainRanks.last();
        }
    }
    for (auto rank = minRanks.constBegin(); rank != minRanks.constEnd(); ++rank) {
        Card::Suit suit = rank.key();
        QVector<Card::Rank> order = suit == m_trumpSuit ? TrumpRanks : PlainRanks;
        if (player->hand().containsSuit(suit))
            for (const Card& c : player->hand().suitSets()[suit])
                if (rankDifference(c.rank(), *rank, order) >= 0)
                    moves << c;
    }
    return moves;
}

Player* Game::nextPlayer(Player* player) const
{
    int index = m_players.indexOf(player);
    return m_players.at(++index % 4);
}

void Game::advancePlayer(Player*& player) const
{
    player = nextPlayer(player);
}
