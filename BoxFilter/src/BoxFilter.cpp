/*
 * @Author: xieydd
 * @since: 2020-06-29 22:44:59
 * @lastTime: 2020-07-09 22:37:50
 * @LastAuthor: Do not edit
 * @message: 
 */
#include "BoxFilter.h"
#include <algorithm>
#include <vector>

#if __ARM_NEON
#include <arm_neon.h>
#endif //__ARM_NEON

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

        // 0-半径列和+半径-直径列和 = cachePtr[shift:shift+radius+1]
        for (int w = 0; w <= radius; ++w)
        {
            tmp += input[w + shift + radius];
            cachePtr[shift + w] = tmp;
        }

        // cachePtr[shift+radius+1:shift+width-radius-1]
        int start = radius + 1;
        int end = width - radius - 1;
        for (int w = start; w <= end; ++w)
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
        colSumPtr[indexW] = 0.f;
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
    for (int h = start; h <= end; ++h)
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

void BoxFilter::filterFastV2NeonIntrinsics(float *input, int radius, int height, int width, float *output)
{
    // Sum Col(Horizonal)
    float *cachePtr = &(cache[0]);
    for (int h = 0; h < height; ++h)
    {
        int shift = h * width;
        float tmp;
        for (int w = 0; w < radius; ++w)
        {
            tmp += input[shift + w];
        }

        for (int w = 0; w <= radius; ++w)
        {
            tmp += input[shift + radius + w];
            cachePtr[w + shift] = tmp;
        }

        int start = radius + 1;
        int end = width - radius - 1;
        for (int w = start; w <= end; ++w)
        {
            tmp += input[shift + w + radius];
            tmp -= input[shift + w - radius - 1];
            cachePtr[w + shift] = tmp;
        }

        start = width - radius;
        for (int w = start; w < end; ++w)
        {
            tmp -= input[shift + w - radius - 1];
            cachePtr[w + shift] = tmp;
        }
    }

    float *colSumPtr = &(colSum[0]);
    for (int indexW = 0; indexW < width; indexW++)
    {
        colSumPtr[indexW] = 0.f;
    }

    // Sum Row(vertical)

    int n = width >> 2;
    int re = width - (n << 2);
    for (int h = 0; h < radius; ++h)
    {
        int shift = h * width;
        float *tmpColSumPtr = colSumPtr;
        float *tmpCachePtr = cachePtr + shift;
        int indexW = 0;

        int nn = n;
        int remain = re;
#if __ARM_NEON
        for (; nn > 0; --nn)
        {
            floar32x4_t _colSum = vld1q_f32(colSumPtr);
            float32x4_t _cache = vld1q_f32(tmpCachePtr);

            float32x4_t _tmp = vaddq_f32(_colSum, _cache);
            vst1q_f32(tmpColSumPtr, _tmp);
            tmpColSumPtr += 4;
            tmpCachePtr += 4;
        }
#endif //__ARM_NEON
        for (; remain > 0; --remain)
        {
            *tmpColSumPtr += *tmpCachePtr;
            tmpColSumPtr++;
            tmpCachePtr++;
        }

        for (int h = 0; h <= radius; ++h)
        {
            float *addPtr = cachePtr + (h + radius) * width;
            int shift = h * width;
            float *outPtr = output + shift;

            int nn = n;
            int remain = re;
#if __ARM_NEON
            for (; nn > 0; nn--)
            {
                float32x4_t _add = vld1q_f32(addPtr);
                float32x4_t _colSum = vld1q_f32(colSumPtr);
                float32x4_t _out = vaddq_f32(_add, _colSum);

                vst1q_f32(colSumPtr, _out);
                vst1q_f32(outPtr, _out);
                colSumPtr += 4;
                outPtr += 4;
                addPtr += 4;
            }
#endif //__ARM_NEON
            for (; remain > 0; --remain)
            {
                *colSumPtr += *addPtr;
                *outPtr = *colSumPtr;
                colSumPtr++;
                addPtr++;
                outPtr++;
            }
        }

        int start = radius + 1;
        int end = height - radius - 1;
        for (int h = start; h < end; ++h)
        {
            int nn = n;
            int remain = re;
            int shift = h * width;
            float *addPtr = cachePtr + (radius + h) * width;
            float *subPtr = cachePtr + (h - radius - 1) * width;
            float *outPtr = output + shift;
#if __ARM_NEON
            for (; nn > 0; --nn)
            {
                float32x4_t _add = vld1q_f32(addPtr);
                float32x4_t _sub = vld1q_f32(subPtr);
                float32x4_t _col_sum = vld1q_f32(colSumPtr);

                float32x4_t _out = vaddq_f32(_col_sum, _add);
                _out = vsubq_f32(_out, _sub);
                vst1q_f32(colSumPtr, _out);
                vst1q_f32(outPtr, _out);

                addPtr += 4;
                subPtr += 4;
                colSumPtr += 4;
                outPtr += 4;
            }
#endif //__ARM_NEON
            for (; remain > 0; --remain)
            {
                *colSumPtr += *addPtr;
                *colSumPtr -= *subPtr;
                *outPtr = *colSumPtr;
                addPtr += 1;
                subPtr += 1;
                outPtr += 1;
                colSumPtr += 1;
            }
        }

        start = height - radius;
        for (int h = start; h < end; ++h)
        {
            int shift = h * width;
            float *subPtr = colSumPtr + (h - radius - 1) * width;
            float *outPtr = output + shift;
            int nn = n;
            int remain = re;
#if __ARM_NEON
            for (; nn > 0; --nn)
            {
                float32x4_t _sub = vld1q_f32(subPtr);
                float32x4_t _colSum = vld1q_f32(colSumPtr);

                float32x4_t _out = vsubq_f32(_colSum, _sub);
                vst1q_f32(outPtr, _out);

                subPtr += 4;
                colSumPtr++ 4;
                outPtr++ 4;
            }
#endif //_ARM_NEON
            for (; remain > 0; --remain)
            {
                *colSumPtr -= *subPtr;
                *outPtr = *colSumPtr;
                subPtr += 1;
                outPtr += 1;
                colSumPtr += 1;
            }
        }
    }
}

