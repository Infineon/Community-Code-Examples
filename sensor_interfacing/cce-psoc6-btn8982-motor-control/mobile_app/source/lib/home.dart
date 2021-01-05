import 'dart:async';
import 'dart:math';
import 'package:flutter/material.dart';
import 'package:flutter_blue/flutter_blue.dart';
import 'package:fluttericon/font_awesome5_icons.dart';
import 'package:fluttericon/typicons_icons.dart';
import 'package:fluttericon/fontelico_icons.dart';
import 'package:flutter_speedometer/flutter_speedometer.dart';
import 'package:control_pad/control_pad.dart';
import 'dialogs.dart';
import 'robotMovement.dart';
import 'bot_ble.dart';

StreamController<RobotMovement> robotControllerStream =
    new StreamController<RobotMovement>.broadcast();

StreamController<int> robotBatteryPercentageStream =
    new StreamController<int>();
StreamController<bool> connectionStatusStream = new StreamController<bool>();

DateTime appStartTime = DateTime.now();
double joyStickSize;

/*
  The Whole Screen is divided into 10 parts on width and 16 parts on height.

  | 3 Width Parts   |  4 Width Parts    |   3 Width Parts   |
  |_________________|___________________|___________________|
  |  BatteryStatus  | Connection Status |   Refresh Button  | 1 Height Part
  |_________________|___________________|___________________|______________     
  |     Options     |  Speed-o-meter    |  Connection Time  | |           |
  |                 |                   |___________________| |           |
  |                 |                   |     Direction     |7 Parts   14 Parts
  |                 |                   |___________________|_|           |
  |                 |                   |     JoyStick      |             |
  |_________________|___________________|___________________|_____________|
  |               Service Robot Control Center              | 1 Height Part
  |_________________________________________________________|
*/

class Home extends StatefulWidget {
  Home({Key key}) : super(key: key);
  @override
  HomeWidgetState createState() => HomeWidgetState();
}

class HomeWidgetState extends State<Home> {
  String _connectButtonText = "Connect";
  bool _connectButtonState = false;
  static StreamSubscription bluetoothStateSubscription;
  static StreamSubscription<List<ScanResult>> scanSubscription;
  static StreamSubscription<BluetoothDeviceState> deviceConnection;
  static String enteredPasskey;
  int gaugeSpeed = 0;

  List<IconData> bluetoothIcons = [Icons.bluetooth, Icons.bluetooth_disabled];
  static IconData _bluetoothIcon = Icons.bluetooth;

  static final GlobalKey<ScaffoldState> dialogKey =
      new GlobalKey<ScaffoldState>();

  bool dataSendLock = false;

  Timer _timer;
  String timeString = "";

