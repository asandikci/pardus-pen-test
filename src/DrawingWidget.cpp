#include "DrawingWidget.h"
#include "WhiteBoard.h"

#ifdef QPRINTER
#include <QPrinter>
#endif
#ifdef LIBARCHIVE
#include "Archive.h"
#endif
#include <stdio.h>


#include <stdlib.h>
#include <locale.h>
#include <libintl.h>

#define _(String) gettext(String)

extern "C" {
#include "settings.h"
}

extern WhiteBoard *board;
extern QWidget * mainWidget;
extern QMainWindow* mainWindow;
extern DrawingWidget *drawing;

extern void updateGoBackButtons();
void removeDirectory(const QString &path);

#define padding 8

/*
penType:
 - 0 eraser
 - 1 pen
 - 2 marker
*/

#include <QDebug>
#include <QMap>

#ifdef QT5
#define points touchPoints
#define position pos
#endif

#ifndef HISTORY
#define HISTORY 100
#endif

class CursorStorage {
public:
    void init(qint64 id){
        if(!images.contains(id)){
            labels[id] = new QLabel("");
            labels[id]->setStyleSheet(QString("background-color: none;"));
            images[id] = new QWidget(mainWidget);
            layouts[id] = new QVBoxLayout(images[id]);
            layouts[id]->addWidget(labels[id]);
            layouts[id]->setContentsMargins(0,0,0,0);
            layouts[id]->setSpacing(0);
            sizes[id] = 0;
        }
    }
    void setPosition(qint64 id, QPointF data) {
        init(id);
        //printf("%lld move\n", id);
        images[id]->move(QPoint(
            data.x() - sizes[id]/2,
            data.y() - sizes[id]/2
        ));
        images[id]->show();
        setCursor(id, sizes[id]);
    }

    void setCursor(qint64 id, int size){
        if(sizes[id] == size){
            return;
        }
        sizes[id] = size;
        QIcon icon = QIcon(":images/cursor.svg");
        QPixmap pixmap = icon.pixmap(
            icon.actualSize(
                QSize(size, size)
            )
        );
        //printf("%lld resize %d\n", id, size);
        labels[id]->setFixedSize(size, size);
        images[id]->setFixedSize(size, size);
        labels[id]->setPixmap(pixmap);
    }

    void hide(qint64 id){
        init(id);
        images[id]->hide();
    }
    QMap<qint64, bool> drawing;

private:
    QMap<qint64, QWidget*> images;
    QMap<qint64, QLabel*> labels;
    QMap<qint64, QVBoxLayout*> layouts;
    QMap<qint64, int> sizes;
};
CursorStorage curs;

class ImageStorage {
public:
    int last_image_num = 1;
    int image_count = 0;
    int pageType = TRANSPARENT;
    int overlayType = NONE;
    int removed = 0;
    void saveValue(qint64 id, QImage data) {
        values[id] = data;
        updateGoBackButtons();
        if(id > HISTORY){
            remove(id-HISTORY);
            removed++;
        }
    }

    void clear(){
        values.clear();
        image_count = 0;
        last_image_num = 1;
        removed = 0;
        updateGoBackButtons();
    }

    QImage loadValue(qint64 id) {
        if(removed >= id) {
            id =  removed +1;
        }
        if (values.contains(id)) {
            return values[id];
        } else {
            QImage image = QImage(mainWindow->geometry().width(),mainWindow->geometry().height(), QImage::Format_ARGB32);
            image.fill(QColor("transparent"));
            return image;
        }
    }

    void remove(qint64 id){
        for (auto it = values.begin(); it != values.end(); ++it) {
            if (it.key() == id) {
                values.erase(it);
                break;
            }
        }
    }

private:
    QMap<qint64, QImage> values;
};
ImageStorage images;


class PageStorage {
public:
    int last_page_num = 0;
    int page_count = 0;
    void saveValue(qint64 id, ImageStorage data) {
        values[id] = data;
    }

