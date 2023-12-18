Structs in Minor C are useful for packaging multiple pieces of data into a
single piece of data that travels as a unit. When given a name and a list of
field and type pairs, the "struct" keyword inserts a new struct definition into
the compiler's internal database.

  $ cat > structs1.minc <<\.
  > struct point
  >   x i32,
  >   y i32.
  > .

The "show-structs" command lists all the structs that have been defined in the
provided set of files.

  $ $MAIN show-structs structs1.minc
  point
    x i32,
    y i32.

Passing multiple files to show-structs causes it to print the structs defined
in all the files.

  $ cat > structs2.minc <<\.
  > struct color
  >   r u8,
  >   g u8,
  >   b u8,
  >   a u8.
  > .

  $ $MAIN show-structs structs1.minc structs2.minc
  point
    x i32,
    y i32.
  
  color
    r u8,
    g u8,
    b u8,
    a u8.

When bad syntax is used, the compiler responds with informative error messages.

  $ test() { cat > bad.minc; $MAIN show-structs bad.minc; }

  $ test <<\.
  > struct hello world.
  > .
  bad.minc:1:19: Expected identifier for type.
  [1]

  $ test <<\.
  > struct hello.
  > .
  bad.minc:1:13: Expected identifier.
  [1]

  $ test <<\.
  > struct hello
  >   x i32
  >   y i32.
  > .
  bad.minc:3:4: Expected ',' or '.'.
  [1]

  $ test <<\.
  > struct hello
  >   x i32,,
  >   y i32.
  > .
  bad.minc:2:9: Expected identifier.
  [1]

  $ test <<\.
  > struct hello.
  > .
  bad.minc:1:13: Expected identifier.
  [1]
