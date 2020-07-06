/*
 * @Author: xieydd
 * @since: 2020-06-29 22:44:59
 * @lastTime: 2020-07-06 22:10:49
 * @LastAuthor: Do not edit
 * @message: 
 */
#include "BoxFilter.h"
#include <algorithm>
#include <vector>

void BoxFilter::filter(float *input, int radius, int height, int width, float *output)
{
    for (int h = 0; h < height; ++h)
    {
        int height_shift = h * width;
        for (int w = 0; w < width; ++w)
        {
            int start_h = std::max(0, h - radius);
            int end_h = std::min(height - 1, h + radius);
            int start_w = std::max(0, w - radius);
            int end_w = std::min(width - 1, w + radius);

            float tmp = 0.f;
            int num = (end_h - start_h) * (end_w - start_w);
            for (int fh = start_h; fh < end_h; ++fh)
            {
                for (int fw = start_w; fw < end_w; ++fw)
                {
                    tmp += input[fh * width + fw];
                }
            }
            output[height_shift + w] = tmp / num;
        }
    }
}
// Cache n row sum for col
void BoxFilter::filterFast(float *input, int radius, int height, int width, float *output)
{
    float *cachePtr = &(cache[0]);
    for (int h = 0; h < height; ++h)
    {
        int height_shift = h * width;
        for (int w = 0; w < width; ++w)
        {
            int start_w = std::max(0, w - radius);
            int end_w = std::min(width - 1, w + radius);

            float tmp = 0.f;
            for (int fw = start_w; fw <= end_w; ++fw)
            {
                tmp += input[height_shift + fw];
            }
            cachePtr[height_shift + w] = tmp;
        }
    }

    for (int h = 0; h < height; ++h)
    {
        int height_shift = h * width;
        int start_h = std::max(0, h - radius);
        int end_h = std::min(height - 1, h + radius);

        for (int fh = start_h; fh <= end_h; ++fh)
        {
            int out_shift = fh * width;
            for (int w = 0; w < width; ++w)
            {
                output[out_shift + w] += cachePtr[height_shift + w];
            }
        }
    }
}

void BoxFilter::filterFastV2(float *input, int radius, int height, int width, float *output)
{
    float *cachePtr = &(cache[0]);
    for (int h = 0; h < height; ++h)
    {
        int shift = h * width;

        float tmp;
        // 计算0-半径列的和
        for (int w = 0; w < radius; ++w)
        {
            tmp += input[w + shift];
        }

        // 0-半径列和+半径-直径列和 = cachePtr[shift:shift+radius]
        for (int w = 0; w < radius; ++w)
        {
            tmp += input[w + shift + radius];
            cachePtr[shift + w] = tmp;
        }

        // cachePtr[shift+radius+1:shift+width-radius-1]
        int start = radius + 1;
        int end = width - radius - 1;
        for (int w = start; w < end; ++w)
        {
            tmp += input[shift + w + radius];
            tmp -= input[shift + w - radius - 1];
            cachePtr[shift + w] = tmp;
        }

        // cachePtr[shift+width-radius-1:shift+width]
        start = width - radius;
        for (int w = start; w < width; ++w)
        {
            tmp -= input[shift + w - radius - 1];
        }
    }

    float *colSumPtr = &(colSum[0]);
    // Initial for col
    for (int indexW = 0; indexW < width; ++indexW)
    {
        colSumPtr[indexW] = 0;
    }

    // row [0:radius]
    for (int h = 0; h < radius; ++h)
    {
        int shift = h * width;
        for (int w = 0; w < width; ++w)
        {
            colSumPtr[w] += cachePtr[w + shift];
        }
    }

    for (int h = 0; h <= radius; ++h)
    {
        // address of current+radius col
        float *addPtr = cachePtr + (h + radius) * width;
        int shift = h * width;
        float *outPtr = output + shift;
        for (int w = 0; w < width; w++)
        {
            colSumPtr[w] += addPtr[w];
            outPtr[w] = colSumPtr[w];
        }
    }

    // row [radius+1:height-radius-1]
    int start = radius + 1;
    int end = height - radius - 1;
    for (int h = start; h < end; ++h)
    {
        float *addPtr = cachePtr + (h + radius) * width;
        float *subPtr = cachePtr + (h - radius - 1) * width;
        int shift = h * width;
        float *outPtr = output + shift;
        for (int w = 0; w < width; ++w)
        {
            colSumPtr[w] += addPtr[w];
            colSumPtr[w] -= subPtr[w];
            outPtr[w] = colSumPtr[w];
        }
    }

    // row [height-radius:height]
    start = height - radius;
    for (int h = start; h < height; ++h)
    {
        float *subPtr = cachePtr + (h - radius - 1) * width;
        int shift = h * width;
        float *outPtr = output + shift;
        for (int w = 0; w < width; ++w)
        {
            colSumPtr[w] -= subPtr[w];
            outPtr[w] = colSumPtr[w];
        }
    }
}