void BoxFilter::filterFastV2NeonAsm(float *input, int radius, int height, int width, float *output)
{
    // Sum Col(Horizonal)
    float *cachePtr = &(cache[0]);
    for (int h = 0; h < height; ++h)
    {
        int shift = h * width;
        float tmp;
        for (int w = 0; w < radius; ++w)
        {
            tmp += input[shift + w];
        }

        for (int w = 0; w <= radius; ++w)
        {
            tmp += input[shift + radius + w];
            cachePtr[w + shift] = tmp;
        }

        int start = radius + 1;
        int end = width - radius - 1;
        for (int w = start; w <= end; ++w)
        {
            tmp += input[shift + w + radius];
            tmp -= input[shift + w - radius - 1];
            cachePtr[w + shift] = tmp;
        }

        start = width - radius;
        for (int w = start; w < end; ++w)
        {
            tmp -= input[shift + w - radius - 1];
            cachePtr[w + shift] = tmp;
        }
    }

    float *colSumPtr = &(colSum[0]);
    for (int indexW = 0; indexW < width; indexW++)
    {
        colSumPtr[indexW] = 0.f;
    }

    // Sum Row(vertical)

    int n = width >> 2;
    int re = width - (n << 2);

    for (int h = 0; h < radius; ++h)
    {
        int shift = height * width;
        float *tmpCachePtr = cachePtr + shift;
        int nn = n;
        int remain = re;
#if __NEON_ARM
        for (; nn > 0; --nn)
        {
            float32x4_t _cache = vld1q_f32(tmpCachePtr);
            float32x4_t _col = vld1q_f32(colSumPtr);
            float32x4_t _tmp = vaddq_f32(_cache, _col);
            vst1q_f32(colSumPtr, _tmp);
            tmpCachePtr += 4;
            colSUmPtr += 4;
        }
#endif //__NEON_ARM
        for (; remain > 0; --remain)
        {
            *colSumPtr += *tmpCachePtr;
            colSumPtr++;
            tmpCachePtr++;
        }
    }

    for (int h = 0; h <= radius; ++h)
    {
        int shift = width * h;
        float *addPtr = cachePtr + (h + radius) * width;
        float *outPtr = output + shift;
        int nn = n;
        int remain = re;
#if __NEON_NEON
        for (; nn > 0; --nn)
        {
            float32x4_t _add = vld1q_f32(addPtr);
            float32x4_t _colSum = vld1q_f32(colSumPtr);
            float32x4_t _tmp = vaddq_f32(_add, _colSum);
            vst1q_f32(colSumPtr, _tmp);
            vst1q_f32(outPtr, _tmp);
            outPtr += 4;
            colSumPtr += 4;
            addPtr += 4;
        }
#endif
        for (; remain > 0; --remain)
        {
            *colSumPtr += *addPtr;
            *outPtr = *colSumPtr;
            outPtr++;
            colSumPtr++;
            addPtr++;
        }
    }

    int start = radius + 1;
    int end = height - radius - 1;
    for (int h = start; h <= end; ++h)
    {
        int shift = h * width;
        int nn = n;
        int remain = re;
        float *addPtr = cachePtr + (h + radius) * width;
        float *subPtr = cachePtr + (h - radius - 1) * width;
        float *outPtr = outPtr + shift;
#if __ARM_NEON
        asm volatile(
            "0:                      \n"
            "vld1.s32 {d0-d1}, [%0]! \n"
            "vld1.s32 {d2-d3}, [%1]! \n"
            "vld1.s32 {d4-d5}, [%2]  \n"
            "vadd.f32 q4, q0, q2     \n"
            "vsub.f32 q3, q4, q1     \n"
            "vst1.s32 {d6,d7}, [%3]! \n"
            "vst1.s32 {d6,d7}, [%2]! \n"
            "subs %4, #1             \n"
            "bne 0b                  \n"
            : "=r"(addPtr),
              "=r"(subPtr),
              "=r"(colSumPtr),
              "=r"(outPtr),
              "=r"(nn)
            : "0"(addPtr),
              "1"(subPtr),
              "2"(colSumPtr),
              "3"(outPtr),
              "4"(nn)
            : "cc", "memory", "q0", "q1", "q2", "q3", "q4");
#endif
        for (; remain > 0; --remain)
        {
            *colSumPtr += *addPtr;
            *colSumPtr -= *subPtr;
            *outPtr = *colSumPtr;
            colSumPtr++;
            outPtr++;
            addPtr++;
            subPtr++;
        }
    }

    start = height - radius;
    for (int h = start; h < end; ++h)
    {
        int shift = h * width;
        float *subPtr = colSumPtr + (h - radius - 1) * width;
        float *outPtr = output + shift;
        int nn = n;
        int remain = re;
#if __ARM_NEON
        for (; nn > 0; --nn)
        {
            float32x4_t _sub = vld1q_f32(subPtr);
            float32x4_t _colSum = vld1q_f32(colSumPtr);

            float32x4_t _out = vsubq_f32(_colSum, _sub);
            vst1q_f32(outPtr, _out);

            subPtr += 4;
            colSumPtr++ 4;
            outPtr++ 4;
        }
#endif //_ARM_NEON
        for (; remain > 0; --remain)
        {
            *colSumPtr -= *subPtr;
            *outPtr = *colSumPtr;
            subPtr += 1;
            outPtr += 1;
            colSumPtr += 1;
        }
    }
}

