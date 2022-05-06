
          global    start

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
          test      rdi, rdi
          jnz       non_zero
          mov       byte [rsi], '0'
          mov       rax, 1
          ret
non_zero:
          xor       r8, r8                  ; length
          inc       r8
          cmp       rdi, 10
          jle       length_calculated
          inc       r8
          cmp       rdi, 100
          jle       length_calculated
          inc       r8
          cmp       rdi, 1000
          jle       length_calculated
          inc       r8
          cmp       rdi, 10000
          jle       length_calculated
          inc       r8
          cmp       rdi, 100000
          jle       length_calculated
          inc       r8
          cmp       rdi, 1000000
          jle       length_calculated
          inc       r8
          cmp       rdi, 10000000
          jle       length_calculated
          inc       r8
          cmp       rdi, 100000000
          jle       length_calculated
          inc       r8
          cmp       rdi, 1000000000
          jle       length_calculated
          inc       r8
          mov       r9, 10000000000
          cmp       r9, rdi
          jle       length_calculated
          inc       r8
          mov       r9, 100000000000
          cmp       r9, rdi
          jle       length_calculated
          inc       r8
          mov       r9, 1000000000000
          cmp       r9, rdi
          jle       length_calculated
          inc       r8
          mov       r9, 10000000000000
          cmp       r9, rdi
          jle       length_calculated
          inc       r8
          mov       r9, 100000000000000
          cmp       r9, rdi
          jle       length_calculated
          inc       r8
          mov       r9, 1000000000000000
          cmp       r9, rdi
          jle       length_calculated
          inc       r8
          mov       r9, 10000000000000000
          cmp       r9, rdi
          jle       length_calculated
          inc       r8
          mov       r9, 100000000000000000
          cmp       r9, rdi
          jle       length_calculated
          inc       r8
          mov       r9, 1000000000000000000
          cmp       r9, rdi
          jle       length_calculated
          inc       r8
length_calculated:
          mov       rax, rdi
          mov       r9, rsi                 ; beginning of string
          add       rsi, r8
          mov       byte [rsi], r8b

          mov       rbx, 10
calc_digits:
          xor       rdx, rdx
          div       rbx                     ; rax is floored result, rdx is remainder
          add       rdx, '0'
          
          mov       byte [rsi], dl
          dec       rsi
          cmp       r9, rsi
          jl        calc_digits

          mov       rax, r8
          inc       rax
          ret


          ; returns length in rax
start:
          
          mov       rax, 20000              ; 20 kilobytes
          call      call_mmap               ; get memory
          xchg      rax, rsi                ; rsi SHOULD be the beginning of the mmap-ed memory?
          mov       r10, rsi
          mov       qword [rsi], "Outp"
          add       rsi, 4
          mov       qword [rsi], `ut:\n`
          add       rsi, 4



          rdtsc
          shl       rdx, 16
          mov       rdi, rdx

          call      u64_to_str

          mov       rax, 0x02000004         ; system call for write
          mov       rdi, 1                  ; file handle 1 is stdout
          mov       rsi, r10                ; address of string to output?
          mov       rdx, rsi
          sub       rdx, r10
          mov       rdx, 200
          syscall                           ; invoke operating system to do the write
          mov       rax, 0x02000001         ; system call for exit
          xor       rdi, rdi                ; exit code 0
          syscall                           ; invoke operating system to exit
