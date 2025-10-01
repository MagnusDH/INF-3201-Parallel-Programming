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

//Number of processes in program
int num_processes = 6;

//Contains info about a single pixel
typedef struct {
    int row;
    int col;
    unsigned char red;
    unsigned char green;
    unsigned char blue;
} PixelInfo;


//Contains info about a whole tile
typedef struct {
    int tile_number;
    // int filter_to_apply;            //1 for sobel, 2 for emboss
    int width;                      //How many pixels in width the tile is
    int height;                     //How many rows the tile is
    int top_ghost;                  //1 if top ghost row is included, 0 if not 
    int bottom_ghost;               //1 if bottom ghost row is included, 0 if not
    PixelInfo *pixels;              //Array of PixelInfo structs
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
-Tile = struct of info about tile and actuall pixel values
-Width = width of tile
-Height = number of rows in tile
*/
void ApplySobel(TileInfo *tile){
    printf("ApplySobel()\n");
    //Create temporary buffer to write new filter values to
    TileInfo *tmp_tile_info = malloc(sizeof(TileInfo));
    tmp_tile_info->pixels = malloc((tile->height*tile->width) * sizeof(PixelInfo));
    // PixelInfo *filtered_pixels = malloc((tile->height*tile->width) * sizeof(PixelInfo));

    //If tile only has bottom ghost row
    if(tile->bottom_ghost == 1 && tile->top_ghost == 0){
        
        //Apply filter for pixels from start but not last row 
        for(int row=0; row<tile->height-1; row++){
            for(int column=0; column<tile->width; column++){
                // printf("here!!!");
                int main_pixel_index = ((row * tile->width) + column);
                
                //if main_pixel is an edge pixel
                if(tile->pixels[main_pixel_index].row == 0 || tile->pixels[main_pixel_index].row == tile->height ||
                   tile->pixels[main_pixel_index].col == 0 || tile->pixels[main_pixel_index].col == tile->width){
                    //set the pixel to its original grayscale value, aka dont do anything
                    tmp_tile_info->pixels[main_pixel_index].red = tile->pixels[main_pixel_index].red;
                }
                //Pixel is not an edge pixel
                else{
                    //find kernel
                    int top_left_index = (((row - 1) * tile->width) + column) - 1;    
                    int top_middle_index = (((row - 1) * tile->width) + column);   
                    int top_right_index = (((row - 1) * tile->width) + column) + 1;   
                    int left_index = main_pixel_index - 1;
                    int right_index = main_pixel_index + 1;
                    int bottom_left_index = (((row + 1) * tile->width) + column) - 1;
                    int bottom_middle_index = (((row + 1) * tile->width) + column);
                    int bottom_right_index = (((row + 1) * tile->width) + column) + 1;

                    
                    // printf("###########################################\n");
                    // printf("top left: %d\n", top_left_index);
                    // printf("top middle: %d\n", top_middle_index);
                    // printf("top right: %d\n", top_right_index);
                    // printf("left: %d\n", left_index);
                    // printf("main: %d\n", main_pixel_index);
                    // printf("right: %d\n", right_index);
                    // printf("bottom left: %d\n", bottom_left_index);
                    // printf("bottom middle: %d\n", bottom_middle_index);
                    // printf("bottom right: %d\n", bottom_right_index);
                    // printf("###########################################\n");


                    //Get actuall pixel values
                    int main_pixel = tile->pixels[main_pixel_index].red;
                    int top_left = tile->pixels[top_left_index].red;    
                    int top_middle = tile->pixels[top_middle_index].red;   
                    int top_right = tile->pixels[top_right_index].red;   
                    int left = tile->pixels[left_index].red;
                    int right = tile->pixels[right_index].red;
                    int bottom_left = tile->pixels[bottom_left_index].red;
                    int bottom_middle = tile->pixels[bottom_middle_index].red;
                    int bottom_right = tile->pixels[bottom_right_index].red;
                    
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
                    tmp_tile_info->pixels[main_pixel_index].red = G;
                    tmp_tile_info->pixels[main_pixel_index].green = G;
                    tmp_tile_info->pixels[main_pixel_index].blue = G;
                }
            }
        }



        //Replace the original image with the new filtered image
        for(int i=0; i<(tile->width*tile->height); i++){
            tile->pixels[i] = tmp_tile_info->pixels[i];
        }

        //Free memory used
        free(tmp_tile_info);
        free(tmp_tile_info->pixels);
    }

    // else{
    //     printf("apply_sobel(): Do something else with tile later\n");
    // }


    //if tile only has top ghost row
    // if(tile.top_ghost == 1 && tile.bottom_ghost == 0){
    //     //Start indexing from i+width to num_pixels
    //         //if pixel is a corner pixel
    //             //copy pixel
    //         //else
    //             //find kernel
    //             //apply filter
    // }

    // //If tile has top and bottom ghost rows
    // if(tile.top_ghost == 1 && tile.bottom_ghost == 1){
    //     //Start indexing from i=0+width to num_pixels-width 
    //         //if pixel is a corner pixel
    //             //copy pixel
    //         //else
    //             //find kernel
    //             //apply filter
    // }

    return;
}


void ApplyEmboss(RGB *img, const int width, const int height){

    //Embos kernel used: https://docs.aspose.com/imaging/net/developer-guide/manipulating-images/kernel-filters/emboss-filter/
    // { -2, -1,  0}
    // { -1,  1,  1}
    // {  0,  1,  2}


    //Convert image to greyscale
    ImageToGrayscale(img, width, height);

    //Create temporary space to write new filter values
    RGB *out_img = malloc(width * height * sizeof(RGB));

    //make a loop that goes through every single pixel in array   
    for(int row=0; row<height; row++){
        for(int col=0; col<width; col++){

            //If pixel is a corner pixel
            if(row == 0 || row == height-1 || col == 0 || col == width-1){
                //Do something special, becuase its a border pixel
                int main_pixel_index = ((row * width) + col);
                out_img[main_pixel_index] = img[main_pixel_index];
            }        

            //Pixel is not corner pixel
            else{               
                //Get pixel INDEX values for access in img->array
                int main_pixel_index = ((row * width) + col);
                int top_left_index = (((row - 1) * width) + col) - 1;    
                int top_middle_index = (((row - 1) * width) + col);   
                int top_right_index = (((row - 1) * width) + col) + 1;   
                int left_index = main_pixel_index - 1;
                int right_index = main_pixel_index + 1;
                int bottom_left_index = (((row + 1) * width) + col) - 1;
                int bottom_middle_index = (((row + 1) * width) + col);
                int bottom_right_index = (((row + 1) * width) + col) + 1;
    
                //Get actuall pixel values
                int main_pixel = img[main_pixel_index].red;
                int top_left = img[top_left_index].red;    
                int top_middle = img[top_middle_index].red;   
                int top_right = img[top_right_index].red;   
                int left = img[left_index].red;
                int right = img[right_index].red;
                int bottom_left = img[bottom_left_index].red;
                int bottom_middle = img[bottom_middle_index].red;
                int bottom_right = img[bottom_right_index].red;
                
                
                //multiply the retrieved pixel values with the correspinding value in the Gx kernel and SUM them
                int G = (-2*top_left + -1*top_middle + 0*top_right) +
                        (-1*left + 1*main_pixel + 1*right) +
                        (0*bottom_left + 1*bottom_middle + 2*bottom_right);

                if(G > 255){
                    G = 255;
                }
                if(G < 0){
                    G = 0;
                }

                // { -2, -1,  0}
                // { -1,  1,  1}
                // {  0,  1,  2}

                //Write the calculated G value to a new temporary image
                out_img[main_pixel_index].red = G;
                out_img[main_pixel_index].green = G;
                out_img[main_pixel_index].blue = G;
            }  
        }
    } 

    //Replace the original image with the new filtered image
    for(int i=0; i<width*height; i++){
        img[i] = out_img[i];
    }

    //Free memory used
    free(out_img);
}


void master_process(int world_rank, int world_size)
{
    //Define variables
    const char *marguerite="../../images/marguerite.bmp";

    //Get image dimensions and store them in width, height
    int width, height;
    GetSize(marguerite, &width, &height);   

    //Allocate enough memory to store RGB structs for each pixel in the image 
    RGB *sobel=malloc(sizeof(RGB)*width*height);
    RGB *emboss=malloc(sizeof(RGB)*width*height);

    //Read the entire BMP image and store it in sobel and emboss
    LoadRegion(marguerite, 0, 0, width, height, sobel);
    LoadRegion(marguerite, 0, 0, width, height, emboss);

    //Convert images to grayscale
    ImageToGrayscale(sobel, width, height);
    ImageToGrayscale(emboss, width, height);


    //Calculate how many tiles the image can be divided into for each process (rectangular shapes going from start_row -> end_row, with specified with)
    int tiles_per_process = height / num_processes;

    
    int tile_number = 1;        //Number for a sent/recieved tile
    int sent_tiles = 0;         //Counter for how many tiles have been sent
    
    //Send tiles to each process, excluded process 0
    for(int process=1; process<world_size; process++){
        
        //Keeps track of rows that has been sent
        int row_counter = 0;

        //First process needs 80 rows + 1 ghost row
        if(process == 1){
            TileInfo tile;
            tile.tile_number = tile_number;
            // tile.filter_to_apply = 1;
            tile.width = width;
            tile.height = tiles_per_process+1;  //add one row
            tile.top_ghost = 0;                 //Tile does not have a top ghost row
            tile.bottom_ghost = 1;              //Tile has a bottom ghost row
            tile.pixels = malloc((tiles_per_process+1)*width * sizeof(PixelInfo));  //Allocate memory
            
            //send 80 rows + 1 ghost row
            int column_counter = 0;
            for(int i=0; i<(tiles_per_process+1)*width; i++){
                //create array
                tile.pixels[i].row = row_counter;
                tile.pixels[i].col = column_counter;
                tile.pixels[i].red = sobel[(row_counter*width) + column_counter].red;
                tile.pixels[i].green = sobel[(row_counter*width) + column_counter].green;
                tile.pixels[i].blue = sobel[(row_counter*width) + column_counter].blue;
                
                //Increment column counter
                column_counter++;

                //Increment row_counter and reset column_counter if full width has been looped over
                if(column_counter % width == 0){
                    row_counter++;
                    column_counter = 0;
                }
            }

            //Send info about pixels to process
            MPI_Send(
                &tile,              //Pointer to the data I want to send
                sizeof(TileInfo),   //number of items in buf
                MPI_BYTE,           //The datatype of what I am sending
                process,            //The ID/world_rank of the process I am sending to
                1,                  //Tag
                MPI_COMM_WORLD      //communicator (usually MPI_COMM_WORLD)
            );

            //Send actuall pixels to process
            MPI_Send(
                tile.pixels,              //Pointer to the data I want to send
                (tiles_per_process+1)*width * sizeof(PixelInfo),   //number of items in buf
                MPI_BYTE,           //The datatype of what I am sending
                process,            //The ID/world_rank of the process I am sending to
                2,                  //Tag
                MPI_COMM_WORLD      //communicator (usually MPI_COMM_WORLD)
            );


            //decrement row_counter
            row_counter--;

            //Increment tile_number
            tile_number++;

            //Increment sent_tiles
            sent_tiles++;
        }
    
        
        //Last process needs 80 rows + 1 ghost row
        if(process == num_processes-1){
            continue;
        }
        

        //Middle process needs 80 rows + 2 ghost rows
        else{
            TileInfo tile;
            tile.tile_number = tile_number;
            tile.filter_to_apply = 1;
            tile.width = width;
            tile.height = tiles_per_process+2;  //add two rows
            tile.top_ghost = 1;                 //Tile does not have a top ghost row
            tile.bottom_ghost = 1;              //Tile has a bottom ghost row
            tile.pixels = malloc((tiles_per_process+2)*width * sizeof(PixelInfo));  //Allocate memory
            
            //send 80 rows + 2 ghost rows
            int column_counter = 0;
            for(int i=0; i<(tiles_per_process+2)*width; i++){
                //create array
                tile.pixels[i].row = row_counter;
                tile.pixels[i].col = column_counter;
                tile.pixels[i].red = sobel[(row_counter*width) + column_counter].red;
                tile.pixels[i].green = sobel[(row_counter*width) + column_counter].green;
                tile.pixels[i].blue = sobel[(row_counter*width) + column_counter].blue;
                
                //Increment column counter
                column_counter++;

                //Increment row_counter and reset column_counter if full width has been looped over
                if(column_counter % width == 0){
                    row_counter++;
                    column_counter = 0;
                }
            }

            //Send info about pixels to process
            MPI_Send(
                &tile,              //Pointer to the data I want to send
                sizeof(TileInfo),   //number of items in buf
                MPI_BYTE,           //The datatype of what I am sending
                process,            //The ID/world_rank of the process I am sending to
                1,                  //Tag
                MPI_COMM_WORLD      //communicator (usually MPI_COMM_WORLD)
            );

            //Send actuall pixels to process
            MPI_Send(
                tile.pixels,              //Pointer to the data I want to send
                (tiles_per_process+1)*width * sizeof(PixelInfo),   //number of items in buf
                MPI_BYTE,           //The datatype of what I am sending
                process,            //The ID/world_rank of the process I am sending to
                2,                  //Tag
                MPI_COMM_WORLD      //communicator (usually MPI_COMM_WORLD)
            );

            //decrement row_counter
            row_counter--;

            //Increment tile_number
            tile_number++;

            //Increment sent_tiles
            sent_tiles++;
        }
    }

    //RECIEVE TILES AND ASSEMBLE IMAGE
    int recieved_sobel_tiles = 0;
    int recieved_emboss_tiles = 0;

    while(SOBEL_COMPLETE != true || EMBOSS_COMPLETE != true){

        //Recieve filtered tile
        MPI_Status message_status;

        //Allocate memory for filtered pixels
        filtered_pixels = malloc(tiles_per_process * width * sizeof(PixelInfo));

        //Receive filtered pixel data
        MPI_Recv(
            filtered_pixels, 
            tiles_per_process * width * sizeof(PixelInfo), 
            MPI_BYTE, 
            MPI_ANY_SOURCE, 
            MPI_ANY_TAG, 
            MPI_COMM_WORLD, 
            &message_status            //Status of message
        );


        //If tag is sobel
        if(message_status.MPI_TAG == 3){
            //Replace original image pixels with filtered pixels
            for(int i=0; i<tiles_per_process * width; i++){
                int current_index = filtered_pixels[i].row * filtered_pixels[i].col;
                sobel[current_index].red = filtered_pixels[current_index].red;
                sobel[current_index].green = filtered_pixels[current_index].green;
                sobel[current_index].blue = filtered_pixels[current_index].blue;
            }
            
            //Increment recieved sobel tiles
            recieved_sobel_tiles++;
            
            //If we have recieved all sobel tiles
            if(recieved_sobel_tiles == sent_tiles){
                SOBEL_COMPLETE = true;
            }
        }

        //If tag is emboss
        if(message_status.MPI_TAG == 4){
            //Replace original image pixels with filtered pixels
            for(int i=0; i<tiles_per_process * width; i++){
                int current_index = filtered_pixels[i].row * filtered_pixels[i].col;
                emboss[current_index].red = filtered_pixels[current_index].red;
                emboss[current_index].green = filtered_pixels[current_index].green;
                emboss[current_index].blue = filtered_pixels[current_index].blue;
            }
            
            //Increment recieved sobel tiles
            recieved_emboss_tiles++;
            
            //If we have recieved all sobel tiles
            if(recieved_emboss_tiles == sent_tiles){
                EMBOSS_COMPLETE = true;
            }
        }
    }


    //Save images
    // const char *outsobel="sobel.bmp";
    // const char *outemboss="emboss.bmp";
    //Create new image
    CreateBMP(sobel,width,height);
    // //Write filtered region to new image
    // WriteRegion(outsobel,0,0,width, height,sobel);

        
    CreateBMP(emboss,width,height);
    // //     WriteRegion(outemboss,0,0,width, height,emboss);
    
    //Free memory
    free(sobel);
    free(emboss);

    return;
}    
    

void worker_process(int world_rank)
{
    //First process recieves 81 tiles
    if(world_rank == 1){
        
        //Recieve info about tile
        TileInfo recieved_tile_info;
        MPI_Recv(
            &recieved_tile_info,    // pointer to receive buffer
            sizeof(TileInfo),       // size in bytes (must match the send)
            MPI_BYTE,               // datatype
            0,                      // source rank (master = 0)
            1,                      // tag (must match send)
            MPI_COMM_WORLD,
            MPI_STATUS_IGNORE
        );

        // printf("Worker %d received:\n", world_rank);
        // printf("Tile number: %d\n", recieved_tile_info.tile_number);
        // printf("Filter to apply: %d\n", recieved_tile_info.filter_to_apply);
        // printf("height: %d\n", recieved_tile_info.height);
        // printf("Width: %d\n", recieved_tile_info.width);
        // printf("top_ghost: %d\n", recieved_tile_info.top_ghost);
        // printf("bottom_ghost: %d\n", recieved_tile_info.bottom_ghost);

        
        //Allocate memory for pixel array based on num_pixels
        recieved_tile_info.pixels = malloc((recieved_tile_info.height*recieved_tile_info.width) * sizeof(PixelInfo));

        //Receive pixel data
        MPI_Recv(
            recieved_tile_info.pixels, 
            (recieved_tile_info.height*recieved_tile_info.width) * sizeof(PixelInfo), 
            MPI_BYTE, 
            0, 
            2, 
            MPI_COMM_WORLD, 
            MPI_STATUS_IGNORE
        );

        // for(int i = 0; i<recieved_tile_info.num_pixels; i++){
        //     printf("(%d,%d) - ", recieved_tile_info.pixels[i].row, recieved_tile_info.pixels[i].col);
        // }

        //Create two buffers for sobel and emboss filtered pixels
        PixelInfo *sobel_filtered = malloc(recieved_tile_info.width*recieved_tile_info.height*sizeof(PixelInfo));
        PixelInfo *emboss_filtered = malloc(recieved_tile_info.width*recieved_tile_info.height*sizeof(PixelInfo));

        //Apply both sobel and emboss filters
        ApplySobel(&recieved_tile_info, &sobel_filtered);
        ApplyEmboss(&recieved_tile_info, &emboss_filtered);


        //Send sobel pixels back to master
        MPI_Send(
            sobel_filtered,  //Pointer to the data I want to send
            (recieved_tile_info.height)*recieved_tile_info.width * sizeof(PixelInfo),   //number of items in buf
            MPI_BYTE,                   //The datatype of what I am sending
            0,                    //The ID/world_rank of the process I am sending to
            4,                          //Tag
            MPI_COMM_WORLD              //communicator (usually MPI_COMM_WORLD)
        );

        //Send emboss pixels back to master
        MPI_Send(
            recieved_tile_info.pixels,  //Pointer to the data I want to send
            (recieved_tile_info.height)*recieved_tile_info.width * sizeof(PixelInfo),   //number of items in buf
            MPI_BYTE,                   //The datatype of what I am sending
            0,                    //The ID/world_rank of the process I am sending to
            4,                          //Tag
            MPI_COMM_WORLD              //communicator (usually MPI_COMM_WORLD)
        );

        }
    }

    // //Last process recieves 81 tiles
    // if(world_rank == num_processes-1){
        
    // }

    // //All other processes recieves 82 tiles
    // else{

    // }

   
    return;

	//As long as image is not complete
	// while(IMAGE_COMPLETE != true){


    //         //Ask master for work
    // 		// long long send_data[2];
    // 		// MPI_Send(
    //             // 	&send_data,			//Pointer to the data I want to send
    //             // 	2,					//number of items in buf
    //             // 	MPI_LONG_LONG,			//The datatype of what I am sending
    //             // 	0,					//The ID/world_rank of the process I am sending to
    //             // 	2,					//Tag
    //             // 	MPI_COMM_WORLD    	//communicator (usually MPI_COMM_WORLD)
    //             // );
                
    //             //Recieve message from master
    //             // long long recieved_data[2];
    //             // MPI_Status recieved_message_status;
    //             // MPI_Recv(
    //             //     &recieved_data,			//buffer to store incoming data
    //             //     2,						//max number of items to receive
    //             //     MPI_LONG_LONG,				//datatype of item to recieve
    //             //     0,						//ID/world_rank of sender
    //             //     MPI_ANY_TAG,			//message tag
    //             //     MPI_COMM_WORLD,			//Communicator
    //             //     &recieved_message_status//pointer to MPI_Status to get info about message
    //             // );
                
                
                
    //         //if tag==2 -> master has sent work
    //         if(recieved_message_status.MPI_TAG == 2){
    //             //Recieve work

    //             //Apply filters
    //             // ApplySobel(sobel, width, height);
    //             // ApplyEmboss(emboss, width, height);

    //             //Send the inner non-overlapping filtered tile back to the master.
    // 		}

    // 		//Master says image is complete. Stop process
    // 		if(recieved_message_status.MPI_TAG == 4){
    // 			IMAGE_COMPLETE = true;
    //             break;
    // 		}
    // }

    // 	printf("%d: stopped working\n", world_rank);
    // return;
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
