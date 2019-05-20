// Copyright 2018 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// You can read about packages here: https://flutter.io/using-packages/

import 'package:flutter/material.dart';
import 'pacientes_screen.dart';

// You can use a relative import, i.e. `import 'category_route.dart;'` or
// a package import, as shown below.
// More details at http://dart-lang.github.io/linter/lints/avoid_relative_lib_imports.html

/// The function that is called when main.dart is run.
void main() {
  runApp(AppTest());
}

/// This widget is the root of our application.
class AppTest extends StatelessWidget {
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      debugShowCheckedModeBanner: false,
      title: 'Monitoreo',
      home: PacientesScreen(),
    );
  }
}
