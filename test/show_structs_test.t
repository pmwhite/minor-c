Structs in Minor C are useful for packaging multiple pieces of data into a
single piece of data that travels as a unit. When given a name and a list of
field and type pairs, the "struct" keyword inserts a new struct definition into
the compiler's internal database.

  $ cat > structs1.minc <<\.
  > struct point
  >   x i32,
  >   y i32;
  > .

The "show-structs" command lists all the structs that have been defined in the
provided set of files.

  $ $MAIN show-structs structs1.minc
  point
    x i32,
    y i32;

Passing multiple files to show-structs causes it to print the structs defined
in all the files.

  $ cat > structs2.minc <<\.
  > struct color
  >   r u8,
  >   g u8,
  >   b u8,
  >   a u8;
  > .

  $ $MAIN show-structs structs1.minc structs2.minc
  point
    x i32,
    y i32;
  
  color
    r u8,
    g u8,
    b u8,
    a u8;

When bad syntax is used, the compiler responds with informative error messages.

  $ test() { cat > bad.minc; $MAIN show-structs bad.minc; }

  $ test <<\.
  > struct hello world;
  > .
  \x1b[1mbad.minc:1:19: \x1b[22mExpected identifier for type. (esc)
  \x1b[1m1 | \x1b[22mstruct hello world (esc)
                       \x1b[1m^\x1b[22m (esc)
  [1]

  $ test <<\.
  > struct hello;
  > .
  \x1b[1mbad.minc:1:13: \x1b[22mExpected identifier. (esc)
  \x1b[1m1 | \x1b[22mstruct hello (esc)
                 \x1b[1m^\x1b[22m (esc)
  [1]

  $ test <<\.
  > struct hello
  >   x i32
  >   y i32;
  > .
  \x1b[1mbad.minc:3:4: \x1b[22mExpected ',' or ';'. (esc)
  \x1b[1m3 | \x1b[22m  y i32 (esc)
        \x1b[1m^\x1b[22m (esc)
  [1]

  $ test <<\.
  > struct hello
  >   x i32,,
  >   y i32;
  > .
  \x1b[1mbad.minc:2:9: \x1b[22mExpected identifier. (esc)
  \x1b[1m2 | \x1b[22m  x i32, (esc)
             \x1b[1m^\x1b[22m (esc)
  [1]

  $ test <<\.
  > struct hello;
  > .
  \x1b[1mbad.minc:1:13: \x1b[22mExpected identifier. (esc)
  \x1b[1m1 | \x1b[22mstruct hello (esc)
                 \x1b[1m^\x1b[22m (esc)
  [1]