  @override
  void initState() {
    _startTimerBotSpeed();

    bluetoothStateSubscription = BotBle.flutterBlue.state.listen((state) {
      BotBle.bluetoothState = state;

      if (BotBle.bluetoothState == BluetoothState.off) {
        CustomDialogs.displayToast("Please enable Bluetooth!");
        connectionStatusStream.add(false);
        BotBle.isBluetoothOn = false;
        BotBle.deviceState = BluetoothDeviceState.disconnected;
        setState(() {
          print("Bluetooth turned off!");
          _connectButtonText = "Connect";
          _connectButtonState = false;
          _bluetoothIcon = bluetoothIcons[0];
        });
      } else if (BotBle.bluetoothState == BluetoothState.on) {
        CustomDialogs.displayToast("Bluetooth turned ON!");
      }
      print("Bluetooth State: " + BotBle.bluetoothState.toString());

      BotBle.flutterBlue.isOn.then((isOn) {
        BotBle.isBluetoothOn = isOn;
      });
    });

    scanSubscription = BotBle.flutterBlue.scanResults.listen((results) {
      for (ScanResult r in results) {
        if (r.device.name == BotBle.DEVICE_NAME) {
          print('${r.device.name} found! Rssi: ${r.rssi}');
          // Connect to the device
          print("Found Device!");
          CustomDialogs.displayToast("Found Device!");

          BotBle.device = r.device;
          BotBle.flutterBlue.stopScan();
          BotBle.isScanning = false;
          BotBle.deviceNotFound = false;

          BotBle.connectDevice().then((value) {
            print("Device State" + value.toString());
            if (value == BluetoothDeviceState.connected) {
              print("Bot Connected!");
              BotBle.discoverServices().then((servicesFlag) {
                if (servicesFlag) {
                  connectionStatusStream.add(true);
                  BotBle.isPaired = true;
                  setState(() {
                    _connectButtonText = "Disconnect";
                    _bluetoothIcon = bluetoothIcons[1];
                  });
                }
              });
            } else if (value == BluetoothDeviceState.disconnected) {
              connectionStatusStream.add(false);
              print("Bot disconnected!");
            }
          });
        }
      }
    });

    robotControllerStream.stream.listen((movement) async {
      List<int> direction = new List();

      if (dataSendLock) return;

      dataSendLock = true;

      if (BotBle.deviceState == BluetoothDeviceState.connected) {
        await BotBle.sendSpeed([movement.getSpeed()]);

        if (BotBle.botSpeed != null) {
          setState(() {
            gaugeSpeed = BotBle.botSpeed;
          });
        }

        if (movement.getDirection() == Direction.FORWARD) {
          direction.add(0);
          await BotBle.sendDirection(direction);
        } else if (movement.getDirection() == Direction.BACKWARD) {
          direction.add(1);
          await BotBle.sendDirection(direction);
        } else if (movement.getDirection() == Direction.RIGHT) {
          direction.add(2);
          await BotBle.sendDirection(direction);
        } else if (movement.getDirection() == Direction.LEFT) {
          direction.add(3);
          await BotBle.sendDirection(direction);
        }
      } else {
        CustomDialogs.displayToast("Bot not connected!");
      }
      dataSendLock = false;
    });

    super.initState();
  }

  void _startTimerBotSpeed() {
    _timer = Timer.periodic(Duration(seconds: 1), (timer) {
      /* Listen to device state every 1s */

      if (BotBle.deviceNotFound) {
        BotBle.deviceNotFound = false;
        setState(() {
          _connectButtonText = "Connect";
          _connectButtonState = false;
          connectionStatusStream.add(false);
          _bluetoothIcon = bluetoothIcons[0];
        });
      }

      if (BotBle.device != null) {
        BotBle.device.state.listen((event) {
          /* Listen to disconnection from device side */
          if (BotBle.deviceState == BluetoothDeviceState.connected) {
            if (event == BluetoothDeviceState.disconnected) {
              BotBle.resetData();
              CustomDialogs.displayToast("Lost connection to Bot!");
              setState(() {
                _connectButtonText = "Connect";
                _connectButtonState = false;
                connectionStatusStream.add(false);
                _bluetoothIcon = bluetoothIcons[0];
              });
            }
          }
        });
      }
    });
  }

