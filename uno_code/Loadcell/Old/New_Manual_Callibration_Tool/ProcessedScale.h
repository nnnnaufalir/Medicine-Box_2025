#ifndef ProcessedScale_h
#define ProcessedScale_h

class ProcessedScale {
private:
    long offset;
    float scale_factor;

public:
    // Constructor
    ProcessedScale();

    // Mengatur offset secara manual
    void setOffset(long new_offset);
    long getOffset();

    // Mengatur faktor skala
    void setScale(float new_scale);
    float getScale();

    // Shortcut untuk mengatur offset ke nilai saat ini
    void tare(long current_filtered_value);

    // Menghitung berat akhir berdasarkan nilai terfilter
    float getWeight(long current_filtered_value);
};

#endif