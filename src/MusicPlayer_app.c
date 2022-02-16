#include<stdio.h>
#include <pthread.h>
#include <termios.h>
#include "musicplayer_status.h"
#include "musicplayer_types.h"
#include "musicplayer_functiondeclaration.h"
int main(int argc,char *argv[])
{
int istatus;
int *istatus1;
unsigned int auiErrorList[2]={0,0};
int iInit_status;
void *vpResult;
int iSeektime;

 
char getch() {
        char buf = 0;
        struct termios old = {0};
        if (tcgetattr(0, &old) < 0)
                perror("tcsetattr()");
        old.c_lflag &= ~ICANON;
        old.c_lflag &= ~ECHO;
        old.c_cc[VMIN] = 1;
        old.c_cc[VTIME] = 0;
        if (tcsetattr(0, TCSANOW, &old) < 0)
                perror("tcsetattr ICANON");
        if (read(0, &buf, 1) < 0)
                perror ("read()");
        old.c_lflag |= ICANON;
        old.c_lflag |= ECHO;
        if (tcsetattr(0, TCSADRAIN, &old) < 0)
                perror ("tcsetattr ~ICANON");
        return (buf);
}
void *playback()
{
int iPlayback_status;

 while(1)
    {
        static long slVolume=50;
        char ch;
        ch=getch();
        
	switch(ch)
        { 
            case 'P':
            case 'p':printf("p\n");
                     iPlayback_status = musicplayer_pauseorplay();
                     if (iPlayback_status == 0)
                     {
                        printf("SUCESS\n");
                     }
                     else if(iPlayback_status == 7)
                     {
                        pthread_create(&pThread_id,NULL,musicplayer_init,&iInit_status);
                        printf("You are in STOP state\n");
                     }
                     break;
            case '>':printf("NEXT\n");
                     iPlayback_status = musicplayer_next();
                     if (iPlayback_status == 7)
                     {
                        printf("STOP STATE\n");
                     }
                     else if(iPlayback_status == 12)
                     {
                        printf("No Next file\n");
                     }
                     else if(iPlayback_status == 0)
                     {
                        printf("SUCCESS\n");
                     }
                     else if(iPlayback_status == 7)
                     {
                        printf("STOP state \n");
                     }
                     
                     break;
            case '<':printf("<\n");
                    iPlayback_status=musicplayer_prev();  
                    if (iPlayback_status == 7)
                     {
                        printf("STOP STATE\n");
                     }
                     else if(iPlayback_status == 13)
                     {
                        printf("NO PREVIOUS FILE\n");
                     }
                     else if(iPlayback_status == 0)
                     {
                     printf("SUCCESS\n");
                     }          
            break;
            case '+':printf("+\n");
                     slVolume+=10;
                     iPlayback_status = SetAlsaMasterVolume(slVolume);
                     if(iPlayback_status == 10)
                     {
                        printf("Volume Excedded 90\n");
                     }
                     
                     else
                     {
                        printf("Success\n");
                     }
               
                     break;
            case '-':printf("-\n");
                     slVolume-=10;
                     iPlayback_status = SetAlsaMasterVolume(slVolume);
                     if(iPlayback_status == 11)
                     {
                        printf("Volume Deceeded to 0 ");
                     }
                     else
                     {
                        printf("Success\n");
                     }
                     break;
            case 'j':
                    printf("Enter the seek time : 'Seek+<time in seconds>'\n");
                    scanf("%d",&iSeektime);
                    iPlayback_status = musicplayer_seek(iSeektime);
                    if (iPlayback_status == 7)
                     {
                        printf("STOP STATE\n");
                     }
                     else if(iPlayback_status == 8)
                     {
                        printf("SEEK EXCEEDS THE DURATION \n");
                     }
                     else if(iPlayback_status == 9)
                     {
                     printf("SEEK DECEEDS DURATION\n");
                     }
                     else if(iPlayback_status == 0)
                     {
                     printf("SUCCESS\n");
                     } 
                    break;
             case 'S':iPlayback_status = musicplayer_stop();
                     if(iPlayback_status == 7)
                     {
                        printf("You are in Stop state\n");
                     }
                     break;       
                            
                              
        }

    }
}






istatus = musicplayer_createplaylist(argc, argv, auiErrorList);
if (istatus == 2)
{
    printf("File Not found\n");
    exit(0);
}
if (istatus == 1)
{
    printf(" Input range exceeded \n");
}
if (auiErrorList[0] > 0)
{
    printf("Number of Inavlid Files :%d\n",auiErrorList[0]);
}
if (auiErrorList[0] > 1)
{
    printf("Number of corrupted files :%d\n",auiErrorList[1]);
}

pthread_create(&pThread_id,NULL,musicplayer_init,&iInit_status);
pthread_create(&pThread_id1,NULL,playback,NULL);
if (vpResult != 0)
{
    if(iInit_status == 6)
    {   
        printf("UNABLE TO OPEN THE FILE\n");
        exit(0);    
    }
}
pthread_join(pThread_id1,NULL);
printf("Ending the playback thread\n");
pthread_join(pThread_id,NULL);

return 0;
}
