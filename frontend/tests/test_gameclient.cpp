//
// Created by sigris on 16.04.2026.
//

#include <QtTest>
#include <QSignalSpy>
#include "GameClient/GameClient.h"

class TestGameClient : public QObject {
Q_OBJECT

private slots:
    void testParseState_FullGame() {
        GameClient client;

        client.m_myTribeId = 2;
        client.m_isTribeConfirmed = true;

        QJsonObject stateJson = {
                {"game_id", 15}, {"status", "playing"}, {"current_player", 2},
                {"map_size", 20}, {"players_count", 2}, {"tribe_id", 2},
                {"balance", 21}, {"available_units", QJsonArray{"Warrior", "Archer"}},
                {"tiles", QJsonArray{
                        QJsonObject{{"x",0},{"y",0},{"terrain","Field"},{"owner",-1},{"hasCity",false},{"hasUnit",false}},
                        QJsonObject{{"x",5},{"y",6},{"terrain","Field"},{"owner",2},{"hasCity",true},{"cityOwner",2},{"hasUnit",true}}
                }},
                {"units", QJsonArray{
                        QJsonObject{{"x",5},{"y",6},{"type","Warrior"},{"tribe",2},{"health",10},{"movement",1},{"attackRange",1}}
                }}
        };

        QSignalSpy spy(&client, &GameClient::stateChanged);
        client.parseState(stateJson);
        QTest::qWait(10);

        QCOMPARE(spy.count(), 1);
        QCOMPARE(client.balance(), 21);
        QCOMPARE(client.currentPlayer(), 2);
        QCOMPARE(client.myTribeId(), 2);
        QCOMPARE(client.isMyTurn(), true);
        QCOMPARE(client.tiles().size(), 2);
        QCOMPARE(client.units().size(), 1);
        QCOMPARE(client.availableUnits().size(), 2);
    }

    void testPollingStrategy_Adaptive() {
        GameClient client;
        client.m_myTribeId = 2;
        client.m_isTribeConfirmed = true;
        client.m_pollIntervalMs = 1500;
        client.startPolling(1500);

        client.parseState({{"current_player", 2}, {"tribe_id", 2}});
        QTest::qWait(10);
        QCOMPARE(client.m_pollIntervalMs, 300);

        client.parseState({{"current_player", 1}, {"tribe_id", 2}});
        QTest::qWait(10);
        QVERIFY(client.m_pollIntervalMs >= 1000);
    }

    void testBacklogControl_Limit() {
        GameClient client;
        client.m_pendingRequests = 14;

        client.m_pendingRequests++;
        QCOMPARE(client.m_pendingRequests, 15);
        QVERIFY(client.m_pendingRequests >= GameClient::MAX_PENDING);

        client.m_pendingRequests--;
        QCOMPARE(client.m_pendingRequests, 14);

        QVERIFY(client.m_pendingRequests < GameClient::MAX_PENDING);
    }

    void testIsMyTurn_Logic() {
        GameClient client;

        client.m_currentPlayer = 2; client.m_myTribeId = 2; client.m_isTribeConfirmed = true;
        QVERIFY(client.isMyTurn());

        client.m_currentPlayer = 1;
        QVERIFY(!client.isMyTurn());

        client.m_currentPlayer = 2; client.m_myTribeId = 0;
        QVERIFY(!client.isMyTurn());
    }

    void testLeaveGame_ResetsState() {
        GameClient client;
        client.m_gameId = 15;
        client.m_hasJoinedGame = true;
        client.m_myTribeId = 2;
        client.m_isTribeConfirmed = true;
        client.m_balance = 100;
        client.m_tiles.append(QVariantMap{{"x",0},{"y",0}});
        client.m_units.append(QVariantMap{{"x",5},{"y",6}});
        client.m_pollTimer.start(500);

        QSignalSpy leftSpy(&client, &GameClient::gameLeft);
        client.leaveGame();
        QTest::qWait(10);

        QCOMPARE(leftSpy.count(), 1);
        QCOMPARE(client.m_gameId, -1);
        QCOMPARE(client.m_hasJoinedGame, false);
        QCOMPARE(client.m_myTribeId, 0);
        QCOMPARE(client.m_isTribeConfirmed, false);
        QCOMPARE(client.m_balance, 0);
        QCOMPARE(client.m_tiles.isEmpty(), true);
        QCOMPARE(client.m_units.isEmpty(), true);
        QVERIFY(!client.m_pollTimer.isActive());
    }

    void testParseState_EdgeCases() {
        GameClient client;

        client.parseState(QJsonObject{});
        QCOMPARE(client.status(), QString(""));

        client.parseState({{"balance", 42}, {"current_player", 3}});
        QCOMPARE(client.balance(), 42);
        QCOMPARE(client.currentPlayer(), 3);

        client.parseState({{"balance", "not_a_number"}, {"tiles", QJsonArray{QJsonObject{{"x","bad"}}}}});
        QCOMPARE(client.balance(), 0);
        QCOMPARE(client.tiles().size(), 1);
    }

    void testActionGuard_NotMyTurn() {
        GameClient client;
        client.m_gameId = 15;
        client.m_currentPlayer = 1;
        client.m_myTribeId = 2;
        client.m_isTribeConfirmed = true;

        QSignalSpy errorSpy(&client, &GameClient::errorOccurred);
        client.moveUnit(5, 6, 5, 7);
        QTest::qWait(10);

        QCOMPARE(errorSpy.count(), 1);
        QVERIFY(errorSpy.takeFirst().at(0).toString().contains("not your turn", Qt::CaseInsensitive));
    }
};

QTEST_MAIN(TestGameClient)
#include "test_gameclient.moc"