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

//Contains info about a single pixel
// typedef struct {
//     int row;
//     int col;
//     unsigned char red;
//     unsigned char green;
//     unsigned char blue;
// } PixelInfo;


//Contains info about a whole tile
typedef struct {
    // int tile_number;
    int width;                      //How many pixels in width the tile is
    int height;                     //How many rows the tile is
    int num_pixels;                 //How many pixels are contained in the tile
    int num_padded_pixels;
    // int top_ghost;                  //1 if top ghost row is included, 0 if not 
    // int bottom_ghost;               //1 if bottom ghost row is included, 0 if not
} TileInfo;

bool SOBEL_COMPLETE = false;
bool EMBOSS_COMPLETE = false;


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

/*
-tile = struct of info about tile
-pixels = unfiltered pixel values
-filtered_pixels = buffer to write filtered pixels into
*/
void ApplySobel(TileInfo *tile, RGB *recieved_pixels, RGB *filtered_pixels){
    printf("ApplySobel()\n");

    //Exclude padding rows when looping
    int i=0;    //Counter for index in "filtered_pixels[]"
    for(int row=1; row<tile->height-1; row++){
        //Exclude padding columns when looping
        for(int column=1; column<tile->width-1; column++){
            //find kernel
            int main_pixel_index = ((row * tile->width) + column);
            int top_left_index = (((row-1) * tile->width) + column) - 1;    
            int top_middle_index = (((row-1) * tile->width) + column);   
            int top_right_index = (((row-1) * tile->width) + column) + 1;   
            int left_index = main_pixel_index - 1;
            int right_index = main_pixel_index + 1;
            int bottom_left_index = (((row+1) * tile->width) + column) - 1;
            int bottom_middle_index = (((row+1) * tile->width) + column);
            int bottom_right_index = (((row+1) * tile->width) + column) + 1;

            //Get actuall pixel values
            int main_pixel = recieved_pixels[main_pixel_index].red;
            int top_left = recieved_pixels[top_left_index].red;    
            int top_middle = recieved_pixels[top_middle_index].red;   
            int top_right = recieved_pixels[top_right_index].red;   
            int left = recieved_pixels[left_index].red;
            int right = recieved_pixels[right_index].red;
            int bottom_left = recieved_pixels[bottom_left_index].red;
            int bottom_middle = recieved_pixels[bottom_middle_index].red;
            int bottom_right = recieved_pixels[bottom_right_index].red;
                    
            //multiply the retrieved pixel values with the correspinding value in the Gx kernel and SUM them
            int Gx_value =  (-1*top_left + 0*top_middle + 1*top_right) +
                            (-2*left + 0*main_pixel + 2*right) +
                            (-1*bottom_left + 0*bottom_middle + 1*bottom_right);
        
        
            //multiply the retrieved pixel values with the correspinding value in the Gy kernel and SUM them
            int Gy_value =  (1*top_left + 2*top_middle + 1*top_right) +
                            (0*left + 0*main_pixel + 0*right) +
                            (-1*bottom_left + -2*bottom_middle + -1*bottom_right);

            //Compute the final pixel value 
            int G = (int) sqrt((pow(Gx_value, 2)) + (pow(Gy_value, 2)));
            if(G > 255){
                G = 255;
            }
            if(G < 0){
                G = 0;
            }

            //Write the calculated G value to a new temporary image
            // filtered_pixels[main_pixel_index].red = G;
            // filtered_pixels[main_pixel_index].green = G;
            // filtered_pixels[main_pixel_index].blue = G;
            filtered_pixels[i].red = G;
            filtered_pixels[i].green = G;
            filtered_pixels[i].blue = G;

            i++;
        }
    }

    return;
}


