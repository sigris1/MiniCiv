//
// Created by sigris on 12.04.2026.
//

#include "GameClient.h"
#include <QDebug>
#include <QUrl>
#include <QJsonArray>

GameClient::GameClient(QObject* parent) : QObject(parent) {
    m_network = new QNetworkAccessManager(this);
    connect(m_network, &QNetworkAccessManager::finished, this, &GameClient::onReplyFinished);
}

void GameClient::connectToServer(const QString& host, int port) {
    m_host = host; m_port = port; m_connected = true; m_status = "connected";
    emit connectionChanged();
}

void GameClient::login(const QString& nick, const QString& password) {
    post("/api/auth/login", {{"nick", nick}, {"password", password}});
}
void GameClient::registerUser(const QString& nick, const QString& password) {
    post("/api/auth/register", {{"nick", nick}, {"password", password}});
}

void GameClient::joinGame(int gameId) {
    if (m_hasJoinedGame) return;
    m_gameId = gameId;
    qDebug() << "[Client] Requesting join Game ID:" << gameId;
    post("/api/game/join", {{"game_id", gameId}});
}

void GameClient::createGame() {
    if (m_hasJoinedGame) return;
    qDebug() << "[Client] Requesting create New Game";
    post("/api/game/create", {});
}

void GameClient::selectTribe(int tribeId) {
    if (m_gameId <= 0) return;
    qDebug() << "[Client] Selecting Tribe ID:" << tribeId << "for Game:" << m_gameId;

    QJsonObject body;
    body["game_id"] = m_gameId;
    body["tribe_id"] = tribeId;
    post("/api/game/tribe/select", body);

    m_myTribeId = tribeId;
    emit myTribeIdChanged();
}

void GameClient::sendAction(int actingIndex, int mainActionIndex, int confirmIndex,
                            int fromX, int fromY, int toX, int toY) {
    if (!isMyTurn()) {
        emit errorOccurred("It's not your turn yet!");
        return;
    }

    QJsonObject json;
    json["game_id"] = m_gameId;

    json["acting"] = actingIndex;

    json["mainAction"] = mainActionIndex;

    json["confirmAction"] = confirmIndex;
    json["fromX"] = fromX;
    json["fromY"] = fromY;
    json["toX"] = toX;
    json["toY"] = toY;

    qDebug() << "[Action >>>] Sending:" << json;
    post("/api/game/action", json);
}

void GameClient::moveUnit(int fromX, int fromY, int toX, int toY) {
    sendAction(0, 0, 0, fromX, fromY, toX, toY);
}

void GameClient::recruitUnit(int unitTypeIndex, int cityX, int cityY) {
    sendAction(3, 8, 0, cityY, cityX, unitTypeIndex + 1, 0);
}

void GameClient::researchTech(int techX, int techY) {
    sendAction(1, 3, 0, 0, 0, techX, techY);
}

void GameClient::endTurn() {
    if (!isMyTurn()) {
        emit errorOccurred("It's not your turn!");
        return;
    }
    qDebug() << "[Action >>>] Sending End Turn (acting=Tribe, mainAction=TribeEndTurn)";
    sendAction(1, 5, 0, 0, 0, 0, 0);
}

void GameClient::startPolling(int intervalMs) {
    if (m_pollTimer.isActive()) return;

    m_pollIntervalMs = intervalMs;
    qDebug() << "[Client] Starting Polling (initial:" << m_pollIntervalMs << "ms)";

    connect(&m_pollTimer, &QTimer::timeout, this, &GameClient::pollState, Qt::UniqueConnection);
    m_pollTimer.setInterval(m_pollIntervalMs);
    m_pollTimer.start();
}

void GameClient::stopPolling() { m_pollTimer.stop(); }

void GameClient::leaveGame() {
    qDebug() << "[Client] Leaving Game -> Returning to Lobby";
    stopPolling();
    m_pendingRequests = 0;
    resetGameState();
    emit gameLeft();
}

