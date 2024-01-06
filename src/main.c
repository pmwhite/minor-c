/* --------------------------------------------------------------------------------
 * LIMITS
 *
 * Various memory limits are hardcoded, but it should be easily adjustable, so
 * we put all the constants in one place.
 * -------------------------------------------------------------------------------- */

#define ONE_MB 10485760
#define TEN_MB 10485760 
#define HUNDRED_MB 104857600
#define MAX_U16 65536
#define PARSE_READ_BUFFER_CAPACITY TEN_MB
#define MAX_LINE_LENGTH_FOR_ERRORS 120
#define STRINGS_ID_MAP_LENGTH MAX_U16
#define EXPRESSION_PARSING_RECURSION_LIMIT 255
#define MAX_STRINGS_DATA HUNDRED_MB
#define MAX_STRUCT_FIELDS MAX_U16
#define MAX_ARRAY_LENGTHS MAX_U16
#define MAX_LOCAL_VARIABLES 1024
#define MAX_EXPRESSIONS TEN_MB

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

void ensure_array_space(size_t current_length, size_t capacity, char* array_name) {
  if (current_length >= capacity) {
    log_string("The compiler has reached the capacity of its '");
    log_string(array_name);
    log_line("' array and cannot continue.");
    syscall_exit(1);
  }
}

/* --------------------------------------------------------------------------------
 * STRINGS
 *
 * Strings that need to be looked up get stored in a single hashmap that maps
 * the string to somewhat small index, effectively providing a way to hash cons
 * a string on demand. Code that would otherwise have needed a string hash map
 * can instead use an array indexed of string's key in the global string
 * hashmap.
 * -------------------------------------------------------------------------------- */

char strings_data[MAX_STRINGS_DATA] = {0};
size_t strings_data_index = 0;

/* The index into this array must be representable by 16 bits. */
char* strings_pointers[STRINGS_ID_MAP_LENGTH];
size_t strings_pointers_count = 0;

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
  ensure_array_space(strings_pointers_count, STRINGS_ID_MAP_LENGTH, "strings_pointers");
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
      ensure_array_space(strings_data_index + length, MAX_STRINGS_DATA, "strings_data");
      strings_pointers[candidate_index] = &strings_data[strings_data_index];
      i = 0;
      while (i < length) {
        strings_data[strings_data_index] = string[i];
        i = i + 1;
        strings_data_index = strings_data_index + 1;
      }
      strings_data[strings_data_index] = 0;
      strings_data_index = strings_data_index + 1;
      strings_pointers_count = strings_pointers_count + 1;
      return candidate_index;
    }
    candidate_index = candidate_index + 1;
  } while (candidate_index != final_hash);
  log_line("impossible");
  return 0;
}

/* -------------------------------------------------------------------------------- */

typedef struct location_t {
  size_t line;
  size_t column;
  size_t start_of_line;
  size_t index;
} location_t;

char parse_read_buffer[PARSE_READ_BUFFER_CAPACITY];
size_t parse_read_buffer_length = 0;

location_t current_location = {
  .index = 0,
  .line = 1,
  .column = 1,
  .start_of_line = 0
};

char const* current_filename;

void parse_log_location(location_t location) {
  log_string(current_filename);
  log_string(":");
  log_size(location.line);
  log_string(":");
  log_size(location.column);
  log_string(": ");
}

void parse_log_current_location() {
  parse_log_location(current_location);
}

