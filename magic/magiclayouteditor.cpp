#include "magiclayouteditor.h"

MagicLayoutEditor::MagicLayoutEditor(QWidget *parent) :
	QGraphicsView(parent),
	magicdata(NULL),
	lefdata(NULL),
	project(NULL),
	filePath(QString()),
	editScene(new QGraphicsScene(this))
{
	editScene->setBackgroundBrush(Qt::white);
	
	//sceneRect = QRectF(0,0,this->width(),this->height());
	//editScene->setSceneRect(sceneRect);
	//setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	//setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	setScene(editScene);
}

void MagicLayoutEditor::resizeEvent(QResizeEvent *event)
{
	redraw();
}

void MagicLayoutEditor::scrollContentsBy(int dx, int dy)
{
	//sceneRect = QRectF(sceneRect.x()+dx,sceneRect.y()+dy,this->width(),this->height());
	//editScene->setSceneRect(sceneRect);
	redraw();
}

void MagicLayoutEditor::addWires()
{
	QGraphicsRectItem *r;
	rects_t layer;
	layer_rects_t rects = magicdata->getRectangles();
	foreach(QString layerN, rects.keys()) {
		layer = rects[layerN];
		foreach (rect_t e, layer) {
			r = new QGraphicsRectItem(e.x1, e.y1, e.x2-e.x1, e.y2-e.y1);
			r->setBrush(QBrush(project->colorMat(layerN)));
			r->setVisible(true);
			editScene->addItem(r);
			layers[layerN].append(r);
		}
	}
}

void MagicLayoutEditor::addModules()
{
	lef::LEFMacro *macro;
	lef::LEFPin *pin;
	lef::LEFPort *port;
	lef::LEFLayer *layer;

	QGraphicsRectItem *r;
	QColor color;
	mods_t mods;

	mods = magicdata->getModules();
	foreach (module_info e, mods) {
		// fill in library content:
		if(lefdata) if(lefdata->isDefinedMacro(e.module_name)) {
			macro = lefdata->getMacro(e.module_name);
			macro->scaleMacro(e.a*(e.x2-e.x1), e.e*(e.y2-e.y1));

			foreach(pin, macro->getPins()) {
				port = pin->getPort();
				foreach(layer, port->getLayers()) {
					if(visibles) if(visibles->layerIsEnabled(layer->getName())) {
						color = project->colorMat(layer->getName());
						foreach(lef::rect_t rect, layer->getRects()) {
							r = new QGraphicsRectItem(rect.x+e.c, rect.y+e.f, rect.w, rect.h);
							r->setBrush(QBrush(color));
							r->setVisible(true);
							editScene->addItem(r);
							layers[layer->getName()].append(r);
						}
					}
				}
			}

			foreach (layer, macro->getObstruction()->getLayers()) {
				if(visibles) if(visibles->layerIsEnabled(layer->getName())) {
					color = project->colorMat(layer->getName());
					foreach(lef::rect_t rect, layer->getRects()) {
						r = new QGraphicsRectItem(rect.x+e.c, rect.y+e.f, rect.w, rect.h);
						r->setBrush(QBrush(color));
						r->setVisible(true);
						editScene->addItem(r);
						layers[layer->getName()].append(r);
					}
				}
			}

		}

		/*moduleAreas[e.instance_name].area = box;
		if(moduleAreas[e.instance_name].isSelected) {
			pen.setColor(Qt::red);
		}*/

		r = new QGraphicsRectItem(e.x1+e.c, e.y1+e.f, e.a*(e.x2-e.x1), e.e*(e.y2-e.y1));
		r->setVisible(true);
		editScene->addItem(r);
		bounding_boxes.append(r);

		// write layout details:
		QGraphicsTextItem *instance_name = new QGraphicsTextItem(e.instance_name);
		instance_name->setPos(e.c,e.f);
		instance_name->setVisible(true);
		editScene->addItem(instance_name);
		instance_labels.append(instance_name);
	}
}

void MagicLayoutEditor::loadFile(QString file)
{
	QString filedest;
	QTemporaryDir temporaryDir;
	filePath = file;
	if(magicdata) delete magicdata;
	magicdata = new magic::MagicData(file);

	if(project->getTechnology()==magicdata->getTechnology()) {
		if(lefdata) delete lefdata;
		lefdata = new lef::LEFData();
		foreach(QString filename, project->getProcessFiles()) {
			filedest = temporaryDir.path()+"/cells.lef";
			QFile::copy(filename, filedest);
			if(QFile(filedest).exists()) {
				lefdata->loadFile(filedest);
			}
		}
	}

	addWires();
	addModules();

	editScene->update();
}

void MagicLayoutEditor::redraw()
{
	QGraphicsRectItem *m;
	QGraphicsTextItem *t;
	bool visible = true;
	foreach(QString layerN, layers.keys()) {
		visible = (visibles)?(visibles->layerIsEnabled(layerN)):true;
		foreach(m, layers[layerN]) {
			m->setVisible(visible);
		}
	}

	visible = (visibles)?(visibles->visibleIsEnabled("bounding_box")):true;
	foreach(m, bounding_boxes) {
		m->setVisible(visible);
	}

	visible = (visibles)?(visibles->visibleIsEnabled("instance_label")):true;
	foreach(t, instance_labels) {
		t->setVisible(visible);
	}

	editScene->update();
}

void MagicLayoutEditor::saveFile()
{
	QRectF r;
	layer_t l;
	QGraphicsRectItem *m;
	foreach(QString n, layers.keys()) {
		l = layers[n];
		foreach(m,l) {
			r = m->boundingRect();
		}
	}
}

void MagicLayoutEditor::setProject(Project *p)
{
	project = p;
}

void MagicLayoutEditor::setVisibles(LayoutVisibles *v)
{
	visibles = v;
	if(visibles) connect(visibles, SIGNAL(refreshLayout()), this, SLOT(redraw()));
}

QString MagicLayoutEditor::getFilePath()
{
	return filePath;
}

bool MagicLayoutEditor::changes()
{
	return false;
}