void GameClient::reconnect() {
    if (m_gameId > 0 && !m_token.isEmpty()) {
        qDebug() << "[Client] Reconnecting to Game ID:" << m_gameId;
        fetchGameState();
    }
}

void GameClient::fetchGameState() {
    if (m_gameId <= 0 || m_token.isEmpty()) return;

    if (m_pendingRequests >= MAX_PENDING) {
        qDebug() << "[Client] Backlog full (" << m_pendingRequests
                 << "), skipping state fetch";
        return;
    }

    QUrl url(QString("http://%1:%2/api/games/%3/state")
                     .arg(m_host).arg(m_port).arg(m_gameId));
    QNetworkRequest request(url);
    request.setRawHeader("Authorization", ("Bearer " + m_token).toUtf8());

    auto reply = m_network->get(request);

    m_pendingRequests++;
    qDebug() << "[Client] Request sent. Pending:" << m_pendingRequests;

    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        reply->deleteLater();
        m_pendingRequests--;
        qDebug() << "[Client] Request done. Pending:" << m_pendingRequests;

        if (m_pendingRequests < MAX_PENDING / 2 && m_pollTimer.isActive()) {
            qDebug() << "[Client] Backlog cleared, fetching fresh state...";
            fetchGameState();
        }
    });
}
void GameClient::pollState() { fetchGameState(); }

void GameClient::post(const QString& endpoint, const QJsonObject& body) {
    QUrl url(QString("http://%1:%2%3").arg(m_host).arg(m_port).arg(endpoint));
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    bool isAuth = endpoint.startsWith("/api/auth/");
    if (!isAuth && !m_token.isEmpty()) {
        request.setRawHeader("Authorization", ("Bearer " + m_token).toUtf8());
    }
    m_network->post(request, QJsonDocument(body).toJson());
}

void GameClient::onReplyFinished(QNetworkReply* reply) {
    reply->deleteLater();
    int httpStatus = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    QString reqUrl = reply->url().toString();

    if (reply == m_currentStateRequest) {
        m_currentStateRequest = nullptr;
    }
    qDebug() << "[Client] <<< Response:" << reqUrl << "Status:" << httpStatus;

    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "[Error <<<] Network on" << reqUrl << ":" << reply->errorString();
        if (reply->error() == QNetworkReply::ConnectionRefusedError) {
            emit errorOccurred("Server connection lost. Click 'Reconnect' to retry.");
        }
        return;
    }

    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll(), &err);
    if (err.error != QJsonParseError::NoError || doc.isNull()) {
        qDebug() << "[Error <<<] Invalid JSON from" << reqUrl;
        return;
    }

    QJsonObject json = doc.object();

    if (!reqUrl.contains("/state")) {
        qDebug() << "[Response <<<]" << reqUrl << json;
    }

    if (json.contains("token")) { m_token = json["token"].toString(); emit loginSuccess(); }
    if (json.contains("error")) {
        qDebug() << "[Error <<<] Server Logic:" << json["error"].toString();
        emit errorOccurred(json["error"].toString());
    }

    if (json.contains("tribe_id")) {
        int serverTribeId = json["tribe_id"].toInt();
        if (serverTribeId > 0) {
            if (m_myTribeId != serverTribeId) {
                m_myTribeId = serverTribeId;
                emit myTribeIdChanged();
            }
            if (!m_isTribeConfirmed) {
                m_isTribeConfirmed = true;
                emit tribeConfirmedChanged();
                qDebug() << "[Client] Tribe confirmed by server:" << m_myTribeId;
                startPolling(500);
            }
        }
    }

    if (reqUrl.contains("/api/game/action") && reply->error() == QNetworkReply::NoError) {
        qDebug() << "[Client] Action successful, fetching updated state...";
        QTimer::singleShot(100, this, &GameClient::fetchGameState);
        return;
    }

    if (!m_hasJoinedGame && reqUrl.contains("/api/game/create")) {
        if (!json.contains("error") && json.contains("game_id")) {
            m_gameId = json["game_id"].toInt();
            m_hasJoinedGame = true;
            qDebug() << "[Client] Game created with ID:" << m_gameId;
            emit gameCreated(m_gameId);
            return;
        }
    }

    if (!m_hasJoinedGame && reqUrl.contains("/api/game/join")) {
        if (!json.contains("error")) {
            if (json.contains("game_id")) m_gameId = json["game_id"].toInt();
            m_hasJoinedGame = true;
            qDebug() << "[Client] Game joined!";
            emit gameJoined();
        }
    }

    if (reqUrl.contains("/state")) {
        static bool firstState = true;
        if (firstState) {
            qDebug() << "[State <<<] First raw response:" << QJsonDocument(json).toJson(QJsonDocument::Compact);
            firstState = false;
        }
        parseState(json);
    }
}

