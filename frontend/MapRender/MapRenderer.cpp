//
// Created by sigris on 13.04.2026.
//

#include "MapRenderer.h"
#include <QJsonArray>

MapRenderer::MapRenderer(QQuickItem *parent) : QQuickPaintedItem(parent) {
    setAcceptedMouseButtons(Qt::AllButtons);
    setAcceptHoverEvents(false);
    m_tileCache.resize(m_mapSize * m_mapSize);
}

void MapRenderer::paint(QPainter *p) {
    p->setRenderHint(QPainter::Antialiasing);
    p->fillRect(boundingRect(), QColor("#0c0c1a"));
    drawTerrain(p);
    drawResources(p);
    drawCities(p);
    drawUnits(p);
    drawSelection(p);
}

void MapRenderer::buildTileCache() {
    m_tileCache.assign(m_mapSize * m_mapSize, TileData{});
    for (const auto &v : m_tiles) {
        QVariantMap m = v.toMap();
        int x = m["x"].toInt(), y = m["y"].toInt();
        if (x >= 0 && x < m_mapSize && y >= 0 && y < m_mapSize) {
            TileData &td = m_tileCache[x + y * m_mapSize];
            td.terrain = m.value("terrain", "Field").toString();
            td.resources = m["resources"].toJsonArray();
            td.hasCity = m["hasCity"].toBool();
            td.cityOwner = m["cityOwner"].toInt();
            td.hasUnit = m["hasUnit"].toBool();
        }
    }
}

const MapRenderer::TileData* MapRenderer::getTileAt(int x, int y) const {
    if (x < 0 || x >= m_mapSize || y < 0 || y >= m_mapSize) return nullptr;
    return &m_tileCache[x + y * m_mapSize];
}

QPointF MapRenderer::screenToTile(const QPointF &s) const {
    return QPointF(qFloor((s.x() - m_offset.x()) / m_zoom),
                   qFloor((s.y() - m_offset.y()) / m_zoom));
}

void MapRenderer::drawTerrain(QPainter *p) {
    for (int y = 0; y < m_mapSize; ++y) {
        for (int x = 0; x < m_mapSize; ++x) {
            const TileData *td = getTileAt(x, y);
            QString terrain = td ? td->terrain : "Field";
            QRectF r(m_offset.x() + x * m_zoom, m_offset.y() + y * m_zoom, m_zoom - 1, m_zoom - 1);
            p->fillRect(r, getTerrainColor(terrain));
            p->setPen(QPen(QColor("#ffffff").darker(150), 0.5));
            p->drawRect(r);
        }
    }
}

void MapRenderer::drawResources(QPainter *p) {
    for (int y = 0; y < m_mapSize; ++y) {
        for (int x = 0; x < m_mapSize; ++x) {
            const TileData *td = getTileAt(x, y);
            if (!td || td->resources.isEmpty()) continue;
            qreal px = m_offset.x() + x * m_zoom + 4;
            qreal py = m_offset.y() + y * m_zoom + 4;
            qreal sz = qMax(4.0, m_zoom / 6);
            for (int i = 0; i < td->resources.size(); ++i) {
                p->setBrush(getResourceColor(td->resources[i].toString()));
                p->setPen(Qt::NoPen);
                p->drawEllipse(QPointF(px + i * (sz + 2), py), sz, sz);
            }
        }
    }
}

void MapRenderer::drawCities(QPainter *p) {
    for (int y = 0; y < m_mapSize; ++y) {
        for (int x = 0; x < m_mapSize; ++x) {
            const TileData *td = getTileAt(x, y);
            if (!td || !td->hasCity) continue;

            QColor cityColor;
            if (m_myTribeId > 0 && td->cityOwner == m_myTribeId) {
                cityColor = QColor("#00e676");
            } else if (td->cityOwner > 0) {
                cityColor = QColor("#ff5252");
            } else {
                cityColor = QColor("#7f8c8d");
            }

            qreal px = m_offset.x() + x * m_zoom;
            qreal py = m_offset.y() + y * m_zoom;
            qreal sz = m_zoom * 0.7;
            QRectF r(px + (m_zoom - sz) / 2, py + (m_zoom - sz) / 2, sz, sz);

            p->setBrush(cityColor);
            p->setPen(QPen(Qt::black, 2));
            p->drawRect(r);
            if (m_zoom > 20) {
                p->setPen(Qt::black);
                p->setFont(QFont("Arial", sz / 1.5));
                p->drawText(r, Qt::AlignCenter, "🏰");
            }
        }
    }
}

