#include<stdio.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>

int main()
{
   int fd,num;
   fd=open("/dev/globalvar",O_RDWR,S_IRWXU|S_IRWXG);
   if(fd!=-1)
   {
      read(fd,&num,sizeof(int));
      printf("The globalvar is %d!\n",num);
      printf("Please input the number written to globalvar!\n");
      scanf("%d",&num);
      write(fd,&num,sizeof(int));
      read(fd,&num,sizeof(int));
      printf("The globalvar is change to %d now!\n",num); 
      close(fd);
   } 
   else
     printf("Device open failure!\n");
   return 0; 
}

