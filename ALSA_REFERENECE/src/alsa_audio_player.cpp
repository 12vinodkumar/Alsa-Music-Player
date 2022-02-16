
/******************************************************************************
 *
 *
 *   ALLGO EMBEDDED SYSTEMS CONFIDENTIAL PROPRIETARY
 *
 *    (C) 2017 ALLGO EMBEDDED SYSTEMS PVT. LTD.
 *
 *   FILENAME        - alsa_audio_player.cpp
 *
 *   COMPILER        - gcc 4.4.4
 *
 ******************************************************************************
 *
 *   CHANGE HISTORY
 *   mm/dd/yy          DESCRIPTION                        Author
 *   --------          -----------                        ------
 *   29/08/2017        Initial Version                    Dipankar Saha
 *   28/09/2017        Cleanup                            Kartik Inani
 *******************************************************************************
 *
 *   DESCRIPTION
 *   ALSA core audio player implementaion.
 *
 ******************************************************************************/

#include <sys/time.h>

#include "alsa_audio_player.h"
#include "aap_error_codes.h"

#define API_TASK 1
/* Buffer_time values for media and guidance channels for a PC Build.
 * In case of SabreAuto, it will be overridden by /etc/asound.conf */
#define DEFAULT_LATENCY_MEDIA_MS 85
#define DEFAULT_LATENCY_GUIDANCE_MS 100


int audio_player_init(AAP_PLAYER_HANDLE* pulAlsaPlayer,
        AAPAlsaCoreCbFunc pfAppCb,
        AAPAudioConfig *psAudioConfig,
        void* pvUserParam)
{
    int uiState = API_TASK;
    int iLatency;
    int iRet = 0;
    snd_output_t* out;
    AlsaConfig *psAlsaConfig = NULL;

    switch(uiState)
    {
        case API_TASK:
            {
                snd_pcm_uframes_t bufferSize, periodSize;
                snd_pcm_stream_t direction =  SND_PCM_STREAM_PLAYBACK;
                AAP_CHAR acAdDevice[AAP_SMALL_ARRAY_LEN + 1] = {'\0'};

                if (NULL == psAudioConfig)
                {
                    printf("ERR::AP::Invalid input parameter psAudioConfig:%p\n", psAudioConfig);
                    iRet = AAP_ERR_INVALID_PARAMS;
                    break;
                }
                psAlsaConfig = static_cast<AlsaConfig *>(malloc(sizeof(AlsaConfig)));
                if (NULL == psAlsaConfig)
                {
                    printf("ERR::AP::Memory allocation failed!\n");
                    iRet = AAP_ERR_OUT_OF_MEM;
                    break;
                }
                memset(psAlsaConfig, 0x0, sizeof(AlsaConfig));
                psAlsaConfig->pcmHandleOut = NULL;
                psAlsaConfig->psAudioConfig = psAudioConfig;
                psAlsaConfig->pfEventFunc = pfAppCb;
                psAlsaConfig->pvUserParam = pvUserParam;

                if (0 == strcmp(psAlsaConfig->psAudioConfig->acAudioDeviceID, ""))
                {
                    strcpy(acAdDevice, "default");
                }
                else
                {
                    strncpy(acAdDevice, psAlsaConfig->psAudioConfig->acAudioDeviceID,
                            AAP_SMALL_ARRAY_LEN);
                }

                iRet = snd_pcm_open (&psAlsaConfig->pcmHandleOut, acAdDevice, direction, 0);

                if (0 != iRet)
                {
                    printf("ERR::AP::Couldn't open ALSA\n");
                    break;
                }
                if (AAP_AUDIO_STREAM_MEDIA == psAlsaConfig->psAudioConfig->eStreamType)
                {
                    iLatency = DEFAULT_LATENCY_MEDIA_MS;
                }
                else
                {
                    iLatency = DEFAULT_LATENCY_GUIDANCE_MS;
                }

                iRet = snd_pcm_set_params (psAlsaConfig->pcmHandleOut,
                        SND_PCM_FORMAT_S16_LE, SND_PCM_ACCESS_RW_INTERLEAVED,
                        psAlsaConfig->psAudioConfig->uiChannels,
                        psAlsaConfig->psAudioConfig->eAudioFreq,
                        TRUE, iLatency * 1000);

                if (0 != iRet)
                {
                    printf("ERR::AP::snd_pcm_set_params failed\n");
                    break;
                }

                if (snd_pcm_nonblock(psAlsaConfig->pcmHandleOut, 0))
                {
                    printf("ERR::AP::Failed to make it block\n");
                }
                else
                {
                    printf("AP::Successfully set it to block\n");
                }
                iRet = snd_pcm_get_params (psAlsaConfig->pcmHandleOut, &bufferSize, &periodSize);
                if (0 != iRet)
                {
                    printf("ERR::AP::snd_pcm_get_params failed\n");
                    break;
                }

                printf("AP::Buffer size=%lu, period size=%lu\n", bufferSize, periodSize);

                iRet = snd_pcm_prepare (psAlsaConfig->pcmHandleOut);
                if (0 != iRet)
                {
                    printf("ERR::AP::snd_pcm_prepare failed\n");
                    break;
                }
                if (bufferSize <= 0)
                {
                    bufferSize = 4096;
                }
                psAlsaConfig->format = SND_PCM_FORMAT_S16_LE;
                psAlsaConfig->isConfigured  = TRUE;

                *pulAlsaPlayer = reinterpret_cast<AAP_PLAYER_HANDLE>(psAlsaConfig);
                printf("AP::Player initialized successfully\n");

                /* Prints the software configurations on initialization */
                snd_output_stdio_attach(&out, stdout, 0);
                snd_pcm_dump_sw_setup(psAlsaConfig->pcmHandleOut, out);
            }
    }
    if (0 != iRet)
    {
        printf("ERR::AP::Player Init failed!\n");
        if (psAlsaConfig)
        {
            free(psAlsaConfig);
        }
    }

    return iRet;
}