  @override
  Widget build(BuildContext context) {
    joyStickSize = calculateJoystickViewSize(context);
    double speedOMeterSize = calculateSpeedOMeterSize(context);

    return Scaffold(
        resizeToAvoidBottomPadding: false,
        resizeToAvoidBottomInset: false,
        body: Column(
          children: <Widget>[
            Expanded(
              flex: 1,
              child: TopRow(),
            ),
            Expanded(
              flex: 13,
              child: Row(
                children: <Widget>[
                  Expanded(
                    flex: 3,
                    child: Padding(
                      padding: const EdgeInsets.only(top: 80.0),
                      child: Column(
                        crossAxisAlignment: CrossAxisAlignment.center,
                        children: <Widget>[
                          Text(
                            "Options",
                            style: TextStyle(
                              fontSize: 25,
                              fontWeight: FontWeight.bold,
                              color: Colors.blue,
                            ),
                          ),
                          Container(
                            width: 150,
                            height: 100,
                            child: Column(
                              mainAxisAlignment: MainAxisAlignment.start,
                              crossAxisAlignment: CrossAxisAlignment.stretch,
                              children: [
                                RaisedButton.icon(
                                  icon: new Icon(_bluetoothIcon,
                                      color: Colors.blue),
                                  label: Text(_connectButtonText,
                                      style: TextStyle(color: Colors.blue)),
                                  color: Colors.white,
                                  onPressed: () {
                                    calculateJoystickViewSize(context);

                                    String currButtonState = _connectButtonText;
                                    _connectButtonState
                                        ? _connectButtonText = "Connect"
                                        : _connectButtonText = "Disconnect";
                                    _connectButtonState = !_connectButtonState;

                                    if (_connectButtonText == "Connect") {
                                      _bluetoothIcon = bluetoothIcons[0];
                                      setState(() {
                                        _connectButtonText;
                                        _bluetoothIcon;
                                      });
                                    } else {
                                      _bluetoothIcon = bluetoothIcons[1];
                                      setState(() {
                                        _connectButtonText;
                                        _bluetoothIcon;
                                      });
                                    }

                                    // Reverse logic since previous lines invert it
                                    if (currButtonState == "Connect") {
                                      _bluetoothIcon = bluetoothIcons[1];

                                      if (BotBle.isBluetoothOn) {
                                        BotBle.startScan(
                                            BotBle.SCANNING_TIMEOUT);
                                      } else {
                                        CustomDialogs.displayToast(
                                            "Please enable bluetooth!");
                                        _connectButtonText = "Connect";
                                        _connectButtonState = false;
                                        _bluetoothIcon = bluetoothIcons[0];
                                        setState(() {
                                          _connectButtonText;
                                          _bluetoothIcon;
                                          connectionStatusStream.add(false);
                                        });
                                      }
                                    } else {
                                      BotBle.disconnect();
                                      _bluetoothIcon = bluetoothIcons[0];
                                      _connectButtonState = false;
                                      _connectButtonText = "Connect";
                                      CustomDialogs.displayToast(
                                          "Disconnected");
                                      setState(() {
                                        _connectButtonText;
                                        _bluetoothIcon;
                                        connectionStatusStream.add(false);
                                      });
                                    }
                                  },
                                ),
                              ],
                            ),
                          )
                        ],
                      ),
                    ),
                  ),
                  Expanded(
                    flex: 4,
                    child: Column(
                      mainAxisAlignment: MainAxisAlignment.center,
                      crossAxisAlignment: CrossAxisAlignment.center,
                      children: <Widget>[
                        Container(
                          height: 300,
                          width: 300,
                          child: Speedometer(
                            size: speedOMeterSize,
                            minValue: 0,
                            maxValue: 100,
                            currentValue: gaugeSpeed,
                            warningValue: 80,
                            displayText: "Speed (m/s)",
                            displayTextStyle: TextStyle(
                              fontSize: 24,
                              fontWeight: FontWeight.w500,
                              color: Colors.orange[800],
                            ),
                            backgroundColor:
                                Colors.grey[50], //Blend with background color
                            kimColor: Colors.black,
                            meterColor: Colors.green,
                            displayNumericStyle: TextStyle(
                                fontSize: 40,
                                fontWeight: FontWeight.bold,
                                color: Colors.red[400]),
                          ),
                        ),
                      ],
                    ),
                  ),
                  Expanded(
                    flex: 3,
                    child: Column(
                      crossAxisAlignment: CrossAxisAlignment.center,
                      mainAxisAlignment: MainAxisAlignment.spaceBetween,
                      children: <Widget>[
                        Padding(
                          padding: const EdgeInsets.only(top: 40.0),
                          child: TimeCounter(),
                        ),
                        JoyStick(),
                      ],
                    ),
                  ),
                ],
              ),
            ),
            Expanded(
              flex: 1,
              child: Row(
                mainAxisAlignment: MainAxisAlignment.center,
                children: <Widget>[
                  Padding(
                      padding: EdgeInsets.only(right: 10.0, bottom: 10),
                      child: Icon(
                        FontAwesome5.robot,
                        size: 20,
                        color: Colors.grey[700],
                      )),
                  Text(
                    "BLE Motor Control Demo",
                    style: TextStyle(
                      fontSize: 18,
                      fontWeight: FontWeight.bold,
                      color: Colors.grey[700],
                    ),
                  ),
                ],
              ),
            ),
          ],
        ));
  }
}

