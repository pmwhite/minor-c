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
 * SYSCALLS
 *
 * Since we do not use the C standard library, we must define our own syscall wrappers.
 * -------------------------------------------------------------------------------- */

void* syscall5(void* number, void* arg1, void* arg2, void* arg3, void* arg4, void* arg5);

i64_t syscall_write(i32_t fd, void const* data, u64_t nbytes) {
  return (i64_t) syscall5((void*)1, (void*)(i64_t)fd, (void*)data, (void*)nbytes, 0, 0);
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

void log_string(char* s) {
  size_t indent_count = min_size(log_indent_count, LOG_BUFFER_LEN_MINUS_ONE);
  size_t s_index = 0;
  if (at_start_of_line) {
    while (log_index < indent_count) {
      log_buffer[log_index] = ' ';
      log_index = log_index + 1;
    }
    at_start_of_line = false;
  }
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
const size_t STRINGS_DATA_LENGTH = 104857600;
char strings_data[104857600] = {0};
size_t strings_data_index = 0;

const size_t STRINGS_MAX_ID = 65536;

/* The index into this array must be representable by 16 bits. */
char* strings_pointers[65536] = {0};

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
        /* TODO: abort the program because there is not enough space for the new string. */
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
  /* TODO: abort the program because the string was not found, and there were
     no empty spots to put it into. */
  return 0;
} 

/* -------------------------------------------------------------------------------- */

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
  log_string("hello");
  log_newline();
  log_indent();
  log_string("hello");
  log_newline();
  log_dedent();
  log_string("hello");
  log_newline();
  return 0;
}