    void clear(){
        values.clear();
        last_page_num = 0;
        page_count = 0;
    }
#ifdef QPRINTER
    void savePdf(const QString& filename){
        saveValue(last_page_num, images);
        QPrinter printer(QPrinter::HighResolution);
        printer.setOutputFormat(QPrinter::PdfFormat);
        printer.setOutputFileName(filename);
        printer.setFullPage(true);
        QSizeF imageSize(mainWindow->geometry().width(),mainWindow->geometry().height());
        QPageSize pageSize(imageSize, QPageSize::Point);
        printer.setPageSize(pageSize);
        printer.setResolution(72); // TODO: replace this
            
        QPainter painter(&printer);
        for(int i=0;i<=page_count;i++){
            QImage im = loadValue(i).loadValue(loadValue(i).last_image_num);
            painter.drawImage(0,0, im);
            printer.newPage();
        }
        painter.end();
        
    }
#endif
#ifdef LIBARCHIVE
    void saveAll(const QString& filename){
        images.overlayType = board->getOverlayType();
        images.pageType = board->getType();
        values[last_page_num] = images;
        QString cfg = "[main]\n";
        cfg += "width="+QString::number(mainWindow->geometry().width())+"\n";
        cfg += "height="+QString::number(mainWindow->geometry().height())+"\n";
        for(int i=0;i<=page_count;i++){
            cfg += "[page"+QString::number(i)+"]\n";
            cfg += "overlay="+QString::number(loadValue(i).overlayType)+"\n";
            cfg += "page="+QString::number(loadValue(i).pageType)+"\n";
            for(int j=1+loadValue(i).removed;j<=loadValue(i).image_count;j++){
                archive_add(QString::number(i)+"/"+QString::number(j-1-loadValue(i).removed), values[i].loadValue(j));
            }
        }
        archive_set_config(cfg);
        archive_create(filename);
    }

    void loadArchive(const QString& filename){
        QMap<QString, QImage> archive = archive_load(filename);
        QString cfg = archive_get_config();
        clear();
        for (auto it = archive.begin(); it != archive.end(); ++it) {
            QString path = it.key();
            QStringList parts = path.split("/");
            QImage image = it.value();
            int page = parts[0].toInt();
            int frame = parts[1].toInt();
            printf("Load: page: %d frame %d\n", page, frame);
            if(page > page_count){
                page_count = page;
            }
            if (!values.contains(page)) {
                ImageStorage data;
                values[page] = data;
                values[page].image_count = 0;
                values[page].last_image_num = 0;
            }
            values[page].saveValue(frame+1, image);
            values[page].image_count++;
            values[page].last_image_num = values[page].image_count;
        }
        QStringList list = cfg.split("\n");
        QString area = "main";
        int page = 0;
        for (const auto &str : std::as_const(list)) {
            if(str.startsWith("[") && str.endsWith("]")) {
                area = str.mid(1,str.length()-2);
                if(area.startsWith("page")){
                    page = area.mid(4,str.length()-1).toInt();
                }
            } else if(str.startsWith("overlay")){
                values[page].overlayType = str.split("=")[1].toInt();
                printf("Load: page: %d overlay %d\n", page, values[page].overlayType);
            } else if(str.startsWith("page")){
                values[page].pageType = str.split("=")[1].toInt();
                printf("Load: page: %d page %d\n", page, values[page].pageType);
            }
        }
        images = values[0];
        board->setType(images.pageType);
        board->setOverlayType(images.overlayType);
        drawing->loadImage(images.last_image_num);
        drawing->update();
        updateGoBackButtons();
    }
#endif

    ImageStorage loadValue(qint64 id) {
        if (id > page_count){
            page_count = id;
        }
        if (values.contains(id)) {
            return values[id];
        } else {
            ImageStorage imgs = ImageStorage();
            imgs.pageType = board->getType();
            return imgs;
        }
    }

private:
    QMap<qint64, ImageStorage> values;
};
PageStorage pages;

int curEventButtons = 0;

DrawingWidget::DrawingWidget(QWidget *parent): QWidget(parent) {
    initializeImage(size());
    penSize[PEN] = get_int((char*)"pen-size");
    penSize[ERASER] = get_int((char*)"eraser-size");
    penSize[MARKER] = get_int((char*)"marker-size");
    penType=PEN;
    penStyle=SPLINE;
    lineStyle=NORMAL;
    penColor = QColor(get_string((char*)"color"));
    penMode = DRAW;
    setMouseTracking(true);
    setAttribute(Qt::WA_AcceptTouchEvents);
    cropWidget = new MovableWidget(mainWindow);
    cropWidget->stackUnder(this);
    QBoxLayout* cropLayout = new QVBoxLayout(cropWidget);
    cropLayout->addWidget(cropWidget->crop);
    cropLayout->setContentsMargins(0,0,0,0);
    cropLayout->setSpacing(0);
    cropWidget->setStyleSheet("border: 2px solid "+penColor.name()+";");
    cropWidget->hide();

    //QScreen *screen = QGuiApplication::primaryScreen();
    fpressure = get_int((char*)"pressure") / 100.0;
}

void DrawingWidget::addPoint(int id, QPointF data) {
    if(geo.size(id) == 0) {
        geo.saveValue(id, 0, data);
        return;
    }
    if((lineStyle ==  NORMAL && penStyle == SPLINE) || penType == ERASER){
        geo.saveValue(id, 1, geo.load(id).loadValue(0));
        geo.saveValue(id, 0, data);
    } else {
        geo.addValue(id, data);
    }
}


