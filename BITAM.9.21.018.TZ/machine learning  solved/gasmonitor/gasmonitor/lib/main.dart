// ignore_for_file: avoid_print, unnecessary_null_comparison, depend_on_referenced_packages

import 'dart:io';

import 'package:firebase_auth/firebase_auth.dart';
import 'package:firebase_core/firebase_core.dart';
import 'package:firebase_ml_model_downloader/firebase_ml_model_downloader.dart';
import 'package:flutter/material.dart';
import 'package:path/path.dart' as path;
import 'package:path_provider/path_provider.dart';

import 'LoginScreen.dart';
import 'dashboard.dart';
import 'signup_screen.dart';

void main() async {
  WidgetsFlutterBinding.ensureInitialized();
  await Firebase.initializeApp();

  // Setup model download conditions and start the download
  FirebaseModelDownloadConditions conditions = FirebaseModelDownloadConditions(
      iosAllowsCellularAccess: true,
      iosAllowsBackgroundDownloading: false,
      androidChargingRequired: false,
      androidWifiRequired: false,
      androidDeviceIdleRequired: false);

  FirebaseModelDownloader.instance
      .getModel('gasmonitor', FirebaseModelDownloadType.localModel, conditions)
      .then((customModel) async {
    if (customModel != null && customModel.file != null) {
      print("Model downloaded successfully to ${customModel.file.path}");

      // Define the path where you want to save the model in the app's documents directory
      Directory appDir = await getApplicationDocumentsDirectory();
      String localModelPath = path.join(appDir.path, 'tensorflowmodel.tflite');

      // Copy the downloaded model to the documents directory
      await customModel.file.copy(localModelPath);
      print("Model saved to $localModelPath");

      // You can now pass this path to your model manager or wherever needed
    } else {
      print("Model download failed or received empty model");
    }
  }).catchError((error) {
    print("Failed to download model: $error");
  });

  runApp(const MyApp());
}

class MyApp extends StatelessWidget {
  const MyApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'Gasmonitor',
      theme: ThemeData(
        primarySwatch: Colors.grey,
      ),
      initialRoute: '/',
      routes: {
        '/': (context) => const AuthWrapper(),
        '/signup': (context) => SignUpScreen(),
        '/login': (context) => LoginScreen(),
        '/dashboard': (context) => const Dashboard(),
      },
      debugShowCheckedModeBanner: false,
    );
  }
}

class AuthWrapper extends StatelessWidget {
  const AuthWrapper({super.key});

  @override
  Widget build(BuildContext context) {
    return StreamBuilder<User?>(
      stream: FirebaseAuth.instance.authStateChanges(),
      builder: (context, snapshot) {
        if (snapshot.connectionState == ConnectionState.active) {
          if (snapshot.hasData) {
            return const Dashboard(); // User is logged in
          }
          return LoginScreen(); // User is not logged in or has logged out
        }
        return const Center(
            child:
                CircularProgressIndicator()); // Waiting for authentication data
      },
    );
  }
}
