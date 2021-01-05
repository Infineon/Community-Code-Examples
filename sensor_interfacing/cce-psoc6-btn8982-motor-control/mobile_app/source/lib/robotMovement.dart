import 'package:flutter/material.dart';

class RobotMovement {
  int _direction;
  int _speed;
  String _directionLabel;

  RobotMovement(
      {@required double degrees, @required double joystickDistanceMoved}) {
    _direction = _calculateDirection(degrees);
    _speed = _calculateSpeed(joystickDistanceMoved);
    _directionLabel = _getMovementDirection(_direction);
  }

  int _calculateDirection(double degrees) {
    int direction;
    if (degrees < 5) {
      direction = Direction.CENTRE;
    } else if (degrees <= 45 || degrees > 315)
      direction = Direction.FORWARD;
    else if (degrees > 45 && degrees <= 135)
      direction = Direction.RIGHT;
    else if (degrees > 135 && degrees <= 225)
      direction = Direction.BACKWARD;
    else if (degrees > 225 && degrees <= 315) direction = Direction.LEFT;
    return direction;
  }

  String _getMovementDirection(int direction) {
    String _directionLabel = "Centre";
    if (direction == Direction.FORWARD) {
      _directionLabel = "Forward";
    } else if (direction == Direction.BACKWARD) {
      _directionLabel = "Backward";
    } else if (direction == Direction.LEFT) {
      _directionLabel = "Left";
    } else if (direction == Direction.RIGHT) {
      _directionLabel = "Right";
    } else {
      _directionLabel = "Centre";
    }
    return _directionLabel;
  }

  int _calculateSpeed(double distanceMoved) {
    int speed;
    if (distanceMoved < 0.20)
      speed = 0;
    else if (distanceMoved < 0.40 && distanceMoved >= 0.20)
      speed = 25;
    else if (distanceMoved < 0.60 && distanceMoved >= 0.40)
      speed = 50;
    else if (distanceMoved < 0.80 && distanceMoved >= 0.60)
      speed = 75;
    else
      speed = 100;
    return speed;
  }

  int getDirection() {
    return _direction;
  }

  int getSpeed() {
    return _speed;
  }

  String getDirectionLabel() {
    return _directionLabel;
  }
}

class Direction {
  static const int CENTRE = 0;
  static const int FORWARD = 1;
  static const int BACKWARD = 2;
  static const int RIGHT = 3;
  static const int LEFT = 4;
}
