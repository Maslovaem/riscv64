.global _start

.section .text
_start:
        li a0, 1
        la a1, s    
        li a2, 14
        li a7, 64
        ecall

        li a0, 0
        li a7, 93
        ecall

.section .data
        s: .ascii "Hello world\n \0"

