// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

import 'package:flutter/material.dart';
import 'package:sms/sms.dart';
import 'paciente.dart';
import 'medicion.dart';
import 'Database.dart';
import 'package:fluttertoast/fluttertoast.dart';
import 'editar_screen.dart';


class InformacionScreen extends StatefulWidget {
  InformacionScreen(this.paciente);

  Paciente paciente;

  @override
  InformacionScreenState createState() => InformacionScreenState();
}

class InformacionScreenState extends State<InformacionScreen> {
  int _selectedIndex = 0;

  @override
  Widget build(BuildContext context) {
    final listView = ListView(
      children: [
        Column(
          children: [
            Padding(
              padding: EdgeInsets.all(8.0),
              child: Card(
                child: Column(
                  children: [
                    ListTile(
                      title: Text('Información general',
                          style: TextStyle(fontSize: 20.0)),
                    ),
                    Divider(),
                    ListTile(
                      title: Text('Teléfono: ${widget.paciente.telefono}',
                          style: TextStyle(fontSize: 16.0)),
                    ),
                    ListTile(
                      title: Text(
                          'Edad: ${(DateTime.now().difference(DateTime.parse(widget.paciente.fechaNacimiento)).inDays / 365).floor()} años',
                          style: TextStyle(fontSize: 16.0)),
                    ),
                    ListTile(
                      title: Text('Sexo: ${widget.paciente.sexo}',
                          style: TextStyle(fontSize: 16.0)),
                    ),
                  ],
                ),
              ),
            ),
            Padding(
              padding: EdgeInsets.all(8.0),
              child: Card(
                child: Column(
                  children: [
                    ListTile(
                      title: Text('Promedio de mediciones',
                          style: TextStyle(fontSize: 20.0)),
                    ),
                    Divider(),
                    ListTile(
                      title: Text(
                          widget.paciente.prom_temp != 0 && widget.paciente.prom_temp != null
                              ? 'Temperatura: ${widget.paciente.prom_temp.toStringAsFixed(2)} °C '
                              : 'Temperatura: No hay registros.',
                          style: TextStyle(fontSize: 16.0)),
                    ),
                    ListTile(
                      title: Text(
                          widget.paciente.prom_lpm != 0 && widget.paciente.prom_lpm != null
                              ? 'Frecuencia cardiaca: ${widget.paciente.prom_lpm.toStringAsFixed(2)} lpm '
                              : 'Frecuencia cardiaca: No hay registros.',
                          style: TextStyle(fontSize: 16.0)),
                    ),
                  ],
                ),
              ),
            )
          ],
        ),
      ],
    );

    final header = Card(
      child: Padding(
        padding: EdgeInsets.all(14.0),
        child: Row(
          children: [
            Expanded(
              child: Column(
                crossAxisAlignment: CrossAxisAlignment.center,
                children: [
                  Text('Fecha', style: TextStyle(fontWeight: FontWeight.bold, fontSize: 17)),
                ],
              ),
            ),
            Expanded(
              child: Column(
                crossAxisAlignment: CrossAxisAlignment.center,
                children: [
                  Text('Frecuencia cardíaca (lpm)', style: TextStyle(fontWeight: FontWeight.bold, fontSize: 17)),
                ],
              ),
            ),
            Expanded(
              child: Column(
                crossAxisAlignment: CrossAxisAlignment.center,
                children: [
                  Text('Temperatura (°C)', style: TextStyle(fontWeight: FontWeight.bold, fontSize: 17)),
                ],
              ),
            ),
          ],
        ),
      ),
    );


    final table = FutureBuilder<List<Medicion>>(
      future: _obtenerMensajes(),
      builder: (BuildContext context, AsyncSnapshot<List<Medicion>> snapshot) {
        print('snapshot: $snapshot');
        if (snapshot.connectionState == ConnectionState.done) {
          if (snapshot.data.length != 0) {
            return ListView.builder(
              itemCount: snapshot.data.length,
              itemBuilder: (BuildContext context, int index) {
                Medicion medicion = snapshot.data[index];
                return Card(
                  child: Padding(
                      padding: EdgeInsets.all(14.0),
                    child: Row(
                      children: [
                        Expanded(
                          child: Column(
                            crossAxisAlignment: CrossAxisAlignment.center,
                            children: [
                              Text(medicion.fecha.substring(0,16), style: TextStyle(fontSize: 16)),
                            ],
                          ),
                        ),
                        Expanded(
                          child: Column(
                            crossAxisAlignment: CrossAxisAlignment.center,
                            children: [
                              Text(medicion.lpm.toString(), style: TextStyle(fontSize: 16)),
                            ],
                          ),
                        ),
                        Expanded(
                          child: Column(
                            crossAxisAlignment: CrossAxisAlignment.center,
                            children: [
                              Text(medicion.temp.toString(), style: TextStyle(fontSize: 16)),
                            ],
                          ),
                        ),
                      ],
                    ),
                  ),
                );
              },
            );
          } else {
            return Center(
                child: Text('No hay información registrada.',
                    style: TextStyle(fontSize: 19)));
          }
        } else {
          return Center(child: CircularProgressIndicator());
        }
      },
    );

    final tableAll = Column(
        children: [
          header,
          Divider(height: 5),
          Expanded(
            child: table,
          ),
        ],
    );

    final List<Widget> _children = [listView, tableAll];

    final appBar = AppBar(
      title: Text(
        widget.paciente.nombre,
        style: TextStyle(),
      ),
      actions: [
        IconButton(
          icon: Icon(Icons.delete),
          onPressed: () {
            print('Eliminar?');
            showDialog(
              context: context,
              builder: (BuildContext context) {
                return AlertDialog(
                  title: Text('¿Desea eliminar al paciente?'),
                  content: Text('Si continúa, se eliminará la información del paciente y las mediciones registradas.'),
                  actions: [
                    FlatButton(
                      child: Text('Sí'),
                      onPressed: () {
                        print('ELIMINAR');
                        if (DBProvider.db.deletePaciente(widget.paciente.telefono) != 0){
                          //Toast
                          Fluttertoast.showToast(
                              msg: 'El paciente se eliminó exitosamente.',
                              backgroundColor: Colors.grey,
                              textColor: Colors.white,
                              toastLength: Toast.LENGTH_LONG
                          );
                          Navigator.pop(context);
                          Navigator.pop(context);
                        } else {
                          //Toast
                          Fluttertoast.showToast(
                              msg: 'Ha ocurrido un problema.',
                              backgroundColor: Colors.grey,
                              textColor: Colors.white,
                              toastLength: Toast.LENGTH_LONG
                          );
                        }
                      },
                    ),
                    FlatButton(
                      child: Text('No'),
                      onPressed: () {
                        Navigator.of(context).pop();
                      },
                    ),
                  ],
                );
              },
            );
          },
        ),
        IconButton(
          icon: Icon(Icons.edit),
          onPressed: () {
            print('Modificar');
            Navigator.push(
              context,
              MaterialPageRoute(builder: (context) => EditarScreen(widget.paciente)),
            );
          },
        ),
      ],
    );

    return Scaffold(
      appBar: appBar,
      body: _children[_selectedIndex],
      bottomNavigationBar: BottomNavigationBar(
        currentIndex: _selectedIndex,
        items: const <BottomNavigationBarItem>[
          BottomNavigationBarItem(
              icon: Icon(Icons.person), title: Text('Información')),
          BottomNavigationBarItem(
              icon: Icon(Icons.favorite), title: Text('Mediciones')),
        ],
        onTap: _onItemTapped,
      ),
    );
  }

