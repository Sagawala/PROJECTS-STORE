// ignore_for_file: library_private_types_in_public_api, avoid_print, use_build_context_synchronously, depend_on_referenced_packages, unnecessary_string_interpolations

import 'dart:async';
import 'dart:io';
import 'dart:math';

import 'package:firebase_database/firebase_database.dart';
import 'package:flutter/material.dart';
import 'package:path_provider/path_provider.dart';
import 'package:tflite_flutter/tflite_flutter.dart';

class AnalysisPage extends StatefulWidget {
  const AnalysisPage({super.key});

  @override
  _AnalysisPageState createState() => _AnalysisPageState();
}

class _AnalysisPageState extends State<AnalysisPage> {
  double weight = 0.0;
  double gasLevel = 0.0;
  Interpreter? _interpreter;

  final DatabaseReference _databaseReference = FirebaseDatabase.instance.ref();
  late StreamSubscription<DatabaseEvent> _weightSubscription;
  late StreamSubscription<DatabaseEvent> _gasLevelSubscription;

  @override
  void initState() {
    super.initState();
    loadModel();
    subscribeToData();
  }

  Future<void> loadModel() async {
    Directory appDir = await getApplicationDocumentsDirectory();
    String modelPath = '${appDir.path}/tensorflowmodel.tflite';
    try {
      final modelFile = File(modelPath);
      _interpreter = Interpreter.fromFile(modelFile);
      print('Model loaded successfully from $modelPath');
    } catch (e) {
      print('Failed to load model: $e');
    }
  }

  void subscribeToData() {
    _weightSubscription = _databaseReference
        .child('Sensors/Loadcell/weight')
        .onValue
        .listen(updateWeight);
    _gasLevelSubscription = _databaseReference
        .child('Sensors/MQ2/gasLevel')
        .onValue
        .listen(updateGasLevel);
  }

  void updateWeight(DatabaseEvent event) {
    if (event.snapshot.exists) {
      try {
        double newWeight = double.parse(event.snapshot.value.toString());
        setState(() => weight = newWeight);
      } catch (e) {
        print('Error parsing weight data: $e');
      }
    }
  }

  void updateGasLevel(DatabaseEvent event) {
    if (event.snapshot.exists) {
      try {
        double newGasLevel = double.parse(event.snapshot.value.toString());
        setState(() => gasLevel = newGasLevel);
      } catch (e) {
        print('Error parsing gas level data: $e');
      }
    }
  }

  Future<void> predictGasLevelAndWeight() async {
    if (_interpreter == null) {
      print('Model not loaded');
      return;
    }

    try {
      double normalizedGasLevel = normalize(gasLevel, 0, 1000);
      double normalizedWeight = normalize(weight, 100, 2000);

      List<double> inputData = [normalizedGasLevel, normalizedWeight];
      var inputTensor = inputData.reshape([1, 2]);
      var output = List<List<double>>.filled(1, List<double>.filled(4, 0.0));
      _interpreter!.run(inputTensor, output);
      int maxIndex = output[0].indexOf(output[0].reduce(max));

      String condition = determineConditionFromIndex(maxIndex);
      print(
          'Weight: $weight, Gas Level: $gasLevel, Prediction Index: $maxIndex, Condition: $condition');
      showPredictionDialog(context, 'Prediction', condition);
    } catch (e) {
      print('Prediction error: $e');
      showPredictionDialog(
          context, 'Prediction', 'An error occurred during prediction.');
    }
  }

  double normalize(double value, double min, double max) {
    return (value - min) / (max - min);
  }

  String determineConditionFromIndex(int index) {
    switch (index) {
      case 0:
        return "Normal Conditions";
      case 1:
        return "Low Gas Level";
      case 2:
        return "Critical Gas Level";
      case 3:
        return "Gas Leak Detected";
      default:
        return "Unknown Condition";
    }
  }

  void showPredictionDialog(
      BuildContext context, String title, String content) {
    showDialog(
      context: context,
      builder: (BuildContext context) {
        return AlertDialog(
          title: Text(title),
          content: Text(content),
          actions: <Widget>[
            TextButton(
              child: const Text('OK'),
              onPressed: () => Navigator.of(context).pop(),
            ),
          ],
        );
      },
    );
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: AppBar(
        title: const Text('Analysis'),
      ),
      body: ListView(
        padding: const EdgeInsets.all(8.0),
        children: [
          buildActionTile(),
        ],
      ),
    );
  }

  Widget buildActionTile() => Container(
        margin: const EdgeInsets.symmetric(vertical: 10),
        child: ListTile(
          leading: const Icon(Icons.analytics, color: Colors.red),
          title: const Text('Predict Gas Level and Weight'),
          subtitle: const Text('Tap to predict based on current data'),
          onTap: () => predictGasLevelAndWeight(),
          tileColor: Colors.red[50], // Using index to access the lighter shade
          shape:
              RoundedRectangleBorder(borderRadius: BorderRadius.circular(10)),
        ),
      );

  @override
  void dispose() {
    _weightSubscription.cancel();
    _gasLevelSubscription.cancel();
    _interpreter?.close();
    super.dispose();
  }
}
