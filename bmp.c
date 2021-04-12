#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bmp.h"


//read a bmp image from a file. return newly created image
BMPImage* read_bmp(FILE* fp, char** error)
{

    rewind(fp); //go to beginning of file

    //declare vars
    BMPHeader header;
    int validate;
    BMPImage* bmp_image;
    //read in all the values for the BMP header
    validate = fread(&header,sizeof(header), 1, fp);

    //error handling for failure to read data
    if(validate != 1)
    {
        char* error_message = "fread failed to read the data";
        *error = malloc((strlen(error_message) + 1) * sizeof(**error));
        strcpy(*error, error_message);
        bmp_image = NULL;
        return bmp_image;
    }

    if(check_bmp_header(&header, fp))//continue forward if valid bmp_header
    {
        fseek(fp, header.offset, SEEK_SET);
        unsigned char* data = malloc(header.image_size_bytes);

        validate = fread(data, header.image_size_bytes, 1, fp);
        if(validate != 1) //error handling for failed read
        {
            char* error_message = "fread failed to read the data";
            *error = malloc((strlen(error_message) + 1) * sizeof(**error));//+1 for '\0' character
            strcpy(*error, error_message);
            bmp_image = NULL;
            return bmp_image;
        }

        //will execute the following if everything is a-ok:
        bmp_image = malloc(sizeof(*bmp_image));

        bmp_image -> header = header;
        bmp_image -> data = data;
    }
    else //error handling for failed bmp header
    {
        char* error_message = "BMPHeader is invalid!";
        *error = malloc((strlen(error_message) + 1) * sizeof(**error)); //+1 for '\0' character
        strcpy(*error, error_message);
        bmp_image = NULL;
    }
    return bmp_image;
}

bool check_bmp_header(BMPHeader* bmp_header, FILE* fp) //ensure the BMPHeader is valid
{

    rewind(fp); //ensure file is at the beginning

    //determine length of file
    fseek(fp, 0, SEEK_END);
    long length = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    int padding = 0;
    while((bmp_header -> width_px * 3 + padding) % 4 != 0) //calculate the padding
    {
        padding++;
    }

    //note: all this could've been done to take less sloc but would make it harder to debug with GDB.
    if(bmp_header -> type != 0x4d42) //test magic type
    {
        return false;
    }
    else if(bmp_header -> offset != BMP_HEADER_SIZE) //image data begins immediately after header
    {
        return false;
    }
    else if(bmp_header -> dib_header_size != DIB_HEADER_SIZE) //DIB header is right size
    {
        return false;
    }
    else if(bmp_header -> num_planes != 1) //only 1 image plane
    {
        return false;
    }

    else if(bmp_header -> compression != 0) //there is no compression
    {
        return false;
    }
    else if(bmp_header -> num_colors != 0 || bmp_header -> important_colors != 0) //num_colors == 0
    {
        return false;
    }
    else if(bmp_header -> bits_per_pixel != 24) //ensure correct number of bits per pixel..for this assignment it can only be 24
    {
        return false;
    }

    else if(bmp_header -> size != length || bmp_header -> image_size_bytes != ((bmp_header -> width_px) * 3 + padding) * bmp_header -> height_px)
    {
        return false;
    }
    else
    {
        return true;
    }
}




bool write_bmp(FILE* fp, BMPImage* image, char** error) //write an image to a binary file
{

    int validate;
    validate = fwrite(image, BMP_HEADER_SIZE, 1 ,fp); //write the header information

    //error handling for fwrite..fwrite should return 1
    if(validate != 1)
    {
        char* error_message = "FWRITE FAILED TO WRITE HEADER!";
        *error = malloc((strlen(error_message) + 1) * sizeof(**error));
        strcpy(*error, error_message);
        return false;
    }

    fwrite(image -> data, image -> header.image_size_bytes, 1, fp); //write the contents of the char* data field

    //error handling for fwrite..fwrite should return 1
    /*
    if(validate != 1)
    {
        char* error_message = "FWRITE FAILED TO WRITE IMAGE DATA!";
        *error = malloc((strlen(error_message) + 1) * sizeof(**error));
        strcpy(*error, error_message);
        return false;
    }
     */
    return true; //fwrite success
}

