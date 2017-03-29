/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2017  Steven Franzen <sfranzen85@gmail.com>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 */

#ifndef CARDIMAGEPROVIDER_H
#define CARDIMAGEPROVIDER_H

#include <QQuickImageProvider>
#include <QSvgRenderer>

/** Card image provider class.
 *
 * This is a simple wrapper around a QSvgRenderer, which paints the requested
 * SVG element onto a pixmap.
 */
class CardImageProvider : public QQuickImageProvider
{
public:
    CardImageProvider();
    QPixmap requestPixmap(const QString& id, QSize* size, const QSize& requestedSize) override;

private:
    QSvgRenderer m_renderer;
};

#endif // CARDIMAGEPROVIDER_H
