/*
 * Copyright (C) 2020 by Johannes Zellner, johannes@zellner.org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

import QtQuick 2.12
import QtQuick.Controls 2.12

Dialog {

    id: batteryManagementDialog
    title: qsTr("Disable battery optimization")
    focus: true
    leftMargin: Math.max(Qt.application.font.pixelSize, (parent.width - 40 * Qt.application.font.pixelSize) / 2)
    rightMargin: leftMargin

    standardButtons: Dialog.Ok | Dialog.Cancel

    Text {
        id: batteryManagementText
        text: qsTr("If you leave your andriod device unplugged and stationary for a period of time, with the screen off, the device enters doze mode. In doze mode, the system attempts to conserve battery by restricting apps' access to CPU-intensive services.

If you experience problems when running Enroute in the background or with the screen turned off you may want to manually whitelist Enroute to partially exempt it from doze and app standby optimizations.

In the next dialog, select \"All apps\" at the top, select this app and select and confirm \"Don\'t optimize\".")
        width: parent.width
        wrapMode: Text.WordWrap
    }

    onAccepted: {
        MobileAdaptor.requestIgnoreBatteryOptimizations()
        close()
    }

    onRejected: {
        close()
    }

    Connections {
        target: sensorGesture
        onDetected: close()
    }
}
