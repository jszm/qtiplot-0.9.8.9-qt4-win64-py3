/***************************************************************************
    File                 : MdiSubWindow.cpp
    Project              : QtiPlot
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief, Knut Franke
    Email (use @ for *)  : ion_vasilief*yahoo.fr, knut.franke*gmx.de
    Description          : MDI sub window

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/
#include "MdiSubWindow.h"
#include "Folder.h"
#include "ApplicationWindow.h"

#include <QApplication>
#include <QMessageBox>
#include <QEvent>
#include <QCloseEvent>
#include <QMouseEvent>
#include <QHash>
#include <QStyle>
#include <QString>
#include <QDateTime>
#include <QMenu>
#include <QTextStream>
#include <QTemporaryFile>

#include <fstream>
#include <string>

using std::ifstream;
using std::string;

namespace {
struct ResizeState
{
	ResizeState() : resizing(false), edges(0) {}

	bool resizing;
	int edges;
	QPoint startGlobal;
	QRect startGeometry;
};

QHash<const MdiSubWindow *, ResizeState> resizeStates;

ResizeState& resizeState(const MdiSubWindow *window)
{
	return resizeStates[window];
}
}

MdiSubWindow::MdiSubWindow(const QString& label, ApplicationWindow *app, const QString& name, Qt::WFlags f):
		QMdiSubWindow (app, f),
		d_app(app),
		d_folder(app->currentFolder()),
		d_label(label),
		d_status(Normal),
		d_caption_policy(Both),
		d_confirm_close(true),
		d_birthdate(QDateTime::currentDateTime ().toString(Qt::LocalDate)),
		d_min_restore_size(QSize())
{
	setObjectName(name);
	setAttribute(Qt::WA_DeleteOnClose);
	setMouseTracking(true);
	setLocale(app->locale());
	if (d_folder)
		d_folder->addWindow(this);
}

void MdiSubWindow::updateCaption()
{
switch (d_caption_policy)
	{
	case Name:
        setWindowTitle(objectName());
	break;

	case Label:
		if (!d_label.isEmpty())
            setWindowTitle(d_label);
		else
            setWindowTitle(objectName());
	break;

	case Both:
		if (!d_label.isEmpty())
            setWindowTitle(objectName() + " - " + d_label);
		else
            setWindowTitle(objectName());
	break;
	}

	d_app->setListViewLabel(objectName(), d_label);
};

void MdiSubWindow::resizeEvent( QResizeEvent* e )
{
	emit resizedWindow(this);
	QMdiSubWindow::resizeEvent( e );
}

bool MdiSubWindow::event(QEvent *event)
{
	if (event->type() == QEvent::MouseButtonPress){
		QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
		if (mouseEvent->button() == Qt::LeftButton && resizeEdgesAt(mouseEvent->pos()) != NoEdge){
			mousePressEvent(mouseEvent);
			return true;
		}
	} else if (event->type() == QEvent::MouseMove){
		QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
		ResizeState &state = resizeState(this);
		if (state.resizing || (!(mouseEvent->buttons() & Qt::LeftButton) && resizeEdgesAt(mouseEvent->pos()) != NoEdge)){
			mouseMoveEvent(mouseEvent);
			return true;
		}
	} else if (event->type() == QEvent::MouseButtonRelease){
		QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
		ResizeState &state = resizeState(this);
		if (state.resizing && mouseEvent->button() == Qt::LeftButton){
			mouseReleaseEvent(mouseEvent);
			return true;
		}
	}

	return QMdiSubWindow::event(event);
}

int MdiSubWindow::resizeEdgesAt(const QPoint& pos) const
{
	if (!parentWidget() || isMaximized() || isMinimized() || isShaded() ||
		(windowFlags() & Qt::MSWindowsFixedSizeDialogHint))
		return NoEdge;

	int margin = style()->pixelMetric(QStyle::PM_MdiSubWindowFrameWidth, 0, this);
	margin = qMax(margin, 6);

	int edges = NoEdge;
	if (pos.x() <= margin)
		edges |= LeftEdge;
	else if (pos.x() >= width() - margin)
		edges |= RightEdge;

	if (pos.y() <= margin)
		edges |= TopEdge;
	else if (pos.y() >= height() - margin)
		edges |= BottomEdge;

	return edges;
}

void MdiSubWindow::updateResizeCursor(int edges)
{
	if ((edges & LeftEdge && edges & TopEdge) || (edges & RightEdge && edges & BottomEdge))
		setCursor(Qt::SizeFDiagCursor);
	else if ((edges & RightEdge && edges & TopEdge) || (edges & LeftEdge && edges & BottomEdge))
		setCursor(Qt::SizeBDiagCursor);
	else if (edges & (LeftEdge | RightEdge))
		setCursor(Qt::SizeHorCursor);
	else if (edges & (TopEdge | BottomEdge))
		setCursor(Qt::SizeVerCursor);
	else
		unsetCursor();
}

void MdiSubWindow::mousePressEvent(QMouseEvent *e)
{
	if (e->button() == Qt::LeftButton){
		int edges = resizeEdgesAt(e->pos());
		if (edges != NoEdge){
			ResizeState &state = resizeState(this);
			state.resizing = true;
			state.edges = edges;
			state.startGlobal = e->globalPos();
			state.startGeometry = geometry();
			updateResizeCursor(edges);
			e->accept();
			return;
		}
	}

	QMdiSubWindow::mousePressEvent(e);
}

void MdiSubWindow::mouseMoveEvent(QMouseEvent *e)
{
	ResizeState &state = resizeState(this);
	if (state.resizing){
		QPoint delta = e->globalPos() - state.startGlobal;
		int x = state.startGeometry.x();
		int y = state.startGeometry.y();
		int w = state.startGeometry.width();
		int h = state.startGeometry.height();

		if (state.edges & LeftEdge){
			x += delta.x();
			w -= delta.x();
		} else if (state.edges & RightEdge)
			w += delta.x();

		if (state.edges & TopEdge){
			y += delta.y();
			h -= delta.y();
		} else if (state.edges & BottomEdge)
			h += delta.y();

		QSize minSize = minimumSize().expandedTo(minimumSizeHint());
		QSize maxSize = maximumSize();
		int minW = qMax(1, minSize.width());
		int minH = qMax(1, minSize.height());
		int maxW = maxSize.width();
		int maxH = maxSize.height();

		if (w < minW){
			if (state.edges & LeftEdge)
				x = state.startGeometry.x() + state.startGeometry.width() - minW;
			w = minW;
		} else if (w > maxW){
			if (state.edges & LeftEdge)
				x = state.startGeometry.x() + state.startGeometry.width() - maxW;
			w = maxW;
		}

		if (h < minH){
			if (state.edges & TopEdge)
				y = state.startGeometry.y() + state.startGeometry.height() - minH;
			h = minH;
		} else if (h > maxH){
			if (state.edges & TopEdge)
				y = state.startGeometry.y() + state.startGeometry.height() - maxH;
			h = maxH;
		}

		if (QWidget *p = parentWidget()){
			QRect parentRect = p->rect();
			if (x < 0){
				w += x;
				x = 0;
			}
			if (y < 0){
				h += y;
				y = 0;
			}
			if (x + w > parentRect.width())
				w = parentRect.width() - x;
			if (y + h > parentRect.height())
				h = parentRect.height() - y;
			w = qMax(w, minW);
			h = qMax(h, minH);
		}

		setGeometry(x, y, w, h);
		e->accept();
		return;
	}

	if (!(e->buttons() & Qt::LeftButton)){
		int edges = resizeEdgesAt(e->pos());
		if (edges != NoEdge){
			updateResizeCursor(edges);
			e->accept();
			return;
		}
	}

	QMdiSubWindow::mouseMoveEvent(e);
}

void MdiSubWindow::mouseReleaseEvent(QMouseEvent *e)
{
	ResizeState &state = resizeState(this);
	if (state.resizing && e->button() == Qt::LeftButton){
		state.resizing = false;
		state.edges = NoEdge;
		updateResizeCursor(resizeEdgesAt(e->pos()));
		e->accept();
		return;
	}

	QMdiSubWindow::mouseReleaseEvent(e);
}

void MdiSubWindow::closeEvent( QCloseEvent *e )
{
	if (d_confirm_close){
    	switch( QMessageBox::information(this, tr("QtiPlot"),
				tr("Do you want to hide or delete") + "<p><b>'" + objectName() + "'</b> ?",
				tr("Delete"), tr("Hide"), tr("Cancel"), 0, 2)){
		case 0:
			emit closedWindow(this);
			e->accept();
		break;

		case 1:
			e->ignore();
			emit hiddenWindow(this);
		break;

		case 2:
			e->ignore();
		break;
		}
    } else {
		emit closedWindow(this);
    	e->accept();
    }
}

QString MdiSubWindow::aspect()
{
QString s = tr("Normal");
switch (d_status)
	{
	case Normal:
	break;

	case Minimized:
		return tr("Minimized");
	break;

	case Maximized:
		return tr("Maximized");
	break;

	case Hidden:
		return tr("Hidden");
	break;
	}
return s;
}

QString MdiSubWindow::sizeToString()
{
return QString::number(sizeof(MdiSubWindow), 'f', 1) + " " + tr("B");
}

void MdiSubWindow::changeEvent(QEvent *event)
{
	if (!isHidden() && event->type() == QEvent::WindowStateChange){
		Status oldStatus = d_status;
		Status newStatus = Normal;
		if( windowState() & Qt::WindowMinimized ){
		    if (oldStatus != Minimized)
                d_min_restore_size = frameSize();
	    	newStatus = Minimized;
		} else if ( windowState() & Qt::WindowMaximized )
	     	newStatus = Maximized;

		if (newStatus != oldStatus){
			d_status = newStatus;
    		emit statusChanged (this);
		}
	}
	QMdiSubWindow::changeEvent(event);
}

bool MdiSubWindow::eventFilter(QObject *object, QEvent *e)
{
	if (e->type() == QEvent::ContextMenu && object == widget()){
        emit showContextMenu();
        return true;
	}

	if (e->type() == QEvent::Move && object == widget()){
		QObjectList lst = children();
		foreach(QObject *o, lst){
			if (o->isA("QMenu") && d_app){
			    d_app->customWindowTitleBarMenu(this, (QMenu *)o);
				break;
			}
		}
	}

	if (e->type() == QEvent::WindowActivate && object == widget() && !parent()){
		d_app->setActiveWindow(this);
		if (d_folder)
			d_folder->setActiveWindow(this);
	}
	return QMdiSubWindow::eventFilter(object, e);
}

void MdiSubWindow::setStatus(Status s)
{
	if (d_status == s)
		return;

	d_status = s;
	emit statusChanged (this);
}

void MdiSubWindow::setHidden()
{
    d_status = Hidden;
    emit statusChanged (this);
    hide();
}

void MdiSubWindow::restoreWindow()
{
	MultiLayer *ml = qobject_cast<MultiLayer *>(this);
	bool resizeLayers = false;
	if (ml){
		resizeLayers = ml->scaleLayersOnResize();
		ml->setScaleLayersOnResize(false);
	}

	switch (d_status){
		case MdiSubWindow::Hidden:
		case MdiSubWindow::Normal:
			showNormal();
			break;

		case MdiSubWindow::Minimized:
			showMinimized();
			break;

		case MdiSubWindow::Maximized:
			showMaximized();
			break;
	}

	if (ml)
		ml->setScaleLayersOnResize(resizeLayers);
}

void MdiSubWindow::setNormal()
{
	showNormal();
	d_status = Normal;
	emit statusChanged (this);
}

void MdiSubWindow::setMinimized()
{
	d_status = Minimized;
	emit statusChanged (this);
	showMinimized();
}

void MdiSubWindow::setMaximized()
{
	showMaximized();
	d_status = Maximized;

	if (d_folder)
		d_folder->setActiveWindow(this);

	emit statusChanged (this);
}

QString MdiSubWindow::parseAsciiFile(const QString& fname, const QString &commentString,
                        int endLine, int ignoreFirstLines, int maxRows, int& rows)
{
	if (endLine == ApplicationWindow::CR)
		return parseMacAsciiFile(fname, commentString, ignoreFirstLines, maxRows, rows);

	//QTextStream replaces '\r\n' with '\n', therefore we don't need a special treatement in this case!

	QFile f(fname);
 	if(!f.open(QIODevice::ReadOnly))
  		return QString::null;

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	QTextStream t(&f);

	QTemporaryFile tempFile;
	tempFile.open();
	QTextStream temp(&tempFile);

	for (int i = 0; i < ignoreFirstLines; i++)//skip first 'ignoreFirstLines' lines
		t.readLine();

	bool validCommentString = !commentString.isEmpty();
	QRegExp rx(commentString);
	rx.setPatternSyntax(QRegExp::Wildcard);
	rows = 0;
	if (maxRows <= 0){//read all valid lines
		while(!t.atEnd()){//count the number of valid rows
			QString s = t.readLine();
			if (validCommentString && s.contains(rx))
				continue;

			rows++;
			temp << s + "\n";
			qApp->processEvents(QEventLoop::ExcludeUserInput);
		}
	} else {//we write only 'maxRows' valid rows to the temp file
		while(!t.atEnd() && rows < maxRows){
			QString s = t.readLine();
			if (validCommentString && s.contains(rx))
				continue;

			rows++;
			temp << s + "\n";
			qApp->processEvents(QEventLoop::ExcludeUserInput);
		}
	}
	f.close();

	tempFile.setAutoRemove(false);
	QString path = tempFile.fileName();
	tempFile.close();

	QApplication::restoreOverrideCursor();
	return path;
}

QString MdiSubWindow::parseMacAsciiFile(const QString& fname, const QString &commentString,
                        				int ignoreFirstLines, int maxRows, int& rows)
{
	ifstream f;
 	f.open(fname.toAscii());
 	if(!f)
  		return QString::null;

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

	QTemporaryFile tempFile;
	tempFile.open();
	QTextStream temp(&tempFile);

	for (int i = 0; i < ignoreFirstLines; i++){//skip first 'ignoreFirstLines' lines
		string s;
		getline(f, s, '\r');
	}

	bool validCommentString = !commentString.isEmpty();
	string comment = commentString.ascii();
	rows = 0;
	if (maxRows <= 0){//read all valid lines
		while(f.good() && !f.eof()){//count the number of valid rows
			string s;
			getline(f, s, '\r');
			if (validCommentString && s.find(comment) != string::npos)
				continue;

			rows++;
			temp << QString(s.c_str()) + "\n";
			qApp->processEvents(QEventLoop::ExcludeUserInput);
		}
	} else {//we write only 'maxRows' valid rows to the temp file
		while(f.good() && !f.eof() && rows < maxRows){
			string s;
			getline(f, s, '\r');
			if (validCommentString && s.find(comment) != string::npos)
				continue;

			rows++;
			temp << QString(s.c_str()) + "\n";
			qApp->processEvents(QEventLoop::ExcludeUserInput);
		}
	}
	f.close();

	tempFile.setAutoRemove(false);
	QString path = tempFile.fileName();
	tempFile.close();

	QApplication::restoreOverrideCursor();
	return path;
}
