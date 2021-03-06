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

#include <QtGlobal>

#include "FlightRoute_Leg.h"


FlightRoute::Leg::Leg(const Waypoint* start, const Waypoint *end, Aircraft *aircraft, Wind *wind, QObject* parent)
    : QObject(parent), _aircraft(aircraft), _wind(wind)
{
    _start = new Waypoint(*start, this);
    _end   = new Waypoint(*end, this);

    if (!_aircraft.isNull())
        connect(_aircraft, &Aircraft::valChanged, this, &FlightRoute::Leg::valChanged);
    if (!_wind.isNull())
        connect(_wind, &Wind::valChanged, this, &FlightRoute::Leg::valChanged);
}


AviationUnits::Distance FlightRoute::Leg::distance() const
{
    // Paranoid safety checks
    if (!isValid())
        return {};

    return AviationUnits::Distance::fromM( _start->coordinate().distanceTo( _end->coordinate() ));
}


double FlightRoute::Leg::Fuel() const
{
    if (!hasDataForWindTriangle())
        return qQNaN();

    return _aircraft->fuelConsumptionInLPH()*Time().toH();
}


AviationUnits::Speed FlightRoute::Leg::GS() const
{
    if (!hasDataForWindTriangle())
        return {};

    auto TASInKT = _aircraft->cruiseSpeedInKT();
    auto WSInKT  = _wind->windSpeedInKT();
    auto WD      = AviationUnits::Angle::fromDEG( _wind->windDirectionInDEG() );

    // Law of cosine for wind triangle
    auto GSInKT = qSqrt( TASInKT*TASInKT + WSInKT*WSInKT - 2.0*TASInKT*WSInKT*AviationUnits::Angle::cos(WD-TH()));

    return AviationUnits::Speed::fromKT(GSInKT);
}


AviationUnits::Angle FlightRoute::Leg::TC() const
{
    // Paranoid safety checks
    if (!isValid())
        return {};
    if( _start->coordinate().distanceTo( _end->coordinate() ) < minLegLength )
        return {};

    return AviationUnits::Angle::fromDEG( _start->coordinate().azimuthTo(_end->coordinate()) );
}


AviationUnits::Angle FlightRoute::Leg::WCA() const
{
    if (!hasDataForWindTriangle())
        return {};

    AviationUnits::Speed TAS = AviationUnits::Speed::fromKT( _aircraft->cruiseSpeedInKT() );
    AviationUnits::Speed WS  = AviationUnits::Speed::fromKT( _wind->windSpeedInKT() );
    AviationUnits::Angle WD  = AviationUnits::Angle::fromDEG( _wind->windDirectionInDEG() );

    // Law of sine for wind triangle
    return AviationUnits::Angle::asin(-AviationUnits::Angle::sin(TC()-WD)*(WS/TAS));
}


bool FlightRoute::Leg::isValid() const
{
    if (_start.isNull())
        return false;
    if (!_start->coordinate().isValid())
        return false;
    if (_end.isNull())
        return false;
    if (!_end->coordinate().isValid())
        return false;
    return true;
}


QString FlightRoute::Leg::description() const
{
    if (!isValid())
        return QString();

    QString result;
    result += QString("%1 NM").arg(distance().toNM(), 0, 'f', 1);
    auto _time = Time();
    if (_time.isFinite())
        result += QString(" • %1 h").arg(_time.toHoursAndMinutes());
    auto TCInDEG = TC().toNormalizedDEG();
    if (qIsFinite(TCInDEG))
        result += QString(" • TC %1°").arg(qRound(TCInDEG));
    double THInDEG = TH().toNormalizedDEG();
    if (qIsFinite(THInDEG))
        result += QString(" • TH %1°").arg(qRound(THInDEG));

    return result;
}


bool FlightRoute::Leg::hasDataForWindTriangle() const
{
    if ( _aircraft.isNull() )
        return false;
    if ( !qIsFinite(_aircraft->cruiseSpeedInKT()) )
        return false;

    if (_wind.isNull())
        return false;
    if ( !qIsFinite(_wind->windSpeedInKT()) )
        return false;
    if ( !qIsFinite(_wind->windDirectionInDEG()) )
        return false;
    if (_wind->windSpeedInKT() > 0.75*_aircraft->cruiseSpeedInKT())
        return false;

    return true;
}
