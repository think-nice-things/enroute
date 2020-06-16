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


package de.akaflieg_freiburg.enroute;

import android.content.Context;
import android.os.Vibrator;
import android.os.Build;
import android.content.Intent;
import android.provider.Settings;
import android.os.PowerManager;
import android.content.ComponentName ;
import android.os.Bundle;

import org.qtproject.qt5.android.QtNative;

import android.util.Log;

public class MobileAdaptor extends org.qtproject.qt5.android.bindings.QtActivity
{
    private static MobileAdaptor m_instance;
    private static Vibrator m_vibrator;

    private static boolean is_ignoring_battery_optimizations = false;

    public MobileAdaptor()
    {
        m_instance = this;
    }

    /* Vibrate once, very briefly */
    
    public static void vibrateBrief()
    {
        if (m_vibrator == null)
            m_vibrator = (Vibrator) m_instance.getSystemService(Context.VIBRATOR_SERVICE);
        m_vibrator.vibrate(20);
    }

    public void onCreate(Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);

        Log.d("MobileAdaptor", "onCreate");

        Intent serviceIntent = new Intent(this, EnrouteService.class);

        ComponentName m_service;
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            m_service = startForegroundService(serviceIntent);
        } else {
            m_service = startService(serviceIntent);
        }

        Log.d("MobileAdaptor", m_service.flattenToString() + " started");
    }

/*
    @Override
    public void onStart()
    {
    }
*/

    @Override
    public void onResume()
    {
        super.onResume();
        Log.d("MobileAdaptor", "onResume");

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M)
        {
            Intent intent = new Intent(Settings.ACTION_IGNORE_BATTERY_OPTIMIZATION_SETTINGS);
            if (intent.resolveActivity(getPackageManager()) != null)
            {
                is_ignoring_battery_optimizations = isIgnoringOptimizations(QtNative.activity());
                // Log.d("MobileAdaptor", "ignoring battery optimizations = " + Boolean.toString(is_ignoring_battery_optimizations));
            }
            else
            {
                Log.d("MobileAdaptor", "couldn't resolve activity.");
            }
        }
    }

    @Override
    public void onStop()
    {
        super.onStop();
    }

    @Override
    public void onDestroy()
    {
        Log.d("MobileAdaptor", "onDestroy");
        stopService(new Intent(this, EnrouteService.class));
        // mEnrouteService.unregisterOnServiceStateChangeListener(this);
        Log.d("MobileAdaptor", "EnrouteService stopped.");

        super.onDestroy();
    }

    private static Boolean isIgnoringOptimizations(Context context)
    {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M)
        {
            PowerManager pm = (PowerManager) context.getSystemService(Context.POWER_SERVICE);
            if (pm == null)
                return null;
            return pm.isIgnoringBatteryOptimizations(BuildConfig.APPLICATION_ID);
        }
        return null;
    }

    public static boolean canModifyDoze()
    {
        return Build.VERSION.SDK_INT >= Build.VERSION_CODES.M;
    }

    public static boolean isIgnoringBatteryOptimizations()
    {
        return is_ignoring_battery_optimizations;
    }

    public static void requestIgnoreBatteryOptimizations()
    {
        if (is_ignoring_battery_optimizations || !canModifyDoze()) {
            return; // do nothing
        }

        try
        {
            m_instance.startActivity(new Intent(Settings.ACTION_IGNORE_BATTERY_OPTIMIZATION_SETTINGS));
        }
        catch (Throwable ex)
        {
            Log.e("MobileAdaptor", "requestIgnoreBatteryOptimizations " + ex.getMessage());
        }
    }

    public static void moveTaskToBack()
    {
        Log.e("MobileAdaptor", "moveTaskToBack");
        m_instance.moveTaskToBack(true);
    }
}
