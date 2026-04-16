//
// Created by sigris on 13.04.2026.
//

#pragma once
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>
#include <QVariantList>
#include <QJsonObject>

class GameClient : public QObject {
Q_OBJECT
    Q_PROPERTY(QVariantList tiles READ tiles NOTIFY stateChanged)
    Q_PROPERTY(QVariantList units READ units NOTIFY stateChanged)
    Q_PROPERTY(int balance READ balance NOTIFY stateChanged)
    Q_PROPERTY(QString status READ status NOTIFY stateChanged)
    Q_PROPERTY(bool connected READ connected NOTIFY connectionChanged)
    Q_PROPERTY(int currentPlayer READ currentPlayer NOTIFY stateChanged)
    Q_PROPERTY(int myTribeId READ myTribeId NOTIFY myTribeIdChanged)
    Q_PROPERTY(bool isMyTurn READ isMyTurn NOTIFY stateChanged)
    Q_PROPERTY(QVariantList availableUnits READ availableUnits NOTIFY stateChanged)
    Q_PROPERTY(bool isTribeConfirmed READ isTribeConfirmed NOTIFY tribeConfirmedChanged)

public:
    explicit GameClient(QObject* parent = nullptr);

    Q_INVOKABLE void connectToServer(const QString& host, int port);
    Q_INVOKABLE void login(const QString& nick, const QString& password);
    Q_INVOKABLE void registerUser(const QString& nick, const QString& password);
    Q_INVOKABLE void joinGame(int gameId);
    Q_INVOKABLE void createGame();
    Q_INVOKABLE void selectTribe(int tribeId);

    Q_INVOKABLE void sendAction(int actingIndex, int mainActionIndex, int confirmIndex,
                                           int fromX, int fromY, int toX, int toY);
    Q_INVOKABLE void moveUnit(int fromX, int fromY, int toX, int toY);
    Q_INVOKABLE void recruitUnit(int unitCode, int cityX, int cityY);
    Q_INVOKABLE void researchTech(int techX, int techY);
    Q_INVOKABLE void endTurn();

    Q_INVOKABLE void startPolling(int intervalMs = 500);
    Q_INVOKABLE void stopPolling();
    Q_INVOKABLE void leaveGame();
    Q_INVOKABLE void reconnect();
    Q_INVOKABLE void setToken(const QString& token) { m_token = token; }

    QVariantList tiles() const { return m_tiles; }
    QVariantList units() const { return m_units; }
    int balance() const { return m_balance; }
    int currentPlayer() const { return m_currentPlayer; }
    QString status() const { return m_status; }
    bool connected() const { return m_connected; }
    int myTribeId() const { return m_myTribeId; }
    bool isMyTurn() const { return m_currentPlayer == m_myTribeId && m_myTribeId > 0; }
    QVariantList availableUnits() const { return m_availableUnits; }
    bool isTribeConfirmed() const { return m_isTribeConfirmed; }
    void updatePollingStrategy();

signals:
    void stateChanged();
    void connectionChanged();
    void errorOccurred(const QString& msg);
    void loginSuccess();
    void gameJoined();
    void gameCreated(int gameId);
    void gameLeft();
    void myTribeIdChanged();
    void tribeConfirmedChanged();
    void inGameChanged();

private slots:
    void onReplyFinished(QNetworkReply* reply);
    void pollState();

private:
    friend class TestGameClient;
    QNetworkAccessManager* m_network = nullptr;
    QTimer m_pollTimer;
    QString m_host;
    int m_port = 8080;
    QString m_token;
    int m_gameId = -1;
    bool m_connected = false;
    bool m_hasJoinedGame = false;
    int m_myTribeId = 0;
    bool m_isTribeConfirmed = false;
    int m_pollIntervalMs = 500;

    QVariantList m_tiles;
    QVariantList m_units;
    QVariantList m_availableUnits;
    int m_balance = 0;
    int m_currentPlayer = 1;
    QString m_status = "disconnected";

    void post(const QString& endpoint, const QJsonObject& body);
    void fetchGameState();
    void parseState(const QJsonObject& json);

    void resetGameState();
    bool m_useLongPoll = false;
    QNetworkReply* m_currentStateRequest = nullptr;
    int m_pendingRequests = 0;
    static constexpr int MAX_PENDING = 15;
};