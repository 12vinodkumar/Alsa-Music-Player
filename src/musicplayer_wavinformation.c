#include<stdio.h>
#include "musicplayer_types.h"
#include "musicplayer_status.h"

void display_info()
{
    printf("format=%d\n",Wav_Information.uiFormat);
    printf("samplingRate=%d\n",Wav_Information.uiSampleRate);
    printf("Bitrate =%d\n",Wav_Information.uiBitrate);
    printf("FileSize=%d\n",Wav_Information.uiFilesize);
    printf("Channels=%d\n",Wav_Information.usiChannels);
    printf("FileType=%s\n",Wav_Information.cpFileType);
    printf("Duration = %d\n",Wav_Information.uiDuration);
}


void musicplayer_wavinformation(int iFile_descriptor)
{
char *cpHeader;
int i,k;
	cpHeader=(char *)malloc(44);
	
	if (read(iFile_descriptor, cpHeader,44) == 44);
	{
  	    printf("Header\n");
  	}    
  	
  	Wav_Information.uiFormat=(*(int *)(cpHeader+16));
  	Wav_Information.uiFilesize=(*(int *)(cpHeader+4));
  	Wav_Information.uiSampleRate=(*(int *)(cpHeader+24));
	Wav_Information.uiBitrate=(*(int *)(cpHeader+28));
	Wav_Information.usiChannels=(*(short int *)(cpHeader+22));
    for(i=0,k=8;i<4,k<12;i++,k++)
    {
        Wav_Information.cpFileType[i]=(*(char *)(cpHeader+k));
    }
    Wav_Information.cpFileType[i++]='\0';
    Wav_Information.uiDuration = (Wav_Information.uiFilesize/Wav_Information.uiBitrate);
    
    display_info();
  /*(  if(strcmp(Wav_Information.cpFileType,"WAVE")==0)
    {
        return MUSICPLAYER_SUCCESS;
    }
    else
    {
        printf("FAILURE\n");
        return MUSICPLAYER_FAILURE;
    }
    */
}

