/*
 * @Author: xieydd
 * @since: 2020-06-29 22:44:59
 * @lastTime: 2020-06-30 10:08:01
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
            int num = (end_h - start_h) * (end_w = start_w);
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