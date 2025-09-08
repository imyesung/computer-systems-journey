# compare_zeroing_sizes.s
# Size probe for zeroing ECX, AT&T syntax

    .text
    .globl _start
_start:
    xorq  %rcx, %rcx        # XOR zero in 64 bits
    xorl  %ecx, %ecx        # XOR zero in 32 bits
    subl  %ecx, %ecx        # SUB zero
    movl  $0, %ecx          # MOV imm zero
    andl  $0, %ecx          # AND zero
    imull $0, %ecx, %ecx    # IMUL zero
    shll  $32, %ecx         # SHL 32 masked to 0 at runtime
    shrl  $32, %ecx         # SHR 32 masked to 0 at runtime
    ret
