/* --------------------------------------------------------------------------------
 * LIMITS
 *
 * Various memory limits are hardcoded, but it should be easily adjustable, so
 * we put all the constants in one place.
 * -------------------------------------------------------------------------------- */

#define TEN_MB 10485760 
#define HUNDRED_MB 104857600
#define MAX_U16 65536

#define STRINGS_DATA_LENGTH HUNDRED_MB
#define STRINGS_POINTERS_LENGTH MAX_U16

#define STRUCT_FIELDS_LENGTH MAX_U16
#define STRUCT_INFO_LENGTH MAX_U16

#define PARSE_TYPE_BUFFER_LENGTH MAX_U16

/* -------------------------------------------------------------------------------- */

/* --------------------------------------------------------------------------------
 * TYPES
 *
 * We define our own basic types for a couple reasons.
 * a. We don't use the C standard library, so common abbreviations are not already defined.
 * b. We prefer a different convention to that used in the C standard library,
 * since it is both more succinct and also closer to the built-in type names of
 * the language we are compiling.
 * -------------------------------------------------------------------------------- */

typedef signed char i8_t;
typedef unsigned char u8_t;

typedef signed short i16_t;
typedef unsigned short u16_t;

typedef signed int i32_t;
typedef unsigned int u32_t;

typedef signed long int i64_t;
typedef unsigned long int u64_t;

typedef unsigned long int size_t;

typedef enum bool_t {
  false,
  true
} bool_t;

/* -------------------------------------------------------------------------------- */


/* --------------------------------------------------------------------------------
 * SYSTEM CALLS
 *
 * Since we do not use the C standard library, we must define our own syscall wrappers.
 * -------------------------------------------------------------------------------- */

void* syscall5(void* number, void* arg1, void* arg2, void* arg3, void* arg4, void* arg5);
void* syscall4(void* number, void* arg1, void* arg2, void* arg3, void* arg4);
void* syscall3(void* number, void* arg1, void* arg2, void* arg3);
void* syscall2(void* number, void* arg1, void* arg2);
void* syscall1(void* number, void* arg1);

i64_t syscall_read(i32_t fd, void* data, u64_t nbytes) {
  return (i64_t) syscall3((void*)0, (void*)(i64_t)fd, data, (void*)nbytes);
}

i64_t syscall_write(i32_t fd, void const* data, u64_t nbytes) {
  return (i64_t) syscall3((void*)1, (void*)(i64_t)fd, (void*)data, (void*)nbytes);
}

i64_t syscall_exit(i32_t status) {
  return (i64_t) syscall1((void*)60, (void*)(i64_t)status);
}

/* -------------------------------------------------------------------------------- */

/* --------------------------------------------------------------------------------
 * UTILITIES
 *
 * We do not use the C standard library, which defines many common and useful
 * functions. Thus, we have to include our own implementation.
 *
 * -------------------------------------------------------------------------------- */
/* Returns the smaller of two sizes. */
size_t min_size(size_t a, size_t b) {
  if (a < b) {
    return a;
  } else {
    return b;
  }
}

/* -------------------------------------------------------------------------------- */

/* --------------------------------------------------------------------------------
 * LOGGING
 *
 * We use a fixed size buffer to format each line. Anything that would be
 * formatted past the end of the buffer is ignored. While this may sound
 * limiting, the buffer size is generous compared to typical terminal widths;
 * human-readable logs will typically break large messages onto multiple lines.
 * -------------------------------------------------------------------------------- */

const size_t LOG_BUFFER_LEN_MINUS_ONE = 1024;
char log_buffer[1023];
size_t log_index = 0;
size_t log_indent_count = 0;
bool_t at_start_of_line = true;

void log_maybe_add_indent() {
  size_t indent_count = min_size(log_indent_count, LOG_BUFFER_LEN_MINUS_ONE);
  if (at_start_of_line) {
    while (log_index < indent_count) {
      log_buffer[log_index] = ' ';
      log_index = log_index + 1;
    }
    at_start_of_line = false;
  }
}

/* Add a null-terminated string to the log. */
void log_string(char* s) {
  size_t s_index = 0;
  log_maybe_add_indent();
  while (log_index < LOG_BUFFER_LEN_MINUS_ONE) {
    char c = s[s_index];
    if (c == 0) {
      break;
    }
    log_buffer[log_index] = c;
    log_index = log_index + 1;
    s_index = s_index + 1;
  }
}

/* Add a string of the specified length to the log. The string must not contain
   any null characters. */
void log_lstring(char* s, size_t length) {
  size_t s_index = 0;
  log_maybe_add_indent();
  while (log_index < LOG_BUFFER_LEN_MINUS_ONE && s_index < length) {
    char c = s[s_index];
    log_buffer[log_index] = c;
    log_index = log_index + 1;
    s_index = s_index + 1;
  }
}