void BoxFilter::filterFastV2NeonAsmV2(float *input, int radius, int height, int width, float *output)
{
    // Sum Col(Horizonal)
    float *cachePtr = &(cache[0]);
    for (int h = 0; h < height; ++h)
    {
        int shift = h * width;
        float tmp;
        for (int w = 0; w < radius; ++w)
        {
            tmp += input[shift + w];
        }

        for (int w = 0; w <= radius; ++w)
        {
            tmp += input[shift + radius + w];
            cachePtr[w + shift] = tmp;
        }

        int start = radius + 1;
        int end = width - radius - 1;
        for (int w = start; w <= end; ++w)
        {
            tmp += input[shift + w + radius];
            tmp -= input[shift + w - radius - 1];
            cachePtr[w + shift] = tmp;
        }

        start = width - radius;
        for (int w = start; w < end; ++w)
        {
            tmp -= input[shift + w - radius - 1];
            cachePtr[w + shift] = tmp;
        }
    }

    float *colSumPtr = &(colSum[0]);
    for (int indexW = 0; indexW < width; indexW++)
    {
        colSumPtr[indexW] = 0.f;
    }

    // Sum Row(vertical)

    int n = width >> 2;
    int re = width - (n << 2);

    for (int h = 0; h < radius; ++h)
    {
        int shift = height * width;
        float *tmpCachePtr = cachePtr + shift;
        int nn = n;
        int remain = re;
#if __NEON_ARM
        for (; nn > 0; --nn)
        {
            float32x4_t _cache = vld1q_f32(tmpCachePtr);
            float32x4_t _col = vld1q_f32(colSumPtr);
            float32x4_t _tmp = vaddq_f32(_cache, _col);
            vst1q_f32(colSumPtr, _tmp);
            tmpCachePtr += 4;
            colSUmPtr += 4;
        }
#endif //__NEON_ARM
        for (; remain > 0; --remain)
        {
            *colSumPtr += *tmpCachePtr;
            colSumPtr++;
            tmpCachePtr++;
        }
    }

    for (int h = 0; h <= radius; ++h)
    {
        int shift = width * h;
        float *addPtr = cachePtr + (h + radius) * width;
        float *outPtr = output + shift;
        int nn = n;
        int remain = re;
#if __NEON_NEON
        for (; nn > 0; --nn)
        {
            float32x4_t _add = vld1q_f32(addPtr);
            float32x4_t _colSum = vld1q_f32(colSumPtr);
            float32x4_t _tmp = vaddq_f32(_add, _colSum);
            vst1q_f32(colSumPtr, _tmp);
            vst1q_f32(outPtr, _tmp);
            outPtr += 4;
            colSumPtr += 4;
            addPtr += 4;
        }
#endif
        for (; remain > 0; --remain)
        {
            *colSumPtr += *addPtr;
            *outPtr = *colSumPtr;
            outPtr++;
            colSumPtr++;
            addPtr++;
        }
    }

    int start = radius + 1;
    int end = height - radius - 1;
    for (int h = start; h <= end; ++h)
    {
        int shift = h * width;
        int nn = width >> 3;
        int remain = width - (nn << 3);
        float *addPtr = cachePtr + (h + radius) * width;
        float *subPtr = cachePtr + (h - radius - 1) * width;
        float *outPtr = outPtr + shift;
#if __ARM_NEON
        asm volatile(
            "0:                      \n"
            "pld    [%0, #256]       \n"
            "vld1.s32 {d0-d3}, [%0]! \n"
            "pld    [%2, #256]       \n"
            "vld1.s32 {d8-d11}, [%2] \n"
            "vadd.f32  q6, q0, q4    \n"
            "pld    [%1, #256]       \n"
            "vld1.s32 {d4-d7}, [%1]  \n"
            "vadd.f32 q7, q1, q5     \n"
            "vsub.f32 q6, q6, q2     \n"
            "vsub.f32 q7, q7, q3     \n"
            "vst1.s32 {d12,d15}, [%3]! \n"
            "vst1.s32 {d12,d15}, [%2]! \n"
            "subs %4, #1             \n"
            "bne 0b                  \n"
            : "=r"(addPtr),
              "=r"(subPtr),
              "=r"(colSumPtr),
              "=r"(outPtr),
              "=r"(nn)
            : "0"(addPtr),
              "1"(subPtr),
              "2"(colSumPtr),
              "3"(outPtr),
              "4"(nn)
            : "cc", "memory", "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "q8");
#endif
        for (; remain > 0; --remain)
        {
            *colSumPtr += *addPtr;
            *colSumPtr -= *subPtr;
            *outPtr = *colSumPtr;
            colSumPtr++;
            outPtr++;
            addPtr++;
            subPtr++;
        }
    }

    start = height - radius;
    for (int h = start; h < end; ++h)
    {
        int shift = h * width;
        float *subPtr = colSumPtr + (h - radius - 1) * width;
        float *outPtr = output + shift;
        int nn = n;
        int remain = re;
#if __ARM_NEON
        for (; nn > 0; --nn)
        {
            float32x4_t _sub = vld1q_f32(subPtr);
            float32x4_t _colSum = vld1q_f32(colSumPtr);

            float32x4_t _out = vsubq_f32(_colSum, _sub);
            vst1q_f32(outPtr, _out);

            subPtr += 4;
            colSumPtr++ 4;
            outPtr++ 4;
        }
#endif //_ARM_NEON
        for (; remain > 0; --remain)
        {
            *colSumPtr -= *subPtr;
            *outPtr = *colSumPtr;
            subPtr += 1;
            outPtr += 1;
            colSumPtr += 1;
        }
    }
}