void master_process(int world_rank, int world_size)
{
    //Define variables
    const char *marguerite="../../images/marguerite.bmp";

    //Get image dimensions and store them in width, height
    int image_width, image_height;
    GetSize(marguerite, &image_width, &image_height);   

    //Allocate enough memory to store RGB structs for each pixel in the image 
    RGB *sobel=malloc(sizeof(RGB)*image_width*image_height);
    RGB *emboss=malloc(sizeof(RGB)*image_width*image_height);

    //Read the entire BMP image and store it in sobel and emboss
    LoadRegion(marguerite, 0, 0, image_width, image_height, sobel);
    LoadRegion(marguerite, 0, 0, image_width, image_height, emboss);

    //Convert images to grayscale
    ImageToGrayscale(sobel, image_width, image_height);
    ImageToGrayscale(emboss, image_width, image_height);

    //How many rows each process should have
    int rows_per_process = image_height / (world_size-1);
    int excess_rows = image_height % (world_size-1);

    //Calculate how many tiles the image can be divided into for each process (rectangular shapes going from start_row -> end_row, with specified with)
    int sent_tiles = 0;     //Counter for how many tiles have been sent

    
    //Send tiles to each process
    int row_counter = 0;    //Keeps track of rows in loop
    for(int process=1; process<world_size; process++){
        
        TileInfo tile;
        tile.num_padded_pixels = 0;
        //first process will get excess rows included
        if(process == 1){
            tile.width = image_width + 2; //Added ghost columns
            tile.height = rows_per_process + excess_rows + 2;   //Added two ghost rows
            tile.num_pixels = tile.width * tile.height;
        }
        else{
            tile.width = image_width + 2;             //Added ghost columns
            tile.height = rows_per_process + 2; //Added two ghost rows
            tile.num_pixels = tile.width * tile.height;
        }
            
        //Create array of pixels to send
        RGB *pixels = malloc(tile.num_pixels*sizeof(RGB));
        int column_counter = 0;
        
        for(int i=0; i<tile.num_pixels; i++){
            // printf("Row_counter: %d, image_height: %d\n", row_counter, image_height);
            //Create padding at top, bottom and sides, but not for 
            if(row_counter == 0 || column_counter == 0 || column_counter == tile.width-1 || ((row_counter-(world_size-1)) == image_height)){
                pixels[i].red = 0;
                pixels[i].green = 0;
                pixels[i].blue = 0;
                
                //Count how many extra pixels are added
                tile.num_padded_pixels++;
            }

            else{
                pixels[i].red = sobel[((row_counter-1)*image_width) + (column_counter-1)].red;
                pixels[i].green = sobel[((row_counter-1)*image_width) + (column_counter-1)].green;
                pixels[i].blue = sobel[((row_counter-1)*image_width) + (column_counter-1)].blue;
            }

            //Increment column counter
            column_counter++;

            //Increment row_counter and reset column_counter if full width has been looped over
            if(column_counter % tile.width == 0){
                row_counter++;
                column_counter = 0;
            }
        }


        //Send TileInfo to worker process
        MPI_Send(
            &tile,              //Pointer to the data I want to send
            sizeof(TileInfo),   //number of items in buf
            MPI_BYTE,           //The datatype of what I am sending
            process,            //The ID/world_rank of the process I am sending to
            1,                  //Tag
            MPI_COMM_WORLD      //communicator (usually MPI_COMM_WORLD)
        );

        //Send pixel values to worker process
        MPI_Send(
            pixels,              //Data I want to send
            (tile.num_pixels*sizeof(RGB)),   //number of items in buf
            MPI_BYTE,           //The datatype of what I am sending
            process,            //The ID/world_rank of the process I am sending to
            2,                  //Tag
            MPI_COMM_WORLD      //communicator (usually MPI_COMM_WORLD)
        );

        sent_tiles++;
        row_counter -= 1;

        
        // if(process == 1){
        //     //Save images
        //     const char *test_sobel="test_sobel.bmp";
        //     //Create new image
        //     CreateBMP(test_sobel,tile.width,tile.height);
        //     WriteRegion(test_sobel,0,0,tile.width,tile.height,pixels);
        // }
    }


    //////GOOD SO FAR ///////////////





    //Recieve filtered pixels from worker processes
    int recieved_tiles = 0;
    while(SOBEL_COMPLETE != true){
        //Recieve status of incoming message
        MPI_Status message_status;
        MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &message_status);

        //Get the number of items in that message
        int message_size;
        MPI_Get_count(&message_status, MPI_UNSIGNED_CHAR, &message_size);

        //Allocate memory for incoming message
        RGB *filtered_pixels = malloc(message_size * sizeof(RGB));
        
        //Receive filtered pixels
        MPI_Recv(
            filtered_pixels, 
            message_size * sizeof(RGB), 
            MPI_BYTE, 
            MPI_ANY_SOURCE, 
            MPI_ANY_TAG, 
            MPI_COMM_WORLD, 
            MPI_STATUS_IGNORE
        );

        // if(message_status.MPI_SOURCE == 5){
        //     //Save images
        //     const char *outsobel="outsobel.bmp";
        //     //Create new image
        //     CreateBMP(outsobel,image_width,image_height);
        //     WriteRegion(outsobel,0,0,image_width,image_height,filtered_pixels);
        // }


        //GOOD SO FAR, pixels are filtered and recieved correctly
            

        //Assemble the filtered pixels into buffer
        if(message_status.MPI_SOURCE == 1){
            for(int i=0; i<message_size; i++){
                sobel[i].red = filtered_pixels[i].red;
                sobel[i].green = filtered_pixels[i].green;
                sobel[i].blue = filtered_pixels[i].blue;
            }
        }

        else{
            int start_row = (rows_per_process * (message_status.MPI_SOURCE-1)) + excess_rows;
            int end_row = start_row + rows_per_process;
            int i = 0;
            
            for(int sobel_index=start_row*image_width; sobel_index<image_width*image_height; sobel_index++){
                sobel[sobel_index].red = filtered_pixels[i].red;
                sobel[sobel_index].green = filtered_pixels[i].green;
                sobel[sobel_index].blue = filtered_pixels[i].blue;

                i++;
            }
        }

        recieved_tiles++;
        
        if(recieved_tiles == sent_tiles){
            SOBEL_COMPLETE = true;
        }
    }

    //Save images
    const char *outsobel="sobel.bmp";
    const char *outemboss="emboss.bmp";
    
    //Create new images
    CreateBMP(outsobel, image_width, image_height);
    WriteRegion(outsobel, 0, 0, image_width, image_height, sobel);
        
    CreateBMP(outemboss, image_width, image_height);
    WriteRegion(outemboss, 0, 0, image_width, image_height, emboss);

    //Free memory
    free(sobel);
    free(emboss);

    return;
}    
    

