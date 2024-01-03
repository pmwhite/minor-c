Running the program with no argument yields some help text.

  $ $MAIN
  Usage: <exe> command file...
  Commands:
    translate   Read the provided Minor C source files and send equivalent C code to stdout.
    sizes       Print the sizes compiler-internal data types.

An error message is displayed when the specified command is unrecognized.

  $ $MAIN non-existent-command
  Unknown command "non-existent-command".
  [1]

Passing multiple files works.

  $ cat > fns1.minc <<\.
  > fn f(a i32, b i32) {}
  > fn g(x f32, y f64) { }
  > .

  $ cat > fns2.minc <<\.
  > fn x(a i32, b i32) {}
  > fn y() { }
  > .

  $ $MAIN translate fns1.minc fns2.minc
  g(x f32, y f64) { ... }
  f(a i32, b i32) { ... }
  y(x(a i32, b i32) { ... }