void DrawingWidget::addImage(QImage img){
    images.last_image_num++;
    images.image_count = images.last_image_num;
    images.saveValue(images.last_image_num, img.copy());
}

void DrawingWidget::initializeImage(const QSize &size) {
    image = QImage(size, QImage::Format_ARGB32);
    image.fill(QColor("transparent"));
}

void DrawingWidget::resizeEvent(QResizeEvent *event) {
    initializeImage(event->size());
    loadImage(images.last_image_num);
    QWidget::resizeEvent(event);
}

void DrawingWidget::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    painter.begin(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter.drawImage(0, 0, image);
    painter.end();
}


void DrawingWidget::clear() {
    image.fill(QColor("transparent"));
    images.clear();
    update();
}



static QPointF last_end = QPointF(0,0);
static QPointF last_begin = QPointF(0,0);

void DrawingWidget::selectionDraw(QPointF startPoint, QPointF endPoint) {
    image = imageBackup;
    painter.begin(&image);
    painter.setPen(Qt::NoPen);
    penColor.setAlpha(127);
    painter.setBrush(QBrush(penColor));
    painter.drawRect(QRectF(startPoint,endPoint));
    painter.end();
    update(QRectF(last_begin, last_end).toRect().normalized().adjusted(-1, -1, +1, +1));
    update(QRectF(startPoint, endPoint).toRect().normalized().adjusted(-1, -1, +1, +1));

    last_begin = startPoint;
    last_end = endPoint;
}

#ifdef LIBARCHIVE
void DrawingWidget::saveAll(QString file){
    if (!file.isEmpty()) {
        if(file.endsWith(".pdf")){
            pages.savePdf(file);
            return;
        }
        if(!file.endsWith(".pen")){
            file += ".pen";
        }
        pages.saveAll(file);
    }
}

void DrawingWidget::loadArchive(const QString& filename){
    pages.loadArchive(filename);
}
#endif
void DrawingWidget::loadImage(int num){
    QImage img = images.loadValue(num);
    if(img.isNull()){
        return;
    }
    img = img.scaled(mainWindow->geometry().width(), mainWindow->geometry().height());
    QPainter p(&image);
    image.fill(QColor("transparent"));
    p.drawImage(QPointF(0,0), img);
    update();
}

void DrawingWidget::goNextPage(){
    images.overlayType = board->getOverlayType();
    images.pageType = board->getType();
    pages.saveValue(pages.last_page_num, images);
    pages.last_page_num++;
    images = pages.loadValue(pages.last_page_num);
    board->setType(images.pageType);
    board->setOverlayType(images.overlayType);
    loadImage(images.last_image_num);
}

void DrawingWidget::goPreviousPage(){
    images.overlayType = board->getOverlayType();
    images.pageType = board->getType();
    pages.saveValue(pages.last_page_num, images);
    pages.last_page_num--;
    images = pages.loadValue(pages.last_page_num);
    board->setType(images.pageType);
    board->setOverlayType(images.overlayType);
    loadImage(images.last_image_num);
}

void DrawingWidget::goPrevious(){
    if(!isBackAvailable()){
        return;
    }
    images.last_image_num--;
    loadImage(images.last_image_num);
}


void DrawingWidget::goNext(){
    if(!isNextAvailable()){
        return;
    }
    images.last_image_num++;
    loadImage(images.last_image_num);
}

