#ifndef LPF_H
#define LPF_H

#include <stdint.h>

typedef  struct
{
	float estimate_value;
    float gain;               // 0 to 100%
} lpf_data_t;

/**
 * @brief			Low pass filter function
 * @param[in]		current Current sensor data, new data will be calculate and replace estimate_value in lpf_data_t
 * @param[in]		measure New raw mesure data
 */
void lpf_update_estimate(lpf_data_t *current, float measure);

#endif /* LPF_H */
