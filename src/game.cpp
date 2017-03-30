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
#include "team.h"

#include <algorithm>
#include <QTime>

const QStringList Game::s_defaultPlayerNames {"North", "East", "South", "West"};

Game::Game(QObject* parent)
    : QObject(parent)
    , m_trumpRule(TrumpRule::Amsterdams)
    , m_bidRule(BidRule::Random)
    , m_round(0)
    , m_contractors(nullptr)
    , m_defenders(nullptr)
{
    for (int  i = 0; i < 32; ++i)
        m_deck.append(Card(Card::Suit(i/8), Card::Rank(i%8)));

    for (int i = 1; i <= 2; ++i)
        m_teams.append(new Team(QString::number(i), this));

    for (int p = 0, t = 0; p < s_defaultPlayerNames.size(); ++p, t = p % 2) {
        Player* newPlayer = new Player(s_defaultPlayerNames[p], this);
        m_teams[t]->addPlayer(newPlayer);
        m_players.append(newPlayer);
    }
    emit teamsChanged();

    qCDebug(klaverjasGame) << "Teams: " << m_teams;

    m_dealer = m_players.first();
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

void Game::start()
{
    for (m_round = 0; m_round < 2; ++m_round) {
        QVector<Trick> roundTricks;
        qDebug() << "Starting round:" << m_round;

        deal();
        selectTrump();
        Player* currentPlayer = m_eldest;
        for (int trick = 0; trick < 8; ++trick) {
            Trick currentTrick(m_trumpSuit);
            for (int i = 0; i < 4; ++i) {
                auto legalMoves = this->legalMoves(currentPlayer, currentTrick);
                Card currentPlay = currentPlayer->performTurn(legalMoves);
                currentTrick.add(currentPlayer, currentPlay);
                advancePlayer(currentPlayer);
            }
            roundTricks << currentTrick;
            currentPlayer = currentTrick.winner();

            qCDebug(klaverjasGame) << "Current trick:" << currentTrick;
            qCDebug(klaverjasGame) << "Trick winner:" << currentPlayer << "Points:" << currentTrick.points();
        }
        m_tricks << roundTricks;
//         m_scores << scoreRound(roundTricks);
        const auto score = scoreRound(roundTricks);
        for (Team* team : m_teams) {
            team->addPoints(score[team]);
        }

        qCDebug(klaverjasGame) << "Round scores: " << score;

        advancePlayer(m_dealer);
        advancePlayer(m_eldest);
    } // round loop
}

void Game::deal()
{
    m_deck.shuffle();
    for (int i = 0; i < m_players.size(); ++i)
        m_players[i]->setHand(m_deck.mid(i*8, 8));
}

void Game::selectTrump()
{
    // Set the allowable bids for the initial bidding
    QVector<Bid> options;
    switch (m_bidRule) {
        case BidRule::Utrechts:
            // No bidding; first player is forced to choose.
            options = { Bid::Spades, Bid::Hearts, Bid::Diamonds, Bid::Clubs };
        case BidRule::Official:
            // Each player may elect a suit or pass. If all pass, first player
            // must elect.
            options.append(Bid::Pass);
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

    // Bidding phase
    // First ask each player to bid, because players may be allowed to pass.
    Player* current = m_eldest;
    Bid bid;
    for (int i = 0; i < 4; ++i) {
        bid = current->bid(options);
        Q_ASSERT(options.contains(bid));
        if (bid != Bid::Pass) {
            setContract((Card::Suit) bid, current);
            return;
        } else {
            advancePlayer(current);
        }
    }

    // All players have passed at this point; depending on the rules a random
    // suit is now chosen or the eldest hand is forced to choose.
    if (m_bidRule == BidRule::Twents) {
        setContract(Card::Suit(std::rand() % 4), m_eldest);
        return;
    }
    if (m_bidRule == BidRule::Official) {
        options.removeLast();
    }
    if (m_bidRule == BidRule::Random) {
        Bid forbidden = options.takeFirst();
        options.clear();
        for (int i = 0; i < 4; ++i)
            if (Bid(i) != forbidden)
                options.append(Bid(i));
    }
    bid = m_eldest->bid(options);
    Q_ASSERT(options.contains(bid));
    setContract((Card::Suit) bid, m_eldest);
}

void Game::setContract(const Card::Suit suit, const Player* player)
{
    m_trumpSuit = suit;
    m_contractors = player->team();
    m_defenders = m_teams.first() == m_contractors ? m_teams.last() : m_teams.first();
    emit trumpSuitChanged(suit);
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
