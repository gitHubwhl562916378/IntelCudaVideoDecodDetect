/*
 * @Author: your name
 * @Date: 2020-08-03 19:00:06
 * @LastEditTime: 2020-08-03 21:25:17
 * @LastEditors: Please set LastEditors
 * @Description: In User Settings Edit
 * @FilePath: \git\NvidiaDecoder\nvidia_decoer_api.h
 */
#pragma once

#ifdef linux
#  define DECODER_EXPORT __attribute__((visibility("default")))
#else
#  define DECODER_EXPORT __declspec(dllimport)
#endif
#include <string>
#include <functional>

class DECODER_EXPORT DecoderApi
{
public:
    virtual ~DecoderApi(){};
    virtual int fps() = 0;
    virtual void stop() = 0;
    virtual void* context() = 0;
    virtual void decode(const std::string &source, const bool useDeviceFrame, const std::function<void(void *ptr, const int format, const int width, const int height, const std::string &error)> call_back) = 0;
};

typedef DecoderApi* (*CreateDecoderFunc)(void);
