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

  $ test() { cat > bad.minc; $MAIN translate bad.minc; }

  $ test <<\.
  > fn abc() {
  > .
  bad.minc:2:2: Expected '}' to finish function body.
  2 | <end-of-file>
      ^
  [1]

  $ test <<\.
  > fn { }
  > .
  bad.minc:1:5: Expected identifier.
  1 | fn { }
         ^
  [1]

  $ test <<\.
  > fn abc {}
  > .
  bad.minc:1:9: Expected '(' to begin argument list.
  1 | fn abc {}
             ^
  [1]

  $ test <<\.
  > fn abc def {}
  > .
  bad.minc:1:9: Expected '(' to begin argument list.
  1 | fn abc def {}
             ^
  [1]

  $ test <<\.
  > fn abc(w x y z) {}
  > .
  bad.minc:1:13: Expected ',' or ')'.
  1 | fn abc(w x y z) {}
                 ^
  [1]

  $ test <<\.
  > fn abc() {
  >   :if test
  > }
  > .

  $ test <<\.
  > fn abc() {
  >   :if test
  >   :else :if abc
  >     :switch x
  >     :case 1
  >       :while true
  >       :end
  >     :end
  >   :end
  > }
  > .
  bad.minc:5:12: Expected identifier.
  5 |     :case 1
                ^
  [1]

  $ test <<\.
  > fn abc() {
  >   :abcdef
  > }
  > .
  bad.minc:2:5: Expected one of 'if', 'else', 'switch', 'case', 'while', or 'end' after ':'.
  2 |   :abcdef
         ^
  [1]

  $ test <<\.
  > fn abc() {
  >   :elsif abcd
  > }
  > .
  bad.minc:2:8: Expected one of 'if', 'else', 'switch', 'case', 'while', or 'end' after ':'.
  2 |   :elsif abcd
            ^
  [1]
