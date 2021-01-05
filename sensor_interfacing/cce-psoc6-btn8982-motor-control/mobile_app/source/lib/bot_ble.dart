import 'package:flutter_blue/flutter_blue.dart';
import 'package:motor_control/dialogs.dart';
import 'package:motor_control/home.dart';

class BotBle {
  static final String DEVICE_NAME = "BleMotor";
  static final int SCANNING_TIMEOUT = 10;

  static FlutterBlue flutterBlue = FlutterBlue.instance;

  /* Control Service */
  static final String ControlServiceUUID =
      "62276C4C-FAA5-4DEA-AD38-0BA0C7692BEF";
  static final String SpeedCharacteristicUUID =
      "EE2F64F2-8574-429A-81A4-C6CE31A57175";
  static final String DirectionCharacteristicUUID =
      "3950866D-BE3F-4810-8C78-999E9E58A3A6";

  static int botSpeed = 0;

  static List<int> DIRECTION = [-1];
  static List<int> SPEED = [-1];
  static BluetoothDevice device;
  static BluetoothState bluetoothState;
  static BluetoothDeviceState deviceState;
  static BluetoothService controlService;
  static bool isBluetoothOn = false;
  static bool checkModifiedState = false;
  static bool isConnected = false;
  static bool isScanning = false;
  static bool isPaired = false;
  static bool deviceNotFound = false;

  static startScan(int timeout) {
    print("Starting Scan!");
    isScanning = true;
    flutterBlue.startScan(timeout: Duration(seconds: timeout)).whenComplete(() {
      if (isScanning) {
        print("Couldn't find device!");
        CustomDialogs.displayToast('Cannot find device');
        flutterBlue.stopScan();
        isScanning = false;
        deviceNotFound = true;
      }
    });
  }

  static Future<BluetoothDeviceState> connectDevice() async {
    if (deviceState != BluetoothDeviceState.connected && isBluetoothOn) {
      try {
        await device.connect();
        deviceState = BluetoothDeviceState.connected;
        print("Connected to device");
        return deviceState;
      } catch (e) {
        print(e.toString());
        return Future.error("Failed to connect!");
      }
    } else {
      print("Device already connected!");
      return deviceState;
    }
  }

  static Future<bool> discoverServices() async {
    if ((deviceState != BluetoothDeviceState.connected || device == null) &&
        !isBluetoothOn) {
      return false;
    }

    List<BluetoothService> services = await device.discoverServices();

    services.forEach((service) {
      // do something with service
      if (service.uuid.toString() == ControlServiceUUID.toLowerCase()) {
        controlService = service;
        print("Found control service");
      }
    });

    if (controlService != null) {
      print("Discovery complete: $device");
      return true;
    } else {
      return false;
    }
  }

  static void sendDirection(List<int> direction) async {
    if (deviceState != BluetoothDeviceState.connected ||
        device == null ||
        controlService == null) {
      print("Control service not found for sending direction!");
    }

    print("Sending Direction: $direction");

    for (BluetoothCharacteristic characteristic
        in controlService.characteristics) {
      if (characteristic.uuid.toString() ==
          DirectionCharacteristicUUID.toLowerCase()) {
        try {
          print("Writing to direction characteristic");
          await characteristic.write(direction, withoutResponse: false);
        } catch (e) {
          print(e.toString());
        }
      }
    }
  }

  static Future<int> sendSpeed(List<int> speed) async {
    if (deviceState != BluetoothDeviceState.connected ||
        device == null ||
        controlService == null) {
      print("Control service not found for sending speed!");
      return 0;
    }

    print("Sending speed: ${speed}");

    for (BluetoothCharacteristic characteristic
        in controlService.characteristics) {
      if (characteristic.uuid.toString() ==
          SpeedCharacteristicUUID.toLowerCase()) {
        try {
          print("Writing to speed characteristic");
          await characteristic.write(speed, withoutResponse: false);
          await characteristic.setNotifyValue(true);
          characteristic.value.listen(_speedChanged);
        } catch (e) {
          print(e.toString());
          return 0;
        }
      }
    }
  }

  static _speedChanged(List<int> values) {
    print('Incoming Data: ' + values.toString());
    if (values.length > 0) {
      botSpeed = values.first;
      print("Bot speed:" + botSpeed.toString());
    }
  }

  static void disconnect() {
    // Disconnect from device
    print("Disconnect issued");
    if (device != null) {
      sendSpeed([0]);
      device.disconnect();
      deviceState = BluetoothDeviceState.disconnected;
      device = null;
      isScanning = false;
      isConnected = false;
      botSpeed = 0;
      controlService = null;
      isPaired = false;
    } else {
      CustomDialogs.displayToast("No device connected!");
    }
  }

  static void resetData() {
    deviceState = BluetoothDeviceState.disconnected;
    isScanning = false;
    isConnected = false;
    botSpeed = 0;
  }
}