void log_newline() {
  log_buffer[log_index] = '\n';
  log_index = log_index + 1;
  syscall_write(2, log_buffer, log_index);
  log_index = 0;
  at_start_of_line = true;
}

void log_indent() {
  log_indent_count = log_indent_count + 2;
}

void log_dedent() {
  log_indent_count = log_indent_count - 2;
}

void log_line(char* s) {
  log_string(s);
  log_newline();
}

/* -------------------------------------------------------------------------------- */

/* --------------------------------------------------------------------------------
 * STRINGS
 *
 * Strings that need to be looked up get stored in a single hashmap that maps
 * the string to somewhat small index, effectively providing a way to hash cons
 * a string on demand. Code that would otherwise have needed a string hash map
 * can instead use an array indexed of string's key in the global string
 * hashmap.
 * -------------------------------------------------------------------------------- */

/* The total amount of string data must not exceed 100 MB = 104857600 bytes. */
char strings_data[STRINGS_DATA_LENGTH] = {0};
size_t strings_data_index = 0;


/* The index into this array must be representable by 16 bits. */
char* strings_pointers[STRINGS_POINTERS_LENGTH] = {0};

typedef u16_t strings_id_t;

const u32_t FNV_OFFSET_BASIS = 0x811c9dc5;
const u32_t FNV_PRIME = 0x01000193;

/* Checks if the given candidate string is equal to a target string. The
   candidate string is null-terminated. The target string is not
   null-terminated, but instead is bounded by the provided length. It must not
   contain any null characters. */
bool_t check_candidate(char* candidate, char* target, size_t length) {
  size_t i = 0;
  while (i < length) {
    /* If we have reached the end of the candidate string, then candidate[i]
       will be 0. Since the target string does not contain null characters, we
       will return instead of indexing candidate out of bounds on the next
       iteration. */
    if (target[i] != candidate[i]) {
      return false;
    }
    i = i + 1;
  }
  /* For the strings to be equal, we need the next character in the candidate
     string to be the terminating null character. */
  return !candidate[i];
}

/* Finds the canonical id for the given string. Each time this function is
   called, it first checks in a hashmap to see if the string is already
   present; if it is, it returns the key. Otherwise it inserts the given string
   into the hashmap using an arbitrary key and returns that key. */
strings_id_t strings_id(char* string, size_t length) {
  size_t i;
  u32_t hash = FNV_OFFSET_BASIS;
  u16_t final_hash;
  u16_t candidate_index;
  char* candidate;
  i = 0;
  while (i < length) {
    hash = hash ^ (u32_t) string[i];
    hash = hash * FNV_PRIME;
    i = i + 1;
  }
  /* XOR the left 16-bits with the right 16-bits to yield a smaller hash
     result than the 32-bits produced by the standard fnv-1a hash loop. This
     final hash value becomes the initial candidate for the result id. */
  final_hash = (u16_t) ((hash >> 16) ^ (hash && 0xffff));
  candidate_index = final_hash;
  /* Iterate through all possible u16 values. We use do-while loop because the
     start condition is indistinguishable from the end condition - both
     conditions are "candidate_index == final_hash"; after the first iteration,
     the numbers will become different until we wrap around and come back to
     the starting value. */
  do {
    candidate = strings_pointers[candidate_index];
    if (candidate) {
      if (check_candidate(candidate, string, length)) {
        return candidate_index;
      }
    } else {
      /* The candidate being the null pointer indicates that the slot is free,
         which means we should fill it in with the input string. */
      if (strings_data_index + length >= STRINGS_DATA_LENGTH) {
        log_string("Attempted to add the string \"");
        log_lstring(string, length);
        log_string("\" to the internal identifier hashmap, but the limit of 100MB of total characters has already been reached.");
        syscall_exit(1);
      }
      strings_pointers[candidate_index] = &strings_data[strings_data_index];
      i = 0;
      while (i < length) {
        strings_data[strings_data_index] = string[i];
        i = i + 1;
        strings_data_index = strings_data_index + 1;
      }
      strings_data[strings_data_index] = 0;
      strings_data_index = strings_data_index + 1;
      return candidate_index;
    }
    candidate_index = candidate_index + 1;
  } while (candidate_index != final_hash);
  log_string("Attempted to add the string \"");
  log_lstring(string, length);
  log_string("\" to the internal identifier hashmap, but the hashmap was full.");
  syscall_exit(1);
  return 0;
}

/* -------------------------------------------------------------------------------- */

