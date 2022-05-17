# clang -O3, symbols stripped
interleave_clmul(uint32_2*, unsigned long*):       # @interleave_clmul(uint32_2*, unsigned long*)
        vpshufd xmm0, xmmword ptr [rdi], 216    # xmm0 = mem[0,2,1,3]
        vpclmulqdq      xmm1, xmm0, xmm0, 17
        vpclmulqdq      xmm0, xmm0, xmm0, 0
        vpaddw  xmm1, xmm1, xmm1
        vpor    xmm0, xmm0, xmm1
        vmovdqu xmmword ptr [rsi], xmm0
        ret
