/******************************************************************************
 *
 *
 *   ALLGO EMBEDDED SYSTEMS CONFIDENTIAL PROPRIETARY
 *
 *    (C) 2013 ALLGO EMBEDDED SYSTEMS PVT. LTD.
 *
 *   FILENAME        - aap_plat_aplayer_interface.cpp
 *
 *   COMPILER        - gcc 4.4.4
 *
 *******************************************************************************
 *   CHANGE HISTORY
 *   ---------------------------------------------------------------------------
 *   DATE           REVISION      AUTHOR            COMMENTS
 *   ---------------------------------------------------------------------------
 *   18/03/2015     1.0         Shiva Kumar         Initial Version
 *   19/03/2015     1.1         Dipankar Saha       Integrated in AAP
 *                                                  audio player.
 *   7/04/2015      2.0         Vaisakh N           Clean up
 *   22/04/2015     2.1         Dipankar Saha       Rework.
 *   01/12/2016     3.0         Dipankar Saha       Separated out GST functionality
 *                                                  from platform interface.
 *   28/09/2017     3.1         Kartik Inani        Generalized for ALSA and GST
 *
 *******************************************************************************
 *
 *   DESCRIPTION
 *   This file contains all audio player related API implimentation.
 *   Player Type can be selected based on environment variable PLAYER_TYPE.
 *   Set it to GST for building Gstreamer audio player. By default ALSA
 *   audio player will be built.
 *
 *   ==For PC Build==
 *   ALSA:: Set Latency(Buffer_time) through DEFAULT_LATENCY_MEDIA_MS and
     DEFAULT_LATENCY_GUIDANCE_MS macros, refer file alsa_audio_player.cpp for values.
 *   GST:: Nothing to be done.
 *
 *   Note: No device_id needed in attributes xml file. No need to add asound.conf
     for PC since it doesn't override the default behaviour if no device_id is
     provided.
 *
 *   ==For SabreAuto==
 *   ALSA:: Provide latency through pcm plugin "alsa_playback" in asound.conf and
 *   use the same as "device_id" for Media and Guidance channels.
 *   GST:: Uses "default" pcm plugin, no "device_id" needed for Media and Guidance.
 *   where "period_time"=5000 and "buffer_time"=15000.
 *
 ******************************************************************************/
#include <string.h>
#include "aap_plat_aplayer_interface.h"
#include "aap_plat_media_player_types.h"
#ifdef GST
#include "gst_audio_player.h"
#else
#include "alsa_audio_player.h"
#endif /* ifdef GST */
#include "aap_error_codes.h"

#define API_TASK 1

/*! \brief The player handle
 */
typedef struct
{
    /*! \brief core player handle*/
    AAP_PLAYER_HANDLE ulCorePlayer;
    /*! \brief Event function */
    AAPPlayerCbFunc pfEventFunc;
    /*! \brief caller context var */
    void* pvCbParam;
    /*! Audio Stream Type i.e, MEDIA/ GUIDANCE/ SYSTEM */
    AAP_StreamType eStreamType;
}AAP_AudioPlayer;

AAP_RetType aap_plat_aplayer_init(AAP_HANDLE* pulPlayerHandle,
        AAPAudioConfig *psAudioConfig,
        AAPPlayerCbFunc pfAppCb, void* pvUserParam)
{
    AAP_RetType iRet = 0;
    AAP_AudioPlayer *psPlayer = NULL;
    AAP_UINT32 uiState = API_TASK;

    switch (uiState)
    {
        case API_TASK:
            {
                if (NULL == psAudioConfig)
                {
                    printf("ERR::AP::Audio config structure is NULL\n");
                    iRet = AAP_FAILURE;
                    break;
                }
                psPlayer = static_cast<AAP_AudioPlayer*>
                    (malloc(sizeof(AAP_AudioPlayer)));
                if (NULL == psPlayer)
                {
                    printf("ERR::AP::Memory allocation failed\n");
                    iRet = AAP_ERR_OUT_OF_MEM;
                    break;
                }
                memset(psPlayer, 0, sizeof(AAP_AudioPlayer));
                psPlayer->pfEventFunc = pfAppCb;
                psPlayer->pvCbParam = pvUserParam;
                psPlayer->eStreamType = psAudioConfig->eStreamType ;
                iRet = audio_player_init(&(psPlayer->ulCorePlayer),
                        pfAppCb,
                        psAudioConfig,
                        pvUserParam);
                if (0 != iRet)
                {
                    printf("ERR::AP::Core player init failed\n");
                    goto ErrorExit;
                }
                printf("AP::Player init success!\n");
                *pulPlayerHandle = reinterpret_cast<AAP_HANDLE>(psPlayer);

ErrorExit :
                if (0 != iRet)
                {
                    printf("AP::Cleaning player \n");
                    if (0 != audio_player_deinit((AAP_PLAYER_HANDLE)
                            psPlayer->ulCorePlayer))
                    {
                        printf("ERR::AP::Player Deinit failed\n");
                    }
                    free(psPlayer);
                }
            }
    }
    return iRet;
}