calculateJoystickViewSize(BuildContext context) {
  /*
    Width is divided into 10 parts.
    These 10 parts are distributed as 3(LeftPart with buttons), 4(SpeedoMeter),
    3(JoyStick Column)
    Hence to get the width of 
    JoyStick Column (TotalWidth/10) * 3
    Height is divided into 16 parts.
    These are distributed as 1(To the Top Bar), 
    14(To the middle row(Options,SpeedoMeter,JoyStick)),1(To the down bar) 
    The Joystick columns height is divided into 7 for joystick and 
    7 for Direction and time.
    Hence To get the height
    ((TotalHeight/16 * 7) - 40) (-40 is just to leave some space). 
  */
  double width = MediaQuery.of(context).size.width;
  double height = MediaQuery.of(context).size.width;

  double w = ((width / 10) * 3) - 40;
  double h = ((height / 16) * 7) - 40;
  return min(w, h);
}

calculateSpeedOMeterSize(BuildContext context) {
  double width = MediaQuery.of(context).size.width;
  double height = MediaQuery.of(context).size.width;

  double w = ((width / 10) * 4) - 80;
  double h = ((height / 16) * 8) - 80;
  return min(w, h);
}

class TopRow extends StatefulWidget {
  @override
  _TopRowState createState() => _TopRowState();
}

class _TopRowState extends State<TopRow> {
  static List<IconData> batteryIcons = [
    Typicons.bat1,
    Typicons.bat2,
    Typicons.bat3,
    Typicons.bat4
  ];

  IconData currentBatteryIcon = batteryIcons[3];
  Color currentBatteryColor = Colors.blue;
  Color connectionStatusColor = Colors.red;
  String connectionString = "Disconnected";
  int currentBatteryPercent = 100;

  @override
  void initState() {
    super.initState();

    robotBatteryPercentageStream.stream.listen((int batteryPercent) {
      setState(() {
        currentBatteryPercent = batteryPercent;
        if (batteryPercent > 75) {
          currentBatteryIcon = batteryIcons[3];
          currentBatteryColor = Colors.green;
        } else if (batteryPercent > 50) {
          currentBatteryIcon = batteryIcons[2];
          currentBatteryColor = Colors.orange;
        } else if (batteryPercent > 25) {
          currentBatteryIcon = batteryIcons[1];
          currentBatteryColor = Colors.orange;
        } else {
          currentBatteryIcon = batteryIcons[0];
          currentBatteryColor = Colors.red;
        }
      });
    });

    connectionStatusStream.stream.listen((bool isConnected) {
      setState(() {
        connectionStatusColor = isConnected ? Colors.green : Colors.red;
        connectionString = isConnected ? "Connected" : "Disconnected";
      });
    });
  }