void worker_process(int world_rank)
{  
    //Recieve info about tile
    TileInfo tile;
    MPI_Recv(
        &tile,              // pointer to receive buffer
        sizeof(TileInfo),   // size in bytes
        MPI_BYTE,           // datatype
        0,                  // source rank (master = 0)
        1,                  // tag (must match send)
        MPI_COMM_WORLD,
        MPI_STATUS_IGNORE
    );

    //Receive padded pixels
    RGB *recieved_pixels = malloc(tile.num_pixels*sizeof(RGB));
    MPI_Recv(
        recieved_pixels, 
        (tile.num_pixels*sizeof(RGB)), 
        MPI_BYTE, 
        0, 
        2, 
        MPI_COMM_WORLD, 
        MPI_STATUS_IGNORE
    );

    // if(world_rank == 5){
    //     for(int i=0; i<tile.height; i++){
    //         for(int j=0; j<tile.width; j++){
    //             printf("%d ", recieved_pixels[(i*tile.width)+j].red);
    //         }
    //         printf("\n");
    //     }
    // }


    
    
    
    
    
    //Create buffers to store sobel and emboss filtered pixels, no padding pixels
    RGB *sobel_pixels = malloc((tile.num_pixels - tile.num_padded_pixels)*sizeof(RGB));
    
    //Apply both sobel and emboss filters
    ApplySobel(&tile, recieved_pixels, sobel_pixels);
    
        
    //////GOOD SO FAR ///////////////


    //Send sobel pixels back to master (No ghost tiles)
    MPI_Send(
        sobel_pixels,                                         //Pointer to the data I want to send
        (tile.num_pixels - tile.num_padded_pixels)*sizeof(RGB), //number of items in buf
        MPI_BYTE,                                               //The datatype of what I am sending
        0,                                                      //The ID/world_rank of the process I am sending to
        3,                                                      //Tag
        MPI_COMM_WORLD                                          //communicator (usually MPI_COMM_WORLD)
    );
        
    return;
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
	
	// Do different work if process is MASTER_PROCESS or worker_process
	if(world_rank == 0){
        //Start master process
		master_process(world_rank, world_size);
	}
	else{
        //Start worker processes
		worker_process(world_rank);
	}

	MPI_Finalize();
	return 0;
}
