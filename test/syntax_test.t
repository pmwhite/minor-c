  $ test() { cat > bad.minc; $MAIN translate bad.minc; }

STRUCTS

  $ test <<\.
  > struct point
  >   x i32,
  >   y i32;
  > struct color
  >   r u8,
  >   g u8,
  >   b u8,
  >   a u8;
  > .
  point
    x i32,
    y i32;
  
  color
    r u8,
    g u8,
    b u8,
    a u8;

  $ test <<\.
  > struct hello world;
  > .
  bad.minc:1:20: Expected identifier for type.
  1 | struct hello world;
                        ^
  [1]

  $ test <<\.
  > struct hello;
  > .
  bad.minc:1:14: Expected identifier.
  1 | struct hello;
                  ^
  [1]

  $ test <<\.
  > struct hello
  >   x i32
  >   y i32;
  > .
  bad.minc:3:4: Expected ',' or ';'.
  3 |   y i32;
        ^
  [1]

  $ test <<\.
  > struct hello
  >   x i32,,
  >   y i32;
  > .
  bad.minc:2:10: Expected identifier.
  2 |   x i32,,
              ^
  [1]

  $ test <<\.
  > struct hello;
  > .
  bad.minc:1:14: Expected identifier.
  1 | struct hello;
                  ^
  [1]

FUNCTIONS

  $ test <<\.
  > fn abc() {
  > .
  bad.minc:2:2: Expected statement or '}'.
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
  >   test = 5u8
  >   :if test
  > }
  > .

  $ test <<\.
  > fn abc() {
  >   x = 5u8
  >   :if 123u8
  >   :else :if x
  >     :switch x
  >     :case 1u8
  >       :while 1u8
  >       :end
  >     :end
  >   :end
  > }
  > .

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

  $ test <<\.
  > fn abc() {
  >   :if test
  > }
  > .
  bad.minc:2:8: Unknown variable 'test'.
  2 |   :if test
            ^
  [1]

  $ test <<\.
  > fn f() {
  >   x = 5u8
  > }
  > fn g() {
  >   f()
  > }
  > .
  bad.minc:5:6: Expected identifier or number literal.
  5 |   f()
          ^
  [1]

TYPES

Named types.

  $ test <<\.
  > fn abc(a b) { }
  > .
  abc(a b) { ... }

Pointer types.

  $ test <<\.
  > fn abc(a b*) { }
  > .
  abc(a b*) { ... }
