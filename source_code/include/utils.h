#ifndef __UTILS_H
#define __UTILS_H

#include <stdint.h>

float map(float x, float in_min, float in_max, float out_min, float out_max);
float fmap(float x, float in_min, float in_max, float out_min, float out_max);
long constrain(long x, long min, long max);
float fconstrain(float x, float min, float max);

#endif