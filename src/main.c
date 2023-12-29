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
#define STRUCT_FIELDS_LENGTH MAX_U16
#define PARSE_TYPE_BUFFER_LENGTH MAX_U16
#define PARSE_READ_BUFFER_CAPACITY TEN_MB
#define MAX_LINE_LENGTH_FOR_ERRORS 120
#define NAMES_IN_SCOPE_LENGTH MAX_U16
#define STRINGS_ID_MAP_LENGTH MAX_U16
#define MAX_FN_ARITY 14
#define MAX_LOCAL_VARIABLES 1024

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

typedef u8_t bool_t;
#define false 0
#define true 1

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

i64_t syscall_open(char const* filename, u64_t flags, u64_t mode) {
  return (i64_t) syscall3((void*)2, (void*)filename, (void*)(i64_t)flags, (void*)(i64_t)mode);
}

#define O_RDONLY 00

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

bool_t string_equal(char* expected, char* actual) {
  size_t i = 0;
  while (true) {
    if (expected[i] != actual[i]) {
      return false;
    } else if (!expected[i]) {
      return true;
    }
    i = i + 1;
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
bool_t log_at_start_of_line = true;

void log_maybe_add_indent() {
  size_t indent_count = min_size(log_indent_count, LOG_BUFFER_LEN_MINUS_ONE);
  if (log_at_start_of_line) {
    while (log_index < indent_count) {
      log_buffer[log_index] = ' ';
      log_index = log_index + 1;
    }
    log_at_start_of_line = false;
  }
}

/* Add a null-terminated string to the log. */
void log_string(char const* s) {
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

/* Prints a size_t to the log and returns the number of characters written,
 * which may be useful when trying to align subsequent lines based on the
 * length of the output number. */
size_t log_size(size_t x) {
  size_t magnitude = 1;
  while (x / magnitude >= 10) {
    magnitude = magnitude * 10;
  }
  size_t original_log_index = log_index;
  while (magnitude > 0 && log_index < LOG_BUFFER_LEN_MINUS_ONE) {
    char c = (char) (x / magnitude) + '0';
    log_buffer[log_index] = c;
    log_index = log_index + 1;
    x = x % magnitude;
    magnitude = magnitude / 10;
  }
  return log_index - original_log_index;
}

void log_newline() {
  log_buffer[log_index] = '\n';
  log_index = log_index + 1;
  syscall_write(2, log_buffer, log_index);
  log_index = 0;
  log_at_start_of_line = true;
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

char strings_data[STRINGS_DATA_LENGTH] = {0};
size_t strings_data_index = 0;


/* The index into this array must be representable by 16 bits. */
char* strings_pointers[STRINGS_ID_MAP_LENGTH];

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
struct_info_t struct_infos[STRINGS_ID_MAP_LENGTH];

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

char parse_read_buffer[PARSE_READ_BUFFER_CAPACITY];
size_t parse_read_buffer_index = 0;
size_t parse_read_buffer_length = 0;
size_t parse_line = 1;
size_t parse_column = 1;
size_t parse_start_of_line = 0;
char const* parse_current_filename;

void parse_log_location() {
  log_string(parse_current_filename);
  log_string(":");
  log_size(parse_line);
  log_string(":");
  log_size(parse_column);
  log_string(": ");
}

void parse_log_current_line_with_location_marker() {
  size_t line_number_length = log_size(parse_line);
  log_string(" | ");
  size_t code_line_error_position = parse_read_buffer_index - parse_start_of_line;
  size_t end_of_line = parse_start_of_line;
  size_t end_of_line_limit = parse_start_of_line + MAX_LINE_LENGTH_FOR_ERRORS;
  bool_t reached_end_of_line = false;
  while (end_of_line < end_of_line_limit) {
    if (end_of_line >= parse_read_buffer_length || parse_read_buffer[end_of_line] == '\n') {
      reached_end_of_line = true;
      break;
    }
    end_of_line = end_of_line + 1;
  }
  size_t code_line_length = end_of_line - parse_start_of_line;
  log_lstring(&parse_read_buffer[parse_start_of_line], code_line_length);
  if (!reached_end_of_line) {
    log_string("...");
  }
  if (parse_read_buffer_index == parse_read_buffer_length) {
    log_string("<end-of-file>");
  }
  log_newline();
  size_t error_position_minus_one = line_number_length + 3 + code_line_error_position - 1;
  size_t i = 0;
  while (i < error_position_minus_one) {
    log_string(" ");
    i = i + 1;
  }
  log_string("^");
  log_newline();
}

/* Returns the next character in the stream. If the current buffer of
   characters has been exhausted, we perform a blocking read on stdin to refill
   it. If the read fail, we abort. A return value of 0 indicates that there are
   no more characters to get. */
char peek_char() {
  if (parse_read_buffer_index < parse_read_buffer_length) {
    return parse_read_buffer[parse_read_buffer_index];
  } else {
    return 0;
  }
}

/* Skip past the current character. The behavior is undefined if the stream is
   already exhausted, so it is necessary to have first gotten a non-zero return
   value from peek_char. */
void advance_char() {
  char c = parse_read_buffer[parse_read_buffer_index];
  bool_t is_newline = c == '\n';
  parse_column = parse_column * !is_newline + 1;
  parse_line = parse_line + is_newline * 1;
  parse_read_buffer_index = parse_read_buffer_index + 1;
  parse_start_of_line = parse_start_of_line * !is_newline + parse_read_buffer_index * is_newline;
}

/* Peek the next character and advance past it if non-zero. We do not abort if
   the stream is ended because there is not enough context in this function to
   give a good error message. The current position is advanced regardless of
   whether the file is empty or not, with the expectation that parsing code
   recognizes the 0 return value and does not call parse_char again. */
char parse_char() {
  char c = peek_char();
  advance_char();
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
  char c;
again:
  c = peek_char();
  if (!c) {
  } else if (parse_is_whitespace(c)) {
    advance_char();
    goto again;
  }
}

/* Skips past whitespace, but aborting if there is no whitespace to be found. */
void parse_skip_whitespace1() {
  if (parse_is_whitespace(parse_char())) {
    parse_skip_whitespace();
  } else {
    parse_log_location();
    log_line("Expected whitespace.");
    parse_log_current_line_with_location_marker();
    syscall_exit(1);
  }
}

void parse_error_expected_declaration_start_keyword() {
    parse_log_location();
    log_line("Expected either 'struct' or 'fn' to begin declaration.");
    parse_log_current_line_with_location_marker();
    syscall_exit(1);
}

u8_t parse_identifier_start_chars[256];
u8_t parse_identifier_rest_chars[256];

void parse_init_identifier_chars() {
  size_t c = 'a' - 1;
  while (c <= 'z') {
    parse_identifier_start_chars[c] = 1;
    parse_identifier_rest_chars[c] = 1;
    c = c + 1;
  }
  c = '0';
  while (c <= '9') {
    parse_identifier_rest_chars[c] = 1;
    c = c + 1;
  }
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
    advance_char();
    parse_log_location();
    log_line("Expected identifier.");
    parse_log_current_line_with_location_marker();
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
    advance_char();
    parse_log_location();
    log_line("Expected identifier for type.");
    parse_log_current_line_with_location_marker();
    syscall_exit(1);
    return 0;
  }
  size_t length = parse_read_buffer_index - start_index;
  return strings_id(&parse_read_buffer[start_index], length);
}

typedef struct parse_local_variable_t {
  strings_id_t name;
  strings_id_t type;
} parse_local_variable_t;

parse_local_variable_t parse_local_variables[MAX_LOCAL_VARIABLES];
size_t parse_local_variables_index = 0;

typedef struct parse_fn_signature_t {
  bool_t exists;
  u16_t arity;
  strings_id_t arg_names[14];
  strings_id_t arg_types[14];
  strings_id_t return_type;
} parse_fn_signature_t;

parse_fn_signature_t parse_fn_signatures[STRINGS_ID_MAP_LENGTH];

void parse_declaration() {
  char c = parse_char();
  switch (c) {
    case 's':
      if (!parse_exactly("truct")) {
        parse_error_expected_declaration_start_keyword();
      }
      parse_skip_whitespace1();
      strings_id_t struct_name = parse_permanent_identifier();
      u16_t first_field_index = struct_fields_index;
      parse_skip_whitespace();
      while (true) {
        strings_id_t field_name = parse_permanent_identifier();
        parse_skip_whitespace();
        strings_id_t field_type = parse_type();
        struct_field_t field = { .name = field_name, .type = field_type };
        types_add_struct_field(field);
        parse_skip_whitespace();
        switch (parse_char()) {
          case ',':
            parse_skip_whitespace();
            break;
          case ';':
            struct_infos[struct_name] = (struct_info_t) {
              .field_count = struct_fields_index - first_field_index,
              .first_field_index = first_field_index,
              .exists = true
            };
            return;
          default:
            parse_log_location();
            log_line("Expected ',' or ';'.");
            parse_log_current_line_with_location_marker();
            syscall_exit(1);
        }
      }
      break;
    case 'f':
      if (!parse_exactly("n")) {
        parse_error_expected_declaration_start_keyword();
      }
      parse_skip_whitespace1();
      strings_id_t fn_name = parse_permanent_identifier();
      parse_skip_whitespace();
      if (!parse_exactly("(")) {
        parse_log_location();
        log_line("Expected '(' to begin argument list.");
        parse_log_current_line_with_location_marker();
        syscall_exit(1);
      }
      char first_char_of_arg_list = peek_char();
      parse_fn_signature_t signature = {0};
      signature.exists = true;
      if (first_char_of_arg_list != ')') {
        while (true) {
          signature.arg_names[signature.arity] = parse_permanent_identifier();
          parse_skip_whitespace();
          signature.arg_types[signature.arity] = parse_type();
          signature.arity = signature.arity + 1;
          parse_skip_whitespace();
          switch (parse_char()) {
            case ',':
              parse_skip_whitespace();
              break;
            case ')':
              goto finished_arg_list;
              break;
            default:
              parse_log_location();
              log_line("Expected ',' or ')'.");
              parse_log_current_line_with_location_marker();
              syscall_exit(1);
              break;
          }
        }
      } else {
        advance_char();
      }
finished_arg_list:
      parse_skip_whitespace();
      if (!parse_exactly("{")) {
        parse_log_location();
        log_line("Expected '{' after argument list to begin function body.");
        parse_log_current_line_with_location_marker();
        syscall_exit(1);
      }
      parse_skip_whitespace();
      if (!parse_exactly("}")) {
        parse_log_location();
        log_line("Expected '}' to finish function body.");
        parse_log_current_line_with_location_marker();
        syscall_exit(1);
      }
      parse_fn_signatures[fn_name] = signature;
      break;
    default:
      parse_error_expected_declaration_start_keyword();
      break;
  }
}

void parse_file(char const* filename) {
  i32_t fd = syscall_open(filename, O_RDONLY, 0);
  if (fd >= 0) {
    bool_t reached_end_of_file = false;
    while (true) {
      size_t remaining_capacity = PARSE_READ_BUFFER_CAPACITY - parse_read_buffer_length;
      if (remaining_capacity == 0) {
        break;
      }
      i64_t read_result = syscall_read(fd, &parse_read_buffer[parse_read_buffer_length], remaining_capacity);
      if (read_result > 0) {
        parse_read_buffer_length = parse_read_buffer_length + read_result;
      } else if (read_result == 0) {
        reached_end_of_file = true;
        break;
      } else {
        log_string("Got unix error code while trying to read file \"");
        log_string(filename);
        log_line("\".");
        syscall_exit(1);
      }
    }
    if (!reached_end_of_file) {
      char single_char[1];
      i64_t read_result = syscall_read(fd, single_char, 1);
      if (read_result > 0) {
        log_string("Reached 10 MB file size limit while reading file \"");
        log_string(filename);
        log_line("\".");
        syscall_exit(1);
      } else if (read_result < 0) {
        log_string("Got unix error code while trying to read file \"");
        log_string(filename);
        log_line("\".");
        syscall_exit(1);
      }
    }
    parse_current_filename = filename;
    parse_skip_whitespace();
    while (peek_char()) {
      parse_declaration();
      parse_skip_whitespace();
    }
    parse_current_filename = 0;
  } else {
    log_string("Got unix error code while trying to open \"");
    log_string(filename);
    log_line("\".");
    syscall_exit(1);
  }
}

i32_t main(i32_t argc, char* argv[]) {
  if (argc < 2) {
    log_line("Usage: <exe> command file...");
    log_line("Commands:");
    log_indent();
    log_line("show-structs     List all the struct definitions in the provided files.");
    log_line("show-fns         List all the function definitions in the provided files.");
    log_dedent();
    return 0;
  }
  char* command = argv[1];
  if (string_equal("show-structs", command)) {
    if (argc < 3) {
      log_line("No source files provided.");
      syscall_exit(1);
    }
    parse_init_identifier_chars();
    i32_t arg_index = 2;
    while (arg_index < argc) {
      parse_file(argv[arg_index]);
      arg_index = arg_index + 1;
    }
    size_t infos_index = 0;
    bool_t after_first_struct = false;
    while (infos_index < STRINGS_ID_MAP_LENGTH) {
      struct_info_t info = struct_infos[infos_index];
      if (info.exists) {
        if (after_first_struct) {
          log_newline();
        } else {
          after_first_struct = true;
        }
        log_line(strings_pointers[infos_index]);
        log_indent();
        size_t fields_index = 0;
        while (fields_index < info.field_count) {
          struct_field_t field = struct_fields[info.first_field_index + fields_index];
          log_string(strings_pointers[field.name]);
          log_string(" ");
          log_string(strings_pointers[field.type]);
          fields_index = fields_index + 1;
          if (fields_index == info.field_count) {
            log_line(";");
          } else {
            log_line(",");
          }
        }
        log_dedent();
      }
      infos_index = infos_index + 1;
    }
  } else if (string_equal("show-fns", command)) {
    if (argc < 3) {
      log_line("No source files provided.");
      syscall_exit(1);
    }
    parse_init_identifier_chars();
    i32_t arg_index = 2;
    while (arg_index < argc) {
      parse_file(argv[arg_index]);
      arg_index = arg_index + 1;
    }
    size_t signatures_index = 0;
    while (signatures_index < STRINGS_ID_MAP_LENGTH) {
      parse_fn_signature_t signature = parse_fn_signatures[signatures_index];
      if (signature.exists) {
        log_string(strings_pointers[signatures_index]);
        log_string("(");
        size_t args_index = 0;
        while (args_index < signature.arity) {
          log_string(strings_pointers[signature.arg_names[args_index]]);
          log_string(" ");
          log_string(strings_pointers[signature.arg_types[args_index]]);
          args_index = args_index + 1;
          if (args_index == signature.arity) {
            log_line(") { ... }");
          } else {
            log_string(", ");
          }
        }
      }
      signatures_index = signatures_index + 1;
    }
  } else {
    log_string("Unknown command \"");
    log_string(command);
    log_line("\".");
    syscall_exit(1);
  }
  return 0;
}
