/* SPDX-License-Identifier: MIT */
#include "output.h"
void z_post(uint32_t trim,uint32_t limiter,float *out,uint32_t frames){uint32_t i;if(trim>127)trim=127;for(i=0;i<frames*2;i++)out[i]=z_output_sample(out[i],trim/100.0f,limiter);}
