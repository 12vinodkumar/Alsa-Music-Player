#include "musicplayer_includes.h"



void musicplayer_playback();
void musicplayer_setplaybackstate();
unsigned int musicplayer_getplaybackstate();
void display();


static struct Alsa
{
    snd_pcm_t *pcm_handle;
    snd_pcm_hw_params_t *params;
    snd_pcm_uframes_t frames;
    int iPeriodtime;
}sAlsa;

pthread_mutex_t lock;

static long int liLoops;
int iToogle=0;
int iTimeSpend;
static unsigned int uiPlayback_state=0;
char *cpBuffer;
unsigned int uiBuffer_size;
static int iIndex=0;




eMusicPlayer_status musicplayer_createplaylist(int iInputFilesCount,char *cpInputFiles[],unsigned int auiErrorList[])
{
int ifile=1,i;
FILE *fpFileOpen;
int iFile_Size=50;
char acReadTxtFile[256];


if(iInputFilesCount < 2)
{
    return MUSICPLAYER_FILES_NOT_FOUND;
}
if (strcmp(cpInputFiles[1] , "-s")==0)
{
    sPlaylist_Var.iRandom=1;
    ifile++;
}

for (i=ifile;i<iInputFilesCount;i++)
{

    char *cpEnd = strrchr(cpInputFiles[i], '.');

	if(strcmp(cpEnd, ".wav") == 0)
    {
        sfiles[sPlaylist_Var.iFilesCount].cpFilename=(char *)malloc(sizeof (char)*strlen(cpInputFiles[i]));
        strcpy(sfiles[(sPlaylist_Var.iFilesCount)++].cpFilename,cpInputFiles[i]);

    }
    else if(strcmp(cpEnd, ".txt")==0)
    {
    	fpFileOpen=fopen(cpInputFiles[i], "r");
        if(NULL == fpFileOpen)
        {
            auiErrorList[1]++;
        }
        while (fgets(acReadTxtFile, iFile_Size, fpFileOpen) != NULL)
 		{

 			sfiles[sPlaylist_Var.iFilesCount].cpFilename=(char *)malloc(sizeof (char)*strlen(acReadTxtFile));
 			acReadTxtFile[strlen(acReadTxtFile)-1]='\0';
 			cpEnd = strrchr(acReadTxtFile, '.');
 			if(0 == strcmp(cpEnd, ".wav"))
 			{
                strcpy(sfiles[(sPlaylist_Var.iFilesCount)++].cpFilename,acReadTxtFile);
		    }
		    else
		    {
		        auiErrorList[0]++;
		    }
		}

        fclose(fpFileOpen);

	}
	else
	{
	    auiErrorList[0]++;
	}
}
if(sPlaylist_Var.iFilesCount > 10000)
{
    return MUSICPLAYER_INPUT_RANGE_EXCEEDED;
}

return MUSICPLAYER_SUCCESS;
}



/* RANDOMIZATION */
void swap (PlayList *a, PlayList *b)
{
    PlayList  temp = *a;
    *a = *b;
    *b = temp;
}

void musicplayer_random()
{
int i,j;
srand (time(NULL));

    for ( i = sPlaylist_Var.iFilesCount-1; i > 0; i--)
    {

         j = rand() % (i + 1);

        swap(&sfiles[i], &sfiles[j]);
    }

}




/*MUSIC_PLAYER INTIALIZATION */