void GameClient::parseState(const QJsonObject& json) {
    m_status = json["status"].toString();
    m_currentPlayer = json["current_player"].toInt(1);
    m_balance = json["balance"].toInt(0);

    if (json.contains("available_units") && json["available_units"].isArray()) {
        m_availableUnits.clear();
        for (const auto& val : json["available_units"].toArray()) {
            m_availableUnits.append(val.toVariant());
        }
    }

    QJsonArray tilesArr;
    if (json.contains("tiles")) tilesArr = json["tiles"].toArray();
    else if (json.contains("map")) tilesArr = json["map"].toArray();

    if (!tilesArr.isEmpty()) {
        m_tiles.clear();
        for (const auto& val : tilesArr) {
            QJsonObject o = val.toObject();
            m_tiles.append(QVariantMap{
                    {"x", o["x"].toInt()}, {"y", o["y"].toInt()},
                    {"terrain", o["terrain"].toString()}, {"owner", o["owner"].toInt(-1)},
                    {"resources", o["resources"].toArray()},
                    {"hasCity", o["hasCity"].toBool(false)}, {"cityOwner", o["cityOwner"].toInt(-1)},
                    {"hasUnit", o["hasUnit"].toBool(false)}
            });
        }
    }

    QJsonArray unitsArr;
    if (json.contains("units")) unitsArr = json["units"].toArray();
    else if (json.contains("army")) unitsArr = json["army"].toArray();

    if (!unitsArr.isEmpty()) {
        m_units.clear();
        for (const auto& val : unitsArr) {
            QJsonObject o = val.toObject();
            m_units.append(QVariantMap{
                    {"x", o["x"].toInt()}, {"y", o["y"].toInt()},
                    {"type", o["type"].toString()}, {"tribe", o["tribe"].toInt()},
                    {"health", o["health"].toInt(100)}
            });
        }
    }

    qDebug() << "[State Parsed] Tiles:" << m_tiles.size() << "| Units:" << m_units.size()
             << "| Turn:" << m_currentPlayer << "| MyTribe:" << m_myTribeId
             << "| Confirmed:" << m_isTribeConfirmed;
    emit stateChanged();
    updatePollingStrategy();
}

void GameClient::resetGameState() {
    qDebug() << "[Client] Resetting game state...";
    m_gameId = -1;
    m_hasJoinedGame = false;
    m_myTribeId = 0;
    m_isTribeConfirmed = false;
    m_currentPlayer = 1;
    m_balance = 0;
    m_status = "lobby";
    m_tiles.clear();
    m_units.clear();
    m_availableUnits.clear();
    emit stateChanged();
    emit myTribeIdChanged();
    emit tribeConfirmedChanged();
}

void GameClient::updatePollingStrategy() {
    int newInterval = (m_currentPlayer == m_myTribeId && m_isTribeConfirmed) ? 300 : 1000;

    if (newInterval != m_pollIntervalMs) {
        m_pollIntervalMs = newInterval;
        if (m_pollTimer.isActive()) {
            m_pollTimer.setInterval(m_pollIntervalMs);
            qDebug() << "[Client] Poll interval:" << m_pollIntervalMs << "ms (turn:" << m_currentPlayer << ")";
        }
    }
}