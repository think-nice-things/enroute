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

#include <QDataStream>
#include <QFile>
#include <QStandardPaths>

#include "AviationUnits.h"
#include "FlightRoute.h"
#include "Waypoint.h"


FlightRoute::FlightRoute(Aircraft *aircraft, Wind *wind, QObject *parent)
    : QObject(parent), _aircraft(aircraft), _wind(wind)
{
    load();
    connect(this, &FlightRoute::waypointsChanged, this, &FlightRoute::save);
    connect(this, &FlightRoute::waypointsChanged, this, &FlightRoute::summaryChanged);
    if (!_aircraft.isNull())
        connect(_aircraft, &Aircraft::valChanged, this, &FlightRoute::summaryChanged);
    if (!_wind.isNull())
        connect(_wind, &Wind::valChanged, this, &FlightRoute::summaryChanged);
}


void FlightRoute::append(QObject *waypoint)
{
    if (waypoint == nullptr)
        return;
    if (!waypoint->inherits("Waypoint"))
        return;

    auto* wp = dynamic_cast<Waypoint*>(waypoint);
    _waypoints.append(new Waypoint(*wp, this));

    updateLegs();
    emit waypointsChanged();
}


QObject* FlightRoute::firstWaypointObject() const
{
    if (_waypoints.isEmpty())
        return nullptr;
    return _waypoints.first();
}


QVariantList FlightRoute::geoPath() const
{
    // Paranoid safety checks
    if (_waypoints.size() < 2)
        return QVariantList();

    QVariantList result;
    for(auto _waypoint : _waypoints) {
        if (!_waypoint->isValid())
            return QVariantList();
        result.append(QVariant::fromValue(_waypoint->coordinate()));
    }

    return result;
}


QObject* FlightRoute::lastWaypointObject() const
{
    if (_waypoints.isEmpty())
        return nullptr;
    return _waypoints.last();
}


void FlightRoute::clear()
{
    qDeleteAll(_waypoints);
    _waypoints.clear();

    updateLegs();
    emit waypointsChanged();
}


void FlightRoute::moveDown(QObject *waypoint)
{
    // Paranoid safety checks
    if (waypoint == nullptr)
        return;
    if (!waypoint->inherits("Waypoint"))
        return;
    if (waypoint == lastWaypointObject())
        return;

    auto swp = dynamic_cast<Waypoint*>(waypoint);

    auto idx = _waypoints.indexOf(swp);
    _waypoints.move(idx, idx+1);

    updateLegs();
    emit waypointsChanged();
}


void FlightRoute::moveUp(QObject *waypoint)
{
    // Paranoid safety checks
    if (waypoint == nullptr)
        return;
    if (!waypoint->inherits("Waypoint"))
        return;
    if (waypoint == firstWaypointObject())
        return;

    auto swp = dynamic_cast<Waypoint*>(waypoint);

    auto idx = _waypoints.indexOf(swp);
    if (idx > 0)
        _waypoints.move(idx, idx-1);

    updateLegs();
    emit waypointsChanged();
}


void FlightRoute::removeWaypoint(QObject *waypoint)
{
    // Paranoid safety checks
    if (waypoint == nullptr)
        return;
    if (!waypoint->inherits("Waypoint"))
        return;

    auto swp = dynamic_cast<Waypoint*>(waypoint);

    _waypoints.removeOne(swp);
    delete swp;

    updateLegs();
    emit waypointsChanged();
}


void FlightRoute::reverse()
{
    std::reverse(_waypoints.begin(), _waypoints.end());
    updateLegs();
    emit waypointsChanged();
}


void FlightRoute::save()
{
    QFile file(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+"/flightPlan.dat");
    file.open(QIODevice::WriteOnly);
    QDataStream out(&file);

    out << streamVersion; // Stream version
    for(auto & _waypoint : _waypoints)
        out << *_waypoint;
}


void FlightRoute::load()
{
    QFile file(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)+"/flightPlan.dat");
    if (!file.exists())
        return;

    file.open(QIODevice::ReadOnly);
    QDataStream in(&file);

    quint16 version;
    in >> version;
    if (version != streamVersion)
        return;

    _waypoints.clear();
    while(!in.atEnd()) {
        auto wp = new Waypoint(in, this);
        if (!wp->isValid()) {
            delete wp;
            _waypoints.clear();
            return;
        }
        _waypoints.append(wp);
    }

    updateLegs();
    emit waypointsChanged();
}


void FlightRoute::updateLegs()
{
    foreach(auto _leg, _legs)
        _leg->deleteLater();
    _legs.clear();

    for(int i=0; i<_waypoints.size()-1; i++)
        _legs.append(new Leg(_waypoints[i], _waypoints[i+1], _aircraft, _wind, this));
}


QList<QObject*> FlightRoute::routeObjects() const
{
    QList<QObject*> result;

    if (_waypoints.isEmpty())
        return result;

    for(int i=0; i<_waypoints.size()-1; i++) {
        result.append(_waypoints[i]);
        result.append(_legs[i]);
    }
    result.append(_waypoints.last());

    return result;
}


QString FlightRoute::summary() const
{
    if (_legs.empty())
        return {};

    QString result;

    auto dist = AviationUnits::Distance::fromM(0.0);
    auto time = AviationUnits::Time::fromS(0.0);
    double fuelInL = 0.0;

    for(auto _leg : _legs) {
        dist += _leg->distance();
        if (dist.toM() > 100) {
            time += _leg->Time();
            fuelInL += _leg->Fuel();
        }
    }

    result += QString("Total: %1&nbsp;NM").arg(dist.toNM(), 0, 'f', 1);
    if (time.isFinite())
        result += QString(" • %1&nbsp;h").arg(time.toHoursAndMinutes());
    if (qIsFinite(fuelInL))
        result += QString(" • %1&nbsp;L").arg(qRound(fuelInL));


    QStringList complaints;
    if (!_aircraft.isNull()) {
        if (!qIsFinite(_aircraft->cruiseSpeedInKT()))
            complaints += tr("Cruise speed not specified.");
        if (!qIsFinite(_aircraft->fuelConsumptionInLPH()))
            complaints += tr("Fuel consumption not specified.");
    }
    if (!_wind.isNull()) {
        if (!qIsFinite(_wind->windSpeedInKT()))
            complaints += tr("Wind speed not specified.");
        if (!qIsFinite(_wind->windDirectionInDEG()))
            if (!qIsFinite(_wind->windDirectionInDEG()))
                complaints += tr("Wind direction not specified.");

    }

    if (!complaints.isEmpty())
        result += QString("<p><font color='red'>Computation incomplete. %1</font></p>").arg(complaints.join(" "));

    return result;
}
