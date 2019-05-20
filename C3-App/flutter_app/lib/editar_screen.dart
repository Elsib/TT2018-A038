// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:intl/intl.dart';
import 'package:flutter/material.dart';
import 'package:flutter/services.dart';
import 'package:datetime_picker_formfield/datetime_picker_formfield.dart';
import 'package:fluttertoast/fluttertoast.dart';

import 'Database.dart';
import 'paciente.dart';

class EditarScreen extends StatefulWidget {
  final Paciente paciente;

  EditarScreen(this.paciente);

  @override
  _EditarScreenState createState() => new _EditarScreenState();
}

class _EditarScreenState extends State<EditarScreen> {

  @override
  void initState() {
    super.initState();
    dropdownStr ??= widget.paciente.sexo;
    antTelefono ??= widget.paciente.telefono;
  }

  final GlobalKey<FormState> _formKey = GlobalKey<FormState>();
  String dropdownStr;
  bool _autoValidate = false;
  Paciente paciente = Paciente();
  String antTelefono;
  DateTime date2;

  @override
  Widget build(BuildContext context) {
    final appBar = AppBar(
      title: Text(
        'Editar paciente',
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
                initialValue: widget.paciente.nombre,
                textCapitalization: TextCapitalization.sentences,
                keyboardType: TextInputType.text,
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
                initialValue: widget.paciente.telefono,
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
                initialValue: DateTime.parse(widget.paciente.fechaNacimiento),
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
                      child: Text('Guardar'), onPressed: _submitForm),
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

      print('Paciente a modificar $antTelefono');
      print('Información del paciente');
      print('Nombre: ${paciente.nombre}');
      print('Telefono: ${paciente.telefono}');
      print('Fecha de nacimiento: ${paciente.fechaNacimiento}');
      print('Sexo: ${paciente.sexo}');
      print('========================================');

      var res = await DBProvider.db.updatePaciente(paciente, antTelefono);

      print(res);

      if(res == null) {
        Fluttertoast.showToast(
            msg: 'El paciente se modificó exitosamente.',
            backgroundColor: Colors.grey,
            textColor: Colors.white,
            toastLength: Toast.LENGTH_LONG
        );

        Navigator.pop(context);
        Navigator.pop(context);

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
