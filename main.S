.intel_syntax noprefix

.text
  .globl _start, syscall5, syscall4, syscall3, syscall2, syscall1

  _start:
    xor rbp,rbp
    pop rdi
    mov rsi,rsp
    and rsp,-16
    call main

    mov rdi,rax
    mov rax,60
    syscall

    ret

  syscall5:
    mov rax,rdi
    mov rdi,rsi
    mov rsi,rdx
    mov rdx, rcx
    mov r10, r8
    mov r8, r9
    syscall
    ret

  syscall4:
    mov rax,rdi
    mov rdi,rsi
    mov rsi,rdx
    mov rdx, rcx
    mov r10, r8
    syscall
    ret

  syscall3:
    mov rax,rdi
    mov rdi,rsi
    mov rsi,rdx
    mov rdx, rcx
    syscall
    ret

  syscall2:
    mov rax,rdi
    mov rdi,rsi
    mov rsi,rdx
    syscall
    ret

  syscall1:
    mov rax,rdi
    mov rdi,rsi
    syscall
    ret
