/*
 * @Author: xieydd
 * @since: 2020-06-29 22:45:02
 * @lastTime: 2020-07-09 09:43:03
 * @LastAuthor: Do not edit
 * @message: 
 */
#include <vector>

class BoxFilter
{
public:
    BoxFilter(){};
    ~BoxFilter()
    {
        free();
    };
    void filter(float *input, int radius, int height, int width, float *output);
    void filterFast(float *input, int radius, int height, int width, float *output);
    void filterFastV2(float *input, int radius, int height, int width, float *output);
    void filterFastV2NeonIntrinsics(float *input, int radius, int height, int width, float *output);
    void filterFastV2NeonAsm(float *input, int radius, int height, int width, float *output);
    void filterFastV2NeonAsmV2(float *input, int radius, int height, int width, float *output);
    void init(int height, int width, int radius)
    {
        free();
        cache.resize(height * width);
        colSum.resize(width);
    }

private:
    std::vector<float> cache;
    std::vector<float> colSum;
    void free()
    {
        std::vector<float>().swap(cache);
        std::vector<float>().swap(colSum);
    }
};