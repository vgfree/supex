/*
*/
#include<stdio.h> //printf
#include<string.h>    //strlen
#include<sys/socket.h>    //socket
#include<arpa/inet.h> //inet_addr
int mfptp_pack_string(char *src, char *buf, int max_len);
int senddata(int fd, char *buf,int dlen,int more);
int main(int argc , char *argv[])
{
    int sock;
    struct sockaddr_in server;
    char message[1024*1024*8] , server_reply[4096];
     
    //Create socket
    sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1)
    {
        printf("Could not create socket");
    }
    puts("Socket created");
     
    //server.sin_addr.s_addr = inet_addr("192.168.1.15");
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons( 8753 );
 
    //Connect to remote server
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("connect failed. Error");
        return 1;
    }
     
    puts("Connected\n");
    int len = mfptp_pack_string("12345678",message,512);
    int ret;
     
    //keep communicating with server
    while(1)
    {
        printf("Enter message : ");
        //scanf("%s" , message);
        getchar();
        puts("send one time");
         
        //Send some data
        if( (ret = send(sock , message , len , 0)) <= 0)
        {
            puts("Send failed");
            return 1;
        }
        printf("发送出去的数据长度是%d\n",ret);
        int i =0;
        for(i=0;i<ret;i++){
            printf("%x ",message[i]);
        }
        printf("\n");
        printf("开始接收微博\n");
         
        //Receive a reply from the server
        while(1){
               if((ret=recv(sock , server_reply , 4096, 0)) <= 0){    
                       puts("recv failed");    
                       break;    
               }    
             
               printf("收到验证OK : %d\n",ret);    
               for(i=0;i<ret;i++){    
                       if(isgraph(server_reply[i])){    
                             printf("%c ",server_reply[i]);    
                       }else{    
                             printf("%x ",server_reply[i]);    
                       }    
                }    
                break;
               printf("\n",ret);    
        }


        printf("\n");
         
        printf("Server reply : %d\n",ret);
        printf("按任意键发送数据帧 \n");
        getchar();
        int len1 = mfptp_pack_frames_with_hdr(1024, message, 3);
        int len2 = mfptp_pack_frames_no_hdr(1024*1024, message+len1, 3);
        int len3 = mfptp_pack_frames_no_hdr(1024, message+len1+len2, 3);
        int len4 = mfptp_pack_frames_no_hdr(1024*4+1, message+len1+len2+len3, 0);
        if( (ret = send(sock, message , len1+len2+len3+len4, 0)) < 0)
        {

            puts("Send failed");
        }
 
        printf("振作%d\n",ret);
        sleep(10);
        len1 = mfptp_pack_frames_with_hdr(5, message, 2);
        len2 = mfptp_pack_frames_no_hdr(1023, message+len1, 0);
        if( (ret = send(sock, message , len1+len2, 0)) < 0)
        {

            puts("Send failed");
        }
        printf("振作%d\n",ret);

    }
    close(sock);
    return 0;
}
int mfptp_pack_string(char *src, char *buf, int max_len)
{
        int ret = -1;
        buf[0] ='#';
        buf[1] ='M';
        buf[2] ='F';
        buf[3] ='P';
        buf[4] ='T';
        buf[5] ='P';
        buf[6] =0x10; /* 版本*/
        buf[7] =0x00; /*压缩、加密*/
        buf[8] =0x04; /* REP */
        buf[9] =0x01; /* 包个数*/
        buf[10]=0x00; /* FP_CONTROL  */

        if(NULL != src){
                int len = strlen(src);
                if(len>255){
                        len = 255;
                }
                buf[11]=len;
                memcpy(&buf[12], src, len);
                ret = len+12;
        }
        return ret;
}
int mfptp_pack_frames_with_hdr(int len, char *buf, int more)
{
        int ret = -1;
        buf[0] ='#';
        buf[1] ='M';
        buf[2] ='F';
        buf[3] ='P';
        buf[4] ='T';
        buf[5] ='P';
        buf[6] =0x10; /* 版本*/
        buf[7] =0x00; /*压缩、加密*/
        buf[8] =0x04; /* REP */
        buf[9] = 0x1; /* 包个数*/
        if(0!=more){
            buf[10]=0x04; /* FP_CONTROL  */
        }else{
            buf[10]=0x00;
        }

        unsigned char *p=(unsigned char *)&len;
        if(len<=255){
            buf[11]=len;
            ret = len+12;
        }else if(len<=65535){
            buf[11]=*(p+1);
            buf[12]=*(p);
            buf[10]=buf[10] | 0x01; /* FP_CONTROL  */
            ret = len+13;
        }else{
            buf[11]=*(p+2);
            buf[12]=*(p+1);
            buf[13]=*(p);
            buf[10]=buf[10]|0x02; /* FP_CONTROL  */
            ret = len+14;
        }
        return ret;
}
int  senddata(int fd, char *buf,int dlen,int more)
{
        int ret; 
        int len = mfptp_pack_frames_with_hdr(dlen, buf, more);
        printf("sending data =%d\n",len);
        if( (ret = send(fd, buf , len, 0)) < 0)
        {

            puts("Send failed");
        }
        printf("振作%d\n",ret);
        return len;

}
int mfptp_pack_frames_no_hdr(int len, char *buf, int more)
{
        int ret = -1;
        if(0!=more){
            buf[0]=0x04; /* FP_CONTROL  */
        }else{
            buf[0]=0x00;
        }

        unsigned char *p=(unsigned char *)&len;
        if(len<=255){
            buf[1]=len;
            ret = len+2;
        }else if(len<=65535){
            buf[0]=buf[0] | 0x01; /* FP_CONTROL  */
            buf[1]=*(p+1);
            buf[2]=*(p);
            ret = len+3;
        }else{
            buf[0]=buf[0]|0x02; /* FP_CONTROL  */
            buf[1]=*(p+2);
            buf[2]=*(p+1);
            buf[3]=*(p);
            ret = len+4;
        }
        return ret;
}
