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
#include "players/humanplayer.h"
#include "players/aiplayer.h"
#include "players/randomplayer.h"
#include "team.h"

#include <algorithm>
#include <QString>
#include <QStringList>

namespace {

using TrumpRule = GameEngine::TrumpRule;
using Suit = Card::Suit;
using Rank = Card::Rank;
const QStringList DefaultNames {"South", "West", "North", "East"};

QVariantList bidOptions(const QVector<Suit> suits = Card::Suits) {
    QVariantList options;
    for (const auto &s : suits)
        options << QVariant::fromValue(s);
    return options;
}

}

Game::Game(QObject *parent, int numRounds)
    : QObject(parent)
    , m_trumpRule(TrumpRule::Amsterdams)
    , m_bidRule(BidRule::Random)
    , m_biddingPhase(true)
    , m_bidCounter(0)
    , m_numRounds(numRounds)
    , m_round(0)
    , m_trick(0)
    , m_turn(0)
    , m_contractors(nullptr)
    , m_defenders(nullptr)
    , m_human(nullptr)
    , m_status(Ready)
    , m_engine(nullptr)
{
    // Reserve vector space
    m_deck.reserve(32);

    for (int  i = 0; i < 32; ++i)
        m_deck.append(Card(Card::Suits[i/8], Card::Ranks[i%8]));

    for (int i = 1; i <= 2; ++i) {
        Team* team = new Team(QString::number(i), this);
        m_teams.append(team);
        m_scores[team] = 0;
    }
}

void Game::addPlayer(Player *player)
{
    if (m_players.size() <= 4) {
        auto human = dynamic_cast<HumanPlayer*>(player);
        if (human)
            m_human = human;
        m_players << std::shared_ptr<Player>(player);
        emit playersChanged();
    }
}

int Game::currentPlayer() const
{
    return playerIndex(m_currentPlayer);
}

int Game::playerIndex(const Player *player) const
{
    const auto target = std::find_if(m_players.begin(), m_players.end(), [&](const GameEngine::Player &p){
        return p.get() == player;
    });
    return m_players.indexOf(*target);
}

Player *Game::playerAt(int index) const
{
    return dynamic_cast<Player*>(m_players[index].get());
}

HumanPlayer *Game::humanPlayer() const
{
    return m_human;
}

QQmlListProperty<Player> Game::players()
{
    using ListPtr = QQmlListProperty<Player>*;
    return QQmlListProperty<Player>(this, this,
        [](ListPtr l, Player *p){ reinterpret_cast<Game*>(l->data)->addPlayer(p); },
        [](ListPtr l){ return reinterpret_cast<Game*>(l->data)->m_players.size(); },
        [](ListPtr l, int i){ return reinterpret_cast<Game*>(l->data)->playerAt(i); },
        [](ListPtr l){ reinterpret_cast<Game*>(l->data)->m_players.clear(); }
    );
}

QQmlListProperty<Team> Game::teams()
{
    using ListPtr = QQmlListProperty<Team>*;
    return QQmlListProperty<Team>(this, this,
        [](ListPtr){ return 2; },
        [](ListPtr l, int i){ return reinterpret_cast<Game*>(l->data)->m_teams.at(i); }
    );
}

const Team *Game::contractors() const
{
    return m_contractors;
}

const Team *Game::defenders() const
{
    return m_defenders;
}

int Game::round() const
{
    return m_round;
}

Card::Suit Game::trumpSuit() const
{
    return m_trumpSuit;
}

const Game::Score & Game::score() const
{
    return m_scores;
}

Game::Status Game::status() const
{
    return m_status;
}

void Game::setStatus(Game::Status newStatus)
{
    if (newStatus != m_status) {
        m_status = newStatus;
        emit statusChanged(newStatus);
    }
}

const Trick& Game::currentTrick() const
{
    return m_currentTrick;
}

void Game::start()
{
    for (int i = m_players.size(); i < 4; ++i)
        addPlayer(new AiPlayer(m_engine, DefaultNames[i], this));

    // Make human player 3rd position
    if (m_human)
        std::rotate(m_players.begin(), m_players.begin() + 2, m_players.end());

    for (int i = 0; i < 4; ++i)
        m_teams[i % 2]->addPlayer(playerAt(i));

    qCDebug(klaverjasGame) << "Teams: " << m_teams;

    m_dealer = dynamic_cast<Player*>(m_players.at(1).get());
    m_eldest = nextPlayer(m_dealer);
    m_currentPlayer = m_eldest;

    deal();
}

void Game::restart()
{
    m_round = 0;
    m_trick = 0;
    m_turn = 0;
    m_tricks.clear();

    for (auto &score : m_scores)
        score = 0;
    for (auto t : m_teams)
        t->resetScore();

    start();
}

