  $ cat > fns1.minc <<\.
  > fn f(a i32, b i32) {}
  > fn g(x f32, y f64) { }
  > .

  $ $MAIN show-fns fns1.minc
  g(x f32, y f64) { ... }
  f(a i32, b i32) { ... }
