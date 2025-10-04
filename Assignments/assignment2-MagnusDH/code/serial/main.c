#include <stdio.h>
#include <stdlib.h>
#include "lib/bmp.h"
#include <math.h>
#include <string.h>
#include <time.h>


#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) < (b) ? (b) : (a))
#define GP(X,Y) GetPixel(img, width, height,(X),(Y))


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

void ApplySobel(RGB *img, const int width, const int height){

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


int main(){
    
    //Define variables
    const char *marguerite="../../images/marguerite.bmp";
    const char *outsobel="sobel.bmp";
    const char *outemboss="emboss.bmp";

    //Initialize a clock to measure the time used to open the lock
	clock_t clock_start;
	clock_t clock_end;
	double time_used;
    clock_start = clock();

    //Get image dimensions and store them in width, height
    int width, height;
    GetSize(marguerite, &width, &height);   

    //Allocate enough memory to store RGB structs for each pixel in the image 
    RGB *sobel=malloc(sizeof(RGB)*width*height);
    RGB *emboss=malloc(sizeof(RGB)*width*height);

    //Loads every pixel in image and store it in given memory location
    LoadRegion(marguerite, 0, 0, width, height, sobel);
    LoadRegion(marguerite, 0, 0, width, height, emboss);

    //Apply filters
    ApplySobel(sobel, width, height);
    ApplyEmboss(emboss, width, height);

    //Save images
    CreateBMP(outsobel,width,height);
    WriteRegion(outsobel,0,0,width, height,sobel);
    
    CreateBMP(outemboss,width,height);
    WriteRegion(outemboss,0,0,width, height,emboss);

    //Free memory
    free(sobel);
    free(emboss);

    //End clock and print result
	clock_end = clock();
    //Calculate how much time was used and convert the result type to "double" to show decimals
	time_used = (double) (clock_end - clock_start) / CLOCKS_PER_SEC;
	printf("Time used to apply filters: %f seconds\n", time_used);

    return(0);
}
