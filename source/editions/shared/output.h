/* SPDX-License-Identifier: MIT — original bounded output stage, shared by CPU0/CPU1/WASM. */
#pragma once
#include <stdint.h>
#if defined(__GNUC__) || defined(__clang__)
#define Z_INLINE static inline __attribute__((always_inline))
#else
#define Z_INLINE static inline
#endif
Z_INLINE float z_output_sample(float x,float gain,uint32_t limiter){
 float a;x*=gain;if(!(x==x))return 0;a=x<0?-x:x;if(a>1000.0f)return x<0?-0.999f:0.999f;
 if(limiter&&a>0.85f){a=0.85f+0.149f*(a-0.85f)/(0.149f+a-0.85f);x=x<0?-a:a;}
 return x>0.999f?0.999f:x< -0.999f?-0.999f:x;
}
Z_INLINE uint32_t z_output_kernel(const float *src,int32_t *dst,uint32_t frames,uint32_t master,uint32_t trim,uint32_t limiter){
 uint32_t i;float peak=0,gain=(float)master*(float)trim/409600.0f;
 for(i=0;i<frames*2;i++){float x=z_output_sample(src[i],gain,limiter),a=x<0?-x:x;if(a>peak)peak=a;dst[i]=(int32_t)(x*8388607.0f);}
 return (uint32_t)(peak*100.0f+0.5f);
}
void z_post(uint32_t trim,uint32_t limiter,float *stereo,uint32_t frames);
