/*
 * This file is part of Klaverjas.
 * Copyright (C) 2018  Steven Franzen <sfranzen85@gmail.com>
 *
 * Klaverjas is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Klaverjas is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

import QtQuick 2.11

/// A circle drawn clockwise, starting at the top
PathView {
    id: circle
    property real radius: 100
    width: 2 * radius
    height: 2 * radius
    path: Path {
        startX: circle.radius
        startY: 0
        PathArc {
            relativeX: 0
            relativeY: 2*circle.radius
            radiusX: circle.radius
            radiusY: circle.radius
        }
        PathArc {
            relativeX:0
            relativeY: -2*circle.radius
            radiusX: circle.radius
            radiusY: circle.radius
        }
    }
}
