  $ cat > fns1.minc <<\.
  > fn f(a i32, b i32) {}
  > fn g(x f32, y f64) { }
  > .

  $ cat > fns2.minc <<\.
  > fn x(a i32, b i32) {}
  > fn y() { }
  > .

  $ $MAIN show-fns fns1.minc fns2.minc
  g(x f32, y f64) { ... }
  f(a i32, b i32) { ... }
  y(x(a i32, b i32) { ... }

  $ test() { cat > bad.minc; $MAIN show-structs bad.minc; }

  $ test <<\.
  > fn abc() {
  > .
  bad.minc:2:2: Expected '}' to finish function body.
  2 | <end-of-file>
      ^
  [1]
