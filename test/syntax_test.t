  $ test() { cat > bad.minc; $MAIN translate bad.minc; }

STRUCTS

  $ test <<\.
  > struct point
  >   x `i32,
  >   y `i32;
  > struct color
  >   r `u8,
  >   g `u8,
  >   b `u8,
  >   a `u8;
  > .

  $ test <<\.
  > struct hello world;
  > .
  bad.minc:1:20: Expected '`' to begin type.
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
  >   x `i32
  >   y `i32;
  > .
  bad.minc:3:4: Expected ',' or ';'.
  3 |   y `i32;
        ^
  [1]

  $ test <<\.
  > struct hello
  >   x `i32,,
  >   y `i32;
  > .
  bad.minc:2:11: Expected identifier.
  2 |   x `i32,,
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
  > fn abc(w `x y `z) {}
  > .
  bad.minc:1:14: Expected ',' or ')'.
  1 | fn abc(w `x y `z) {}
                  ^
  [1]

  $ test <<\.
  > fn abc() {
  >   test = 5u8
  >   if test
  > }
  > .

  $ test <<\.
  > fn abc() {
  >   x = 5u8
  >   if 123u8
  >   else if x
  >     switch x
  >     case 1
  >       while 1u8
  >       end
  >     end
  >   end
  > }
  > .

  $ test <<\.
  > fn abc() {
  >   abcdef
  > }
  > .
  bad.minc:3:1: Expected statement or '}'.
  3 | }
     ^
  [1]

  $ test <<\.
  > fn abc() {
  >   elsif abcd
  > }
  > .
  bad.minc:2:9: Expected statement or '}'.
  2 |   elsif abcd
             ^
  [1]

  $ test <<\.
  > fn abc() {
  >   if test
  > }
  > .
  bad.minc:2:7: Unknown variable 'test'.
  2 |   if test
           ^
  [1]

  $ test <<\.
  > fn f() {
  > }
  > fn g() {
  >   f()
  > }
  > .

  $ test <<\.
  > fn g() {
  >   f()
  > }
  > .
  bad.minc:2:4: Unknown function 'f'.
  2 |   f()
        ^
  [1]

  $ test <<\.
  > fn f(x `i32) {
  > }
  > fn g(x `i32) {
  >   f(x)
  > }
  > .

Group expressions.

  $ test <<\.
  > fn f(x `i32) {
  > }
  > fn g(x `i32) {
  >   y = (((f(x))))
  > }
  > .

Operator expressions.

  $ test <<\.
  > fn f(x `i32) {
  >   y = x + 1i32
  > }
  > .

  $ test <<\.
  > fn f(x `i32) {
  >   y = x + 1i32 - 10i32
  > }
  > .
  bad.minc:2:17: Expected statement or '}'.
  2 |   y = x + 1i32 - 10i32
                     ^
  [1]

  $ test <<\.
  > fn f(x `i32) {
  >   y = x + (1i32 - 10i32)
  > }
  > .

  $ test <<\.
  > fn f(x `i32) {
  >   y = (x + 1i32) -*< 10i32
  > }
  > .

Function with return type.

  $ test <<\.
  > fn f(x `i32) `i32 {
  >   y = (x + 1i32) -*< 10i32
  > }
  > .

Casting expressions.

  $ test <<\.
  > fn cast_to_void(x `i32) `void* {
  >   y = x@`void*@`void*
  > }
  > .

Type-annotated expressions.

  $ test <<\.
  > fn cast_to_void(x `i32) `void* {
  >   y = x`i32`i32
  > }
  > .

  $ test <<\.
  > fn f(x `i32) { }
  > fn cast_to_void(x `i32) `void* {
  >   y = f(0i32)`i32
  > }
  > .

CONSTANTS

Constants can be used as expressions.

  $ test <<\.
  > const false = 0
  > const true = 1
  > fn f() {
  >   if true
  >     while false
  >     end
  >     while false
  >     end
  >   end
  > }
  > .

Constants can be used in cases constants.

  $ test <<\.
  > const false = 0
  > const true = 1
  > fn f() `u8 {
  >   switch true
  >     case true
  >       return true
  >     case false
  >       return false
  >   end
  > }
  > .

Arrays sizes can be named constants.

  $ test <<\.
  > const length = 1000
  > struct x
  >   y `u8[length];
  > .

TYPES

Named types.

  $ test <<\.
  > fn abc(a `b) { }
  > .

Pointer types.

  $ test <<\.
  > fn abc(a `b*) { }
  > .

Array types.

  $ test <<\.
  > fn abc(a `b*[100]) { }
  > .

  $ test <<\.
  > fn abc(a `b*[1000000000000000000000000000000000]) { }
  > .
  bad.minc:1:35: Integer constant too large; it must be representable in 64 bits.
  1 | fn abc(a `b*[1000000000000000000000000000000000]) { }
                                       ^
  [1]
