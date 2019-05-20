class Paciente {
  String nombre;
  String telefono;
  String sexo;
  String fechaNacimiento;
  double prom_temp;
  double prom_lpm;

  Paciente({this.nombre, this.telefono, this.sexo, this.fechaNacimiento, this.prom_temp = 0, this.prom_lpm = 0});

  //Para la base de datos
  Map<String, dynamic> toMap() {
    return {
      'telefono': telefono,
      'nombre': nombre,
      'sexo': sexo,
      'fecha_nacimiento': fechaNacimiento,
    };
  }

  factory Paciente.fromMap(Map<String, dynamic> res) => new Paciente(
    telefono: res['telefono'],
    nombre: res['nombre'],
    sexo: res['sexo'],
    fechaNacimiento: res['fecha_nacimiento'],
    prom_temp: res['prom_temp'],
    prom_lpm: res['prom_lpm'],
  );

}