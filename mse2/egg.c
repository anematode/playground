for (size_t i = 0; i < bytes / 8; i += 4) {
	// Output 96 bytes per loop 
	uint64_t k = src_c[i];

#define GRAB_PARTS(a, b, c)  \
	a = _pdep_u64(k, BIT_MASK_1);	          /* first 22 bits (little endian) */ \
	b = _pdep_u64(k >> 22, BIT_MASK_2);       /* middle 21 bits */ \
	c = _pdep_u64(k >> 43, BIT_MASK_3);       /* last 21 bits */

	GRAB_PARTS(p1, p2, p3)	
	k = src_c[i+1];
	GRAB_PARTS(p4, p5, p6)
	
	_mm256_store_si256((__m256i*) &dst_c[3 * i], _mm256_set_epi64x(p4, p3, p2, p1));

	k = src_c[i+2];
	GRAB_PARTS(p1, p2, p3)

	k = src_c[i+3];
	
	_mm256_store_si256((__m256i*) &dst_c[3 * i + 4], _mm256_set_epi64x(p2, p1, p6, p5));
	GRAB_PARTS(p4, p5, p6)
	    
	_mm256_store_si256((__m256i*) &dst_c[3 * i + 8], _mm256_set_epi64x(p6, p5, p4, p3));
    }