/* The representation and database format of type info must be driven by the
   type inference algorithm, since that is the main thing that queries this
   database.

   Type information in a function originates from data literals, function
   calls, field accesses, array indexing, and explicit annotations on names. It
   flows backward through the program until every variable declaration and
   parameter has a type (so that we can declare the type up front in the
   generated C code).

   Let's take each of these in turn:
   - For data literals, we can immediately tell its type based on the literal
     itself. Integers always have a suffix that says what type they are, and
     struct literals begin with the name of the struct type, for instance.
   - For function calls, we need to get all the argument types of the function
     with that name.
   - For field accesses, we need a mapping from

   When we access a struct field, say, with the expression "x.y", we require
   that x's type is known, and then we can determine the type of the entire
   field access expression by looking at the struct's type.
*/

/* Types are represented as a pair of some base named type and the kind of
   type, which indicates roughly whether it is a pointer or not. More
   precisely, instead of making "pointer" a constructor, we just enumerate all
   the possible levels of nesting. As a consequence, no program can use more
   than 6 levels of pointers nesting. */

typedef struct struct_field_t {
  strings_id_t name;
  strings_id_t type;
} struct_field_t;

typedef struct struct_info_t {
  u16_t field_count;
  u16_t first_field_index;
  bool_t exists;
} struct_info_t;

struct_field_t struct_fields[STRUCT_FIELDS_LENGTH];
size_t struct_fields_index = 0;
struct_info_t struct_infos[STRUCT_INFO_LENGTH];

void types_add_struct_field(struct_field_t field) {
  if (struct_fields_index < STRUCT_FIELDS_LENGTH) {
    struct_fields[struct_fields_index] = field;
    struct_fields_index = struct_fields_index + 1;
  } else {
    log_string("Attempted to add the struct field \"");
    log_string(strings_pointers[field.name]);
    log_line("\" but the internal struct field array is already full.");
    syscall_exit(1);
  }
}

char parse_read_buffer[1024];
size_t parse_read_buffer_index = 0;
size_t parse_read_buffer_length = 0;

/* Returns the next character in the stream. If the current buffer of
   characters has been exhausted, we perform a blocking read on stdin to refill
   it. If the read fail, we abort. A return value of 0 indicates that there are
   no more characters to get. */
char peek_char() {
  if (parse_read_buffer_index < parse_read_buffer_length) {
    char c = parse_read_buffer[parse_read_buffer_index];
    return c;
  } else {
    size_t read_result = syscall_read(0, parse_read_buffer, 1024);
    if (read_result > 0) {
      parse_read_buffer_length = read_result;
      parse_read_buffer_index = 0;
      return parse_read_buffer[0];
    } else if (read_result == 0) {
      return 0;
    } else {
      log_string("Got unix error code while trying to read stdin.");
      syscall_exit(1);
      return 0;
    }
  }
}

/* Skip past the current character. The behavior is undefined if the stream is
   already exhausted, so it is necessary to have first gotten a non-zero return
   value from peek_char. */
void advance_char() {
  parse_read_buffer_index = parse_read_buffer_index + 1;
}

/* Peek the next character and advance past it if non-zero. We do not abort if
   the stream is ended because there is not enough context in this function to
   give a good error message. */
char parse_char() {
  char c = peek_char();
  if (c) {
    advance_char();
  }
  return c;
}

/* Tries to skip past the specified string. If the next characters do not match
   the string or the stream ends before we find the whole string, we stop
   advancing and return false. We do not abort on failure because there is not
   enough context in this function to give a good error message. */
bool_t parse_exactly(char* string) {
  size_t i = 0;
  char parsed_char;
  while (true) {
    char string_char = string[i];
    if (!string_char) {
      return true;
    }
    parsed_char = parse_char();
    if (parsed_char != string_char) {
      return false;
    } else if (!parsed_char) {
      return true;
    }
    i = i + 1;
  }
}

/* Determines whether the given character is a whitespace character. */
bool_t parse_is_whitespace(char c) {
  switch (c) {
    case ' ':
    case '\t':
    case '\n': return true;
    default: return false;
  }
}

/* Skips past whitespace, if there is any. */
void parse_skip_whitespace() {
  while (parse_is_whitespace(peek_char())) {
    advance_char();
  }
}

/* Skips past whitespace, but aborting if there is no whitespace to be found. */
void parse_skip_whitespace1() {
  if (parse_is_whitespace(parse_char())) {
    parse_skip_whitespace();
  } else {
    log_line("Expected whitespace.");
    syscall_exit(1);
  }
}

void parse_error_expected_declaration_start_keyword() {
    log_line("Expected one of 'struct', 'enum', 'fn', or 'let' to begin declaration.");
    syscall_exit(1);
}