int audio_player_play(AAP_PLAYER_HANDLE ulAlsaPlayer)
{
    int uiState = API_TASK;
    int iRet = 0;

    switch(uiState)
    {
        case API_TASK:
            {
                if (!ulAlsaPlayer)
                {
                    printf("ERR::AP::Passed a NULL Handle\n");
                    iRet = AAP_ERR_INVALID_PARAMS;
                    break;
                }
                AlsaConfig *psAlsaConfig = reinterpret_cast<AlsaConfig *>(ulAlsaPlayer);
                /* When first call to push buffer happens it will prepare the pcm
                 * which takes some time resulting in underrun, instead doing it
                 * on the call to play itself.
                 * Initial underrun was not observed after this. */
                snd_pcm_prepare(psAlsaConfig->pcmHandleOut);
            }
    }
    return iRet;
}

int audio_player_pause(AAP_PLAYER_HANDLE ulAlsaPlayer)
{
    int uiState = API_TASK;
    int iRet = 0;

    switch(uiState)
    {
        case API_TASK:
            {

                if (!ulAlsaPlayer)
                {
                    printf("ERR::AP::Passed a NULL Handle\n");
                    iRet = AAP_ERR_INVALID_PARAMS;
                    break;
                }
            }
    }
    return iRet;
}

static int audio_stream_recover(snd_pcm_t *pcmHandle, int iInError)
{
    int iErrRet;

    if (-EINTR == iInError)
    {
        iErrRet = 0;
    }
    else if (-EPIPE == iInError)
    { /* Underrun */
        iErrRet = snd_pcm_prepare (pcmHandle);
    }
    else if (-ESTRPIPE == iInError)
    { /* Hardware suspended */
        for (int i = 0; i < 100; ++i)
        {
            iErrRet = snd_pcm_resume (pcmHandle);
            if (-EAGAIN != iErrRet)
            {
                break;
            }
            usleep (10000);
        }
        if (iErrRet)
        {
            iErrRet = snd_pcm_prepare (pcmHandle);
        }
    }
    else
    {
        iErrRet = iInError;
    }

    return iErrRet;
}


int audio_player_push_buffer(AAP_PLAYER_HANDLE ulAlsaPlayer,
        unsigned char* pucData,
        unsigned int uiSize,
        uint64_t ulTimeStamp)
{
    (void)ulTimeStamp;
    int uiState = API_TASK;
    int iRet = 0;

    switch (uiState)
    {
        case API_TASK:
            {
                int iErr;
                ssize_t n;
                int uiFrames = 0;

                if (!ulAlsaPlayer)
                {
                    printf("ERR::AP::Passed a NULL Handle\n");
                    iRet = AAP_ERR_INVALID_PARAMS;
                    break;
                }
                AlsaConfig *psAlsaConfig = reinterpret_cast<AlsaConfig *>(ulAlsaPlayer);
                size_t const bytesPerUnit = 2 * psAlsaConfig->psAudioConfig->uiChannels;
                snd_pcm_t *const pcmHandle = psAlsaConfig->pcmHandleOut;
                uiFrames = uiSize / bytesPerUnit;

                while (uiFrames > 0)
                {
                    n = snd_pcm_writei(pcmHandle, (void*)pucData, uiFrames);
                    if (n <= 0)
                    {
                        iErr = audio_stream_recover(psAlsaConfig->pcmHandleOut, n);
                        if (iErr != 0)
                        {
                            printf("ERR::AP::Audio stream recover: failed %d\n", iErr);
                            if (psAlsaConfig->pfEventFunc)
                            {
                                AAPPlayer_Events ePlayerEvent = E_AAP_PLAYER_FACED_ERROR;
                                psAlsaConfig->pfEventFunc(ePlayerEvent,
                                        0,
                                        NULL,
                                        psAlsaConfig->pvUserParam);
                            }
                            break;
                        }
                    }
                    else
                    {
                        pucData += (n * bytesPerUnit);
                        uiFrames -= n;
                    }
                }
            }
    }
    return iRet;
}

int audio_player_stop(AAP_PLAYER_HANDLE ulAlsaPlayer)
{
    int uiState = API_TASK;
    int iRet = 0;

    switch(uiState)
    {
        case API_TASK:
            {
                if (!ulAlsaPlayer)
                {
                    printf("ERR::AP::Passed a NULL Handle\n");
                    iRet = AAP_ERR_INVALID_PARAMS;
                    break;
                }
            }
    }
    return iRet;
}


int audio_player_deinit(AAP_PLAYER_HANDLE ulAlsaPlayer)
{
    int uiState = API_TASK;
    int iRet = 0;

    switch(uiState)
    {
        case API_TASK:
            {

                if (!ulAlsaPlayer)
                {
                    printf("ERR::AP::Passed a NULL Handle\n");
                    iRet = AAP_ERR_INVALID_PARAMS;
                    break;
                }
                AlsaConfig *psAlsaConfig = reinterpret_cast<AlsaConfig *>(ulAlsaPlayer);
                if (psAlsaConfig->pcmHandleOut)
                {
                    snd_pcm_close(psAlsaConfig->pcmHandleOut);
                    psAlsaConfig->pcmHandleOut = NULL;
                }
                free(psAlsaConfig);
            }
    }
    return iRet;
}
