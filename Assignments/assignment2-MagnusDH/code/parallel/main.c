#include <stdio.h>
#include <stdlib.h>
#include "lib/bmp.h"
#include <mpi.h>
#include <math.h>
#include <string.h>
#include <stdbool.h>

#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) < (b) ? (b) : (a))
#define GP(X,Y) GetPixel(img, width, height,(X),(Y))

bool IMAGE_COMPLETE = false
/*
MPI tags:
0 = something wrong
1 = process wants work
2 = block/tile data from master to apply Sobel on
3 = block/tile data from master to apply Emboss on
4 = complete block/tile data from worker with Sobel filter
5 = complete block/tile data from worker with Emboss filter
6 = image is complete, terminate
*/

/**
 * @brief Convert an image to grayscale
 *
 * @param img
 * @param width
 * @param height
 */
void ImageToGrayscale(RGB *img, const int width, const int height){
    for(int i=0;i<width*height;i++){
        char grayscale=img[i].red*0.3+img[i].green*0.59+img[i].blue*0.11;
        img[i].red=grayscale;
        img[i].green=grayscale;
        img[i].blue=grayscale;
    }
}


/**
 * @brief Return a pixel no matter the coordinate
 *
 * @param img
 * @param width
 * @param height
 * @param x
 * @param y
 * @return RGB
 */
RGB GetPixel(RGB *img, const int width, const int height, const int x, const int y){
	if(x<0 || y<0 || x>=width || y >=height){
		int approxX=MIN(MAX(x,0),width);
		int approxY=MIN(MAX(y,0),height);
		return(img[approxX+approxY*width]);
	}
	return(img[x+y*width]);
}


void master_process()
{
    //Define variables
    const char *marguerite="../../images/marguerite.bmp";
    const char *outsobel="sobel.bmp";
    const char *outemboss="emboss.bmp";

    //Get image dimensions and store them in width, height
    int width, height;
    GetSize(marguerite, &width, &height);   

    //Allocate enough memory to store RGB structs for each pixel in the image 
    RGB *sobel=malloc(sizeof(RGB)*width*height);
    RGB *emboss=malloc(sizeof(RGB)*width*height);

    //Read the entire BMP image and store it in sobel and emboss
    LoadRegion(marguerite, 0, 0, width, height, sobel);
    LoadRegion(marguerite, 0, 0, width, height, emboss);

    //Convert image to grayscale
    ImageToGrayscale(img, width, height);

    //Save images
    CreateBMP(outsobel,width,height);
    WriteRegion(outsobel,0,0,width, height,sobel);
    
    CreateBMP(outemboss,width,height);
    WriteRegion(outemboss,0,0,width, height,emboss);

    //Free memory
    free(sobel);
    free(emboss);

    return(0);






   
    
	while(IMAGE_COMPLETE != true){
        //Wait until a message from any process is recieved arrived
		// long long recieved_data[2];
		MPI_Status recieved_message;
		MPI_Recv(
            // &recieved_data,	//buffer to store incoming data
			// 2,				//max number of items to receive
			// MPI_LONG_LONG,	//datatype of item to recieve
			MPI_ANY_SOURCE,		//ID/world_rank of sender
			MPI_ANY_TAG,		//message tag
			MPI_COMM_WORLD,		//Communicator
			&recieved_message   //pointer to MPI_Status to get info about message
		);
		
        //if tag==0 -> something has gone wrong
		if(recieved_message_status.MPI_TAG == 0){
            printf("Something has gone wrong\n");
        }

            
		//if tag==1 -> a process requests work
		if(recieved_message_status.MPI_TAG == 1){
            //Divide the image into tiles/blocks (add extra borders) 

            //send tile/block to worker using MPI_Send
            // MPI_Send(
            //     &send_data,			//Pointer to the data I want to send
            //     2,					//number of items in buf
            //     MPI_LONG_LONG,			//The datatype of what I am sending
            //     recieved_message_status.MPI_SOURCE,	//The ID/world_rank of the process I am sending to
            //     1,					//Tag
            //     MPI_COMM_WORLD    	//communicator (usually MPI_COMM_WORLD)
            // );
		}

		//If tag==3 -> recieved complete block/tile from process 
		if(recieved_message_status.MPI_TAG == 3){
            //Receive the filtered tiles back from workers using MPI_Recv.
            //Combines the tiles

            //If image is complete
                //Set indicator that image is complete
                //IMAGE_COMPLETE = true;
                //Writes image to disk
                //Send message to workers to stop
                // for(int i=1; i<world_size; i++){
                    // 	long long send_data[2];
                    // 	MPI_Send(
                        // 		&send_data,		//Pointer to the data I want to send
                        // 		2,				//number of items in buf
                        // 		MPI_LONG_LONG,	//The datatype of what I am sending
                        // 		i,				//The ID/world_rank of the process I am sending to
                        // 		3,				//Tag
                        // 		MPI_COMM_WORLD  //communicator (usually MPI_COMM_WORLD)
                        // 	);
                        // }
                //break out of loop and stop execution
        }
	}
	printf("MASTER: stopped working\n");
}


void worker_process(int world_rank, int world_size, int sequence_length)
{
    //WOKRER PROCESS:
    
    
	//As long as image is not complete
	while(IMAGE_COMPLETE != true){
        //Ask master for work
		// long long send_data[2];
		// MPI_Send(
            // 	&send_data,			//Pointer to the data I want to send
            // 	2,					//number of items in buf
            // 	MPI_LONG_LONG,			//The datatype of what I am sending
            // 	0,					//The ID/world_rank of the process I am sending to
            // 	2,					//Tag
            // 	MPI_COMM_WORLD    	//communicator (usually MPI_COMM_WORLD)
            // );
            
            //Recieve message from master
            // long long recieved_data[2];
            // MPI_Status recieved_message_status;
            // MPI_Recv(
            //     &recieved_data,			//buffer to store incoming data
            //     2,						//max number of items to receive
            //     MPI_LONG_LONG,				//datatype of item to recieve
            //     0,						//ID/world_rank of sender
            //     MPI_ANY_TAG,			//message tag
            //     MPI_COMM_WORLD,			//Communicator
            //     &recieved_message_status//pointer to MPI_Status to get info about message
            // );
            
            
            
        //if tag==2 -> master has sent work
        if(recieved_message_status.MPI_TAG == 2){
            //Recieve work

            //Apply filters
            // ApplySobel(sobel, width, height);
            // ApplyEmboss(emboss, width, height);

            //Send the inner non-overlapping filtered tile back to the master.
		}

		//Master says image is complete. Stop process
		if(recieved_message_status.MPI_TAG == 4){
			IMAGE_COMPLETE = true;
            break;
		}
    }

	printf("%d: stopped working\n", world_rank);
}


int main()
{
	//Initialize MessagePassingInterface
	MPI_Init(NULL, NULL);

	//Initialize MPI variables
	int world_rank, world_size;

	//Find the ID/world_rank for this process
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

	//Find the number of total processes being run
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);

	//Get hostname for process and node justt to keep things readable
	char hostname[MPI_MAX_PROCESSOR_NAME];
	int name_len;
	MPI_Get_processor_name(hostname, &name_len);
	// printf("I am process %d - running on node %s\n", world_rank, hostname);
	
	//Do different work if process is MASTER_PROCESS or worker_process
	if(world_rank == 0){
        //Start master process
		master_process();
	}
	else{
        //Start worker processes
		worker_process();
	}

	MPI_Finalize();
	return 0;
}
