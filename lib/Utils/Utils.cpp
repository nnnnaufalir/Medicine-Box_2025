#include "Utils.h"

// --- Implementasi Kelas EMAFilter ---

EMAFilter::EMAFilter(float alpha)
{
    _alpha = alpha;
    _filteredValue = 0.0f; // Inisialisasi nilai filter awal
}

float EMAFilter::filter(float newValue)
{
    // Rumus EMA: filtered_value = (alpha * new_value) + ((1 - alpha) * previous_filtered_value)
    _filteredValue = (_alpha * newValue) + ((1.0f - _alpha) * _filteredValue);
    return _filteredValue;
}

void EMAFilter::reset(float initialValue)
{
    _filteredValue = initialValue;
}

void EMAFilter::reset(float initialValue, float alpha)
{
    _filteredValue = initialValue;
    _alpha = alpha;
}
