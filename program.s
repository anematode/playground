
          global    start

          section .data
digit_table:                                ; table of powers of 2 in decimal, for base conversion, in packed word form
          array dq `\00\00\00\00\00\00\00\00\00\01`, `\00\00\00\00\00\00\00\00\00\02`, `\00\00\00\00\00\00\00\00\00\04`, `\00\00\00\00\00\00\00\00\00\08`, `\00\00\00\00\00\00\00\00\00\10`, `\00\00\00\00\00\00\00\00\00\20`, `\00\00\00\00\00\00\00\00\00\40`, `\00\00\00\00\00\00\00\00\01\1c`, `\00\00\00\00\00\00\00\00\02\38`, `\00\00\00\00\00\00\00\00\05\0c`, `\00\00\00\00\00\00\00\00\0a\18`, `\00\00\00\00\00\00\00\00\14\30`, `\00\00\00\00\00\00\00\00\28\60`, `\00\00\00\00\00\00\00\00\51\5c`, `\00\00\00\00\00\00\00\01\3f\54`, `\00\00\00\00\00\00\00\03\1b\44`, `\00\00\00\00\00\00\00\06\37\24`, `\00\00\00\00\00\00\00\0d\0a\48`, `\00\00\00\00\00\00\00\1a\15\2c`, `\00\00\00\00\00\00\00\34\2a\58`, `\00\00\00\00\00\00\01\04\55\4c`, `\00\00\00\00\00\00\02\09\47\34`, `\00\00\00\00\00\00\04\13\2b\04`, `\00\00\00\00\00\00\08\26\56\08`, `\00\00\00\00\00\00\10\4d\48\10`, `\00\00\00\00\00\00\21\37\2c\20`, `\00\00\00\00\00\00\43\0a\58\40`, `\00\00\00\00\00\01\22\15\4d\1c`, `\00\00\00\00\00\02\44\2b\36\38`, `\00\00\00\00\00\05\24\57\09\0c`, `\00\00\00\00\00\0a\49\4a\12\18`, `\00\00\00\00\00\15\2f\30\24\30`, `\00\00\00\00\00\2a\5e\60\48\60`, `\00\00\00\00\00\55\59\5d\2d\5c`, `\00\00\00\00\01\47\4f\56\5b\54`, `\00\00\00\00\03\2b\3b\49\53\44`, `\00\00\00\00\06\57\13\2f\43\24`, `\00\00\00\00\0d\4a\26\5f\22\48`, `\00\00\00\00\1b\30\4d\5a\45\2c`, `\00\00\00\00\36\61\37\51\26\58`, `\00\00\00\01\09\5f\0b\3e\4d\4c`, `\00\00\00\02\13\5a\17\19\37\34`, `\00\00\00\04\27\50\2e\33\0b\04`, `\00\00\00\08\4f\3c\5d\02\16\08`, `\00\00\00\11\3b\15\56\04\2c\10`, `\00\00\00\23\12\2b\48\08\58\20`, `\00\00\00\46\24\57\2c\11\4c\40`, `\00\00\01\28\49\4a\58\23\35\1c`, `\00\00\02\51\2f\31\4c\47\06\38`, `\00\00\05\3e\5e\63\35\2a\0d\0c`, `\00\00\0b\19\59\63\06\54\1a\18`, `\00\00\16\33\4f\62\0d\44\34\30`, `\00\00\2d\03\3b\60\1b\25\04\60`, `\00\00\5a\07\13\5c\36\4a\09\5c`, `\00\01\50\0e\27\55\09\30\13\54`, `\00\03\3c\1c\4f\46\12\60\27\44`, `\00\07\14\39\3b\28\25\5c\4f\24`, `\00\0e\29\0f\12\50\4b\55\3a\48`, `\00\1c\52\1e\25\3d\33\47\11\2c`, `\00\39\40\3c\4b\17\03\2a\22\58`, `\01\0f\1d\15\32\2e\06\54\45\4c`, `\02\1e\3a\2b\00\5c\0d\45\27\34`, `\04\3d\10\56\01\54\1b\26\4f\04`, `\09\16\21\48\03\44\36\4d\3a\08`