/** Advance the state of the current game.
 *
 * This slot controls the state of an interactive game, which must be able to
 * wait for player input. It sets the waiting flag to true when expecting a
 * signal from a player, blocking further calls until this flag is reset by the
 * acceptBid or acceptMove slots.
 */
void Game::advance()
{
    // Block further calls
    if (m_status != Ready)
        return;

    if (m_biddingPhase) {
        setStatus(Waiting);
        proposeBid();
    } else if (m_turn == 4) {
        // Trick complete
        m_turn = 0;
        ++m_trick;
        m_roundTricks << m_currentTrick;
        qCInfo(klaverjasGame) << "Current trick:" << m_currentTrick;
        qCInfo(klaverjasGame) << "Trick winner:" << m_currentPlayer << "Points:" << m_currentTrick.points();
        m_currentTrick = Trick(m_trumpSuit);
        emit newTrick();
        advance();
    } else if (!m_engine->isFinished()) {
        // Trick-taking phase, proceed automatically until it is the human
        // player's turn or a new trick is about to start.
        connect(this, &Game::moveRequested, m_currentPlayer, &Player::selectMove);
        connect(m_currentPlayer, &Player::moveSelected, this, &Game::acceptMove);
        setStatus(Waiting);
        emit moveRequested(legalMoves());
        ++m_turn;
        if (m_turn < 4)
            advance();
    } else if (m_round < m_numRounds) {
        // One round (game) completed
        m_tricks << m_roundTricks;
        const auto score = scoreRound(m_roundTricks);
        qCInfo(klaverjasGame) << "Round scores: " << score;
        for (Team* team : m_teams) {
            team->addPoints(score[team]);
            m_scores[team] += score[team];
        }
        m_roundTricks.clear();
        m_trick = 0;
        m_turn = 0;
        ++m_round;
        advancePlayer(m_dealer);
        advancePlayer(m_eldest);
        m_currentPlayer = m_eldest;
        m_biddingPhase = true;
        deal();
        setStatus(Ready);
    } else {
        setStatus(Finished);
        qCInfo(klaverjasGame) << "Game finished";
    }
}

void Game::deal()
{
    std::random_shuffle(m_deck.begin(), m_deck.end());
    for (int i = 0; i < m_players.size(); ++i)
        m_players[i]->setHand(m_deck.mid(i*8, 8));
}

void Game::proposeBid()
{
    const auto allOptions = bidOptions();
    if (m_bidCounter == 0) {
        switch (m_bidRule) {
        case BidRule::Official:
            // Each player may elect a suit or pass. If all pass, first player
            // must elect.
            m_bidOptions = {allOptions, QVariant()};
            break;
        case BidRule::Utrechts:
            // No bidding; first player is forced to choose.
            m_bidOptions = allOptions;
            break;
        case BidRule::Random:
            // Choose random suit followed by bidding. If all pass, first
            // player must elect a different suit.
        case BidRule::Twents:
            // Like Random, but if all players pass, the trump suit will also
            // be random.
            if (m_round == 0)
                m_bidOptions = bidOptions({Suit::Clubs}) << QVariant();
            else
                m_bidOptions = {allOptions[std::rand() % 4], QVariant()};
            break;
        }
    } else if (m_bidCounter == 4) {
        // All players have passed in the first round of bidding.
        if (m_bidRule == BidRule::Twents) {
            acceptBid(allOptions[std::rand() % 4]);
            return;
        }
        if (m_bidRule == BidRule::Official) {
            m_bidOptions.removeLast();
        }
        if (m_bidRule == BidRule::Random) {
            auto forbidden = m_bidOptions.first();
            m_bidOptions.clear();
            for (int i = 0; i < 4; ++i) {
                const auto bid = allOptions[i];
                if (bid != forbidden)
                    m_bidOptions << bid;
            }
        }
    }
    ++m_bidCounter;
    connect(this, &Game::bidRequested, m_currentPlayer, &Player::selectBid);
    connect(m_currentPlayer, &Player::bidSelected, this, &Game::acceptBid);
    emit bidRequested(m_bidOptions, m_currentPlayer);
    qCDebug(klaverjasGame) << "Requesting a bid";
}

void Game::acceptBid(QVariant bid)
{
    if (!m_biddingPhase)
        return;
    disconnect(this, &Game::bidRequested, 0, 0);
    disconnect(m_currentPlayer, &Player::bidSelected, 0, 0);
    if (bid.isNull()) {
        qCDebug(klaverjasGame) << "Player" << m_currentPlayer << "passed";
        advancePlayer(m_currentPlayer);
        proposeBid();
    } else {
        m_biddingPhase = false;
        m_bidCounter = 0;
        setContract(bid.value<Card::Suit>(), m_currentPlayer);
        m_currentTrick = Trick(m_trumpSuit);
        emit newTrick();
    }
    setStatus(Ready);
}

