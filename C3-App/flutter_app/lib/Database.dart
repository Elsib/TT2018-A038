import 'dart:async';

import 'package:path/path.dart';
import 'package:sqflite/sqflite.dart';

import 'paciente.dart';
import 'medicion.dart';

class DBProvider {
  Exception e;
  DBProvider._();
  static final DBProvider db = DBProvider._();
  static Database _database;
  final resetDB = 1;
  final insertDB = 0;

  Future<Database> get database async {
    if(_database != null) {
      return _database;
    }

    _database = await initDB();
    return _database;
  }

  initDB() async {
    if(resetDB == 1) {
      await deleteDatabase(join(await getDatabasesPath(), 'paciente_database.db'));
    }

    return await openDatabase(
      join(await getDatabasesPath(), 'paciente_database.db'),
      onCreate: (db, version) async {
        await db.execute(
            "CREATE TABLE pacientes("
                "telefono TEXT PRIMARY KEY, "
                "nombre TEXT, "
                "sexo TEXT, "
                "fecha_nacimiento TEXT, "
                "prom_temp REAL, "
                "prom_lpm REAL"
            ")"
        );
        await db.execute(
            "CREATE TABLE mediciones("
                "telefono TEXT, "
                "fecha TEXT, "
                "lpm INT, "
                "temp REAL, "
                "PRIMARY KEY (telefono, fecha), "
                "FOREIGN KEY (telefono) REFERENCES pacientes(telefono) "
                "ON DELETE CASCADE "
                "ON UPDATE CASCADE"
            ")"
        );
        await db.execute("DROP TRIGGER IF EXISTS trg_actualizaProm_after_insert");
        await db.execute(
          "CREATE TRIGGER trg_actualizaProm_after_insert AFTER INSERT ON mediciones "
          "FOR EACH ROW "
          "BEGIN "
            "UPDATE pacientes "
            "SET "
            "prom_temp = (SELECT avg(temp) FROM mediciones WHERE telefono = NEW.telefono), "
            "prom_lpm = (SELECT avg(lpm) FROM mediciones WHERE telefono = NEW.telefono) "
            "WHERE telefono = NEW.telefono; "
          "END;"
        );

        if(insertDB == 1) {
          await db.execute(
              "INSERT INTO pacientes(telefono, nombre, sexo, fecha_nacimiento, prom_temp, prom_lpm) VALUES('5564211272', 'Carlos', 'Masculino', '1995-07-14 00:00:00.000', 0, 0)");
          await db.execute(
              "INSERT INTO mediciones(telefono, fecha, lpm, temp) VALUES('5564211272', '2019-02-10 00:00:00.000', 80, 36.5)");
          await db.execute(
              "INSERT INTO mediciones(telefono, fecha, lpm, temp) VALUES('5564211272', '2019-02-09 00:00:00.000', 80, 36.5)");
          await db.execute(
              "INSERT INTO mediciones(telefono, fecha, lpm, temp) VALUES('5564211272', '2019-02-12 00:00:00.000', 80, 36.5)");
          await db.execute(
              "INSERT INTO mediciones(telefono, fecha, lpm, temp) VALUES('5564211272', '2019-02-11 00:00:00.000', 80, 36.5)");

          print(await db.rawQuery("select * from pacientes"));
        }
      },
      version: 1,
    );
  }

  buscarPaciente(String telefono) async {
    final db = await database;
    var res = await db.rawQuery('select * from pacientes where telefono =$telefono');
    return res.isNotEmpty ? Paciente.fromMap(res.first) : Null;
  }

  insertPaciente(Paciente p) async{
    final db = await database;
    var res;

    try {
      res = await db.insert(
              'pacientes',
              p.toMap(),
            );
    } catch (e) {
      print(e);
      if(e.toString().contains("UNIQUE")) {
        return -1;
      }
      return null;
    }

    return res;
  }

  Future<List<Paciente>> getAllPacientes() async {
    final db = await database;
    final List<Map<String, dynamic>> maps = await db.rawQuery('select * from pacientes order by nombre');

    return List.generate(maps.length, (i) {
      return Paciente(
        telefono : maps[i]['telefono'],
        nombre : maps[i]['nombre'],
        sexo: maps[i]['sexo'],
        fechaNacimiento: maps[i]['fecha_nacimiento'],
        prom_lpm: maps[i]['prom_lpm'],
        prom_temp: maps[i]['prom_temp'],
      );
    });
  }

  Future<List<Medicion>> getAllMediciones(String tel) async {
    final db = await database;
    final List<Map<String, dynamic>> maps = await db.query(
      'mediciones',
      where: 'telefono = ?',
      whereArgs: [tel],
      orderBy: 'fecha desc',
    );

    return List.generate(maps.length, (i) {
      return Medicion(
        telefono: maps[i]['telefono'],
        fecha: maps[i]['fecha'],
        temp: maps[i]['temp'],
        lpm: maps[i]['lpm'],
      );
    });
  }

  deletePaciente(String tel) async {
    final db = await database;
    await db.delete('pacientes', where: 'telefono = ?', whereArgs: [tel]);
  }
  
  updatePaciente(Paciente p, String tel) async {
    try {
      final db = await database;
      await db.execute("PRAGMA foreign_keys = ON;");
      await db.rawUpdate("UPDATE pacientes SET "
          "nombre = '${p.nombre}', "
          "telefono = '${p.telefono}', "
          "fecha_nacimiento = '${p.fechaNacimiento}', "
          "sexo = '${p.sexo}' "
          "WHERE "
          "telefono = '$tel'");
    } catch (e) {
      print('Error: $e');
    }
  }

  insertMedicion(Medicion m) async {
    final db = await database;
    await db.insert(
        'mediciones',
        m.toMap(),
        conflictAlgorithm: ConflictAlgorithm.ignore
    );
  }

}