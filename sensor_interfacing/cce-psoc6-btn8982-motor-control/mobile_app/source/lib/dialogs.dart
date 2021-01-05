import 'package:flutter/rendering.dart';
import 'package:flutter/material.dart';
import 'dart:async';
import 'package:android_intent/android_intent.dart';
import 'dart:io' show Platform;
import 'package:fluttertoast/fluttertoast.dart';

class CustomDialogs {
  static Future<String> goToBluetoothSettings(BuildContext context) {
    return showDialog(
        context: context,
        builder: (dialogContext) {
          return SingleChildScrollView(
              child: AlertDialog(
                  title: Text(
                    "Please turn on Bluetooth!",
                    style: TextStyle(fontSize: 20, color: Colors.blue),
                  ),
                  actions: <Widget>[
                MaterialButton(
                    elevation: 5.0,
                    child: Text("Go to Bluetooth Settings",
                        style: TextStyle(fontSize: 16, color: Colors.pink)),
                    onPressed: () {
                      if (Platform.isAndroid) {
                        final AndroidIntent intent = AndroidIntent(
                          action: 'android.settings.BLUETOOTH_SETTINGS',
                        );
                        intent.launch();
                        Navigator.of(dialogContext).pop();
                      }
                    })
              ]));
        });
  }

  static displayToast(String message) {
    Fluttertoast.showToast(
        msg: message,
        toastLength: Toast.LENGTH_SHORT,
        gravity: ToastGravity.CENTER,
        timeInSecForIosWeb: 2,
        backgroundColor: Colors.blue,
        textColor: Colors.white,
        fontSize: 16.0);
  }
}
