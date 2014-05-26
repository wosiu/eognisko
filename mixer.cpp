/*
 * Projekt zaliczeniowy 3 "E-ognisko" - SIK lab
 * Autor: Michal Wos mw336071, MIM UW 2014
 */
#include "mixer.hpp"

#include <climits>
#include <boost/asio.hpp>

void mixer(struct mixer_input* inputs, size_t n, void* output_buf,
		size_t* output_size, unsigned long tx_interval_ms) {

	// TODO uncomment
	//*output_size = 176.4f * tx_interval_ms;
	*output_size = 100;
	*output_size -= *output_size % 2;

	for (size_t i = 0; i < n; ++i) {
		inputs[i].consumed = 0;
	}

	for (size_t i = 0; i < *output_size / 2; ++i) {
		int32_t sum = 0;

		for (size_t j = 0; j < n; ++j) {
			if (inputs[j].consumed + 1 < inputs[j].len) {
				sum += ((int16_t *) inputs[j].data)[i];
				inputs[j].consumed += 2;
			}
		}

		int16_t limited;

		if (sum <= INT16_MIN)
			limited = INT16_MIN;
		else if (sum >= INT16_MAX)
			limited = INT16_MAX;
		else
			limited = sum;

		((int16_t *) output_buf)[i] = limited;
	}
}
