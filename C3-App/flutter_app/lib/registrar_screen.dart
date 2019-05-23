// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:intl/intl.dart';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:datetime_picker_formfield/datetime_picker_formfield.dart';
import 'package:fluttertoast/fluttertoast.dart';
import 'pacientes_screen.dart';

import 'Database.dart';
import 'paciente.dart';

class RegistrarScreen extends StatefulWidget {
  @override
  _RegistrarScreenState createState() => new _RegistrarScreenState();
}

class _RegistrarScreenState extends State<RegistrarScreen> {
  final GlobalKey<FormState> _formKey = GlobalKey<FormState>();
  String dropdownStr = '';
  bool _autoValidate = false;
  Paciente paciente = Paciente();
  DateTime date2;

  @override
  Widget build(BuildContext context) {
    final appBar = AppBar(
      title: Text(
        'Registrar paciente',
        style: TextStyle(
          fontSize: 20.0,
        ),
      ),
    );

    return Scaffold(
      appBar: appBar,
      body: SafeArea(
        top: false,
        bottom: false,
        child: Form(
          key: _formKey,
          autovalidate: _autoValidate,
          child: ListView(
            padding: EdgeInsets.symmetric(horizontal: 16.0),
            children: [
              TextFormField(
                decoration: InputDecoration(
                  icon: Icon(
                    Icons.person,
                    color: Colors.black87,
                    size: 29.0,
                  ),
                  labelText: 'Nombre',
                ),
                keyboardType: TextInputType.text,
                textCapitalization: TextCapitalization.sentences,
                inputFormatters: [new LengthLimitingTextInputFormatter(30)],
                validator: validarNombre,
                onSaved: (val) => paciente.nombre = val,
              ),
              TextFormField(
                decoration: InputDecoration(
                  icon: Icon(
                    Icons.phone,
                    color: Colors.black87,
                    size: 29.0,
                  ),
                  labelText: 'Teléfono',
                ),
                keyboardType: TextInputType.phone,
                inputFormatters: [
                  WhitelistingTextInputFormatter.digitsOnly,
                ],
                validator: validarTelefono,
                onSaved: (val) => paciente.telefono = val,
              ),
              FormField<String>(
                builder: (FormFieldState<String> state) {
                  return InputDecorator(
                    decoration: InputDecoration(
                      icon: Icon(
                        Icons.people,
                        color: Colors.black87,
                        size: 29.0,
                      ),
                      labelText: 'Sexo',
                      errorText: state.hasError ? state.errorText : null,
                    ),
                    isEmpty: dropdownStr == '',
                    child: DropdownButtonHideUnderline(
                      child: DropdownButton<String>(
                        value: dropdownStr,
                        isDense: true,
                        onChanged: (String newValue) {
                          setState(() {
                            paciente.sexo = newValue;
                            dropdownStr = newValue;
                            state.didChange(newValue);
                          });
                        },
                        items: <String>['','Femenino', 'Masculino']
                            .map((String value) {
                          return DropdownMenuItem<String>(
                            value: value,
                            child: Text(value),
                          );
                        }).toList(),
                      ),
                    ),
                  );
                },
                validator: (val) {
                  if(val == null){
                    return 'Seleccione una opción válida';
                  }
                  return val != '' ? null : 'Seleccione una opción válida';
                },
              ),
              DateTimePickerFormField(
                inputType: InputType.date,
                format: DateFormat("dd/MM/yyyy"),
                firstDate: new DateTime(1900),
                initialDate: DateTime.now(),
                lastDate: DateTime.now(),
                editable: false,
                decoration: InputDecoration(
                    labelText: 'Fecha de nacimiento',
                    hasFloatingPlaceholder: false),
                onChanged: (dt) {
                  setState(() => date2 = dt);

                  print('Selected date: $date2');
                },
                validator: (val) =>
                val == null ? 'Ingrese la fecha de nacimiento' : null,
                onSaved: (val) => paciente.fechaNacimiento = val.toString(),
              ),
              Padding(
                padding: EdgeInsets.only(top: 35.0),
                child: Center(
                  child: RaisedButton(
                      child: Text('Registrar'), onPressed: _submitForm),
                ),
              )
            ],
          ),
        ),
      ),
    );
  }

  void _submitForm() async {
    if (!_formKey.currentState.validate()) {
      setState(() {
        _autoValidate = true;
      });
    } else {
      _formKey.currentState.save();

      print('Información del paciente');
      print('Nombre: ${paciente.nombre}');
      print('Telefono: ${paciente.telefono}');
      print('Fecha de nacimiento: ${paciente.fechaNacimiento}');
      print('Sexo: ${paciente.sexo}');
      print('========================================');

      var res = await DBProvider.db.insertPaciente(paciente);

      print(res);

      if(res > 0) {
        Fluttertoast.showToast(
            msg: 'El paciente se registró exitosamente.',
            backgroundColor: Colors.grey,
            textColor: Colors.white,
            toastLength: Toast.LENGTH_LONG
        );

        Navigator.pop(
          context,
          MaterialPageRoute(builder: (context) => PacientesScreen()),
        );

      } else if(res == -1) {
        Fluttertoast.showToast(
            msg: 'No puede registrar un número duplicado.',
            backgroundColor: Colors.grey,
            textColor: Colors.white,
            toastLength: Toast.LENGTH_LONG
        );
      } else {
        Fluttertoast.showToast(
          msg: 'Ha ocurrido un error.',
          backgroundColor: Colors.grey,
          textColor: Colors.white,
          toastLength: Toast.LENGTH_LONG
        );
      }
    }
  }

  String validarNombre(String value) {
    if(value.isEmpty) {
      return 'Este campo es obligatorio.';
    }
    else if (value.length > 60) {
      return 'Este campo debe contener máximo 60 caracteres.';
    } else {
      return null;
    }
  }

  String validarTelefono(String value) {
    if(value.isEmpty) {
      return 'Este campo es obligatorio.';
    }
    else if (value.length < 10) {
      return 'Este campo debe contener al menos 10 digitos';
    } else {
      return null;
    }
  }
}