void *musicplayer_init(void *iInit_status)
{
    int iPcm;
    if(sPlaylist_Var.iRandom)
    {
        musicplayer_random();
    }
    
    
    
    for(iIndex=0; iIndex<sPlaylist_Var.iFilesCount;iIndex++)
    {
    iFileDescriptor=open(sfiles[iIndex].cpFilename,O_RDONLY);
    musicplayer_wavinformation(iFileDescriptor);
    
    if (iPcm = snd_pcm_open(&sAlsa.pcm_handle, PCM_DEVICE,SND_PCM_STREAM_PLAYBACK, 0) < 0)
    {
         *((int*)iInit_status) = UNABLE_TO_OPEN_THE_PCM_DEVICE;
         pthread_exit(iInit_status);
    }
    
    snd_pcm_hw_params_alloca(&sAlsa.params);
    snd_pcm_hw_params_any(sAlsa.pcm_handle, sAlsa.params);


	if (iPcm = snd_pcm_hw_params_set_access(sAlsa.pcm_handle, sAlsa.params,SND_PCM_ACCESS_RW_INTERLEAVED) < 0)
	{

         *((int*)iInit_status) = MUSICPLAYER_HARDWARE_INTIALISATION_ERROR;
         pthread_exit(iInit_status);
    }


	if (iPcm = snd_pcm_hw_params_set_format(sAlsa.pcm_handle, sAlsa.params,SND_PCM_FORMAT_S16_LE) < 0)
	{
        *((int*)iInit_status) = MUSICPLAYER_HARDWARE_INTIALISATION_ERROR;
         pthread_exit(iInit_status);
    }


	if (iPcm = snd_pcm_hw_params_set_channels(sAlsa.pcm_handle, sAlsa.params, Wav_Information.usiChannels) < 0)
	{
        *((int*)iInit_status) = MUSICPLAYER_HARDWARE_INTIALISATION_ERROR;
         pthread_exit(iInit_status);
    }

	if (iPcm = snd_pcm_hw_params_set_rate_near(sAlsa.pcm_handle, sAlsa.params, &Wav_Information.uiSampleRate, 0) < 0)
	{
        *((int*)iInit_status) = MUSICPLAYER_HARDWARE_INTIALISATION_ERROR;
         pthread_exit(iInit_status);
    }


	if ( iPcm= snd_pcm_hw_params(sAlsa.pcm_handle, sAlsa.params) < 0)
	{
        *((int*)iInit_status) = MUSICPLAYER_HARDWARE_INTIALISATION_ERROR;
         pthread_exit(iInit_status);
    }
    snd_pcm_hw_params_get_period_size(sAlsa.params, &sAlsa.frames, 0);
    snd_pcm_hw_params_get_period_time(sAlsa.params, &sAlsa.iPeriodtime, NULL);
    
    musicplayer_playback();
	}
}


void musicplayer_playback()
{

int iPcm;


pthread_mutex_init(&lock, NULL);
uiBuffer_size = (sAlsa.frames * 2 * Wav_Information.usiChannels);
cpBuffer = (char *) malloc(uiBuffer_size);

    for (liLoops = (Wav_Information.uiDuration * 1000000) /sAlsa.iPeriodtime; liLoops > 0; liLoops--)
    {    
        pthread_mutex_lock(&lock);  
        read(iFileDescriptor, cpBuffer, uiBuffer_size);
        if (iPcm = snd_pcm_writei(sAlsa.pcm_handle, cpBuffer, sAlsa.frames) == -EPIPE)
		{
     	    
			snd_pcm_prepare(sAlsa.pcm_handle);
		}
		
        pthread_mutex_unlock(&lock);
	}

	  snd_pcm_drain(sAlsa.pcm_handle);
	  snd_pcm_close(sAlsa.pcm_handle);
	  free(cpBuffer);

}


/*getting current PlayBackState*/
unsigned int musicplayer_getplaybackstate()
{
    return uiPlayback_state;
}

/* set playback state */
void musicplayer_setplaybackstate(int state)
{
    uiPlayback_state=state;
    printf("playback state =%d\n",uiPlayback_state);
}


/*  NEXT PLAYBACK CONTROL */
eMusicPlayer_status musicplayer_next()
{


if (uiPlayback_state == STOP_STATE)
{
    return MUSICPLAYER_STOP_STATE;
}
else if (iIndex == sPlaylist_Var.iFilesCount-1)
{
    
    return MUSICPLAYER_NO_NEXT_FILES;

}
else
{
   
    pthread_mutex_destroy(&lock);
    liLoops=0;
    musicplayer_setplaybackstate(PLAY_STATE);
    return MUSICPLAYER_SUCCESS;
}
    

}

/*PREVIOUS CONTROL*/

eMusicPlayer_status musicplayer_prev()
{

if (uiPlayback_state == STOP_STATE)
{
    return MUSICPLAYER_STOP_STATE;
}
else if ( iIndex < 1 )
{
    return MUSICPLAYER_NO_PREV_FILES;

}
else
{

    pthread_mutex_destroy(&lock);
    liLoops=0;
    iIndex = iIndex-2;
    musicplayer_setplaybackstate(PLAY_STATE); 
    return MUSICPLAYER_SUCCESS;   
}

}



