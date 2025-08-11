import 'dart:math';

import 'package:flutter/material.dart';

class CircleProgress extends CustomPainter {
  final double value;
  final bool isWeight;
  final double strokeWidth;
  final Color weightColor;
  final Color gasColor;
  final int maximumValue;

  CircleProgress({
    required this.value,
    required this.isWeight,
    this.strokeWidth = 14.0,
    this.weightColor = Colors.green,
    this.gasColor = Colors.red,
    this.maximumValue = 20000, // Updated to 20,000 grams
  });

  @override
  bool shouldRepaint(CustomPainter oldDelegate) {
    if (oldDelegate is CircleProgress) {
      return value != oldDelegate.value || isWeight != oldDelegate.isWeight;
    }
    return true;
  }

  @override
  void paint(Canvas canvas, Size size) {
    Paint outerCircle = Paint()
      ..strokeWidth = strokeWidth
      ..color = Colors.grey
      ..style = PaintingStyle.stroke;

    Paint arcPaint = Paint()
      ..strokeWidth = strokeWidth
      ..style = PaintingStyle.stroke
      ..strokeCap = StrokeCap.round;

    Offset center = Offset(size.width / 2, size.height / 2);
    double radius = min(size.width / 2, size.height / 2) - strokeWidth / 2;
    canvas.drawCircle(center, radius, outerCircle);

    double angle = 2 * pi * (value / maximumValue);

    arcPaint.color = isWeight ? weightColor : gasColor;

    canvas.drawArc(
      Rect.fromCircle(center: center, radius: radius),
      -pi / 2,
      angle,
      false,
      arcPaint,
    );
  }
}
