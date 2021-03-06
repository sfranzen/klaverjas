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
#include "trick.h"
#include "players/humanplayer.h"
#include "players/aiplayer.h"
#include "players/randomplayer.h"
#include "team.h"

#include <QString>
#include <QStringList>
#include <QLoggingCategory>

#include <algorithm>

Q_DECLARE_LOGGING_CATEGORY(klaverjasGame)

namespace {

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
    , m_engine(nullptr)
    , m_contractors(nullptr)
    , m_defenders(nullptr)
    , m_human(nullptr)
    , m_bidCounter(0)
    , m_turn(0)
    , m_round(0)
    , m_numRounds(numRounds)
    , m_trumpRule(TrumpRule::Amsterdams)
    , m_bidRule(BidRule::Random)
    , m_biddingPhase(true)
    , m_status(Ready)
{
    // Reserve vector space
    m_deck.reserve(32);
    m_roundCards.reserve(numRounds);

    for (int  i = 0; i < 32; ++i)
        m_deck.append(Card(Card::Suits[i/8], Card::Ranks[i%8]));

    for (int i = 1; i <= 2; ++i) {
        Team* team = new Team(QString::number(i), this);
        m_teams.append(team);
    }
}

void Game::addPlayer(Player *player)
{
    if (m_players.size() <= 4) {
        connect(this, &Game::newRound, player, &Player::bidSort);
        connect(this, &Game::newContract, player, &Player::playSort);
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

Player *Game::currentPlayerPtr() const
{
    return m_currentPlayer;
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

const QVector<Team*> &Game::teams(int) const
{
    return m_teams;
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

const GameEngine *Game::engine() const
{
    return m_engine.get();
}

void Game::start()
{
    for (int i = m_players.size(); i < 4; ++i)
        addPlayer(new AiPlayer(DefaultNames[i], this));

    // Make human player 3rd position
    if (m_human) {
        std::rotate(m_players.begin(), m_players.begin() + 2, m_players.end());
        emit playersChanged();
    }

    for (int i = 0; i < 4; ++i)
        m_teams[i % 2]->addPlayer(playerAt(i));
    qCDebug(klaverjasGame) << "Teams: " << m_teams;

    m_dealer = dynamic_cast<Player*>(m_players.at(1).get());
    m_eldest = nextPlayer(m_dealer);
    m_currentPlayer = m_eldest;
    deal();
    emit newRound();
}

void Game::deal()
{
    std::random_shuffle(m_deck.begin(), m_deck.end());
    for (int i = 0; i < m_players.size(); ++i)
        m_players[i]->setHand(m_deck.mid(i*8, 8));
}

void Game::restart()
{
    m_round = 0;
    m_turn = 0;
    m_roundCards.clear();
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
        const auto trick = const_cast<const GameEngine*>(m_engine.get())->currentTrick();
        qCDebug(klaverjasGame) << "Current trick:" << trick;
        qCDebug(klaverjasGame) << "Trick winner:" << m_currentPlayer << "Points:" << trick.score().sum();
        advance();
    } else if (!m_engine->isFinished()) {
        handleTrick();
    } else if (m_round < m_numRounds) {
        // One round (game) completed
        handleRound();
    } else {
        setStatus(Finished);
        qCInfo(klaverjasGame) << "Game finished";
    }
}

void Game::proposeBid()
{
    const auto allOptions = bidOptions();
    if (m_bidCounter == 0) {
        m_bidOptions = initialBidOptions();
        emit biddingStarted();
    } else if (m_bidCounter % 4 == 0)
        // All players have passed in the first round of bidding.
        refineBidOptions();
    ++m_bidCounter;
    connect(this, &Game::bidRequested, m_currentPlayer, &Player::selectBid);
    connect(m_currentPlayer, &Player::bidSelected, this, &Game::acceptBid);
    qCDebug(klaverjasGame) << "Requesting a bid";
    emit bidRequested(m_bidOptions, m_currentPlayer);
}

QVariantList Game::initialBidOptions() const
{
    const auto allOptions = bidOptions();
    switch (m_bidRule) {
    case BidRule::Official:
        return {allOptions, QVariant()};
    case BidRule::Utrechts:
        return allOptions;
    default:
        // For Random and Twents games, the first choice in a game is Clubs,
        // otherwise a random suit is chosen.
        if (m_round == 0)
            return bidOptions({Suit::Clubs}) << QVariant();
        else
            return {allOptions[std::rand() % 4], QVariant()};
    }
}

void Game::refineBidOptions()
{
    Q_ASSERT(m_bidRule != BidRule::Utrechts);
    const auto allOptions = bidOptions();
    if (m_bidRule == BidRule::Random) {
        auto forbidden = m_bidOptions.first();
        m_bidOptions.clear();
        for (const auto bid : allOptions) {
            if (bid != forbidden)
                m_bidOptions << bid;
        }
    } else if (m_bidRule == BidRule::Official)
        m_bidOptions.removeLast();
    else
        acceptBid(allOptions[std::rand() % 4]);
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
        setStatus(Ready);
    }
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
    emit newContract(suit, m_contractors);
}

void Game::handleTrick()
{
    if (m_turn == 0) {
        emit newTrick();
    }
    connect(this, &Game::moveRequested, m_currentPlayer, &Player::selectMove);
    connect(m_currentPlayer, &Player::moveSelected, this, &Game::acceptMove);
    setStatus(Waiting);
    emit moveRequested(m_engine->validMoves());
}

void Game::acceptMove(Card card)
{
    if (m_status != Waiting || m_biddingPhase)
        return;
    disconnect(this, &Game::moveRequested, 0, 0);
    disconnect(m_currentPlayer, &Player::moveSelected, this, &Game::acceptMove);
    qCDebug(klaverjasGame) << m_currentPlayer << "played" << card;
    emit cardPlayed(currentPlayer(), card);
    m_engine->doMove(card);
    m_currentPlayer = playerAt(m_engine->currentPlayer());
    setStatus(Ready);
    ++m_turn;
    if (m_turn < 4)
        advance();
}

void Game::handleRound()
{
    m_roundCards << m_engine->cardsPlayed();
    const auto scores = m_engine->scores();
    for (int i : {0, 1})
        m_teams[i]->addPoints(scores[i]);
    qCInfo(klaverjasGame) << "Round scores: " << scores;
    m_turn = 0;
    ++m_round;
    advancePlayer(m_dealer);
    advancePlayer(m_eldest);
    m_currentPlayer = m_eldest;
    m_biddingPhase = true;
    deal();
    setStatus(Ready);
    emit newRound();
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
