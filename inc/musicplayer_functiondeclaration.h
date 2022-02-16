#include "musicplayer_status.h"
eMusicPlayer_status musicplayer_createplaylist(int iFilescount,char *cpInputFiles[],unsigned int auiErrorList[]);
void *musicplayer_init();
//void musicplayer_next();

eMusicPlayer_status musicplayer_next();

eMusicPlayer_status musicplayer_prev();
eMusicPlayer_status SetAlsaMasterVolume(long volume);
eMusicPlayer_status musicplayer_pauseorplay();
eMusicPlayer_status musicplayer_seek(int iSeconds);
eMusicPlayer_status musicplayer_stop();

