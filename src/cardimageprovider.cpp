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

#include "cardimageprovider.h"

#include <QPainter>

CardImageProvider::CardImageProvider()
    : QQuickImageProvider(QQuickImageProvider::Pixmap)
    , m_renderer(QString("/usr/share/carddecks/svg-oxygen-air/oxygen-air.svgz"))
{
}

QPixmap CardImageProvider::requestPixmap(const QString& id, QSize* size, const QSize& requestedSize)
{
    QPixmap pixmap;
    if (!m_renderer.elementExists(id))
        return pixmap;
    QSize cardSize = m_renderer.boundsOnElement(id).toAlignedRect().size();
    if (size)
        *size = cardSize;
    if (requestedSize.width() > 0)
        cardSize.setWidth(requestedSize.width());
    if (requestedSize.height() > 0)
        cardSize.setHeight(requestedSize.height());
    pixmap = QPixmap(cardSize);
    QPainter painter(&pixmap);
    m_renderer.render(&painter, id);
    return pixmap;
}
