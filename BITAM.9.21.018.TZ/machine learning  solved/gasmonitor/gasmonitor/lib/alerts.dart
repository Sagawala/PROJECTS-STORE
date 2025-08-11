// ignore_for_file: avoid_print, unused_local_variable, prefer_collection_literals, library_private_types_in_public_api

import 'dart:async';

import 'package:firebase_database/firebase_database.dart';
import 'package:flutter/material.dart';
import 'package:flutter_local_notifications/flutter_local_notifications.dart';
import 'package:flutter_staggered_animations/flutter_staggered_animations.dart';

class AlertsScreen extends StatefulWidget {
  const AlertsScreen({super.key});

  @override
  State<AlertsScreen> createState() => _AlertsScreenState();
}

class _AlertsScreenState extends State<AlertsScreen> {
  final DatabaseReference _alertsRef = FirebaseDatabase.instance.ref('alerts');
  Map<String, bool> _alerts = {};
  final Set<String> _notifiedAlerts = Set<String>();
  late final FlutterLocalNotificationsPlugin flutterLocalNotificationsPlugin;

  @override
  void initState() {
    super.initState();
    flutterLocalNotificationsPlugin = FlutterLocalNotificationsPlugin();
    _initializeNotifications();
    _alertsRef.onValue.listen(_handleAlertUpdates, onError: _handleError);
  }

  Future<void> _initializeNotifications() async {
    const AndroidInitializationSettings initializationSettingsAndroid =
        AndroidInitializationSettings('app_icon');
    const InitializationSettings initializationSettings =
        InitializationSettings(android: initializationSettingsAndroid);

    await flutterLocalNotificationsPlugin.initialize(
      initializationSettings,
      onDidReceiveNotificationResponse: _onNotificationResponse,
    );
  }

  void _handleAlertUpdates(DatabaseEvent event) {
    final data = event.snapshot.value as Map<dynamic, dynamic>?;
    if (data != null) {
      setState(() {
        _alerts =
            data.map((key, value) => MapEntry(key as String, value as bool));
        _updateAlertNotifications();
      });
    }
  }

  void _updateAlertNotifications() {
    for (var entry in _alerts.entries) {
      bool isActive = entry.value;
      if (isActive && !_notifiedAlerts.contains(entry.key)) {
        _showNotification(entry.key, _getAlertMessage(entry.key));
        _notifiedAlerts.add(entry.key);
      } else if (!isActive && _notifiedAlerts.contains(entry.key)) {
        _notifiedAlerts.remove(entry.key);
      }
    }
  }

  void _showNotification(String key, String message) async {
    const AndroidNotificationDetails androidDetails =
        AndroidNotificationDetails(
      'alert_channel_id',
      'Alert Channel',
      channelDescription: 'This channel is used for alert notifications',
      importance: Importance.max,
      priority: Priority.high,
      sound: RawResourceAndroidNotificationSound('alert_sound'),
      ticker: 'ticker',
    );

    const NotificationDetails platformChannelSpecifics =
        NotificationDetails(android: androidDetails);
    await flutterLocalNotificationsPlugin.show(
      key.hashCode,
      'Gas Monitoring Alert',
      message,
      platformChannelSpecifics,
      payload: key,
    );
  }

  void _onNotificationResponse(NotificationResponse response) {
    print('Notification Interaction: ${response.payload}');
  }

  void _handleError(Object error) {
    print('Error listening to alerts: $error');
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('Alerts'),
        backgroundColor: const Color.fromARGB(255, 113, 129, 126),
      ),
      body: AnimationLimiter(
        child: ListView.builder(
          itemCount: _alerts.length,
          itemBuilder: (context, index) {
            String key = _alerts.keys.elementAt(index);
            bool isActive = _alerts[key] ?? false;
            return AnimationConfiguration.staggeredList(
              position: index,
              duration: const Duration(milliseconds: 375),
              child: SlideAnimation(
                verticalOffset: 50.0,
                child: FadeInAnimation(
                  child: Card(
                    elevation: 4,
                    margin: const EdgeInsets.all(8),
                    child: ListTile(
                      leading: Icon(
                        isActive ? Icons.warning : Icons.check_circle,
                        color: isActive ? Colors.red : Colors.green,
                      ),
                      title: Text(
                        _getAlertMessage(key),
                        style: const TextStyle(
                          fontWeight: FontWeight.bold,
                          fontSize: 16,
                        ),
                      ),
                      subtitle: Text(
                        isActive ? 'Alert is active' : 'No alert',
                        style: TextStyle(
                          color: isActive ? Colors.red : Colors.green,
                        ),
                      ),
                      onTap: () => _handleAlertTap(key),
                    ),
                  ),
                ),
              ),
            );
          },
        ),
      ),
    );
  }

  void _handleAlertTap(String key) {
    showDialog(
      context: context,
      builder: (context) => AlertDialog(
        title: const Text('Alert Details'),
        content: Text(_getAlertMessage(key)),
        actions: [
          TextButton(
            onPressed: () => Navigator.of(context).pop(),
            child: const Text('OK'),
          ),
        ],
      ),
    );
  }

  String _getAlertMessage(String key) {
    switch (key) {
      case 'criticalGasLevel':
        return 'Critical gas level detected! Immediate action required.';
      case 'lowGasLevel':
        return 'Low gas level detected. Consider refilling soon.';
      case 'gasLeak':
        return 'Gas leak detected! Ensure safety protocols are followed.';
      default:
        return 'Unknown alert. Please check the system.';
    }
  }

  @override
  void dispose() {
    flutterLocalNotificationsPlugin.cancelAll();
    super.dispose();
  }
}
