class Medicion {
  String telefono;
  String fecha;
  int lpm;
  double temp;

  Medicion({this.telefono, this.fecha, this.lpm, this.temp});

  Map<String, dynamic> toMap() {
    return {
      'telefono': telefono,
      'fecha': fecha,
      'lpm': lpm,
      'temp': temp
    };
  }

  factory Medicion.fromMap(Map<String, dynamic> res) => new Medicion(
    telefono: res['telefono'],
    fecha: res['fecha'],
    lpm: int.parse(res['lpm']),
    temp: double.parse(res['temp'])
  );

}