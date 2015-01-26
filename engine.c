/*************************************************************************
	> File Name: engine.c
	> Author: 
	> Mail: 
	> Created Time: 2015年01月24日 星期六 10时55分45秒
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <malloc.h>
#include <sys/types.h>
#include <time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <signal.h>
#include <unistd.h>
#include "video_capture.h"
#include "config.h"
#include "mjpeg_rtp.h"
#include "config.h"
struct camera *cam = NULL;
pthread_t cam_thread;
pthread_t socket_thread;

volatile int stop_all = 0;/*1:stop,0:run*/

static struct camera *cam_init(void);
static void create_cam_pthread(struct camera *cam);

void stop_engine(void)
{
    stop_all = 1;
    usleep(1000*100);
   free(cam->device_name);
    v4l2_exit(cam);
    //exit_socket(xxx);
}

void start_engine(void)
{
    //init_socket(xxx);
    cam = cam_init();
    v4l2_init(cam);
    create_cam_pthread(cam);
}

static struct camera * cam_init(void)
{
    struct camera *pcam = (struct camera *)malloc(sizeof(struct camera));
    pcam->device_name   = strdup(CAM_DEVICE);
    pcam->buffers       = NULL;
    pcam->width         = CAM_WIDGH;
    pcam->height        = CAM_HEIGHT;
    pcam->display_depth = 5;
    return pcam;
}

void capture_encode_thread(void)
{
    int ret = -1;
    int imagesize = 0;
    unsigned char *bigbuffer = (unsigned char *)malloc(cam->width*cam->height*3*sizeof(unsigned char ));
    while(!stop_all){
        ret = read_frame(cam,bigbuffer,&imagesize);
        if(!ret){
            /*选择要压缩的格式类型*/
#ifdef CAM_MJPEG
            printf("imagesize = %d\n",imagesize);
            jpeg_rtp(bigbuffer,cam->width,cam->height,imagesize);/*直接从摄像头得到的数据就是jpeg数据*/
#else
            jpeg_encode_yuyv422_rtp(bigbuffer,cam->width,cam->height);
#endif
            dbug("jpeg_encode");
        }
    }
    dbug("free");
    free(bigbuffer);
}

void create_cam_pthread(struct camera *pcam)
{
    if(0!=pthread_create(&cam_thread,NULL,(void*)capture_encode_thread,NULL))
        fprintf(stderr,"thread create fail\n");
    pthread_join(cam_thread,NULL);
    dbug("pthread_join");
}
