//
// Created by sigris on 13.04.2026.
//

#pragma once
#include <QQuickPaintedItem>
#include <QPainter>
#include <QVariantList>
#include <QPointF>
#include <QJsonArray>
#include <QVector>

class MapRenderer : public QQuickPaintedItem {
Q_OBJECT
    Q_PROPERTY(int mapSize READ mapSize WRITE setMapSize NOTIFY mapSizeChanged)
    Q_PROPERTY(qreal zoom READ zoom WRITE setZoom NOTIFY zoomChanged)
    Q_PROPERTY(QVariantList tiles READ tiles WRITE setTiles NOTIFY tilesChanged)
    Q_PROPERTY(QVariantList units READ units WRITE setUnits NOTIFY unitsChanged)
    Q_PROPERTY(QPointF selectedUnitPos READ selectedUnitPos WRITE setSelectedUnitPos NOTIFY selectedUnitPosChanged)
    Q_PROPERTY(int myTribeId READ myTribeId WRITE setMyTribeId NOTIFY myTribeIdChanged)

public:
    explicit MapRenderer(QQuickItem *parent = nullptr);
    void paint(QPainter *painter) override;

    int mapSize() const { return m_mapSize; }
    void setMapSize(int size);
    qreal zoom() const { return m_zoom; }
    void setZoom(qreal z);
    QVariantList tiles() const { return m_tiles; }
    void setTiles(const QVariantList &tiles);
    QVariantList units() const { return m_units; }
    void setUnits(const QVariantList &units);
    QPointF selectedUnitPos() const { return m_selectedUnitPos; }
    void setSelectedUnitPos(QPointF pos);
    int myTribeId() const { return m_myTribeId; }
    void setMyTribeId(int id);

    Q_INVOKABLE QPointF screenToTile(const QPointF &screenPos) const;

signals:
    void tileClicked(int x, int y);
    void mapSizeChanged();
    void zoomChanged();
    void tilesChanged();
    void unitsChanged();
    void selectedUnitPosChanged();
    void myTribeIdChanged();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    int m_mapSize = 20;
    qreal m_zoom = 40.0;
    QPointF m_offset;
    QPointF m_selectedUnitPos = QPointF(-1, -1);
    int m_myTribeId = 0;

    QVariantList m_tiles;
    QVariantList m_units;

    struct TileData {
        QString terrain = "Field";
        QJsonArray resources;
        bool hasCity = false;
        int cityOwner = -1;
        bool hasUnit = false;
    };
    QVector<TileData> m_tileCache;

    void buildTileCache();
    const TileData* getTileAt(int x, int y) const;

    void drawTerrain(QPainter *p);
    void drawResources(QPainter *p);
    void drawCities(QPainter *p);
    void drawUnits(QPainter *p);
    void drawSelection(QPainter *p);
    QColor getTerrainColor(const QString &t) const;
    QColor getResourceColor(const QString &r) const;

    bool m_isDragging = false;
    QPointF m_startPos;
    QPointF m_lastMousePos;
};