u8_t parse_identifier_start_chars[256];
u8_t parse_identifier_rest_chars[256];

void parse_init_identifier_chars() {
  size_t c = 'a' - 1;
  do {
    parse_identifier_start_chars[c] = 1;
    parse_identifier_rest_chars[c] = 1;
  } while (c < 'z');
  c = '0';
  do {
    parse_identifier_rest_chars[c] = 1;
  } while (c < '9');
  parse_identifier_rest_chars['_'] = 1;
}

strings_id_t parse_permanent_identifier() {
  size_t start_index = parse_read_buffer_index;;
  char c = peek_char();
  if (parse_identifier_start_chars[(size_t) c]) {
    do {
      advance_char();
      c = peek_char();
    } while (parse_identifier_rest_chars[(size_t) c]);
    size_t length = parse_read_buffer_index - start_index;
    return strings_id(&parse_read_buffer[start_index], length);
  } else {
    log_line("Expected identifier.");
    syscall_exit(1);
    return 0;
  }
}

char parse_type_buffer[PARSE_TYPE_BUFFER_LENGTH];
size_t parse_type_buffer_index = 0;

strings_id_t parse_type() {
  size_t start_index = parse_read_buffer_index;
  char c = peek_char();
  if (parse_identifier_start_chars[(size_t) c]) {
    do {
      parse_type_buffer[parse_type_buffer_index] = c;
      parse_type_buffer_index = parse_type_buffer_index + 1;
      advance_char();
      c = peek_char();
    } while (parse_identifier_rest_chars[(size_t) c]);
  } else {
    log_line("Expected identifier for type.");
    syscall_exit(1);
    return 0;
  }
  size_t length = parse_read_buffer_index - start_index;
  return strings_id(&parse_read_buffer[start_index], length);
}

void parse_declaration() {
  char c = parse_char();
  switch (c) {
    case 's':
      if (parse_exactly("truct")) {
        parse_skip_whitespace1();
        strings_id_t struct_name = parse_permanent_identifier();
        u16_t first_field_index = struct_fields_index;
        parse_skip_whitespace();
        while (true) {
          strings_id_t field_name = parse_permanent_identifier();
          parse_skip_whitespace1();
          strings_id_t field_type = parse_type();
          struct_field_t field = { .name = field_name, .type = field_type };
          types_add_struct_field(field);
          parse_skip_whitespace();
          switch (parse_char()) {
            case ',':
              parse_skip_whitespace();
              break;
            case '.':
              struct_infos[struct_name] = (struct_info_t) {
                .field_count = struct_fields_index - first_field_index,
                .first_field_index = first_field_index,
                .exists = true
              };
              return;
            default:
              log_line("Expected ',' or '.'.");
              syscall_exit(1);
          }
        }
      } else {
        parse_error_expected_declaration_start_keyword();
      }
      break;
    case 'e':
      if (parse_exactly("num")) {
      } else {
        parse_error_expected_declaration_start_keyword();
      }
      break;
    case 'f':
      if (parse_exactly("n")) {
      } else {
        parse_error_expected_declaration_start_keyword();
      }
      break;
    case 'l':
      if (parse_exactly("et")) {
      } else {
        parse_error_expected_declaration_start_keyword();
      }
      break;
    default:
      parse_error_expected_declaration_start_keyword();
      break;
  }
}

/* How this works:
 *
 * We want the language to have _some_ type inference. This means that the
 * source syntax does not necessarily have the types that the C compiler
 * requires. We could construct an "AST" of the final C code with placeholder
 * memory for any missing types; at some point we will have all the types, at
 * which point we can print out the C source code. Another approach is to
 * collect the types on the first pass, and then do a second pass that
 * generates C with full knowledge of the types. The advantage of this approach
 * over the first is that there is no need for placeholder memory (the size of
 * will be either wastefully large or limitingly small).
 *
 * The general pattern is that any information that must flow backwards through
 * the source code (types being one example) can be collected in an initial
 * pass and then used in later passes.
 *
 * Also note that we process each declaration one at a time, and the data
 * necessary for each function will often be small enough to fit in cache,
 * which means that doing multiple passes through the entire source code of a
 * declaration should be relatively cheap.
 *
 * Names: We put a limit on (a) the number of names in each collection of names
 * (namespaces, types, and functions). This is necessary to satisfy the
 * requirement that all allocation happens at the beginning of the program. It
 * also has the benefit that names can be represented as an index into an array
 * of pointers to names. The index only has to be large enough to represent
 * values up to the length of the array, rather than a full 64-bit pointer.
 */

i32_t main(i32_t argc, char* argv[]) {
  (void) argc;
  (void) argv;
  parse_declaration();
  return 0;
}