void Game::setContract(const Card::Suit suit, const Player *player)
{
    qCInfo(klaverjasGame) << "Player" << player << "elected" << suit;
    m_trumpSuit = suit;
    m_contractors = player->team();
    m_defenders = m_teams.first() == m_contractors ? m_teams.last() : m_teams.first();
    m_currentPlayer = m_eldest;
    auto currentPos = GameEngine::Position(currentPlayer());
    auto contractorPos = GameEngine::Position(playerIndex(player));
    if (m_engine)
        m_engine->reset(currentPos, contractorPos, m_trumpSuit);
    else
        m_engine = GameEngine::create(m_players, currentPos, contractorPos, m_trumpRule, m_trumpSuit);
    emit trumpSuitChanged(suit);
}

void Game::acceptMove(Card card)
{
    disconnect(this, &Game::moveRequested, 0, 0);
    disconnect(m_currentPlayer, &Player::moveSelected, this, &Game::acceptMove);
    if (m_status != Waiting || m_biddingPhase)
        return;
    qCDebug(klaverjasGame) << m_currentPlayer << "played" << card;
    m_engine->doMove(card);
    m_currentTrick.add(currentPlayer(), card);
    m_currentPlayer = playerAt(m_engine->currentPlayer());
    setStatus(Ready);
}

Game::Score Game::scoreRound(const QVector<Trick> tricks) const
{
    Score newScore;
    int total = 10;
    for (Team* t : m_teams)
        newScore[t] = 0;

    // A "march" is achieved if all tricks are taken by a single team
    Team* const first = playerAt(tricks.first().winner())->team();
    bool march = true;
    for (const auto t : tricks) {
        Team* winners = playerAt(t.winner())->team();
        const int points = t.points();
        newScore[winners] += points;
        total += points;
        if (march && winners != first)
            march = false;
    }
    newScore[playerAt(tricks.last().winner())->team()] += 10;
    if (march)
        newScore[first] += 88;
    if (newScore[m_contractors] < total / 2 + 1) {
        newScore[m_contractors] = 0;
        newScore[m_defenders] = total;
    }
    return newScore;
}

const QVector<Card> Game::legalMoves() const
{
    QVector<Card> moves;
    moves.reserve(8);
    if (m_currentTrick.players().isEmpty()) {
        for (const auto card : m_currentPlayer->hand())
            moves << card;
        return moves;
    }
    if (m_currentPlayer->hand().isEmpty())
        return moves;

    const Card::Suit suitLed = m_currentTrick.suitLed();
    QMap<Card::Suit,Card::Rank> minRanks;
    if (m_currentPlayer->hand().containsSuit(suitLed)) {
        // Following suit has the highest priority. If the suit led is trumps,
        // players must always overtrump if they can.
        if (m_currentTrick.suitLed() != m_trumpSuit) {
            minRanks[suitLed] = PlainRanks.last();
        } else if (m_currentPlayer->canBeat(*m_currentTrick.winningCard(), TrumpRanks)) {
            minRanks[suitLed] = m_currentTrick.winningCard()->rank();
        } else {
            minRanks[suitLed] = TrumpRanks.last();
        }
    } else {
        // The player cannot follow suit.
        if (m_currentPlayer->hand().containsSuit(m_trumpSuit)) {
            // A player must generally (over)trump, but is exempt from this
            // under Amsterdam rules if his partner is the current winner of
            // the trick.
            if (m_trumpRule == TrumpRule::Amsterdams
                && m_currentPlayer->team()->players().contains(playerAt(m_currentTrick.winner())))
            {
                for (const Card::Suit suit : Card::Suits)
                    minRanks[suit] = suit == m_trumpSuit ? TrumpRanks.last() : PlainRanks.last();
            } else if (m_currentTrick.winningCard()->suit() == m_trumpSuit
                && m_currentPlayer->canBeat(*m_currentTrick.winningCard(), TrumpRanks))
            {
                // Player must overtrump his opponent
                minRanks[m_trumpSuit] = m_currentTrick.winningCard()->rank();
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

    // Compile the list of moves
    const auto suitSets = m_currentPlayer->hand().suitSets();
    for (auto set = suitSets.cbegin(); set != suitSets.cend(); ++set) {
        Card::Suit suit = set.key();
        QVector<Card::Rank> order = suit == m_trumpSuit ? TrumpRanks : PlainRanks;
        if (minRanks.contains(suit)) {
            for (auto c = set.value().cbegin(); c != set.value().cend(); ++c) {
                if (rankDifference(c->rank(), minRanks[suit], order) >= 0)
                    moves << *c;
            }
        }
    }
    return moves;
}

Player *Game::nextPlayer(Player *player) const
{
    int index = playerIndex(player);
    return dynamic_cast<Player*>(m_players.at(++index % 4).get());
}

void Game::advancePlayer(Player *&player) const
{
    player = nextPlayer(player);
}
