/*
 * @Author: xieydd
 * @since: 2020-06-29 23:46:55
 * @lastTime: 2020-07-06 22:11:17
 * @LastAuthor: Do not edit
 * @message: 
 */
#include "gtest/gtest.h"
#include <BoxFilter.h>
#include <random>
#include <vector>
#include <chrono>

static void print(float *input, int height, int width)
{
    for (int h = 0; h < height; ++h)
    {
        int height_sift = h * width;
        std::cout << std::endl;
        for (int w = 0; w < width; ++w)
        {
            std::cout << input[height_sift + w] << " ";
        }
    }
    std::cout << std::endl;
};

static int loop = 10;
static int height = 2000;
static int width = 2000;
static int radius = 3;
static int printMat = 0;

TEST(netTest, fliter)
{
    std::vector<float> input;
    std::vector<float> output;

    int size = height * width;
    input.resize(size);
    output.resize(size);

    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_real_distribution<> dis(-2.0, 2.0);
    for (int i = 0; i < size; i++)
    {
        input[i] = dis(gen);
    }

    BoxFilter boxf;
    boxf.init(height, width, radius);

    // Count time
    float avgTime = 0.f;
    double tmp;
    for (int i = 0; i < loop; i++)
    {
        auto startClock = std::chrono::system_clock::now();
        boxf.filter(&input[0], radius, height, width, &output[0]);
        auto endClock = std::chrono::system_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endClock - startClock);
        tmp = double(duration.count()) * std::chrono::microseconds::period::num / 1000;
        avgTime += tmp;
        std::cout << "          [" << i << "]"
                  << " BoxFilfer Cost time: " << tmp << "ms" << std::endl;
    }

    std::cout << "\n          BoxFilfer Average Cost time: " << avgTime / loop << "ms" << std::endl;

    if (printMat == 1)
    {
        std::cout << "result: " << std::endl;
        print(&output[0], height, width);
    }
}

TEST(netTest, fliterFast)
{
    std::vector<float> input;
    std::vector<float> output;

    int size = height * width;
    input.resize(size);
    output.resize(size);

    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_real_distribution<> dis(-2.0, 2.0);
    for (int i = 0; i < size; i++)
    {
        input[i] = dis(gen);
        output[i] = 0.f;
    }

    BoxFilter boxf;
    boxf.init(height, width, radius);

    // Count time
    float avgTime = 0.f;
    double tmp;
    for (int i = 0; i < loop; i++)
    {
        auto startClock = std::chrono::system_clock::now();
        boxf.filterFast(&input[0], radius, height, width, &output[0]);
        auto endClock = std::chrono::system_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endClock - startClock);
        tmp = double(duration.count()) * std::chrono::microseconds::period::num / 1000;
        avgTime += tmp;
        std::cout << "          [" << i << "]"
                  << " Fast BoxFilfer Cost time: " << tmp << "ms" << std::endl;
    }

    std::cout << "\n          Fast BoxFilfer Average Cost time: " << avgTime / loop << "ms" << std::endl;

    if (printMat == 1)
    {
        std::cout << "result: " << std::endl;
        print(&output[0], height, width);
    }
}

TEST(netTest, filterFastV2)
{
    std::vector<float> input;
    std::vector<float> output;
    int size = height * width;
    input.resize(size);
    output.resize(size);

    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_real_distribution<> dis(-2.0, 2.0);
    for (int i = 0; i < size; i++)
    {
        input[i] = dis(gen);
        output[i] = 0.f;
    }

    BoxFilter boxf;
    boxf.init(height, width, radius);

    // Count time
    float avgTime = 0.f;
    double tmp;
    for (int i = 0; i < loop; i++)
    {
        auto startClock = std::chrono::system_clock::now();
        boxf.filterFastV2(&input[0], radius, height, width, &output[0]);
        auto endClock = std::chrono::system_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endClock - startClock);
        tmp = double(duration.count()) * std::chrono::microseconds::period::num / 1000;
        avgTime += tmp;
        std::cout << "          [" << i << "]"
                  << " Fast V2 BoxFilfer Cost time: " << tmp << "ms" << std::endl;
    }

    std::cout << "\n          Fast V2 BoxFilfer Average Cost time: " << avgTime / loop << "ms" << std::endl;

    if (printMat == 1)
    {
        std::cout << "result: " << std::endl;
        print(&output[0], height, width);
    }
}