  void _onItemTapped(int index) {
    print("seleccionado: $index");

    setState(() {
      _selectedIndex = index;
    });
  }

  Future<List<Medicion>> _obtenerMensajes() async {
    SmsQuery query = new SmsQuery();
    ///Obtiene los mensajes recibidos con el número del paciente
    List<SmsMessage> messages = await query.querySms(address: widget.paciente.telefono);

    //List<SmsMessage> messages = await query.querySms(address: '6644589012');
    print("-- Busca mensajes --");

    if(messages.isNotEmpty) {
      for (var sms in messages) {
        ///Crea la Medicion con la información del sms
        ///
        /// sms => lpm:X,temp:X,fecha:X
        print(
            "Address: ${sms.address} "
            "Body: ${sms.body} "
        );

        //String test = "lpm:79,temp:36.5,fecha:1232313213123";
        List<String> campos = sms.body.split(",");
        print("lpm: ${campos[0].substring(4)}");
        print("temp: ${campos[1].substring(5)}");
        print("fecha: ${campos[2].substring(6)}");

        Medicion m = new Medicion(telefono: widget.paciente.telefono, lpm: int.parse(campos[0].substring(4)), temp: double.parse(campos[1].substring(5)), fecha: campos[2].substring(6));
        await DBProvider.db.insertMedicion(m);
      }
    } else {
      print("-- No se encontraron mensajes --");
    }

    ///Actualizar la información del paciente por lo que se vuelve a asignar al widget después de buscarlo
    widget.paciente = await DBProvider.db.buscarPaciente(widget.paciente.telefono);

    return await DBProvider.db.getAllMediciones(widget.paciente.telefono);
  }
}