:num_to_digits:
          db        "000123456789101112131415161718192021222324252627282930313233343536373839404142434445464748495051525354555657585960616263646566676869707172737475767778798081828384858687888990919293949596979899"
          section .text
%define MAP_FAILED -1                       ; returned if mmap fails
call_mmap:                                  ; takes in rax as the number of bytes to allocate
                                            ; sets rax to -1 if mmap fails
                                            ; otherwise, set rax to the beginning address and rbx to the length
          cmp       rax, 1                  
          jge       valid_size_request      ; ensure rax is not negative (although mmap will do this anyway...)
          mov       rax, MAP_FAILED
          ret
valid_size_request:
          mov       rsi, rax                ; allocate rax bytes
          mov       rax, 0x020000c5         ; system call for mmap
          xor       rdi, rdi                ; addr = 0, mmap location will be done by kernel
          
%define PROT_READ 0x01
%define PROT_WRITE 0x02
%define PROT_READ_WRITE (PROT_READ | PROT_WRITE)
%define MAP_ANON 0x1000
          mov       rdx, PROT_READ_WRITE    ; allow read and write 
          mov       r10, MAP_ANON           ; anonymous mapping
          xor       r8, r8                  ; unused params
          xor       r9, r9
          syscall                           ; mmap (sets rax according to our conventions)
          ret
u64_to_str:                                 ; convert rdi as a 64-bit integer, write decimal to [rsi] (no bounds checking...)
                                            ; returns length in rax
                                            ; clobbers rax, rbx, rdi, xmm0/1
          push      rbp                     ; base pointer
          mov       rbp, rsp
                                            ; we repeatedly add decimal powers of two vertically
                                            ; 0b1000101
                                            ; ... 0 0 0 0 01
                                            ; ... 0 0 0 0 04
                                            ; ... 0 0 0 0 64
                                            ;     0 0 0 0 69

                                            ; then we perform the carry, and write the result into rsi
          pxor      xmm0, xmm0              ; stores the decimal bytes (higher)
          pxor      xmm1, xmm1              ; (lower)
          mov       r8, digit_table         ; for some reason it complains about 32-bit addressing mode...
          mov       rbx, 0
vertical_add:
          shr       rax, 1                  ; sets CF if 0 bit set
          jnc       no_add_needed
          phaddw    xmm0, oword [r8+rbx*8]
          phaddw    xmm1, oword [r8+rbx*8+16]
no_add_needed:
          add       rbx, 4
          cmp       rbx, 252                ; iterate 64 times
          jl        vertical_add
                                            ; CARRY TIME
          mov       ebx, 0
carry_gang1:
          pextrb    ecx, xmm0, ebx 
          add       ecx, '0'
          mov       [rsi+rbx], cx
          inc       bx
          cmp       rbx, 15
          jl        carry_gang1
          mov       ebx, 0
carry_gang2:
          pextrb    ecx, xmm1, bx
          add       ecx, '0'
          mov       [rsi+rbx], cx
          inc       bx
          cmp       rbx, 15
          jl        carry_gang2

          mov       rax, 30

          mov       rsp, rbp
          pop       rbp                     ; preserve rbp
          ret
start:
          
          mov       rax, 20000              ; 20 kilobytes
          call      call_mmap               ; get memory
          xchg      rax, rsi                ; rsi SHOULD be the beginning of the mmap-ed memory?
          mov       qword [rsi], "poop"     ; store poop
          add       rsi, 4                  ; move forward
          mov       rdi, 12345              ; store number
          call      u64_to_str              ; convert rdi to decimal, storing in [rsi]

          mov       rax, 0x02000004         ; system call for write
          mov       rdi, 1                  ; file handle 1 is stdout
         ;mov       rsi, rsi                ; address of string to output?
          mov       rdx, rax                  ; number of bytes
          syscall                           ; invoke operating system to do the write
          mov       rax, 0x02000001         ; system call for exit
          xor       rdi, rdi                ; exit code 0
          syscall                           ; invoke operating system to exit