  @override
  Widget build(BuildContext context) {
    return Row(
      mainAxisAlignment: MainAxisAlignment.spaceBetween,
      crossAxisAlignment: CrossAxisAlignment.start,
      children: <Widget>[
        Row(
          crossAxisAlignment: CrossAxisAlignment.start,
          children: <Widget>[
            IconButton(
              icon: new Icon(
                currentBatteryIcon,
                color: currentBatteryColor,
              ),
              iconSize: 30,
              padding: EdgeInsets.only(top: 2.5, left: 5),
              alignment: Alignment.topCenter,
            ),
            Padding(
              padding: const EdgeInsets.only(top: 2.5),
              child: Text(
                "$currentBatteryPercent%",
                style: TextStyle(
                  fontSize: 25,
                  fontWeight: FontWeight.bold,
                  color: currentBatteryColor,
                ),
              ),
            )
          ],
        ),
        Padding(
          padding: const EdgeInsets.only(top: 8.0),
          child: Row(
            crossAxisAlignment: CrossAxisAlignment.start,
            children: <Widget>[
              Icon(
                FontAwesome5.circle,
                color: connectionStatusColor,
                size: 20,
              ),
              SizedBox(width: 5),
              Text(
                connectionString,
                style: TextStyle(
                    fontStyle: FontStyle.normal,
                    fontSize: 18,
                    color: connectionStatusColor,
                    fontWeight: FontWeight.bold),
              )
            ],
          ),
        ),
        Padding(
          padding: const EdgeInsets.only(top: 7.5),
          child: Row(
            crossAxisAlignment: CrossAxisAlignment.start,
            children: <Widget>[
              Text(
                "Reserved",
                style: TextStyle(
                  fontSize: 18,
                  fontWeight: FontWeight.bold,
                  color: Colors.blue,
                ),
              ),
              IconButton(
                icon: Icon(Fontelico.spin3, color: Colors.blue),
                iconSize: 16,
                padding: EdgeInsets.only(top: 5.0, right: 15.0),
                alignment: Alignment.topCenter,
                onPressed: () {
                  CustomDialogs.displayToast(
                      "Not used currently, stay tuned for Rev_1");
                },
              ),
            ],
          ),
        ),
      ],
    );
  }
}

class TimeCounter extends StatefulWidget {
  TimeCounter({Key key}) : super(key: key);

  @override
  _TimeCounterState createState() => _TimeCounterState();
}

class _TimeCounterState extends State<TimeCounter> {
  Timer _timer;
  String timeString = "";

  @override
  void initState() {
    super.initState();
    _startTimer();
  }

  @override
  Widget build(BuildContext context) {
    return Text(
      timeString,
      style: TextStyle(
        fontSize: 25,
        fontWeight: FontWeight.bold,
        color: Colors.grey[600],
      ),
    );
  }

  void _startTimer() {
    _timer = Timer.periodic(Duration(seconds: 1), (timer) {
      setState(() {
        var newTime = DateTime.now();
        var difference = newTime.difference(appStartTime);
        var hours = difference.inHours;
        var minutes = difference.inMinutes - (60 * hours);
        var seconds = difference.inSeconds - (60 * minutes) - (60 * 60 * hours);
        timeString = "\u{23F1} $hours:$minutes:$seconds";
      });
    });
  }

  void _stopTimer() {
    _timer.cancel();
  }
}

class JoyStick extends StatefulWidget {
  @override
  _JoyStickState createState() => _JoyStickState();
}

class _JoyStickState extends State<JoyStick> {
  String _direction = "Direction: Centre";

  @override
  void initState() {
    robotControllerStream.stream.listen((movement) {
      setState(() {
        if (movement.getDirection() == Direction.CENTRE)
          _direction = "Direction: Centre";
        else if (movement.getDirection() == Direction.FORWARD)
          _direction = "Direction: Forward";
        else if (movement.getDirection() == Direction.LEFT)
          _direction = "Direction: Left";
        else if (movement.getDirection() == Direction.RIGHT)
          _direction = "Direction: Right";
        else
          _direction = "Direction: Reverse";
      });
    });
    super.initState();
  }

  @override
  Widget build(BuildContext context) {
    return Column(
      children: <Widget>[
        Padding(
          padding: const EdgeInsets.only(bottom: 30.0),
          child: Text(
            _direction,
            style: TextStyle(
              fontSize: 20,
              fontWeight: FontWeight.bold,
              color: Colors.grey[600],
            ),
          ),
        ),
        JoystickView(
          size: joyStickSize,
          onDirectionChanged: (degrees, distanceMoved) {
            print("Moved");
            robotControllerStream.add(new RobotMovement(
                degrees: degrees, joystickDistanceMoved: distanceMoved));
            //var rand = new Random();
            //robotBatteryPercentageStream.add(rand.nextInt(100));
          },
          opacity: 0.75,
          showArrows: true,
        ),
      ],
    );
  }
}
