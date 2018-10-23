/*
*   note:
*       SDL2 show YUV data, so we should change our video_frame to YUV data.
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <SDL2/SDL.h>

#ifdef __cplusplus
}
#endif

typedef enum status_e {
    STATUS_FALSE,
    STATUS_TRUE
}status_e;

typedef enum pthread_status_e {
    PTHREAD_STATUS_STOP,
    PTHREAD_STATUS_START,
    PTHREAD_STATUS_EXIT
}pthread_status_e;

typedef enum SDL2_EVENT_e {
    SDL2_EVENT_REFREASH = SDL_USEREVENT+1,
    SDL2_EVENT_EXITFREASH = SDL2_EVENT_REFREASH + 1
}SDL2_EVENT_e;

typedef struct videoPlayerManager_s{
    pthread_status_e mPthreadStatus;
}videoPlayerManager_s;

static videoPlayerManager_s gManager = {0};

static void show_YUVData(const AVFrame * pYUVdata)
{
    printf("data->[0]:%x, [1]:%x, [2]:%x...", pYUVdata->data[0], pYUVdata->data[1], pYUVdata->data[2]);
    printf("line->[0]:%x, [1]:%x, [2]:%x...\n", pYUVdata->linesize[0], pYUVdata->linesize[1], pYUVdata->linesize[2]);
}

static void SDL2_EventTask(void * param)
{
    SDL_Event eventMsg;
    printf("[%s][%d]! start...\n", __func__, __LINE__);
    while(gManager.mPthreadStatus != PTHREAD_STATUS_EXIT)
    {
        if(gManager.mPthreadStatus == PTHREAD_STATUS_START)
        {
            eventMsg.type = SDL2_EVENT_REFREASH;
            SDL_PushEvent(&eventMsg);
            SDL_Delay(40);
        }
    }

    eventMsg.type = SDL2_EVENT_EXITFREASH;
    SDL_PushEvent(&eventMsg);

    return;
}

static void SDL2_ReSource_deinit(void)
{
    SDL_Quit();

    return;
}

static status_e SDL2_ReSource_init(const SDL_Rect * pSDLRect, SDL_Window ** pSDLWindow, SDL_Renderer ** pSDLRender, SDL_Texture ** pSDLTexture, SDL_Thread ** pSDLThread)
{
    if(!pSDLRect || pSDLWindow == NULL || pSDLRender == NULL || pSDLTexture == NULL)
    {
        printf("[%s][%d]! pAVCodeCt param error\n", __func__, __LINE__);
        return STATUS_FALSE;
    }

    SDL_Window * pSDLWindowt = NULL;
    SDL_Renderer * pSDLRendert = NULL;
    SDL_Texture * pSDLTexturet = NULL;
    SDL_Thread  * pSDLThreadt = NULL;
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER))
    {
        printf("[%s][%d]! SDL_Init failed\n", __func__, __LINE__);
        return STATUS_FALSE;
    }


    pSDLWindowt = SDL_CreateWindow("this is sample video player", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                    pSDLRect->w, pSDLRect->h, SDL_WINDOW_OPENGL);
    if(!pSDLWindowt)
    {
        printf("[%s][%d]! SDL_CreateWindow failed\n", __func__, __LINE__);
        return STATUS_FALSE;
    }

    pSDLRendert = SDL_CreateRenderer(pSDLWindowt, -1, 0);
    if(!pSDLRendert)
    {
        printf("[%s][%d]! SDL_CreateRenderer failed\n", __func__, __LINE__);
        return STATUS_FALSE;
    }

    pSDLTexturet = SDL_CreateTexture(pSDLRendert, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING, pSDLRect->w, pSDLRect->h);
    if(!pSDLTexturet)
    {
        printf("[%s][%d]! SDL_CreateRenderer failed\n", __func__, __LINE__);
        return STATUS_FALSE;
    }

    gManager.mPthreadStatus = PTHREAD_STATUS_START;
    pSDLThreadt = SDL_CreateThread(SDL2_EventTask, NULL, NULL);

    *pSDLWindow = pSDLWindowt;
    *pSDLRender = pSDLRendert;
    *pSDLTexture = pSDLTexturet;
    *pSDLThread = pSDLThreadt;

    return STATUS_TRUE;
}

status_e player_main(int argc, char * argv[])
{
    status_e retValue = STATUS_FALSE;
    int i32Ret = 0, got_picture = 0;
    unsigned int index = 0, videoFirstIndex = -1;
    AVFormatContext * pAVFormatCt = NULL;
    AVCodecContext * pAVCodeCt = NULL;
    AVCodec * pAVCodec = NULL;
    AVFrame * pAVframe = NULL;
    AVFrame * pAVframeYUV = NULL;
    AVPacket * pAVPacket = NULL;
    char *out_buffer = NULL;

    struct SwsContext * pSwsCt = NULL;

    SDL_Rect       SDLRect = {0};
    SDL_Window *   pSDLWindow = NULL;
    SDL_Renderer * pSDLRender = NULL;
    SDL_Texture *  pSDLTexture = NULL;
    SDL_Thread  *  pEventTask = NULL;
    SDL_Event mEvent = {0};

    char * uri = "../../../av/test.avi";

    //1. init ffmpeg, register & net init
    av_register_all();//register all encodc,decodc,demux,mux,parse
    avformat_network_init();

    //2. malloc AVFormatContext resource
    pAVFormatCt = avformat_alloc_context();
    if(pAVFormatCt == NULL)
    {
        printf("[%s][%d]! malloc av format context faild\n", __func__, __LINE__);
        return STATUS_FALSE;
    }

    //3. open input media file
    if(avformat_open_input(&pAVFormatCt, uri, NULL, NULL) != 0)
    {
        printf("[%s][%d]! open input %s failed\n", __func__, __LINE__, uri);
        goto exit_4;
    }

    //4. find audio, video stream info
    if(avformat_find_stream_info(pAVFormatCt, NULL) != 0)
    {
        printf("[%s][%d]! find streaminfo failed\n", __func__, __LINE__);
        goto exit_4;
    }

    av_dump_format(pAVFormatCt, 0, uri, 0);

    //5. find video or audio stream info
    printf("[%s][%d]! find %d streams\n", __func__, __LINE__, pAVFormatCt->nb_streams);
    for(index; index < pAVFormatCt->nb_streams; index++)
    {
        if(pAVFormatCt->streams[index]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            videoFirstIndex = index;
            printf("[%s][%d]! first video stream %d\n", __func__, __LINE__, videoFirstIndex);
            break;
        }
    }

    if(videoFirstIndex == -1)
    {
        printf("[%s][%d]! find video indedx failed\n", __func__, __LINE__);
        goto exit_4;
    }

    pAVCodeCt = pAVFormatCt->streams[videoFirstIndex]->codec;

    //6.find decodec by codecID
    pAVCodec = avcodec_find_decoder(pAVCodeCt->codec_id);
    if(!pAVCodec)
    {
        printf("[%s][%d]! find video avcodec failed\n", __func__, __LINE__);
        goto exit_4;
    }

    //7. init & open decodec
    if(avcodec_open2(pAVCodeCt, pAVCodec, NULL) != 0)
    {
        printf("[%s][%d]! open codec failed\n", __func__, __LINE__);
        goto exit_4;
    }

    //8. malloc packet buffer, when av_read_frame need
    pAVPacket = av_packet_alloc();
    if(!pAVPacket)
    {
        printf("[%s][%d]! open codec failed\n", __func__, __LINE__);
        goto exit_4;
    }
    //9. init av packet
    av_init_packet(pAVPacket);

    //10. malloc original data buffer
    if((pAVframe = av_frame_alloc()) == NULL)
    {
        printf("[%s][%d]! open codec failed\n", __func__, __LINE__);
        goto exit_3;
    }

    //11. malloc YUV original data buffer
    if((pAVframeYUV = av_frame_alloc()) == NULL)
    {
        printf("[%s][%d]! open codec failed\n", __func__, __LINE__);
        av_frame_free(pAVframe);
        goto exit_3;
    }

    out_buffer =(unsigned char *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV420P,  pAVCodeCt->width, pAVCodeCt->height,1));
    av_image_fill_arrays(pAVframeYUV->data, pAVframeYUV->linesize,out_buffer,
            AV_PIX_FMT_YUV420P,pAVCodeCt->width, pAVCodeCt->height,1);

    //12. create swop context
    pSwsCt = sws_getContext(pAVCodeCt->width, pAVCodeCt->height, pAVCodeCt->pix_fmt,
                                pAVCodeCt->width, pAVCodeCt->height, AV_PIX_FMT_YUV420P,
                                SWS_BICUBIC, NULL, NULL, NULL);
    if(!pSwsCt)
    {
        printf("[%s][%d]! sws_getContext failed\n", __func__, __LINE__);
        goto exit_2;
    }

    SDLRect.x = 0;
    SDLRect.y = 0;
    SDLRect.w = pAVCodeCt->width;
    SDLRect.h = pAVCodeCt->height;

    if(SDL2_ReSource_init(&SDLRect, &pSDLWindow, &pSDLRender, &pSDLTexture, &pEventTask) == STATUS_FALSE)
    {
        goto exit_1;
    }

    while(1)
    {
        SDL_WaitEvent(&mEvent);
        //printf("[%s][%d]! mEvent.type:%x\n", __func__, __LINE__, mEvent.type);
        switch(mEvent.type)
        {
            case SDL2_EVENT_REFREASH:
                while(1)
                {
                    if(av_read_frame(pAVFormatCt, pAVPacket) < 0)
                    {
                        gManager.mPthreadStatus = PTHREAD_STATUS_EXIT;
                        break;
                    }

                    if(pAVPacket->stream_index == videoFirstIndex)
                    {
                        break;
                    }
                }

                if(gManager.mPthreadStatus != PTHREAD_STATUS_EXIT)
                {
                    i32Ret = avcodec_decode_video2(pAVCodeCt, pAVframe, &got_picture, pAVPacket);
                    if(i32Ret < 0)
                    {
                        gManager.mPthreadStatus = PTHREAD_STATUS_EXIT;
                        break;
                    }
                    if(got_picture > 0)
                    {
                        sws_scale(pSwsCt, pAVframe->data, pAVframe->linesize, 0, pAVframe->height, pAVframeYUV->data, pAVframeYUV->linesize);
                        SDL_UpdateYUVTexture(pSDLTexture, &SDLRect,
                                    pAVframeYUV->data[0], pAVframeYUV->linesize[0],
                                    pAVframeYUV->data[1], pAVframeYUV->linesize[1],
                                    pAVframeYUV->data[2], pAVframeYUV->linesize[2]);
                        SDL_RenderClear(pSDLRender);
                        SDL_RenderCopy(pSDLRender, pSDLTexture, NULL, &SDLRect);
                        SDL_RenderPresent(pSDLRender);
                    }
                    av_free_packet(pAVPacket);
                }

                break;

            case SDL2_EVENT_EXITFREASH:

                break;

            case SDL_KEYDOWN:
                switch(mEvent.key.keysym.sym)
                {
                    case SDLK_SPACE:
                       if(gManager.mPthreadStatus == PTHREAD_STATUS_STOP)
                        {
                            gManager.mPthreadStatus = PTHREAD_STATUS_START;
                        }
                        else
                        {
                            gManager.mPthreadStatus = PTHREAD_STATUS_STOP;
                        }

                        break;

                    case SDLK_F1:
                        gManager.mPthreadStatus = PTHREAD_STATUS_STOP;
                        break;

                    case SDLK_F2:
                        gManager.mPthreadStatus = PTHREAD_STATUS_START;
                        break;
                }

                break;

            case SDL_QUIT:
                gManager.mPthreadStatus = PTHREAD_STATUS_EXIT;
                break;
        }

        if(gManager.mPthreadStatus == PTHREAD_STATUS_EXIT)
        {
            printf("end play video\n");
            break;
        }
    }
    retValue = STATUS_TRUE;

exit_1:
    SDL2_ReSource_deinit();
    sws_freeContext(pSwsCt);

exit_2:
    av_frame_free(&pAVframe);
    av_frame_free(&pAVframeYUV);
    av_free(out_buffer);
exit_3:
    av_packet_free(&pAVPacket);
    avcodec_close(pAVCodeCt);

exit_4:
    avformat_close_input(&pAVFormatCt);

    return retValue;
}
