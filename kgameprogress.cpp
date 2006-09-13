/* This file is part of the KDE libraries
   Copyright (C) 1996 Martynas Kunigelis

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/
/**
 * KGameProgress -- a progress indicator widget for KDE.
 */

#include <QPaintEvent>
#include <QPainter>
#include <QPixmap>
#include <QString>
#include <QRegExp>
#include <QStyle>
#include <QFrame>

#include <kglobalsettings.h>
#include "kgameprogress.h"

#include <kapplication.h>

KGameProgress::KGameProgress(QWidget *parent)
	: QFrame(parent)
{
	initialize();
	slider->setOrientation(Qt::Horizontal);
}

KGameProgress::KGameProgress(Qt::Orientation orientation, QWidget *parent)
	: QFrame(parent)
{
	initialize();
	slider->setOrientation(orientation);
}

KGameProgress::~KGameProgress()
{
	delete slider;
	delete bar_pixmap;
}

void KGameProgress::advance(int offset)
{
	setValue(value() + offset);
}

void KGameProgress::initialize()
{
	slider = new QAbstractSlider(this);

	slider->setMinimum(0);
	slider->setMaximum(100);
	slider->setValue(0);

	format_ = "%p%";
	use_supplied_bar_color = false;
	bar_pixmap = 0;
	bar_style = Solid;
	text_enabled = true;
	connect(slider, SIGNAL(valueChanged(int)), this, SLOT(valueChange(int)));
	connect(KGlobalSettings::self(), SIGNAL(appearanceChanged()), this, SLOT(paletteChange()));
	paletteChange();
}

void KGameProgress::paletteChange()
{
	QPalette p = kapp->palette();
	if (!use_supplied_bar_color)
		bar_color = p.color( QPalette::Active, QPalette::Highlight );
	bar_text_color = p.color( QPalette::Active, QPalette::HighlightedText );
	text_color = p.color( QPalette::Active, QPalette::Text );
	setPalette(p);

	adjustStyle();
}


void KGameProgress::setBarPixmap(const QPixmap &pixmap)
{
	if (pixmap.isNull())
		return;
	if (bar_pixmap)
		delete bar_pixmap;

	bar_pixmap = new QPixmap(pixmap);
}

void KGameProgress::setBarColor(const QColor &color)
{
	bar_color = color;
	use_supplied_bar_color = true;
	if (bar_pixmap) {
		delete bar_pixmap;
		bar_pixmap = 0;
	}
}

void KGameProgress::setBarStyle(BarStyle style)
{
	if (bar_style != style) {
		bar_style = style;
		update();
	}
}

void KGameProgress::setOrientation(Qt::Orientation orientation)
{
	if (this->orientation() != orientation) {
		setOrientation(orientation);
		update();
	}
}

void KGameProgress::setValue(int value)
{
	slider->setValue( value );
}

void KGameProgress::setMinimum(int value)
{
	slider->setMinimum( value );
}

void KGameProgress::setMaximum(int value)
{
	slider->setMaximum( value );
}

void KGameProgress::setTextEnabled(bool enable)
{
	text_enabled = enable;
}

const QColor & KGameProgress::barColor() const
{
	return bar_color;
}

const QPixmap * KGameProgress::barPixmap() const
{
	return bar_pixmap;
}

bool KGameProgress::textEnabled() const
{
	return text_enabled;
}

QSize KGameProgress::sizeHint() const
{
	QSize s( size() );

	if(orientation() == Qt::Vertical) {
		s.setWidth(24);
	} else {
		s.setHeight(24);
	}

	return s;
}

QSize KGameProgress::minimumSizeHint() const
{
	return sizeHint();
}

QSizePolicy KGameProgress::sizePolicy() const
{
	if ( orientation()==Qt::Vertical )
		return QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Expanding );
	else
		return QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );
}

Qt::Orientation KGameProgress::orientation() const
{
	return slider->orientation();
}

KGameProgress::BarStyle KGameProgress::barStyle() const
{
	return bar_style;
}

int KGameProgress::recalcValue(int range)
{
	int abs_value = value() - minimum();
	int abs_range = maximum() - minimum();
	return abs_range ? range * abs_value / abs_range : 0;
}

void KGameProgress::valueChange(int newValue)
{
	Q_UNUSED(newValue);

	repaint(contentsRect());
	emit percentageChanged(recalcValue(100));
}

void KGameProgress::styleChange(QStyle&)
{
	adjustStyle();
}