static int num_of_press = 0;
void DrawingWidget::eventHandler(int source, int type, int id, QPointF pos, float pressure){
    int ev_pen = penType;
    switch(type) {
        case PRESS:
            if (curs.drawing[id] && curs.drawing.contains(id)) {
                break;
            }
            num_of_press++;
            curs.drawing[id] = true;
            mergeSelection();
            imageBackup = image;
            if(floatingSettings->isVisible()){
                floatingSettings->setHide();
            }
            geo.clear(id);
            addPoint(id, pos);
            if(penMode != DRAW) {
                return;
            }
            if(source & Qt::RightButton) {
                ev_pen = ERASER;
            }else if(source & Qt::MiddleButton) {
                ev_pen = MARKER;
            }
            curs.init(id);
            curs.setCursor(id, penSize[ev_pen]);
            curs.setPosition(id, pos);
            if(ev_pen != ERASER) {
                curs.hide(id);
            }
            curEventButtons = source;
            break;
        case MOVE:
            if(source & Qt::RightButton) {
                penType = ERASER;
            }else if(source & Qt::MiddleButton) {
                penType = MARKER;
            }
            if (curs.drawing[id]) {
                switch(penMode) {
                    case DRAW:
                        curs.setPosition(id, pos);
                        addPoint(id, pos);
                        drawLineToFunc(id, pressure);
                        break;
                    case SELECTION:
                        selectionDraw(geo.first(id), pos);
                        break;
                }
            }
            penType = ev_pen;
            break;
        case RELEASE:
            if (!curs.drawing[id]) {
                break;
            }
            num_of_press--;
            curs.drawing[id] = false;
            if(curEventButtons & Qt::LeftButton && geo.size(id) < 2) {
                addPoint(-1, pos+QPointF(0,1));
                drawLineToFunc(id, pressure);
            }
            if(penMode == SELECTION) {
                addPoint(id, pos);
                createSelection();
                update();
                return;
            }

            curEventButtons = 0;
            curs.hide(id);
            if(num_of_press == 0) {
                addImage(image);
            }
            break;
    }
}
bool DrawingWidget::event(QEvent *ev) {
    switch (ev->type()) {
        case QEvent::TouchBegin:
        case QEvent::TouchEnd:
        case QEvent::TouchUpdate: {
            QTouchEvent *touchEvent = static_cast<QTouchEvent*>(ev);
            QList<QTouchEvent::TouchPoint> touchPoints = touchEvent->points();
            foreach(const QTouchEvent::TouchPoint &touchPoint, touchPoints) {
                QPointF pos = touchPoint.position();
                if ((Qt::TouchPointState)touchPoint.state() == Qt::TouchPointPressed) {
                    eventHandler(Qt::LeftButton, PRESS, touchPoint.id(), pos, touchPoint.pressure());
                } else if ((Qt::TouchPointState)touchPoint.state() == Qt::TouchPointReleased) {
                    eventHandler(Qt::LeftButton, RELEASE, touchPoint.id(), pos, touchPoint.pressure());
                } else {
                    eventHandler(Qt::LeftButton, MOVE, touchPoint.id(), pos, touchPoint.pressure());
                }
            }
            break;
        }
        case QEvent::TabletPress: {
            QTabletEvent *tabletEvent = static_cast<QTabletEvent*>(ev);
            eventHandler(tabletEvent->buttons(), PRESS, -2, tabletEvent->position(), tabletEvent->pressure());
            break;
        }
        case QEvent::TabletRelease: {
            QTabletEvent *tabletEvent = static_cast<QTabletEvent*>(ev);
            eventHandler(tabletEvent->buttons(), RELEASE, -2, tabletEvent->position(), tabletEvent->pressure());
            break;
        }
        case QEvent::TabletMove: {
            QTabletEvent *tabletEvent = static_cast<QTabletEvent*>(ev);
            eventHandler(tabletEvent->buttons(), MOVE, -2, tabletEvent->position(), tabletEvent->pressure());
            break;
        }
        case QEvent::MouseButtonPress: {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(ev);
            eventHandler(mouseEvent->buttons(), PRESS, -1, mouseEvent->position(), 1.0);
            break;
        }
        case QEvent::MouseButtonRelease: {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(ev);
            eventHandler(mouseEvent->buttons(), RELEASE, -1, mouseEvent->position(), 1.0);
            break;
        }
        case QEvent::MouseMove: {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(ev);
            eventHandler(mouseEvent->buttons(), MOVE, -1, mouseEvent->position(), 1.0);
            break;
        }

        default:
            break;
    }
    return QWidget::event(ev);
}

int DrawingWidget::getPageNum(){
    return pages.last_page_num;
}

bool DrawingWidget::isBackAvailable(){
    //printf("%d %d\n", images.last_image_num, images.image_count );
    return images.last_image_num > images.removed +1;
}

bool DrawingWidget::isNextAvailable(){
    //printf("%d %d\n", images.last_image_num, images.image_count );
    return images.last_image_num < images.image_count;
}


QColor convertColor(QColor color) {
    int tot =  color.red() + color.blue() + color.green();
    if (tot > 382) {
        return QColor(0,0,0, color.alpha());
    } else {
        return QColor(255, 255, 255, color.alpha());

    }
}

void removeDirectory(const QString &path) {
    QDir dir(path);
    if (!dir.exists())
        return;

    QFileInfoList fileInfoList = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries | QDir::Hidden);
    foreach (const QFileInfo &fi, fileInfoList) {
        if (fi.isDir()) {
            removeDirectory(fi.absoluteFilePath());
        } else {
            QFile::remove(fi.absoluteFilePath());
        }
    }

    dir.rmdir(path);
}

void qImageToFile(const QImage& image, const QString& filename) {
    QFileInfo fileInfo(filename);
    QString dirname = fileInfo.dir().absolutePath();
    QDir dir(dirname);
    if (!dir.exists(dirname)) {
        dir.mkpath(dirname);
    }
    QPixmap pixmap = QPixmap::fromImage(image);
      if (pixmap.save(filename, "PNG")) {
            qDebug() << "Image saved successfully as" << filename;
        } else {
            qDebug() << "Failed to save image at" << filename;
        }
}

