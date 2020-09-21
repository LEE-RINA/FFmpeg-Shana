/*
 * Copyright (c) 2018 Gianluigi Tiesi
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef AVCODEC_LIBFDK_AAC_INTERNAL_H
#define AVCODEC_LIBFDK_AAC_INTERNAL_H

#ifdef _WIN32
#include <windows.h>
#define LIBNAME "libfdk-aac-2.dll"
#define dlopen(fname, f) ((void *) LoadLibraryA(fname))
#define dlclose(handle) FreeLibrary((HMODULE) handle)
#define dlsym(handle, name) GetProcAddress((HMODULE) handle, name)
#else
#include <dlfcn.h>
#define LIBNAME "libfdk-aac.so.2"
#endif

#define DLSYM(x) \
    do \
    { \
        s->pfn.x = ( imp_##x ) dlsym(s->hLib, AV_STRINGIFY(x)); \
        if (!s->pfn.x ) \
        { \
            av_log(avctx, AV_LOG_ERROR, "Unable to find symbol " AV_STRINGIFY(x) " in dynamic " LIBNAME "\n"); \
            return -1; \
        } \
    } while (0)


#include <fdk-aac/aacenc_lib.h>
#include <fdk-aac/aacdecoder_lib.h>

typedef AACENC_ERROR (*imp_aacEncOpen)(HANDLE_AACENCODER *phAacEncoder, const UINT encModules, const UINT maxChannels);
typedef AACENC_ERROR (*imp_aacEncClose)(HANDLE_AACENCODER *phAacEncoder);
typedef AACENC_ERROR (*imp_aacEncEncode)(const HANDLE_AACENCODER hAacEncoder, const AACENC_BufDesc *inBufDesc, const AACENC_BufDesc *outBufDesc, const AACENC_InArgs *inargs, AACENC_OutArgs *outargs);
typedef AACENC_ERROR (*imp_aacEncInfo)(const HANDLE_AACENCODER hAacEncoder, AACENC_InfoStruct *pInfo);
typedef AACENC_ERROR (*imp_aacEncoder_SetParam)(const HANDLE_AACENCODER hAacEncoder, const AACENC_PARAM param, const UINT value);

typedef struct _aacEncLib {
    imp_aacEncOpen aacEncOpen;
    imp_aacEncClose aacEncClose;
    imp_aacEncEncode aacEncEncode;
    imp_aacEncInfo aacEncInfo;
    imp_aacEncoder_SetParam aacEncoder_SetParam;
} aacEncLib;

typedef LINKSPEC_H HANDLE_AACDECODER (*imp_aacDecoder_Open)(TRANSPORT_TYPE transportFmt, UINT nrOfLayers);
typedef LINKSPEC_H void (*imp_aacDecoder_Close)(HANDLE_AACDECODER self);
typedef LINKSPEC_H AAC_DECODER_ERROR (*imp_aacDecoder_Fill)(HANDLE_AACDECODER self, UCHAR *pBuffer[], const UINT bufferSize[], UINT *bytesValid);
typedef LINKSPEC_H AAC_DECODER_ERROR (*imp_aacDecoder_DecodeFrame)(HANDLE_AACDECODER self, INT_PCM *pTimeData, const INT timeDataSize, const UINT flags);
typedef LINKSPEC_H CStreamInfo* (*imp_aacDecoder_GetStreamInfo)(HANDLE_AACDECODER self);
typedef LINKSPEC_H AAC_DECODER_ERROR (*imp_aacDecoder_ConfigRaw)(HANDLE_AACDECODER self, UCHAR *conf[], const UINT length[]);
typedef LINKSPEC_H AAC_DECODER_ERROR (*imp_aacDecoder_SetParam)(const HANDLE_AACDECODER self, const AACDEC_PARAM param, const INT value);
typedef LINKSPEC_H AAC_DECODER_ERROR (*imp_aacDecoder_AncDataInit)(HANDLE_AACDECODER self, UCHAR *buffer, int size);

typedef struct _aacDecLib {
    imp_aacDecoder_Open aacDecoder_Open;
    imp_aacDecoder_Close aacDecoder_Close;
    imp_aacDecoder_Fill aacDecoder_Fill;
    imp_aacDecoder_DecodeFrame aacDecoder_DecodeFrame;
    imp_aacDecoder_ConfigRaw aacDecoder_ConfigRaw;
    imp_aacDecoder_GetStreamInfo aacDecoder_GetStreamInfo;
    imp_aacDecoder_SetParam aacDecoder_SetParam;
    imp_aacDecoder_AncDataInit aacDecoder_AncDataInit;
} aacDecLib;

#endif /* AVCODEC_LIBFDK_AAC_INTERNAL_H */
