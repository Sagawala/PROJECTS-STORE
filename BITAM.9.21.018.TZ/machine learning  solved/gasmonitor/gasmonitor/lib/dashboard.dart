// ignore_for_file: library_private_types_in_public_api, constant_identifier_names, use_build_context_synchronously

import 'package:firebase_auth/firebase_auth.dart';
import 'package:firebase_database/firebase_database.dart';
import 'package:flutter/material.dart';
import 'package:gasmonitor/LoginScreen.dart';
import 'package:gasmonitor/alerts.dart';
import 'package:gasmonitor/analysis.dart'; // Import the AnalysisPage
import 'package:percent_indicator/circular_percent_indicator.dart';

class Dashboard extends StatefulWidget {
  const Dashboard({super.key});

  @override
  _DashboardState createState() => _DashboardState();
}

class _DashboardState extends State<Dashboard> {
  final DatabaseReference _databaseReference = FirebaseDatabase.instance.ref();
  late DatabaseReference _weightRef;
  late DatabaseReference _gasLevelRef;

  double weight = 0.0;
  double gasLevel = 0.0;

  static const double MAX_WEIGHT = 20000.0; // Updated to 20,000 grams
  static const int MAX_GAS_LEVEL = 3000; // Maximum ADC value for gas sensor

  @override
  void initState() {
    super.initState();
    _weightRef = _databaseReference.child('Sensors/Loadcell/weight');
    _gasLevelRef = _databaseReference.child('Sensors/MQ2/gasLevel');

    _weightRef.onValue.listen((DatabaseEvent event) {
      final value = event.snapshot.value;
      if (value != null && value is num) {
        setState(() {
          weight = value.toDouble();
        });
      }
    });

    _gasLevelRef.onValue.listen((DatabaseEvent event) {
      final value = event.snapshot.value;
      if (value != null && value is num) {
        setState(() {
          gasLevel = value.toDouble();
        });
      }
    });
  }

  @override
  Widget build(BuildContext context) {
    final double weightPercentage = (weight / MAX_WEIGHT) * 100;
    final double gasLevelPercentage = (gasLevel / MAX_GAS_LEVEL) * 100;
    final User? currentUser = FirebaseAuth.instance.currentUser;

    return Scaffold(
      appBar: AppBar(
        title: const Text('Dashboard'),
      ),
      drawer: Drawer(
        child: ListView(
          padding: EdgeInsets.zero,
          children: <Widget>[
            UserAccountsDrawerHeader(
              accountEmail: Text(currentUser?.email ?? ''),
              decoration: const BoxDecoration(
                color: Color.fromARGB(255, 139, 161, 155),
              ),
              accountName: null,
            ),
            ListTile(
              leading: Icon(Icons.analytics,
                  color: Theme.of(context).colorScheme.onSurface),
              title: const Text('Analysis'),
              onTap: () {
                Navigator.push(
                  context,
                  MaterialPageRoute(builder: (context) => const AnalysisPage()),
                );
              },
            ),
            ListTile(
              leading: Icon(Icons.notifications_active,
                  color: Theme.of(context).colorScheme.onSurface),
              title: const Text('Alerts'),
              onTap: () {
                Navigator.push(
                  context,
                  MaterialPageRoute(builder: (context) => const AlertsScreen()),
                );
              },
            ),
            ListTile(
              leading: Icon(Icons.exit_to_app,
                  color: Theme.of(context).colorScheme.onSurface),
              title: const Text('Sign Out'),
              onTap: () async {
                await FirebaseAuth.instance.signOut();
                Navigator.of(context).pushAndRemoveUntil(
                  MaterialPageRoute(builder: (context) => LoginScreen()),
                  (Route<dynamic> route) => false,
                );
              },
            ),
          ],
        ),
      ),
      body: Center(
        child: Column(
          mainAxisAlignment: MainAxisAlignment.center,
          children: [
            CircularPercentIndicator(
              radius: 100.0,
              lineWidth: 10.0,
              percent: weightPercentage / 100,
              center: Text(
                '${weight.toStringAsFixed(2)} g\n${weightPercentage.toStringAsFixed(2)}%',
                textAlign: TextAlign.center,
                style: const TextStyle(fontSize: 20),
              ),
              progressColor: Colors.blue,
            ),
            const SizedBox(height: 20),
            CircularPercentIndicator(
              radius: 100.0,
              lineWidth: 10.0,
              percent: gasLevelPercentage / 100,
              center: Text(
                '${gasLevel.toStringAsFixed(2)} ppm\n${gasLevelPercentage.toStringAsFixed(2)}%',
                textAlign: TextAlign.center,
                style: const TextStyle(fontSize: 20),
              ),
              progressColor: Colors.green,
            ),
          ],
        ),
      ),
    );
  }
}
