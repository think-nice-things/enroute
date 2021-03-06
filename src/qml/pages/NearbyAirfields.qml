/***************************************************************************
 *   Copyright (C) 2019 by Stefan Kebekus                                  *
 *   stefan.kebekus@gmail.com                                              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Controls.Material 2.12
import QtQuick.Layouts 1.12

import enroute 1.0

Page {
    id: page
    title: qsTr("Nearby Airfields")
    focus: true

    Component {
        id: waypointDelegate

        ItemDelegate {
            id: idel
            text: model.modelData.richTextName +
                  ((satNav.status == SatNav.OK) ? ("<br>" + model.modelData.wayFrom(satNav.lastValidCoordinate)) : "")
            icon.source: "/icons/waypoints/"+model.modelData.get("CAT")+".svg"
            icon.color: "transparent"

            anchors.left: parent.left
            anchors.right: parent.right
            Layout.fillWidth: true

            onClicked: {
                MobileAdaptor.vibrateBrief()
                dialogLoader.active = false
                dialogLoader.waypoint = model.modelData
                dialogLoader.text = ""
                dialogLoader.source = "../dialogs/WaypointDescription.qml"
                dialogLoader.active = true
            }
        }

    }

    ListView {
        id: wpList
        anchors.fill: parent

        clip: true

        delegate: waypointDelegate
        ScrollIndicator.vertical: ScrollIndicator {}

        Component.onCompleted: wpList.model = geoMapProvider.nearbyAirfields(satNav.lastValidCoordinate)
    }

} // Page