void parse_log_location_line_with_column_marker(location_t location) {
  size_t line_number_length = log_size(location.line);
  log_string(" | ");
  size_t code_line_error_position = location.index - location.start_of_line;
  size_t end_of_line = location.start_of_line;
  size_t end_of_line_limit = location.start_of_line + MAX_LINE_LENGTH_FOR_ERRORS;
  bool_t reached_end_of_line = false;
  while (end_of_line < end_of_line_limit) {
    if (end_of_line >= parse_read_buffer_length || parse_read_buffer[end_of_line] == '\n') {
      reached_end_of_line = true;
      break;
    }
    end_of_line = end_of_line + 1;
  }
  size_t code_line_length = end_of_line - location.start_of_line;
  log_lstring(&parse_read_buffer[location.start_of_line], code_line_length);
  if (!reached_end_of_line) {
    log_string("...");
  }
  if (location.index >= parse_read_buffer_length) {
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

void parse_log_current_location_line_with_column_marker() {
  parse_log_location_line_with_column_marker(current_location);
}

/* Returns the next character in the stream. If the current buffer of
   characters has been exhausted, we perform a blocking read on stdin to refill
   it. If the read fail, we abort. A return value of 0 indicates that there are
   no more characters to get. */
char peek_char() {
  if (current_location.index < parse_read_buffer_length) {
    return parse_read_buffer[current_location.index];
  } else {
    return 0;
  }
}

void advance_location(location_t* location) {
  char c = parse_read_buffer[location->index];
  bool_t is_newline = c == '\n';
  location->column = location->column * !is_newline + 1;
  location->line = location->line + is_newline * 1;
  location->index = location->index + 1;
  location->start_of_line = location->start_of_line * !is_newline + location->index * is_newline;
}

/* Skip past the current character. The behavior is undefined if the stream is
   already exhausted, so it is necessary to have first gotten a non-zero return
   value from peek_char. */
void advance_char() {
  char c = parse_read_buffer[current_location.index];
  bool_t is_newline = c == '\n';
  current_location.column = current_location.column * !is_newline + 1;
  current_location.line = current_location.line + is_newline * 1;
  current_location.index = current_location.index + 1;
  current_location.start_of_line = current_location.start_of_line * !is_newline + current_location.index * is_newline;
}

/* Peek the next character and advance past it if non-zero. We do not abort if
   the stream is ended because there is not enough context in this function to
   give a good error message. The current position is advanced even if the
   stream has ended because the convention is that the parsing index should sit
   immediately after the index that gave it trouble; in other words, we parse
   first, ask questions later. */
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
  while (parse_is_whitespace(peek_char())) {
    advance_char();
  }
}

/* Skips past whitespace, but aborting if there is no whitespace to be found. */
void parse_skip_whitespace1() {
  if (parse_is_whitespace(parse_char())) {
    parse_skip_whitespace();
  } else {
    parse_log_current_location();
    log_line("Expected whitespace.");
    parse_log_current_location_line_with_column_marker();
    syscall_exit(1);
  }
}

void parse_error_expected_declaration_start_keyword() {
  parse_log_current_location();
  log_line("Expected either 'struct' or 'fn' to begin declaration.");
  parse_log_current_location_line_with_column_marker();
  syscall_exit(1);
}

void parse_error_expected_control_flow_keyword() {
  parse_log_current_location();
  log_line("Expected one of 'if', 'else', 'switch', 'case', 'while', or 'end' after ':'.");
  parse_log_current_location_line_with_column_marker();
  syscall_exit(1);
}

u8_t parse_identifier_start_chars[256];
u8_t parse_identifier_rest_chars[256];
u8_t parse_digit_chars[256];

void parse_init_char_tables() {
  size_t c = 'a' - 1;
  while (c <= 'z') {
    parse_identifier_start_chars[c] = 1;
    parse_identifier_rest_chars[c] = 1;
    c = c + 1;
  }
  c = '0';
  while (c <= '9') {
    parse_identifier_rest_chars[c] = 1;
    parse_digit_chars[c] = 1;
    c = c + 1;
  }
  parse_identifier_rest_chars['_'] = 1;
}

strings_id_t parse_permanent_identifier() {
  size_t start_index = current_location.index;;
  char c = peek_char();
  if (parse_identifier_start_chars[(size_t) c]) {
    do {
      advance_char();
      c = peek_char();
    } while (parse_identifier_rest_chars[(size_t) c]);
    size_t length = current_location.index - start_index;
    return strings_id(&parse_read_buffer[start_index], length);
  } else {
    advance_char();
    parse_log_current_location();
    log_line("Expected identifier.");
    parse_log_current_location_line_with_column_marker();
    syscall_exit(1);
    return 0;
  }
}

u64_t array_lengths[MAX_ARRAY_LENGTHS];
size_t array_lengths_index = 0;

typedef struct type_t {
  /* The named type from which this type is derived. */
  strings_id_t base;
  /* For example, i32*** has three modifiers (each pointer is a modifier). */
  u8_t modifier_count;
  /* The modifier_count least significant bits say whether each modifier is a pointer (0) or an array (1) */
  u8_t modifiers;
  /* If any of the modifiers are arrays, then the lengths of those arrays will
     be stored in array_lengths, starting at first_array_length_index. */
  u16_t first_array_length_index;
} type_t;

u64_t parse_integer_literal() {
  parse_log_current_location();
  log_line("The compiler does not yet support parsing integer literals.");
  parse_log_current_location_line_with_column_marker();
  syscall_exit(1);
  return 0;
}

type_t parse_type() {
  if (!parse_exactly("`")) {
    parse_log_current_location();
    log_line("Expected '`' to begin type.");
    parse_log_current_location_line_with_column_marker();
    syscall_exit(1);
    return (type_t) {0};
  }
  strings_id_t base = parse_permanent_identifier();
  type_t result = {
    .base = base,
    .modifier_count = 0,
    .modifiers = 0,
    .first_array_length_index = array_lengths_index
  };
  while (true) {
    char c = peek_char();
    if (c == '*') {
      advance_char();
      result.modifier_count = result.modifier_count << 1;
    } else if (c == '[') {
      advance_char();
      ensure_array_space(array_lengths_index, MAX_ARRAY_LENGTHS, "array_lengths");
      u64_t length = parse_integer_literal();
      array_lengths[array_lengths_index] = length;
    } else {
      break;
    }
  }
  return result;
}

typedef struct struct_field_t {
  strings_id_t name;
  type_t type;
} struct_field_t;

typedef struct struct_info_t {
  u16_t field_count;
  u16_t first_field_index;
  bool_t exists;
} struct_info_t;

struct_field_t struct_fields[MAX_STRUCT_FIELDS];
size_t struct_fields_index = 0;
struct_info_t struct_infos[STRINGS_ID_MAP_LENGTH];

typedef struct parse_local_variable_t {
  strings_id_t name;
  type_t type;
} parse_local_variable_t;

typedef struct parse_fn_signature_t {
  bool_t exists;
  u16_t arity;
  parse_local_variable_t args[14];
  strings_id_t return_type;
} parse_fn_signature_t;

parse_fn_signature_t parse_fn_signatures[STRINGS_ID_MAP_LENGTH];

typedef u8_t expression_kind_t;
#define expression_kind_operation 0
#define expression_kind_integer 1
#define expression_kind_identifier 2

parse_local_variable_t parse_local_variables[MAX_LOCAL_VARIABLES];
size_t parse_local_variables_index = 0;

typedef struct expression_t {
  expression_kind_t kind;
  /* Arity is the number of operands passed to the operation. */
  u8_t arity;
  /* Data is the id for one of (a) the name of the operation, (b) the
     identifier, or (c) the digits of the integer, depending on the kind. */
  strings_id_t data;
} expression_t;

expression_t parse_expressions[MAX_EXPRESSIONS];
size_t parse_expression_index = 0;

void parse_expression(u8_t depth);

void parse_call_arguments(u8_t depth, location_t name_location, strings_id_t name) {
  if (depth >= EXPRESSION_PARSING_RECURSION_LIMIT) {
    log_line("Reached max expression depth.");
    syscall_exit(1);
  }
  parse_fn_signature_t fn = parse_fn_signatures[name];
  if (fn.exists) {
    parse_expressions[parse_expression_index] = (expression_t) {
      .kind = expression_kind_operation,
      .arity = fn.arity,
      .data = name
    };
    parse_expression_index = parse_expression_index + 1;
    size_t i = 0;
    while (i < fn.arity) {
      parse_expression(depth + 1);
      parse_skip_whitespace();
      i = i + 1;
    }
    if (!parse_exactly(")")) {
      parse_log_current_location();
      log_string("Expected ')' because the function '");
      log_string(strings_pointers[name]);
      log_string("' has arity ");
      log_size((size_t) fn.arity);
      log_line(".");
      parse_log_current_location_line_with_column_marker();
      syscall_exit(1);
    }
  } else {
    advance_location(&name_location);
    parse_log_location(name_location);
    log_string("Unknown function '");
    log_string(strings_pointers[name]);
    log_line("'.");
    parse_log_location_line_with_column_marker(name_location);
    syscall_exit(1);
  }
}

void parse_expression(u8_t depth) {
  if (depth == EXPRESSION_PARSING_RECURSION_LIMIT) {
    log_line("Reached max expression depth.");
    syscall_exit(1);
  }
  char c = peek_char();
  if (parse_identifier_start_chars[(size_t) c]) {
    location_t name_location = current_location;
    strings_id_t name = parse_permanent_identifier();
    parse_skip_whitespace();
    switch(peek_char()) {
      case '(':
        advance_char();
        parse_call_arguments(depth + 1, name_location, name);
        break;
      default:
        (void) 0;
        size_t local_variable_index = 0;
        bool_t found_variable = false;
        parse_local_variable_t variable;
        while (local_variable_index < parse_local_variables_index) {
          variable = parse_local_variables[local_variable_index];
          if (variable.name == name) {
            found_variable = true;
            break;
          }
          local_variable_index = local_variable_index + 1;
        }
        if (found_variable) {
          parse_expressions[parse_expression_index] = (expression_t) {
            .kind = expression_kind_identifier,
            .arity = 0,
            .data = name
          };
          parse_expression_index = parse_expression_index + 1;
        } else {
          /* Advance the identifier location because we saved it at the
              character _before_ the identifier began. */
          advance_location(&name_location);
          parse_log_location(name_location);
          log_string("Unknown variable '");
          log_string(strings_pointers[name]);
          log_line("'.");
          parse_log_location_line_with_column_marker(name_location);
          syscall_exit(1);
        }
        break;
    }
  } else if (parse_digit_chars[(size_t) c]) {
    do {
      advance_char();
    } while (parse_digit_chars[(size_t) peek_char()]);
    char signedness = parse_char();
    if (signedness != 'i' && signedness != 'u') {
      parse_log_current_location();
      log_line("Expected 'u' or 'i' after digits to specify signedness.");
      parse_log_current_location_line_with_column_marker();
      syscall_exit(1);
    }
    if (!parse_digit_chars[(size_t) parse_char()]) {
      parse_log_current_location();
      log_line("Expected digits after signedness to specify size.");
      parse_log_current_location_line_with_column_marker();
      syscall_exit(1);
    }
    while (parse_digit_chars[(size_t) peek_char()]) {
      advance_char();
    }
  } else {
    advance_char();
    parse_log_current_location();
    log_line("Expected identifier or number literal.");
    parse_log_current_location_line_with_column_marker();
    syscall_exit(1);
  }
}

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
        type_t field_type = parse_type();
        struct_field_t field = { .name = field_name, .type = field_type };
        ensure_array_space(struct_fields_index, MAX_STRUCT_FIELDS, "struct_fields");
        struct_fields[struct_fields_index] = field;
        struct_fields_index = struct_fields_index + 1;
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
            parse_log_current_location();
            log_line("Expected ',' or ';'.");
            parse_log_current_location_line_with_column_marker();
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
        parse_log_current_location();
        log_line("Expected '(' to begin argument list.");
        parse_log_current_location_line_with_column_marker();
        syscall_exit(1);
      }
      char first_char_of_arg_list = peek_char();
      parse_fn_signature_t signature = {0};
      signature.exists = true;
      parse_local_variables_index = 0;
      if (first_char_of_arg_list != ')') {
        while (true) {
          parse_local_variable_t variable = {0};
          variable.name = parse_permanent_identifier();
          parse_skip_whitespace();
          variable.type = parse_type();
          signature.args[signature.arity] = variable;
          parse_local_variables[signature.arity] = variable;
          signature.arity = signature.arity + 1;
          parse_local_variables_index = signature.arity;
          parse_skip_whitespace();
          switch (parse_char()) {
            case ',':
              parse_skip_whitespace();
              break;
            case ')':
              goto finished_arg_list;
              break;
            default:
              parse_log_current_location();
              log_line("Expected ',' or ')'.");
              parse_log_current_location_line_with_column_marker();
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
        parse_log_current_location();
        log_line("Expected '{' after argument list to begin function body.");
        parse_log_current_location_line_with_column_marker();
        syscall_exit(1);
      }
      parse_fn_signatures[fn_name] = signature;
      while (true) {
        parse_skip_whitespace();
        char c = peek_char();
        if (c == ':') {
          advance_char();
          switch (parse_char()) {
            case 'i':
              if (!parse_exactly("f")) {
                parse_error_expected_control_flow_keyword();
              }
              parse_skip_whitespace();
              parse_expression(0);
              break;
            case 'e':
              switch (parse_char()) {
                case 'l':
                  if (!parse_exactly("se")) {
                    parse_error_expected_control_flow_keyword();
                  }
                  break;
                case 'n':
                  if (!parse_exactly("d")) {
                    parse_error_expected_control_flow_keyword();
                  }
                  break;
                default:
                  parse_error_expected_control_flow_keyword();
                  break;
              }
              break;
            case 's':
              if (!parse_exactly("witch")) {
                parse_error_expected_control_flow_keyword();
              }
              parse_skip_whitespace();
              parse_expression(0);
              break;
            case 'c':
              if (!parse_exactly("ase")) {
                parse_error_expected_control_flow_keyword();
              }
              parse_skip_whitespace();
              parse_expression(0);
              break;
            case 'w':
              if (!parse_exactly("hile")) {
                parse_error_expected_control_flow_keyword();
              }
              parse_skip_whitespace();
              parse_expression(0);
              break;
            default:
              parse_error_expected_control_flow_keyword();
              break;
          }
        } else if (c == '}') {
          advance_char();
          goto finished_fn_body;
        } else if (parse_identifier_start_chars[(size_t) c]) {
          location_t name_location = current_location;
          strings_id_t name = parse_permanent_identifier();
          parse_skip_whitespace();
          char c = peek_char();
          if (c == '=') {
            advance_char();
            parse_skip_whitespace();
            parse_expression(0);
            parse_local_variables[parse_local_variables_index] = (parse_local_variable_t) {
              .name = name,
              .type = {0}
            };
            parse_local_variables_index = parse_local_variables_index + 1;
          } else if (c == '(') {
            advance_char();
            parse_call_arguments(0, name_location, name);
          } else {
            parse_log_current_location();
            log_line("Expected statement or '}'.");
            parse_log_current_location_line_with_column_marker();
            syscall_exit(1);
          }
        } else {
          advance_char();
          parse_log_current_location();
          log_line("Expected statement or '}'.");
          parse_log_current_location_line_with_column_marker();
          syscall_exit(1);
        }
      }
finished_fn_body:
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
    current_filename = filename;
    parse_skip_whitespace();
    while (peek_char()) {
      parse_declaration();
      parse_skip_whitespace();
    }
    current_filename = 0;
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
    log_line("translate   Read the provided Minor C source files and send equivalent C code to stdout.");
    log_line("sizes       Print the sizes of compiler-internal data types.");
    log_dedent();
    return 0;
  }
  char* command = argv[1];
  if (string_equal("translate", command)) {
    if (argc < 3) {
      log_line("No source files provided.");
      syscall_exit(1);
    }
    parse_init_char_tables();
    i32_t arg_index = 2;
    while (arg_index < argc) {
      parse_file(argv[arg_index]);
      arg_index = arg_index + 1;
    }
  } else if (string_equal("sizes", command)) {
    log_string("type_t: ");
    log_size(sizeof(type_t));
    log_newline();
    log_string("struct_field_t: ");
    log_size(sizeof(struct_field_t));
    log_newline();
    log_string("struct_info_t: ");
    log_size(sizeof(struct_info_t));
    log_newline();
    log_string("parse_fn_signature_t: ");
    log_size(sizeof(parse_fn_signature_t));
    log_newline();
    log_string("parse_local_variable_t: ");
    log_size(sizeof(parse_local_variable_t));
    log_newline();
    log_string("expression_t: ");
    log_size(sizeof(expression_t));
    log_newline();
  } else {
    log_string("Unknown command \"");
    log_string(command);
    log_line("\".");
    syscall_exit(1);
  }
  return 0;
}