/*VOLUME PLAYBACK CONTROL*/
eMusicPlayer_status SetAlsaMasterVolume(long volume)
{

   

    long min, max;
    snd_mixer_t *handle;
    snd_mixer_selem_id_t *sid;
    const char *card = "default";
    const char *selem_name = "Master";

    snd_mixer_open(&handle, 0);
    snd_mixer_attach(handle, card);
    snd_mixer_selem_register(handle, NULL, NULL);
    snd_mixer_load(handle);

    snd_mixer_selem_id_alloca(&sid);
    snd_mixer_selem_id_set_index(sid, 0);
    snd_mixer_selem_id_set_name(sid, selem_name);
    snd_mixer_elem_t* elem = snd_mixer_find_selem(handle, sid);

    snd_mixer_selem_get_playback_volume_range(elem, &min, &max);
    snd_mixer_selem_set_playback_volume_all(elem, volume * max / 100);

    snd_mixer_close(handle);
    if(volume > 90)
    {
        return MUSICPLAYER_VOLUME_EXCEEDS_90;
    }
    else if ( volume < 10 )
    {
        return MUSICPLAYER_VOLUME_DECEEDS_0;
    }
    else
    {
    
        return MUSICPLAYER_SUCCESS;
    }
}

/* PAUSE_OR_PLAY  PLAYBACK CONTROL*/
eMusicPlayer_status musicplayer_pauseorplay()
{  
if(musicplayer_getplaybackstate() == STOP_STATE)
{  
	
    return MUSICPLAYER_STOP_STATE;
}
else
{
    if(iToogle)
    {
        snd_pcm_pause(sAlsa.pcm_handle,0);
        iToogle=0;
        musicplayer_setplaybackstate(PLAY_STATE);
        pthread_mutex_unlock(&lock);
            
    }
    else
    {
        snd_pcm_pause(sAlsa.pcm_handle,1);
	    iToogle=1;
	    musicplayer_setplaybackstate(PAUSE_STATE);
    }
    return MUSICPLAYER_SUCCESS;
}
}


/* SEEK PLAYBACK CONTROL*/
eMusicPlayer_status musicplayer_seek(int iSeconds)
{
unsigned int uiState=musicplayer_getplaybackstate();
off_t off_set=0; 
int iSeekTime;   
int iTimeSpend;
int iTotalTime;
        pthread_mutex_init(&lock, NULL); 
        iTimeSpend=(liLoops/1000000.0)*sAlsa.iPeriodtime;
        iSeekTime = Wav_Information.uiDuration -  iTimeSpend;
        iTotalTime = iSeekTime + iSeconds;
        if (uiState == STOP_STATE)
        {
            return MUSICPLAYER_STOP_STATE;
        }
        else
        {        
        if(iTotalTime > iTimeSpend )
        {
            return MUSICPLAYER_SEEK_EXCEEDS_DURATION;
        }
        
        
        else if((-iTotalTime) > iSeekTime )
        {
        return MUSICPLAYER_SEEK_DECEEDS_DURATION;
        }
        
        else if(uiState == PAUSE_STATE)
        {
            off_set=(iTotalTime*uiBuffer_size*(1000000/sAlsa.iPeriodtime));
            off_set-=off_set%2;
            printf("offset  =%ld\n",off_set);
            lseek(iFileDescriptor,off_set,SEEK_SET);
            pthread_mutex_lock(&lock);
            return MUSICPLAYER_SUCCESS;        
        }
        else
        {
            off_set=(iTotalTime*uiBuffer_size*(1000000/sAlsa.iPeriodtime));
            off_set-=off_set%2;
            printf("offset  =%ld\n",off_set);
            lseek(iFileDescriptor,off_set,SEEK_SET);
            return MUSICPLAYER_SUCCESS;
        }
    }
}    


/* STOP PLAYBACK CONTROL*/
eMusicPlayer_status musicplayer_stop()
{

    iIndex=0;
    snd_pcm_drain(sAlsa.pcm_handle);
	snd_pcm_close(sAlsa.pcm_handle);
    pthread_cancel(pThread_id);
    musicplayer_setplaybackstate(STOP_STATE);
    return MUSICPLAYER_STOP_STATE;

}
void display()
{

for(int i=0;i<sPlaylist_Var.iFilesCount;i++)
{
    printf("%s\n",sfiles[i].cpFilename);
}

}


