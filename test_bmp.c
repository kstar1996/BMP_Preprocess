#include <stdio.h>
#include <stdlib.h>
#include "bmp.h"
#include <assert.h>

int main(int argc, char* argv[])
{

    FILE* file = fopen("lena_color.bmp", "rb"); //open binary file for reading
    FILE* file_copy = fopen("lenacopy.bmp", "wb"); //create a binary file for writing
    FILE* file_crop = fopen("lena_crop.bmp", "wb"); //create a file to write cropped image


    char* error = NULL;
    BMPImage* bmp_image = read_bmp(file, &error);
    if(error != NULL)
    {
        printf("%s", error);
        fclose(file);
        fclose(file_copy);
        fclose(file_crop);
        free(bmp_image);
        free(error);
        return EXIT_FAILURE;
    }
    bool write_success = write_bmp(file_copy, bmp_image, &error);
    if(error != NULL)
    {
        printf("%s", error);
        fclose(file);
        fclose(file_copy);
        fclose(file_crop);
        free_bmp(bmp_image);
        free(error);
        return EXIT_FAILURE;
    }
    printf("%d", write_success);


    //crop an image starting from coordinate (300,200) new width size=200. new height size=200.
    BMPImage* crop_image = crop_bmp(bmp_image, 300,200,200,200,&error);
    if(error != NULL)
    {
        printf("%s", error);
        fclose(file);
        fclose(file_copy);
        fclose(file_crop);
        free_bmp(bmp_image);
        free(error);
        return EXIT_FAILURE;
    }
    write_bmp(file_crop, crop_image, &error);
    if(error != NULL)
    {
        printf("%s", error);
        fclose(file);
        fclose(file_copy);
        fclose(file_crop);
        free_bmp(bmp_image);
        free_bmp(crop_image);
        free(error);
        return EXIT_FAILURE;
    }

    free_bmp(bmp_image);
    free_bmp(crop_image);
    fclose(file);
    fclose(file_copy);
    fclose(file_crop);


    return EXIT_SUCCESS;
}