void MapRenderer::drawUnits(QPainter *p) {
    for (const auto &v : m_units) {
        QVariantMap u = v.toMap();
        int x = u["x"].toInt(), y = u["y"].toInt(), tr = u["tribe"].toInt();
        QString tp = u["type"].toString();
        qreal cx = m_offset.x() + x * m_zoom + m_zoom / 2;
        qreal cy = m_offset.y() + y * m_zoom + m_zoom / 2;
        qreal r = m_zoom / 3.5;

        QColor unitColor = (tr == m_myTribeId) ? QColor("#00e676") : QColor("#e63946");
        p->setBrush(unitColor);
        p->setPen(QPen(Qt::white, 1.5));
        p->drawEllipse(QPointF(cx, cy), r, r);

        if (m_zoom > 20 && !tp.isEmpty() && tp != "None") {
            p->setPen(Qt::white);
            p->setFont(QFont("Arial", r * 1.2));
            p->drawText(QPointF(cx - r / 2.5, cy + r / 2), tp.left(1).toUpper());
        }
    }
}

void MapRenderer::drawSelection(QPainter *p) {
    if (m_selectedUnitPos.x() < 0 || m_selectedUnitPos.y() < 0) return;
    int x = (int)m_selectedUnitPos.x(), y = (int)m_selectedUnitPos.y();
    qreal cx = m_offset.x() + x * m_zoom + m_zoom / 2;
    qreal cy = m_offset.y() + y * m_zoom + m_zoom / 2;
    p->setPen(QPen(QColor("#00ff00"), 3));
    p->setBrush(Qt::NoBrush);
    p->drawEllipse(QPointF(cx, cy), m_zoom / 2.5, m_zoom / 2.5);
}

QColor MapRenderer::getTerrainColor(const QString &t) const {
    if (t == "Forest") return QColor("#2d6a4f");
    if (t == "Mountain") return QColor("#6c757d");
    if (t == "Water") return QColor("#48cae4");
    if (t == "DeepWater") return QColor("#0077b6");
    return QColor("#e9c46a");
}
QColor MapRenderer::getResourceColor(const QString &r) const {
    if (r == "Forest") return QColor("#52b788");
    if (r == "Fish") return QColor("#90e0ef");
    if (r == "Mining") return QColor("#adb5bd");
    if (r == "Animal") return QColor("#d4a373");
    if (r == "Fruit") return QColor("#ff9f1c");
    if (r == "Farm") return QColor("#80ed99");
    return QColor("#fff");
}

void MapRenderer::setMapSize(int s) { if(m_mapSize!=s){m_mapSize=s;m_tileCache.resize(s*s);emit mapSizeChanged();update();} }
void MapRenderer::setZoom(qreal z){m_zoom=qBound(10.0,z,150.0);emit zoomChanged();update();}
void MapRenderer::setTiles(const QVariantList &t){m_tiles=t;buildTileCache();emit tilesChanged();update();}
void MapRenderer::setUnits(const QVariantList &u){m_units=u;emit unitsChanged();update();}
void MapRenderer::setSelectedUnitPos(QPointF p){m_selectedUnitPos=p;emit selectedUnitPosChanged();update();}
void MapRenderer::setMyTribeId(int id){if(m_myTribeId!=id){m_myTribeId=id;emit myTribeIdChanged();update();}}

void MapRenderer::mousePressEvent(QMouseEvent *e) {
    if (e->button() == Qt::LeftButton) { m_startPos = e->position(); m_lastMousePos = e->position(); m_isDragging = false; e->accept(); }
}
void MapRenderer::mouseMoveEvent(QMouseEvent *e) {
    if (e->buttons() & Qt::LeftButton) {
        if (QLineF(m_startPos, e->position()).length() > 5.0) {
            if (!m_isDragging) { m_isDragging = true; }
            QPointF delta = e->position() - m_lastMousePos;
            m_offset += delta; m_lastMousePos = e->position(); update(); e->accept();
        }
    }
}
void MapRenderer::mouseReleaseEvent(QMouseEvent *e) {
    if (e->button() == Qt::LeftButton) {
        if (!m_isDragging) {
            QPointF t = screenToTile(e->position());
            if (t.x() >= 0 && t.x() < m_mapSize && t.y() >= 0 && t.y() < m_mapSize)
                    emit tileClicked(t.x(), t.y());
        }
        m_isDragging = false; e->accept();
    }
}
void MapRenderer::wheelEvent(QWheelEvent *e) {
    QPoint d = e->pixelDelta(); if (d.isNull()) d = e->angleDelta() / 8;
    m_offset += QPointF(d.x(), d.y()); update(); e->accept();
}