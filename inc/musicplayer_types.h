#ifndef _MUSICPLAYER_TYPES_H_
#define _MUSICPLAYER_TYPES_H_

#include <alsa/asoundlib.h>
typedef struct  
{
    char *cpFilename;
}PlayList;
PlayList sfiles[1000];
struct wav
{
    unsigned int uiFilesize;
    char cpFileType[4];
    unsigned int uiFormat;
    unsigned short int usiChannels;
    unsigned int uiSampleRate;
    unsigned int uiBitrate;
    unsigned int uiDuration;    
    
}Wav_Information;

struct temp
{
int iRandom;
unsigned int auiErrorList[2]; 
 int iFilesCount;
 
}sPlaylist_Var; 
int iFileDescriptor;
#endif