//crop an image starting from coordinate (x,y) w = new width size. h = new height size.
BMPImage* crop_bmp(BMPImage* image, int x, int y, int w, int h, char** error)
{
    BMPImage* new_image;

    //error handling
    if(x < 0 || y < 0 || w <= 0 || h <= 0)
    {
        char* message = "Input is out of bounds. Must be positive integers.";
        *error = malloc(strlen(message) + 1 * sizeof(**error));
        strcpy(*error, message);
        new_image = NULL;
        return new_image;
    }

    //error handling
    if(x + y >= image -> header.width_px || y + h >= image -> header.height_px)
    {
        char* message = "Cannot crop out of bounds of original image.";
        *error = malloc(strlen(message) + 1 * sizeof(**error));
        strcpy(*error, message);
        new_image = NULL;
        return new_image;
    }

    int h_coord = image -> header.height_px - y - h; //flip the h coord because the data in a bmp file is stored backwards.
    int w_coord = x;
    int width = w * 3;

    int orig_width = image -> header.image_size_bytes / image -> header.height_px;

    int new_padding = 0;
    while((w * 3 + new_padding) % 4!= 0)
    {
        new_padding++; //calc padding of new cropped image
    }
    int new_width =  w * 3 + new_padding;



    new_image = malloc(sizeof(*new_image));
    if(new_image == NULL)
    {
        char* message = "Creating memory for cropped image failed!";
        *error = malloc(strlen(message) + 1 * sizeof(**error));
        strcpy(*error, message);
        return NULL;
    }

    //assign fields of the cropped image
    new_image -> header = image -> header;
    new_image -> header.size = new_width * h + BMP_HEADER_SIZE;
    new_image -> header.width_px = w;
    new_image -> header.height_px = h;
    new_image -> header.image_size_bytes = new_image -> header.size - BMP_HEADER_SIZE;

    h_coord = h_coord * orig_width;
    w_coord = w_coord * 3;

    int total_offset = h_coord + w_coord;
    unsigned char* new_data = malloc(sizeof(*new_data) * new_image -> header.image_size_bytes);

    //error handling for memory allocation failrue
    if(new_data == NULL)
    {
        char* message = "Failed to allocate memory for new_data!";
        *error = malloc(strlen(message) + 1 * sizeof(**error));
        strcpy(*error, message);
        return NULL;
    }


    //initialize loop control variables
    int i = 0;
    int h_count = 0;
    int w_count = 0;
    unsigned char* orig_data = image -> data;

    //"copy and paste" necessary bytes from orig image to newly cropped image
    for(h_count = 0; h_count < h; h_count++)
    {
        for(w_count = 0; w_count < width + new_padding; w_count++)
        {
            if(w_count < width)
            {
                new_data[i] = orig_data[total_offset + i];
            }
            else
            {
                new_data[i] = 0;
            }

            i++;
        }

        for(w_count = 0; w_count < orig_width - w_coord - width - new_padding; w_count++)
        {
            total_offset++;
        }
        total_offset += w_coord;

    }

    new_image -> data = new_data;

    return new_image;

}

BMPImage* rgb_to_gray_bmp(BMPImage *image, char **error){

    BMPImage* new_image;

    int width = image -> header.width_px;
    int height = image -> header.height_px;

    new_image = malloc(sizeof(*new_image));

    unsigned char* new_data = malloc(sizeof(*new_data) * new_image -> header.image_size_bytes);

    //error handling for memory allocation failure
    if(new_data == NULL)
    {
        char* message = "Failed to allocate memory for new_data!";
        *error = malloc(strlen(message) + 1 * sizeof(**error));
        strcpy(*error, message);
        return NULL;
    }

    //initialize loop control variables
    int i = 0;
    int h_count = 0;
    int w_count = 0;
    unsigned char* orig_data = image -> data;

    //"copy and paste" necessary bytes from orig image to newly cropped image
    for(h_count = 0; h_count < height; h_count++)
    {
        for(w_count = 0; w_count < width; w_count++)
        {
            if(w_count < width)
            {
                new_data[i] = orig_data[i];
            }
            else
            {
                new_data[i] = 0;
            }

            i++;
        }
    }

    new_image -> data = new_data;

    return new_image;
}


void free_bmp(BMPImage* image) {
    free(image -> data);
    free(image);
}
