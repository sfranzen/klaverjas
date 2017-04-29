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
#include "team.h"

#include <algorithm>
#include <QString>
#include <QTime>
#include <QTimer>

const QStringList Game::s_defaultPlayerNames {"South", "West", "North", "East"};

Game::Game(bool interactive, QObject* parent)
    : QObject(parent)
    , m_trumpRule(TrumpRule::Amsterdams)
    , m_bidRule(BidRule::Random)
    , m_interactive(interactive)
    , m_biddingPhase(false)
    , m_waiting(false)
    , m_paused(false)
    , m_bidCounter(0)
    , m_round(0)
    , m_trick(0)
    , m_turn(0)
    , m_contractors(nullptr)
    , m_defenders(nullptr)
    , m_human(nullptr)
{
    // Initialise PRNG
    std::srand(QTime::currentTime().msec());

    for (int  i = 0; i < 32; ++i)
        m_deck.append(Card(Card::Suit(i/8), Card::Rank(i%8)));

    for (int i = 1; i <= 2; ++i)
        m_teams.append(new Team(QString::number(i), this));

    for (int p = 0, t = 0; p < 4; ++p, t = p % 2) {
        Player* newPlayer;
        if (interactive && p == 0) {
            m_human = new HumanPlayer("You", this);
            newPlayer = m_human;
        }
        else
            newPlayer = new AiPlayer(s_defaultPlayerNames[p], this);

        m_teams[t]->addPlayer(newPlayer);
        m_players.append(newPlayer);
    }
    emit teamsChanged();

    qCDebug(klaverjasGame) << "Teams: " << m_teams;

    m_dealer = m_players.last();
    m_eldest = nextPlayer(m_dealer);
    m_currentPlayer = m_eldest;

    deal();

    if (m_interactive)
        m_biddingPhase = true;

    m_solver.moveToThread(&m_solverThread);
    connect(&m_solver, &ISMC::Solver::searchComplete, this, &Game::acceptMove);
    connect(&m_solver, &ISMC::Solver::searchComplete, this, &Game::advance);
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

Card::Suit Game::trumpSuit() const
{
    return m_trumpSuit;
}

QQmlListProperty<Player> Game::players()
{
    return QQmlListProperty<Player>(this, m_players);
}

HumanPlayer* Game::humanPlayer() const
{
    return m_human;
}

QQmlListProperty<Team> Game::teams()
{
    return QQmlListProperty<Team>(this, m_teams);
}

int Game::currentPlayer() const
{
    return playerIndex(m_currentPlayer);
}

int Game::playerIndex(Player* player) const
{
    return m_players.indexOf(player);
}

Player* Game::playerAt(int index) const
{
    return m_players.at(index);
}

const Team* Game::contractors() const
{
    return m_contractors;
}

const Team* Game::defenders() const
{
    return m_defenders;
}

const QVector<Card> Game::cardsPlayed() const
{
    return m_cardsPlayed;
}

Game* Game::cloneAndRandomize(int observer) const
{
    Game* clone = new Game(false);
    // Main state
    clone->m_trick = m_trick;
    clone->m_turn = m_turn;
    clone->m_trumpSuit = m_trumpSuit;
    clone->m_cardsPlayed = m_cardsPlayed;
    clone->m_currentTrick = m_currentTrick;
    clone->m_roundTricks = m_roundTricks;

    // Current player
    const int pIdx = currentPlayer();
    clone->m_currentPlayer = clone->playerAt(pIdx);
    clone->m_dealer = clone->playerAt(playerIndex(m_dealer));
    clone->m_eldest = clone->nextPlayer(clone->m_dealer);

    // Team setup
    const int cIdx = m_teams.indexOf(m_contractors);
    clone->m_contractors = clone->m_teams.at(cIdx);
    clone->m_defenders = clone->m_teams.at((cIdx + 1) % 2);
    for (int i = 0; i < m_teams.size(); ++i)
        clone->m_teams[i]->addPoints(m_teams.at(i)->score());

    // Randomly distribute the cards not known to the observer
    QVector<Card> seenCards, unseenCards;
    seenCards << playerAt(observer)->hand();
    seenCards << m_cardsPlayed;
    for (auto card = m_deck.cbegin(); card != m_deck.cend(); ++card)
        if (!seenCards.contains(*card))
            unseenCards << *card;
    std::random_shuffle(unseenCards.begin(), unseenCards.end());
    for (int i = 0; i < m_players.size(); ++i) {
        Player* player = clone->playerAt(i);
        if (i == observer) {
            player->setHand(playerAt(i)->hand());
        } else {
            int numCards = playerAt(i)->hand().size();
            player->setHand(unseenCards.mid(0, numCards));
            unseenCards.remove(0, numCards);
        }
    }
    return clone;
}

qreal Game::getResult(int playerIndex) const
{
    qreal result = -1;
    if (m_roundTricks.isEmpty())
        result = playerAt(playerIndex)->team()->score() / 162.0;
    return result;
}

/** Advance the state of the current game.
 *
 * This slot controls the state of an interactive game, which must be able to
 * wait for human input. It halts and sets the waiting flag to true whenever it
 * is the human player's turn, blocking further calls until this flag is reset
 * by acceptBid or acceptMove.
 */
void Game::advance()
{
    // Block further calls
    if (m_waiting)
        return;

    if (m_biddingPhase)
        proposeBid();
    else {
        // Trick-taking phase, proceed automatically until it is the human
        // player's turn or a new trick is about to start.
        if (m_paused) {
            m_paused = false;
            return;
        }
        if (m_currentTrick.players().isEmpty())
            emit newTrick();

        m_waiting = true;
        if (m_currentPlayer == m_human) {
            emit moveRequested();
        } else {
            m_paused = m_turn == 3;
            m_solver.treeSearch(this);
        }
    }
}

void Game::deal()
{
    std::random_shuffle(m_deck.begin(), m_deck.end());
    m_cardsPlayed.clear();
    for (int i = 0; i < m_players.size(); ++i)
        m_players[i]->setHand(m_deck.mid(i*8, 8));
}

void Game::proposeBid()
{
    static QVariantList options;
    if (m_bidCounter == 0) {
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
    } else if (m_bidCounter == 4) {
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
    ++m_bidCounter;
    connect(this, &Game::bidRequested, m_currentPlayer, &Player::selectBid);
    connect(m_currentPlayer, &Player::bidSelected, this, &Game::acceptBid);
    qCDebug(klaverjasGame) << "Requesting a bid";
    m_waiting = true;
    emit bidRequested(options, m_currentPlayer);
}

void Game::acceptBid(Game::Bid bid)
{
    if (!m_biddingPhase)
        return;
    disconnect(this, &Game::bidRequested, 0, 0);
    disconnect(m_currentPlayer, &Player::bidSelected, 0, 0);
    if (bid == Bid::Pass) {
        if (m_interactive)
            qCInfo(klaverjasGame) << "Player" << m_currentPlayer << "passed";
        advancePlayer(m_currentPlayer);
        proposeBid();
    } else {
        setContract((Card::Suit) bid, m_currentPlayer);
        if (m_interactive)
            qCInfo(klaverjasGame) << "Player" << m_currentPlayer << "elected" << m_trumpSuit;
        m_currentPlayer = m_eldest;
        m_biddingPhase = false;
        m_bidCounter = 0;
        m_currentTrick = Trick(m_trumpSuit);
        emit newTrick();
    }
    m_waiting = false;
}

void Game::setContract(const Card::Suit suit, const Player* player)
{
    m_trumpSuit = suit;
    m_contractors = player->team();
    m_defenders = m_teams.first() == m_contractors ? m_teams.last() : m_teams.first();
    emit trumpSuitChanged(suit);
}

void Game::acceptMove(Card card)
{
    if (m_biddingPhase)
        return;
    if (m_round == 16) {
        qCInfo(klaverjasGame) << "Game finished";
        return;
    }
    if (m_trick < 8) {
        if (m_turn < 4) {
            m_turn++;
            m_currentTrick.add(currentPlayer(), card);
            if (m_interactive) {
                qCInfo(klaverjasGame) << m_currentPlayer << "played" << card;
                emit cardPlayed(currentPlayer(), card);
            }
            m_currentPlayer->removeCard(card);
//             m_cardsInPlay.remove(card);
            m_cardsPlayed.append(card);
            advancePlayer(m_currentPlayer);
        }
        if (m_turn == 4) {
            m_turn = 0;
            m_trick++;
            m_roundTricks << m_currentTrick;
            m_currentPlayer = playerAt(m_currentTrick.winner());
            if (m_interactive) {
                qCInfo(klaverjasGame) << "Current trick:" << m_currentTrick;
                qCInfo(klaverjasGame) << "Trick winner:" << m_currentPlayer << "Points:" << m_currentTrick.points();
            }
            m_currentTrick = Trick(m_trumpSuit);
        }
    }
    if (m_trick == 8) {
        // One round (game) completed
        m_tricks << m_roundTricks;
        const auto score = scoreRound(m_roundTricks);
        if (m_interactive)
            qCInfo(klaverjasGame) << "Round scores: " << score;
        for (Team* team : m_teams) {
            team->addPoints(score[team]);
        }
        m_roundTricks.clear();
        m_trick = 0;
        m_turn = 0;
        m_round++;
        advancePlayer(m_dealer);
        advancePlayer(m_eldest);
        m_currentPlayer = m_eldest;
        // Set up for the next round only if we are interactive. Otherwise,
        // stop advancing here to make the AI simulation terminate.
        if (m_interactive) {
            m_biddingPhase = true;
            deal();
        }
    }
    m_waiting = false;
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

Player* Game::nextPlayer(Player* player) const
{
    int index = m_players.indexOf(player);
    return m_players.at(++index % 4);
}

void Game::advancePlayer(Player*& player) const
{
    player = nextPlayer(player);
}
