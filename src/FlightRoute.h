/***************************************************************************
 *   Copyright (C) 2019-2020 by Stefan Kebekus                             *
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

#ifndef FLIGHTROUTE_H
#define FLIGHTROUTE_H

#include <QLocale>
#include <QPointer>

#include "Aircraft.h"
#include "Waypoint.h"
#include "Wind.h"


/*! \brief Intended flight route
 *
 * This class represents an intended flight route. In essence, this class is
 * little more than a list of waypoint and a number of methods that do the
 * following.
 *
 * - Expose the list of waypoints and legs to QML and allow some manipulation
 *   from there, such as adding or re-arranging waypoints.
 *
 * - Compute length and true course for the legs in the flight path, as well as
 *   a total length and expose this data to QML.
 */

class FlightRoute : public QObject
{
    Q_OBJECT

    class Leg;

public:
    /*! \brief Construct a flight route
     *
     * This default constructor calls load(), restoring the last saved route.
     *
     * @param aircraft Pointer to aircraft info that is used in route
     * computations. The aircraft object to supposed to exist throughout the
     * liftime of this object.
     *
     * @param wind Pointer to wind info that is used in route computations. The
     * wind object to supposed to exist throughout the liftime of this object.
     *
     * @param parent The standard QObject parent pointer.
     */
    explicit FlightRoute(Aircraft *aircraft, Wind *wind, QObject *parent = nullptr);

    // No copy constructor
    FlightRoute(FlightRoute const&) = delete;

    // No assign operator
    FlightRoute& operator =(FlightRoute const&) = delete;

    // No move constructor
    FlightRoute(FlightRoute&&) = delete;

    // No move assignment operator
    FlightRoute& operator=(FlightRoute&&) = delete;

    // Standard destructor
    ~FlightRoute() override = default;

    /*! \brief Adds a waypoint to the end of the route
     *
     * @warning This method accepts a pointer to a QObject and not a pointer to
     * a Waypoint to make it more easily accessible from QML. The
     * behaviour of this method is undefined if pointers to other objects are
     * passed as a parameter.
     *
     * @param waypoint Pointer to a waypoint, which must be of type
     * Waypoint. This method makes a private copy of the argument, so no
     * assumptions are made about the lifetime of *waypoint.
     */
    Q_INVOKABLE void append(QObject *waypoint);

    /*! \brief Adds a waypoint to the end of the route
     *
     * This method generates a generic waypoint with the given coordinates.
     *
     * @param position Coordinates of the waypoint.
     */
    Q_INVOKABLE void append(const QGeoCoordinate& position) { append(new Waypoint(position, this)); }

    /*! \brief First waypoint in the route
     *
     * This property holds a pointer to the first waypoint in the route, or a
     * nullptr if the route is empty. In order to make the waypoints accessible
     * to QML, the pointers are returned as pointers to QObjects rather than
     * Waypoints. The waypoints are owned by this route.
     *
     * @see lastWaypointObject
     */
    Q_PROPERTY(QObject* firstWaypointObject READ firstWaypointObject NOTIFY waypointsChanged)

    /*! \brief Getter function for the property with the same name
     *
     * @returns Property firstWaypointObject
     */
    QObject* firstWaypointObject() const;

    /*! \brief List of coordinates for the waypoints
     *
     * This property holds a list of coordinates of the waypoints, suitable for
     * drawing the flight path on a QML map. For better interaction with QML,
     * the data is returned in the form of a QVariantList rather than
     * QList<QGeoCoordinate>.
     */
    Q_PROPERTY(QVariantList geoPath READ geoPath NOTIFY waypointsChanged)

    /*! \brief Getter function for the property with the same name
     *
     * @returns Property geoPath
     */
    QVariantList geoPath() const;

    /*! \brief True if the list of waypoints is empty*/
    Q_PROPERTY(bool isEmpty READ isEmpty NOTIFY waypointsChanged)

    /*! \brief Getter function for the property with the same name
     *
     * @returns Property isEmpty
     */
    bool isEmpty() const {return _waypoints.isEmpty();}

    /*! \brief Last waypoint in the route
     *
     * This property holds a pointer to the last waypoint in the route, or a
     * nullptr if the route is empty. In order to make the waypoints accessible
     * to QML, the pointers are returned as pointers to QObjects rather than
     * Waypoints. The waypoints are owned by this route.
     *
     * @see firstWaypointObject
     */
    Q_PROPERTY(QObject* lastWaypointObject READ lastWaypointObject NOTIFY waypointsChanged)

    /*! \brief Getter function for the property with the same name
     *
     * @returns Property lastWaypointObject
     */
    QObject* lastWaypointObject() const;

    /*! \brief List of waypoints and legs
     *
     * This property lists all the waypoints and legs contained in the route. It
     * can be empty, and it can contain a single waypoint. If there is more than
     * one waypoints, then waypoints and legs alternate, following the pattern:
     * waypoint - leg - waypoint - … - leg - waypoint.
     */
    Q_PROPERTY(QList<QObject*> routeObjects READ routeObjects NOTIFY waypointsChanged)

    /*! \brief Getter function for the property with the same name
     *
     * @returns Property routeObjects
     */
    QList<QObject*> routeObjects() const;

    /*! \brief Human-readable summary of the flight route*/
    Q_PROPERTY(QString summary READ summary NOTIFY summaryChanged)

    /*! \brief Getter function for the property with the same name
     *
     * @returns Property summary
     */
    QString summary() const;

public slots:
    /*! \brief Deletes all waypoints in the current route */
    void clear();

    /*! \brief Move waypoint one position down in the list of waypoints
     *
     * If the waypoint is contained in the route, the method returns immediately
     *
     * @param waypoint Pointer to the waypoint
     */
    void moveDown(QObject *waypoint);

    /*! \brief Move waypoint one position up in the list of waypoints
     *
     * If the waypoint is contained in the route, the method returns immediately
     *
     * @param waypoint Pointer to the waypoint
     */
    void moveUp(QObject *waypoint);

    /*! \brief Remove waypoint from the current route
     *
     * If the waypoint is contained in the route, the method returns immediately
     *
     * @param waypoint Pointer to the waypoint
     */
    void removeWaypoint(QObject *waypoint);

    /*! \brief Reverse the route */
    void reverse();

signals:
    /*! \brief Notification signal for the property with the same name */
    void waypointsChanged();

    /*! \brief Notification signal for the property with the same name */
    void summaryChanged();

private slots:
    // Saves the route in "flightRoute.dat" contained in
    // QStandardPaths::writableLocation(QStandardPaths::AppDataLocation). This
    // slot is called whenever the route changes, so that the file will always
    // contain the current route.
    void save();

    // Loads the route from "flightRoute.dat" contained in
    // QStandardPaths::writableLocation(QStandardPaths::AppDataLocation). This
    // method is called on construction, so that the last saved flightpath is
    // restored automatically
    void load();

    void updateLegs();

private:
    // Used to check compatibility when loading/saving
    static const quint16 streamVersion = 1;

    QList<Waypoint*> _waypoints;

    QList<Leg*> _legs;

    QPointer<Aircraft> _aircraft {nullptr};
    QPointer<Wind> _wind {nullptr};

    QLocale myLocale;
};

#include "FlightRoute_Leg.h"

#endif