void KGameProgress::adjustStyle()
{
	switch (style()->styleHint(QStyle::SH_GUIStyle)) {
		case Qt::WindowsStyle:
			setFrameStyle(QFrame::WinPanel | QFrame::Sunken);
			break;
		case Qt::MotifStyle:
		default:
			setFrameStyle(QFrame::Panel | QFrame::Sunken);
			setLineWidth( 2 );
			break;
	}
	update();
}

void KGameProgress::paletteChange( const QPalette &p )
{
	// This never gets called for global color changes 
	// because we call setPalette() ourselves.
	QFrame::paletteChange(p);
}

void KGameProgress::drawText(QPainter *p)
{
	QRect r(contentsRect());
	//QColor c(bar_color.rgb() ^ backgroundColor().rgb());

	// Rik: Replace the tags '%p', '%v' and '%m' with the current percentage,
	// the current value and the maximum value respectively.
	QString s(format_);

	s.replace(QRegExp(QString::fromLatin1("%p")), QString::number(recalcValue(100)));
	s.replace(QRegExp(QString::fromLatin1("%v")), QString::number(value()));
	s.replace(QRegExp(QString::fromLatin1("%m")), QString::number(maximum()));

	p->setPen(text_color);
	QFont font = p->font();
	font.setBold(true);
	p->setFont(font);
	//p->setRasterOp(XorROP);
	p->drawText(r, Qt::AlignCenter, s);
	p->setClipRegion( fr );
	p->setPen(bar_text_color);
	p->drawText(r, Qt::AlignCenter, s);
}

void KGameProgress::paintEvent( QPaintEvent *e )
{
	//paint the frame
	QFrame::paintEvent(e);

	//now for the inside of the Widget
	QPainter p(this);
	
	QRect cr = contentsRect(), er = cr;
	fr = cr;
        QBrush fb(bar_color), eb(palette().color(backgroundRole()));

	if (bar_pixmap)
		fb.setTexture(*bar_pixmap);

        QPixmap bkgnd_pix = palette().brush(backgroundRole()).texture();
	if (!bkgnd_pix.isNull())
		eb.setTexture(bkgnd_pix);

	switch (bar_style) {
		case Solid:
			if (orientation() == Qt::Horizontal) {
				fr.setWidth(recalcValue(cr.width()));
				er.setLeft(fr.right() + 1);
			} else {
				fr.setTop(cr.bottom() - recalcValue(cr.height()));
				er.setBottom(fr.top() - 1);
			}

			p.setBrushOrigin(cr.topLeft());
			p.fillRect(fr, fb);

			p.fillRect(er, eb);

			break;

		case Blocked:
			const int margin = 2;
			int max, num, dx, dy;
			if (orientation() == Qt::Horizontal) {
				fr.setHeight(cr.height() - 2 * margin);
				fr.setWidth((int)(0.67 * fr.height()));
				fr.moveTopLeft(QPoint(cr.left() + margin, cr.top() + margin));
				dx = fr.width() + margin;
				dy = 0;
				max = (cr.width() - margin) / (fr.width() + margin) + 1;
				num = recalcValue(max);
			} else {
				fr.setWidth(cr.width() - 2 * margin);
				fr.setHeight((int)(0.67 * fr.width()));
				fr.moveBottomLeft(QPoint(cr.left() + margin, cr.bottom() - margin));
				dx = 0;
				dy = - (fr.height() + margin);
				max = (cr.height() - margin) / (fr.height() + margin) + 1;
				num = recalcValue(max);
			}
			p.setClipRect(cr.x() + margin, cr.y() + margin,
			               cr.width() - margin, cr.height() - margin);
			for (int i = 0; i < num; i++) {
				p.setBrushOrigin(fr.topLeft());
				p.fillRect(fr, fb);
				fr.translate(dx, dy);
			}
			
			if (num != max) {
				if (orientation() == Qt::Horizontal)
					er.setLeft(fr.right() + 1);
				else
					er.setBottom(fr.bottom() + 1);
				if (!er.isNull()) {
					p.setBrushOrigin(cr.topLeft());
					p.fillRect(er, eb);
				}
			}

			break;
	}

	if (text_enabled && bar_style != Blocked)
		drawText(&p);
}

void KGameProgress::setFormat(const QString & format)
{
	format_ = format;
}

QString KGameProgress::format() const
{
	return format_;
}

#include "kgameprogress.moc"
