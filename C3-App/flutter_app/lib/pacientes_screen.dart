// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:flutter/material.dart';
import 'registrar_screen.dart';
import 'informacion_screen.dart';
import 'Database.dart';
import 'paciente.dart';


class PacientesScreen extends StatefulWidget {
  @override
  _PacientesScreenState createState() => new _PacientesScreenState();
}


class _PacientesScreenState extends State<PacientesScreen> {

  @override
  Widget build(BuildContext context) {
    final appBar = AppBar(
      title: Text(
        'Lista de pacientes',
        style: TextStyle(
          fontSize: 20.0,
        ),
      ),
    );

    return Scaffold(
      appBar: appBar,
      body: FutureBuilder<List<Paciente>> (
        future: DBProvider.db.getAllPacientes(),
        builder: (BuildContext context, AsyncSnapshot<List<Paciente>> snapshot) {
          print('snapshot: $snapshot');
          if(snapshot.connectionState == ConnectionState.done) {
            if (snapshot.data.length != 0) {
              return ListView.builder(
                itemCount: snapshot.data.length,
                itemBuilder: (BuildContext context, int index) {
                  Paciente paciente = snapshot.data[index];
                  return ListTile(
                    leading: CircleAvatar(
                      child: Text(
                        paciente.nombre.substring(0, 1),
                        style: TextStyle(fontSize: 22.0),
                      ),
                    ),
                    title: Text(
                      paciente.nombre,
                      style: TextStyle(fontSize: 17.0),
                    ),
                    onTap: () {
                      print('-> Info paciente');
                      Navigator.push(
                        context,
                        MaterialPageRoute(
                            builder: (context) => InformacionScreen(paciente)),
                      );
                    },
                  );
                },
              );
            } else {
              return Center(child: Text('No hay información registrada.',
                  style: TextStyle(fontSize: 19)));
            }
          } else {
            return Center(child: CircularProgressIndicator());
          }
        },
      ),
      floatingActionButton: FloatingActionButton(
        elevation: 3.0,
        child: Icon(
          Icons.add,
          size: 35.0,
        ),
        onPressed: (){
          print("-> Registrar");
          Navigator.push(
            context,
            MaterialPageRoute(builder: (context) => RegistrarScreen()),
          );
        },
      )
    );
  }
}
