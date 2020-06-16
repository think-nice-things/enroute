/*!
 * Copyright (C) 2020 by Johannes Zellner, johannes@zellner.org
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

package de.akaflieg_freiburg.enroute;

import android.util.Log;
import android.app.NotificationManager;
import android.content.Context;
import android.app.Notification;
import android.app.Notification.Builder;
import android.app.PendingIntent;
import android.content.Intent;
import android.os.IBinder;
import android.app.Service;
import android.content.pm.ServiceInfo;
import android.os.Build;
import android.app.NotificationChannel;
import android.graphics.BitmapFactory;

import org.qtproject.qt5.android.QtNative;
import org.qtproject.qt5.android.bindings.QtService;

public class EnrouteService extends QtService
{
    private NotificationManager mNM;
    private final int NOTIFICATION_ID = 2604;

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        //TODO do something useful
        Log.d("EnrouteService", "onStartCommand()");
        return Service.START_NOT_STICKY;
    }

    @Override
    public IBinder onBind(Intent intent) {
        //TODO for communication return IBinder implementation
        Log.d("EnrouteService", "onBind()");
        return null;
    }

    @Override
    public void onCreate()
    {
        super.onCreate();
        Log.d("EnrouteService", "onCreate()");

        mNM = (NotificationManager)getSystemService(Context.NOTIFICATION_SERVICE);

        // Display a notification about us starting.  We put an icon in the status bar.
        showNotification();
    }

    @Override
    public void onTaskRemoved(Intent rootIntent)
    {
        Log.d("EnrouteService", "onTaskRemoved()");
        super.onTaskRemoved(rootIntent);
        this.stopSelf();
    }

    @Override
    public void onDestroy()
    {
        super.onDestroy();
        stopForeground(true);
        Log.d("EnrouteService", "super.onDestroy() done.");
    }

    /**
     * Show a notification while this service is running.
     */
    private void showNotification() {

        Log.d("EnrouteService", "showNotification()");

        String channelId = getNotificationChannel();

        // The PendingIntent to launch our activity if the user selects this notification
        //
        Intent notificationIntent = new Intent(this, MobileAdaptor.class);
        PendingIntent pendingIntent = PendingIntent.getActivity(this, 0, notificationIntent, PendingIntent.FLAG_UPDATE_CURRENT);

        // Set the info for the views that show in the notification panel.
        Notification notification = new Notification.Builder(this, channelId)
                .setSmallIcon(R.drawable.bw)  // the status icon
                .setTicker("Enroute Location Service")  // used only if accessibility is turned on
                .setWhen(System.currentTimeMillis())  // the time stamp
                .setContentTitle("Enroute is running")  // the label of the entry
                .setContentIntent(pendingIntent)  // The intent to send when the entry is clicked
                .build();

                // .setLargeIcon(BitmapFactory.decodeResource(getResources(), R.drawable.icon))
                // .setContentText("setContentText")  // the contents of the entry

        notification.flags = Notification.FLAG_NO_CLEAR | Notification.FLAG_ONGOING_EVENT | Notification.FLAG_FOREGROUND_SERVICE;
        startForeground(NOTIFICATION_ID, notification, ServiceInfo.FOREGROUND_SERVICE_TYPE_LOCATION);

        /* foreground notification

        Notification notification =
                  new Notification.Builder(this, CHANNEL_DEFAULT_IMPORTANCE)
            .setContentTitle(getText(R.string.notification_title))
            .setContentText(getText(R.string.notification_message))
            .setTicker(getText(R.string.ticker_text))
            .build();
            */

    }

    private String getNotificationChannel() {

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {

            String channelId = "de.akaflieg_freiburg.enroute";
            String channelName = "Enroute Location Service";
            NotificationChannel channel = new NotificationChannel(channelId, channelName, NotificationManager.IMPORTANCE_LOW); // IMPORTANCE_NONE
            String description = "Channel Description";
            channel.setDescription(description);
            channel.setSound(null, null);
            channel.enableVibration(false);
            channel.setShowBadge(false);
            channel.setLockscreenVisibility(Notification.VISIBILITY_PRIVATE);

            mNM.createNotificationChannel(channel);
            return channelId;

        } else {
            return "";
        }
    }
}
