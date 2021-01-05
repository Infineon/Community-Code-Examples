import 'package:flutter/material.dart';
import 'package:rflutter_alert/rflutter_alert.dart';
class Alerts{

static successAlert({BuildContext context, String title, String description}) {
  var style =
      _getAlertStyle(titleColor: Colors.green, borderColor: Colors.green);
  _showAlert(
      context: context,
      title: title,
      desc: description,
      type: AlertType.success,
      style: style);
}

static warningAlert({BuildContext context, String title, String description}) {
  var style =
      _getAlertStyle(titleColor: Colors.orange, borderColor: Colors.orange);
  _showAlert(
      context: context,
      title: title,
      desc: description,
      type: AlertType.warning,
      style: style);
}

static failureAlert({BuildContext context, String title, String description}) {
  var style = _getAlertStyle(titleColor: Colors.red, borderColor: Colors.red);
  _showAlert(
      context: context,
      title: title,
      desc: description,
      type: AlertType.error,
      style: style);
}

static _showAlert(
    {BuildContext context,
    String title,
    String desc,
    AlertType type,
    AlertStyle style}) {
  return Alert(
    context: context,
    style: style,
    type: type,
    title: title,
    desc: desc,
    buttons: [],
    closeFunction: () {},
  ).show();
}

static showMaterialAlert(BuildContext context, String title, String description){
  
  showDialog(
      context: context,
      builder: (BuildContext context) {
        // return object of type Dialog
        return AlertDialog(
          title: new Text(title),
          content: new Text(description),
          actions: <Widget>[
            // usually buttons at the bottom of the dialog
            new FlatButton(
              child: new Text("Close"),
              onPressed: () {
                Navigator.of(context, rootNavigator: true).pop();
              },
            ),
          ],
        );
      },
    );
}

static _getAlertStyle(
    {@required Color titleColor,
    @required Color borderColor,
    bool isCloseButtonRequired = true}) 
{
  return AlertStyle(
    animationType: AnimationType.fromTop,
    isCloseButton: isCloseButtonRequired,
    isOverlayTapDismiss: false,
    descStyle: TextStyle(fontWeight: FontWeight.bold),
    animationDuration: Duration(milliseconds: 400),
    alertBorder: RoundedRectangleBorder(
      borderRadius: BorderRadius.circular(0.0),
      side: BorderSide(
        color: borderColor,
      ),
    ),
    titleStyle: TextStyle(
      color: titleColor,
    ),
    constraints: BoxConstraints.expand(width: 300),
  );
}

}
