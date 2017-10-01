#ifndef MAGICLAYOUTEDITOR_H
#define MAGICLAYOUTEDITOR_H

#include "ieditor.h"
#include "magic/magicdata.h"
#include "lef/lefdata.h"
#include "project.h"
#include "layoutvisibles.h"

#include "qgraphicswireitem.h"
#include "qgraphicsmacroitem.h"
#include "qgraphicslayoutrectitem.h"

#include <QWidget>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsWidget>
#include <QGridLayout>
#include <QAbstractScrollArea>
#include <QFileInfo>
#include <QResource>
#include <QDebug>
#include <QTemporaryDir>
#include <QAction>
#include <QComboBox>

enum drawing_operations {
	DRAWING_OPERATION_NONE,
	DRAWING_OPERATION_SELECT,
	DRAWING_OPERATION_RECTANGLE,
	DRAWING_OPERATION_POLYGON,
	DRAWING_OPERATION_DRAG
};

namespace magic {
	class MagicData;
}

namespace lef {
	class LEFData;
}

typedef QVector<QGraphicsRectItem*> layer_macro_wires_t;

typedef QVector<QGraphicsLayoutRectItem*> rects_layer_t;

class MagicLayoutEditor : public QGraphicsView, public IEditor
{
	Q_OBJECT

public:
	explicit MagicLayoutEditor(QWidget *parent = nullptr);

	void loadFile(QString);
	void setProject(Project *p);
	void setVisibles(LayoutVisibles *v);
	void setActiveLayerSelection(QComboBox *s);
	void saveFile();
	QString getFilePath();

	bool changes();

	void setDrawingOperation(drawing_operations o);

protected:
	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);

signals:
	void contentChanged();
	void contentSaved();

public slots:
	void visibles_action(QString s);
	void resizeEvent(QResizeEvent *event);
	void scrollContentsBy(int dx, int dy);

private:
	/* magic output functions */
	void saveFileWriteHeader(QTextStream &outputStream);
	void saveFileWriteRects(QTextStream &outputStream);
	void saveFileWriteMacros(QTextStream &outputStream);
	/* end of magic output functions */

	void setRecentVisible(QString s);
	void redraw();
	void addWires();
	void addModules();

	QString filePath;
	QGraphicsScene *editScene;
	magic::MagicData *magicdata;
	lef::LEFData *lefdata;

	QRectF sceneRect;
	Project *project;
	LayoutVisibles *visibles;
	QComboBox *activeLayerSelection;

	QGraphicsLayoutRectItem *recentRectangle;
	QPointF lastOrig;

	QVector<QGraphicsMacroItem*> macros;
	QVector<QGraphicsTextItem*> macro_texts;
	QMap<QString,layer_macro_wires_t> macro_wires;
	QMap<QString,rects_layer_t> layer_rects;

	drawing_operations recentOperation;
};

#endif // MAGICLAYOUTEDITOR_H
