#include "ProcessedScale.h" 

ProcessedScale::ProcessedScale() {
    offset = 0;
    scale_factor = 1.0; // Default scale
}

void ProcessedScale::setOffset(long new_offset) {
    offset = new_offset;
}

long ProcessedScale::getOffset() {
    return offset;
}

void ProcessedScale::setScale(float new_scale) {
    scale_factor = new_scale;
}

float ProcessedScale::getScale() {
    return scale_factor;
}

void ProcessedScale::tare(long current_filtered_value) {
    offset = current_filtered_value;
}

float ProcessedScale::getWeight(long current_filtered_value) {
    // Menghindari pembagian dengan nol
    if (scale_factor == 0) {
        return 0.0f;
    }
    return (float)(current_filtered_value - offset) / scale_factor;
}
