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

#include "DownloadableGroup.h"


DownloadableGroup::DownloadableGroup(QObject *parent)
    : QObject(parent), _cachedDownloading(false), _cachedUpdatable(false)
{
}


void DownloadableGroup::addToGroup(Downloadable *downloadable)
{
    // Avoid double entries
    if (_downloadables.contains(downloadable))
        return;

    // Add element to group
    _downloadables.append(downloadable);

    connect(downloadable, &Downloadable::downloadingChanged, this, &DownloadableGroup::elementChanged);
    connect(downloadable, &Downloadable::updatableChanged, this, &DownloadableGroup::elementChanged);
    connect(downloadable, &Downloadable::hasFileChanged, this, &DownloadableGroup::filesChanged);
    connect(downloadable, &Downloadable::fileContentChanged, this, &DownloadableGroup::localFileContentChanged);
    connect(downloadable, &QObject::destroyed, this, &DownloadableGroup::cleanUp);
    elementChanged();

    emit downloadablesChanged();
}


void DownloadableGroup::removeFromGroup(Downloadable *downloadable)
{
    auto index = _downloadables.indexOf(downloadable);

    // Avoid double entries
    if (index < 0)
        return;

    _downloadables.takeAt(index);
    disconnect(downloadable, nullptr, this, nullptr);
    elementChanged();
    emit downloadablesChanged();
}


bool DownloadableGroup::downloading() const
{
    foreach(auto _downloadable, _downloadables) {
        if (_downloadable.isNull())
            continue;
        if (_downloadable->downloading())
            return true;
    }

    return false;
}


QStringList DownloadableGroup::files() const
{
    QStringList result;

    foreach(auto _downloadable, _downloadables) {
        if (_downloadable.isNull())
            continue;
        if (!_downloadable->hasFile())
            continue;
        result += _downloadable->fileName();
    }

    return result;
}


bool DownloadableGroup::updatable() const
{
    foreach(auto _downloadable, _downloadables) {
        if (_downloadable.isNull())
            continue;
        if (_downloadable->updatable())
            return true;
    }

    return false;
}


void DownloadableGroup::cleanUp()
{
    auto idx = _downloadables.indexOf(nullptr);
    _downloadables.removeAll(nullptr);
    if (idx >= 0)
        emit downloadablesChanged();
}


void DownloadableGroup::elementChanged()
{
    bool newDownloading = downloading();
    bool newUpdatable   = updatable();

    if (newDownloading != _cachedDownloading) {
        _cachedDownloading = newDownloading;
        emit downloadingChanged();
    }

    if (newUpdatable != _cachedUpdatable) {
        _cachedUpdatable = newUpdatable;
        emit updatableChanged();
    }
}


QList<Downloadable *> DownloadableGroup::downloadables() const
{
    QList<Downloadable *> result;
    foreach(auto _downloadable, _downloadables) {
        if (_downloadable.isNull())
            continue;
        result += _downloadable;
    }

    // Sort Downloadables according to lower boundary
    std::sort(result.begin(), result.end(), [](Downloadable* a, Downloadable* b)
    {
        if (a->section() != b->section())
            return (a->section() < b->section());
        return (a->fileName() < b->fileName());
    }
    );

    return result;
}
