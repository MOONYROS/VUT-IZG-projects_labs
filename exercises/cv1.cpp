/**
 * IZG - Zaklady pocitacove grafiky - FIT VUT
 * Lab 01 - Redukce barevneho prostoru
 *
 * Copyright (c) 2022-23 by Michal Vlnas, ivlnas@fit.vutbr.cz
 *
 * Tato sablona slouzi vyhradne pro studijni ucely, sireni kodu je bez vedomi autora zakazano.
 */

#include "base.h"

/**
 * Converts image into grayscale.
 * Use empiric equation presented in lecture.
 */


void ImageTransform::grayscale()
{
    uint8_t grey;
    for (uint32_t y = 0; y < cfg->h; y++)
    {
        for (uint32_t x = 0; x < cfg->w; x++)
        {
            auto p = getPixel(x, y);
            grey = 0.299f * p.r + 0.587f * p.g + 0.114f * p.b;
            setPixel(x, y, RGB(grey));
        }
    }
}

/**
 * Evaluate threshold value for further usage in thresholding,
 * has to be called after grayscale().
 *
 * Note: evaluation method will DIFFER for each group!
 *
 * @return threshold
 */
uint8_t ImageTransform::evaluateThreshold()
{
    uint8_t th = 128;

    // TODO: write code here or in evaluateThreshold(x, y)

    IZG_INFO("Evaluated threshold: " + std::to_string(th));
    return th;
}

/**
 * Evaluate threshold value for further usage in thresholding,
 * has to be called after grayscale().
 *
 * Note: evaluation method will DIFFER for each group!
 *
 * @param x Input pixel coordinate
 * @param y Input pixel coordinate
 * @return  threshold
 */
uint8_t ImageTransform::evaluateThreshold(int32_t x, int32_t y)
{
    (void)x;
    (void)y;

    uint8_t th = 128;

    // TODO: write code here or in evaluateThreshold()
    uint32_t preth = 0;
    
    for(unsigned int x = 0; x < cfg->w; x++)
        preth += getPixel(x, y).r;

    preth = preth / cfg->w;

    th = preth;
    return th;
}

void ImageTransform::threshold()
{
    // TODO: write code here
    grayscale();

    uint8_t th;
    uint32_t x = 0;
    for (uint32_t y = 0; y < cfg->h; y++)
    {
        th = evaluateThreshold(x, y);
        for (x = 0; x < cfg->w; x++)
        {
            if(th > getPixel(x, y).r)
                setPixel(x, y, COLOR_BLACK);
            else
                setPixel(x, y, COLOR_WHITE);
        }
    }
}

void ImageTransform::randomDithering()
{
    grayscale();

    for (uint32_t y = 0; y < cfg->h; y++)
    {
        for (uint32_t x = 0; x < cfg->w; x++)
        {
            auto p = getPixel(x, y);
            uint8_t value = p.r > getRandom() ? 255 : 0;

            setPixel(x, y, RGB(value));
        }
    }
}

void ImageTransform::orderedDithering()
{
    static uint8_t matrix[m_side][m_side] =
    {
        { 0,   254, 51,  255 },
        { 68,  136, 187, 119 },
        { 34,  238, 17,  221 },
        { 170, 102, 153, 85  }
    };

    // TODO: write code here
    uint8_t grey;
    for (uint32_t y = 0; y < cfg->h; y++)
    {
        for (uint32_t x = 0; x < cfg->w; x++)
        {
            uint32_t i = x % m_side;
            uint32_t j = y % m_side;

            auto p = getPixel(x, y);
            grey = 0.299f * p.r + 0.587f * p.g + 0.114f * p.b;
            
            if(grey > matrix[i][j])
                setPixel(x, y, COLOR_WHITE);
            else
                setPixel(x, y, COLOR_BLACK);
        }
    }
}

void ImageTransform::updatePixelWithError(uint32_t x, uint32_t y, float err)
{
    (void)x;
    (void)y;
    (void)err;

    // TODO: write code here
    if(x >= cfg->w || y >= cfg->h)
        return;

    uint8_t value = std::round(std::min(float(255), std::max(float(0), err + getPixel(x, y).r)));
    setPixel(x, y, value);
}

void ImageTransform::errorDistribution()
{
    // TODO: write code here
    uint8_t tresh = 127;
    float err;
    grayscale();

    for (uint32_t y = 0; y < cfg->h; y++)
    {
        for (uint32_t x = 0; x < cfg->w; x++)
        {
            if(tresh < getPixel(x, y).r)
            {
                setPixel(x, y, COLOR_WHITE);
                err = getPixel(x, y).r - 255;
            }
            else
            {
                setPixel(x, y, COLOR_BLACK);
                err = getPixel(x, y).r;
            }
            updatePixelWithError(x+1, y, 3.0/8.0*err);
            updatePixelWithError(x, y+1, 3.0/8.0*err);
            updatePixelWithError(x+1, y+1, 2.0/8.0*err);
        }
    }
}