/* Pushes buffer to player */
AAP_RetType aap_plat_aplayer_process_data(AAP_HANDLE ulPlayerHandle,
        unsigned char *pucData, unsigned int uiSize, uint64_t ulTimeStamp)
{
    AAP_AudioPlayer* psPlayer = NULL;
    AAP_RetType iRet = 0;
    AAP_UINT32 uiState = API_TASK;
    switch (uiState)
    {
        case API_TASK:
            {
                if (!ulPlayerHandle)
                {
                    printf("ERR::AP::Passed a NULL handle \n");
                    iRet = -1;
                    break;
                }
                psPlayer = reinterpret_cast<AAP_AudioPlayer*>(ulPlayerHandle);
                if (NULL == psPlayer)
                {
                    printf("ERR::AP::Invalid args:Player is NULL\n");
                    iRet = 1;
                    break;
                }
                if (NULL == pucData  || 0 == uiSize)
                {
                    /* Invalid data*/
                    printf("ERR::AP::Invalid input pucData: %p uiSize %u\n", pucData, uiSize);
                    iRet = 1;
                    break;
                }
                if (0 != audio_player_push_buffer(
                            (AAP_PLAYER_HANDLE)psPlayer->ulCorePlayer,
                            pucData, uiSize, ulTimeStamp))
                {
                    iRet = E_AAP_ERROR_PLAYER_PUSH_BUFFER;
                    break;
                }
            }
    }
    return iRet;
}

AAP_RetType aap_plat_aplayer_play(AAP_HANDLE ulPlayerHandle)
{
    AAP_RetType iRet = 0;
    AAP_AudioPlayer *psPlayer = NULL;
    if (!ulPlayerHandle)
    {
        printf("ERR::AP::Passed a NULL handle\n");
        iRet = 1;
    }
    else
    {
        psPlayer = reinterpret_cast<AAP_AudioPlayer*>(ulPlayerHandle);
        iRet = audio_player_play(psPlayer->ulCorePlayer);
        if (0 != iRet)
        {
            printf("ERR::AP::Failed to play audio\n");
        }
    }
    return iRet;
}

AAP_RetType aap_plat_aplayer_pause(AAP_HANDLE ulPlayerHandle)
{
    AAP_RetType iRet = 0;
    AAP_AudioPlayer *psPlayer = NULL;
    if (!ulPlayerHandle)
    {
        printf("ERR::AP::Passed a NULL handle\n");
        iRet = 1;
    }
    else
    {
        psPlayer = reinterpret_cast<AAP_AudioPlayer*>(ulPlayerHandle);
        iRet = audio_player_pause(psPlayer->ulCorePlayer);
        if (0 != iRet)
        {
            printf("ERR::AP::Failed to pause audio\n");
        }
    }
    return iRet;
}

/* stop the player */
AAP_RetType aap_plat_aplayer_stop(AAP_HANDLE ulPlayerHandle)
{
    AAP_RetType iRet = 0;
    AAP_AudioPlayer* psPlayer = NULL;
    AAP_UINT32 uiState = API_TASK;
    switch (uiState)
    {
        case API_TASK:
            {
                if (!ulPlayerHandle)
                {
                    printf("ERR::AP::Passed a NULL audio player handle \n");
                    iRet = 1;
                    break;
                }
                psPlayer = reinterpret_cast<AAP_AudioPlayer*>(ulPlayerHandle);

                if (NULL == psPlayer)
                {
                    printf("ERR::AP::Invalid args psPlayer is NULL \n");
                    iRet = 1;
                    break;
                }
                printf("AP::Going to stop audio player.\n");
                iRet = audio_player_stop(psPlayer->ulCorePlayer);
                if (0 != iRet)
                {
                    printf("ERR::AP::Failed to stop audio player \n");
                }
            }
    }
    return iRet;
}

AAP_RetType aap_plat_aplayer_deinit(AAP_HANDLE *pulPlayerHandle)
{
    AAP_RetType iRet = 0;
    AAP_AudioPlayer* psPlayer = NULL;
    if (NULL == pulPlayerHandle)
    {
        printf("ERR::AP::Passed a NULL handle \n");
        iRet = -1;
    }
    else
    {
        psPlayer = reinterpret_cast<AAP_AudioPlayer*>(*pulPlayerHandle);
        if (psPlayer && psPlayer->ulCorePlayer)
        {
            iRet = audio_player_deinit(psPlayer->ulCorePlayer);
            if (0 != iRet)
            {
                printf("ERR::AP::Failed uninit core player\n");
            }
        }
        if (NULL != psPlayer)
        {
            free(psPlayer);
        }
        *pulPlayerHandle = 0;
    }
    return iRet;
}

