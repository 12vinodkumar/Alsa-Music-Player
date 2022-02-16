/******************************************************************************
 *
 *
 *   ALLGO EMBEDDED SYSTEMS CONFIDENTIAL PROPRIETARY
 *
 *    (C) 2013 ALLGO EMBEDDED SYSTEMS PVT. LTD.
 *
 *   FILENAME        - alsa_audio_player.h
 *
 *   COMPILER        - gcc 4.4.4
 *
 ******************************************************************************
 *
 *   CHANGE HISTORY
 *   mm/dd/yy          DESCRIPTION                        Author
 *   --------          -----------                        ------
 *   29/08/2017        Initial Version                    Dipankar Saha
 *   28/09/2017        Rework                             Kartik Inani
 *******************************************************************************
 *
 *   DESCRIPTION
 *   AAP ALSA core player header
 *
 ******************************************************************************/

#ifndef _ALSA_AUDIO_PLAYER_H_
#define _ALSA_AUDIO_PLAYER_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "aap_plat_media_player_types.h"
#include "aap_plat_aplayer_interface.h"
#include <alsa/asoundlib.h>

#if defined __cplusplus
extern "C" {
#endif

typedef void (*AAPAlsaCoreCbFunc)(AAPPlayer_Events eEvtId,
        unsigned int uiDataLen,
        void* pvData,
        void* pvCbParam);

typedef struct
{
    /* pcm Device Handle provided by snd_pcm_open */
    snd_pcm_t *pcmHandleOut;
    /* Format of the audio data. */
    snd_pcm_format_t format;
    /* Contains the configuration of a particular channel */
    AAPAudioConfig *psAudioConfig;
    /* Stores callback function pointer */
    AAPAlsaCoreCbFunc pfEventFunc;
    /* User parameter */
    void *pvUserParam;
    /* set to true once player initialization is done */
    AAP_BOOL isConfigured;
}AlsaConfig;

int audio_player_init(AAP_PLAYER_HANDLE* pulAlsaPlayer,
        AAPAlsaCoreCbFunc pfAppCb,
        AAPAudioConfig *psAudioConfig,
        void* pvUserParam);
int audio_player_play(AAP_PLAYER_HANDLE ulAlsaPlayer);
int audio_player_pause(AAP_PLAYER_HANDLE ulAlsaPlayer);
int audio_player_push_buffer(AAP_PLAYER_HANDLE ulAlsaPlayer,
        unsigned char* pucData,
        unsigned int uiSize,
        uint64_t ulTimeStamp);
int audio_player_stop(AAP_PLAYER_HANDLE ulAlsaPlayer);
int audio_player_deinit(AAP_PLAYER_HANDLE ulAlsaPlayer);

#if defined __cplusplus
}
#endif

#endif /* ifndef _ALSA_AUDIO_PLAYER_H_ */
