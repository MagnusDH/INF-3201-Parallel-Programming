#include <stdio.h>
#include <stdlib.h>
#include "lib/bmp.h"
#include <math.h>
#include <string.h>

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
    printf("APPLYSOBEL()\n");

    //Convert image to greyscale
    ImageToGrayscale(img, width, height);

    RGB main_pixel = img[100];

    // Print each component
    printf("Red: %u, Green: %u, Blue: %u\n", main_pixel.red, main_pixel.green, main_pixel.blue);

    //make a loop that goes through every single pixel
    // for(int curY=0; curY<height+1; curY++){
    //     for(int curX=0; curX<width+1; curX++){
    //         //If pixel is top pixel OR left edge pixel OR right edge pixel OR bottom pixel
    //         if(curX == 0 || curX == width || curY == 0 || curY == height){
    //             //Do something special, becuase its a border pixel
    //             printf("I am a corner pixel, do something ")
    //         }            
    //         else{
    //             //Get main pixel
    //             RGB main_pixel = sobel[curY + curX];

    //             // Print each component
    //             printf("Red: %u, Green: %u, Blue: %u\n", main_pixel.red, main_pixel.green, main_pixel.blue);


    //             //multiply the neighbouring pixels with the correspinding value in the Gx kernel and SUM them


                


    //             //multiply the neighbouring pixels with the correspinding value in the Gy kernel and SUM them
    //             //Compute the final pixel value with G= sqrt((Gx_value**2) + (Gy_value**2))
    //             //Replace the original pixel in the original image with the calulated G-value

    //         }
            
    //     }
    // } 
        
        
        

}

void ApplyEmboss(RGB *img, const int width, const int height){
    printf("APPLYEMBOSS()\n");
    printf("Implement me...\n");
}


int main(){
    printf("MAIN()\n");
    
    //Define variables
    const char *marguerite="../../images/marguerite.bmp";
    const char *outsobel="sobel.bmp";
    const char *outemboss="emboss.bmp";


    //Get image dimensions and store them in width, height
    int width, height;
    GetSize(marguerite, &width, &height);   
    printf("width: %d, height: %d\n", width, height);

    //Allocate enough memory to store RGB structs for each pixel in the image 
    RGB *sobel=malloc(sizeof(RGB)*width*height);
    RGB *emboss=malloc(sizeof(RGB)*width*height);

    //Loads every pixel in image and store it in given memory location
    LoadRegion(marguerite, 0, 0, width, height, sobel);
    // LoadRegion(marguerite, 0, 0, width, height, emboss);

    //Apply filters
    ApplySobel(sobel, width, height);
    // ApplyEmboss(emboss, width, height);

    // Save images
    CreateBMP(outsobel,width,height);
    WriteRegion(outsobel,0,0,width, height,sobel);
    
    // CreateBMP(outemboss,width,height);
    // WriteRegion(outemboss,0,0,width, height,emboss);

    // Free memory
    free(sobel);
    free(emboss);

    return(0);
}
