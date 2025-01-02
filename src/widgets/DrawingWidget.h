#ifndef DRAWINGWIDGET_H
#define DRAWINGWIDGET_H

#include <QWidget>
#include <QImage>
#include <QPoint>
#include <QPainter>
#include <QMouseEvent>
#include <QApplication>
#include <QScreen>
#include <iostream>

#include <QtWidgets>

#include <QImage>
#include <QPainter>


#define ERASER 0
#define PEN 1
#define MARKER 2

#define NORMAL 0
#define DOTLINE 1
#define LINELINE 2

#include "../utils/Selection.h"
#include "FloatingSettings.h"

#define LINE 0
#define CIRCLE 1
#define SPLINE 2
#define RECTANGLE 3
#define TRIANGLE 4

#define DRAW 0
#define SELECTION 1

#define PRESS 0
#define MOVE 1
#define RELEASE 2

class ValueStorage {
public:
    int size() {
        return values.size();
    }
    
    void clear() {
        return values.clear();
    }

    void saveValue(qint64 id, QPointF data) {
        values[id] = data;
    }
    QPointF last() {
        return loadValue(values.size()-1);
    }

    QPointF first() {
        return loadValue(0);
    }

    QPointF loadValue(qint64 id) {
        if (values.contains(id)) {
            return values[id];
        } else {
            // -1 -1 means disable drawing
            return QPointF(-1,-1);
        }
    }
    QMap<qint64, QPointF> values;
};

class GeometryStorage {
public:
    QPointF point;
    QPointF point_last;

    void addValue(qint64 id, QPointF data) {
        values[id].saveValue(values[id].size(), data);
    }
    
    void saveValue(qint64 id, qint64 id2, QPointF data) {
        values[id].saveValue(id2, data);
    }
    
    QPointF last(qint64 id){
        return load(id).last();
    }
    
    QPointF first(qint64 id){
        return load(id).first();
    }
    
    int size(qint64 id) {
        return values[id].size();
    } 

    ValueStorage load(qint64 id) {
        if (values.contains(id)) {
            return values[id];
        } else {
            // -1 -1 means disable drawing
            ValueStorage v;
            return v;
        }
    }
    void clear(qint64 id) {
        return values[id].clear();
    }

private:
    QMap<qint64, ValueStorage> values;
};

class DrawingWidget : public QWidget {
public:
    explicit DrawingWidget(QWidget *parent = nullptr);

    QImage image;
    QColor penColor;
    MovableWidget* cropWidget;
    int penSize[3];
    void initializeImage(const QSize &size);
    void goPrevious();
    void goPage(int i);
    void goNext();
    void goPreviousPage();
    void goNextPage();
    void clear();
#ifdef LIBARCHIVE
    void saveAll(QString filename);
    void loadArchive(const QString& filename);
#endif
    int penType;
    int penStyle;
    int lineStyle;
    int penMode;
    void syncPageType(int type);
    int getPageNum();
    bool isBackAvailable();
    bool isNextAvailable();
    void loadImage(int num);
    void mergeSelection();
    void clearSelection();
    void addImage(QImage img);
    void eventHandler(int source, int type, int id, QPointF pos, float pressure);

protected:
    float fpressure;
    QImage imageBackup;
    bool eraser;
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void updateCursorMouse(qint64 i, QPoint pos);
    void updateCursorMouse(qint64 i, QPointF pos);
    void createSelection();
    void drawLineToFunc(qint64 id, qreal pressure);
    void selectionDraw(QPointF startPoint, QPointF endPoint);
    void addPoint(int id, QPointF data);
    bool event(QEvent * ev) override;
    float normalizePressure(float pressure);
    GeometryStorage geo;
    QPainter painter;
};

QColor convertColor(QColor color);
void qImageToFile(const QImage& image, const QString& filename);

#endif // DRAWINGWIDGET_H

