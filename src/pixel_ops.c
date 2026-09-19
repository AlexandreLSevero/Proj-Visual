#include "pixel_ops.h"

#include <math.h>

u8 pixel_rgb_to_gray(u8 r, u8 g, u8 b) {
    double y = 0.2125 * (double)r + 0.7154 * (double)g + 0.0721 * (double)b;

    if (y < 0.0) {
        y = 0.0;
    } else if (y > 255.0) {
        y = 255.0;
    }

    return (u8)(y + 0.5); /* arredondamento para o inteiro mais próximo */
}

int pixel_is_gray(u8 r, u8 g, u8 b) {
    return r == g && g == b;
}

void histogram_compute(const u8 *gray_pixels, size_t count, Histogram *out) {
    size_t i;
    double sum = 0.0;
    double sum_sq = 0.0;

    for (i = 0; i < HISTOGRAM_LEVELS; i++) {
        out->counts[i] = 0;
    }
    out->total_pixels = count;

    for (i = 0; i < count; i++) {
        out->counts[gray_pixels[i]]++;
        sum += (double)gray_pixels[i];
    }

    if (count == 0) {
        out->mean = 0.0;
        out->stddev = 0.0;
        return;
    }

    out->mean = sum / (double)count;

    for (i = 0; i < count; i++) {
        double diff = (double)gray_pixels[i] - out->mean;
        sum_sq += diff * diff;
    }
    out->stddev = sqrt(sum_sq / (double)count);
}

const char *histogram_classify_brightness(double mean) {
    if (mean < 85.0) {
        return "escura";
    }
    if (mean <= 170.0) {
        return "média";
    }
    return "clara";
}

const char *histogram_classify_contrast(double stddev) {
    if (stddev < 42.0) {
        return "baixo";
    }
    if (stddev <= 85.0) {
        return "médio";
    }
    return "alto";
}

void histogram_equalize(const u8 *src, u8 *dst, size_t count) {
    Histogram hist;
    long cdf[HISTOGRAM_LEVELS];
    long cdf_min = -1;
    int lut[HISTOGRAM_LEVELS];
    size_t i;
    long running;

    histogram_compute(src, count, &hist);

    running = 0;
    for (i = 0; i < HISTOGRAM_LEVELS; i++) {
        running += hist.counts[i];
        cdf[i] = running;
        if (cdf_min < 0 && hist.counts[i] > 0) {
            cdf_min = cdf[i];
        }
    }

    if (count == 0 || cdf_min < 0 || (long)count == cdf_min) {
        /* Imagem vazia ou de uma única cor: não há o que equalizar. */
        for (i = 0; i < count; i++) {
            dst[i] = src[i];
        }
        return;
    }

    for (i = 0; i < HISTOGRAM_LEVELS; i++) {
        double value = ((double)(cdf[i] - cdf_min) / (double)((long)count - cdf_min)) * 255.0;
        if (value < 0.0) {
            value = 0.0;
        } else if (value > 255.0) {
            value = 255.0;
        }
        lut[i] = (int)(value + 0.5);
    }

    for (i = 0; i < count; i++) {
        dst[i] = (u8)lut[src[i]];
    }
}
