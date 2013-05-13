#include <stdio.h>
#include <stdint.h>

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <iostream>
#include <vector>
#include <algorithm>


#include "celfile.h"


void get_pal(std::string pal_filename, colour* pal)
{
    FILE * pal_file;

    pal_file = fopen(pal_filename.c_str(), "rb");
    
    for(int i = 0; i < 256; i++)
    {
            fread(&pal[i].r, 1, 1, pal_file);
            fread(&pal[i].g, 1, 1, pal_file);
            fread(&pal[i].b, 1, 1, pal_file);
    }

    fclose(pal_file);
}

size_t get_num_frames(FILE* cel_file)
{
    uint32_t num_frames;
    
    fread(&num_frames, 4, 1, cel_file);
    std::cout << ftell(cel_file) << ": Num frames: " << num_frames << std::endl;

    return num_frames;
}

void get_frame_offsets(FILE* cel_file, uint32_t* frame_offsets, size_t num_frames)
{
    for(int i = 0; i < num_frames; i++){
            fread(&frame_offsets[i], 4, 1, cel_file);
            std::cout << ftell(cel_file) << ": offset " << i << ": " << frame_offsets[i] << std::endl;
    }

    fread(&frame_offsets[num_frames], 4, 1, cel_file);
    std::cout << ftell(cel_file) << ": end offset: " << frame_offsets[num_frames] << std::endl;
}

void fix_image(std::vector<colour>& raw_image, size_t width)
{
    for(int i = 0; i < raw_image.size()/2; i++)
    {
        colour tmp = raw_image[i];
        raw_image[i] = raw_image[raw_image.size() - i];
        raw_image[raw_image.size() - i] = tmp;
    }

    for(int i = 0; i < raw_image.size(); i++)
    {
        int x = i%width;
        if(x < width/2)
        {
            colour tmp = raw_image[i];
            raw_image[i] = raw_image[(i-x+width) - x];
            raw_image[(i-x+width) - x] = tmp;
        }
    }
}

void print_cel(uint8_t* frame, int size)
{
    for(int i = 0; i < size; i++)
    {
        std::cout << i << ": " << (unsigned int) frame[i] << std::endl;
    }
}

void fill_t(size_t pixels, std::vector<colour>& raw_image)
{
    for(int px = 0; px < pixels; px++)
        raw_image.push_back(colour(255, 255, 255, false));
}


bool greater_than_first(uint8_t* frame, size_t frame_size)
{
    return frame_size >= 196 &&
    frame[  2] == 0 && frame[  3] == 0 &&
    frame[ 14] == 0 && frame[ 15] == 0 &&
    frame[ 34] == 0 && frame[ 35] == 0 &&
    frame[ 62] == 0 && frame[ 63] == 0 &&
    frame[ 98] == 0 && frame[ 99] == 0 &&
    frame[142] == 0 && frame[143] == 0 &&
    frame[194] == 0 && frame[195] == 0;
}

bool greater_than_second(uint8_t* frame, size_t frame_size)
{
    return frame_size >= 196 &&
    frame[254] == 0 && frame[255] == 0 &&
    frame[318] == 0 && frame[319] == 0 &&
    frame[374] == 0 && frame[375] == 0 &&
    frame[422] == 0 && frame[423] == 0 &&
    frame[462] == 0 && frame[463] == 0 &&
    frame[494] == 0 && frame[495] == 0 &&
    frame[518] == 0 && frame[519] == 0 &&
    frame[534] == 0 && frame[535] == 0;
}

bool is_greater_than(uint8_t* frame, size_t frame_size)
{
    return greater_than_first(frame, frame_size);
}

void decode_greater_than(uint8_t* frame, size_t frame_size, colour* pal, std::vector<colour>& raw_image)
{
    int segment;
    int i = 0;

    raw_image.push_back(pal[frame[i]]);
    i++;
    raw_image.push_back(pal[frame[i]]);
    i++;
   
    fill_t(30, raw_image);

    for(segment = 0; segment < 7; segment++)
    {
        i += 2;

        int xdraw = (segment+1)*4;
        int xoffs = 0;

        std::cout << "\tdraw: " << xdraw << std::endl;

        int px; 
        for(px = xoffs; px < xdraw; px++)
        {
                raw_image.push_back(pal[frame[i]]);
                i++;
        }
        fill_t(32-xdraw, raw_image);
        

        
        xdraw = (segment+1)*4 +2;
        xoffs = 0;

        std::cout << "\tdraw: " << xdraw << std::endl;

        
        for(px = xoffs; px < xdraw; px++)
        {
                raw_image.push_back(pal[frame[i]]);
                i++;
        }
        fill_t(32-xdraw, raw_image);
        
        
       
        std::cout << "len: " << raw_image.size() << std::endl;
    }
    
    if(greater_than_second(frame, frame_size))
    {
        for(; segment < 15; segment++)
        {
            i += 2;

            int xdraw = (15-segment)*4;
            int xoffs = 0;

            std::cout << "\tdraw: " << xdraw << std::endl;

            int px; 
            for(px = xoffs; px < xdraw; px++)
            {
                    raw_image.push_back(pal[frame[i]]);
                    i++;
            }
            fill_t(32-xdraw, raw_image);
            

            
            xdraw = ((15-segment)*4) -2;
            xoffs = 0;

            std::cout << "\tdraw: " << xdraw << std::endl;

            
            for(px = xoffs; px < xdraw; px++)
            {
                    raw_image.push_back(pal[frame[i]]);
                    i++;
            }
            fill_t(32-xdraw, raw_image);
            
            
           
            std::cout << "len: " << raw_image.size() << std::endl;
        }
    }
    else
    {
        for(int i = 256; i < frame_size; i++)
            raw_image.push_back(pal[frame[i]]);
    }

}

bool less_than_first(uint8_t* frame, size_t frame_size)
{
    return frame_size >= 226 &&
    frame[  0] == 0 && frame[  1] == 0 &&
    frame[  8] == 0 && frame[  9] == 0 &&
    frame[ 24] == 0 && frame[ 25] == 0 &&
    frame[ 48] == 0 && frame[ 49] == 0 &&
    frame[ 80] == 0 && frame[ 81] == 0 &&
    frame[120] == 0 && frame[121] == 0 &&
    frame[168] == 0 && frame[169] == 0 &&
    frame[224] == 0 && frame[225] == 0;
}

bool less_than_second(uint8_t* frame, size_t frame_size)
{
    return frame_size >= 530 &&
    frame[288] == 0 && frame[289] == 0 &&
    frame[348] == 0 && frame[349] == 0 &&
    frame[400] == 0 && frame[401] == 0 &&
    frame[444] == 0 && frame[445] == 0 &&
    frame[480] == 0 && frame[481] == 0 &&
    frame[508] == 0 && frame[509] == 0 &&
    frame[528] == 0 && frame[529] == 0;
}

bool is_less_than(uint8_t* frame, size_t frame_size)
{
    return less_than_first(frame, frame_size);
}





void decode_less_than(uint8_t* frame, size_t frame_size, colour* pal, std::vector<colour>& raw_image)
{
    int segment;
    int i = 0;

    for(segment = 0; segment < 8; segment++)
    {
        i += 2;

        int xdraw = segment*4 +2;
        int xoffs = 32 - xdraw;

        std::cout << "\tdraw: " << xdraw << std::endl;

           
        fill_t(xoffs, raw_image);
        for(int px = xoffs; px < 32; px++)
        {
                raw_image.push_back(pal[frame[i]]);
                i++;
        }
        

        
        xdraw = segment*4 +4;
        xoffs = 32 - xdraw;

        std::cout << "\tdraw: " << xdraw << std::endl;

        
        fill_t(xoffs, raw_image);
        for(int px = xoffs; px < 32; px++)
        {
                raw_image.push_back(pal[frame[i]]);
                i++;
        }
        
        
       
        std::cout << "len: " << raw_image.size() << std::endl;
    }
    
    if(less_than_second(frame, frame_size))
    {
        for(; segment < 16; segment++)
        {
            i += 2;

            int xdraw = (15-segment)*4 +2;
            int xoffs = 32 - xdraw;

            std::cout << "\tdraw2: " << xdraw << std::endl;

            fill_t(xoffs, raw_image);
               
            for(int px = xoffs; px < 32; px++)
            {
                    raw_image.push_back(pal[frame[i]]);
                    i++;
            }
            

            
            xdraw = (15-segment)*4;
            xoffs = 32 - xdraw;

            std::cout << "\tdraw2: " << xdraw << std::endl;

            
            fill_t(xoffs, raw_image);
            for(int px = xoffs; px < 32; px++)
            {
                    raw_image.push_back(pal[frame[i]]);
                    i++;
            }
            
            
           
            std::cout << "len2: " << raw_image.size() << std::endl;
        }
    }
    else
    {
        for(int i = 256; i < frame_size; i++)
            raw_image.push_back(pal[frame[i]]);
    }

}

int32_t normal_width(uint8_t* frame, size_t frame_size, bool from_header, uint16_t offset)
{
    
    // If we have a header, we know that offset points to the start of the 32nd line.
    // So, when we reach that point, we will have produced 31 lines of pixels, so we 
    // can divide the number of pixels we have passed at this point by 31, to get the 
    // width.
    if(from_header)
    {
        int32_t width_header = 0; 
        
        for(size_t i = 11; i < frame_size; i++){
            
            if(i == offset && from_header)
            {
                width_header = width_header/31;
                break;
            }
            // Regular command
            if(frame[i] <= 127){
                width_header += frame[i];
                i += frame[i];
            }

            // Transparency command
            else if(128 <= frame[i]){
                width_header += 256 - frame[i];
            }
        }

        return width_header;
    }
    
    // If we do not have a header we probably (definitely?) don't have any transparency.
    // The maximum stretch of opaque pixels following a command byte is 127.
    // Since commands can't wrap over lines (it seems), if the width is shorter than 127,
    // the first (command) byte will indicate an entire line, so it's value is the width.
    // If the width is larger than 127, it will be some sequence of 127 byte long stretches,
    // followed by some other value to bring it to the end of a line (presuming the width is
    // not divisible by 127).
    // So, for all image except those whose width is divisible by 127, we can determine width
    // by looping through control bits, adding 127 each time, until we find some value which
    // is not 127, then add that to the 127-total and that is our width.
    else
    {
        int32_t width_reg = 0;
        
        for(size_t i = 0; i < frame_size; i++){

            // Regular command
            if(frame[i] <= 127){
                width_reg += frame[i];
                i += frame[i];
            }

            // Transparency command - who knows, it might be possible
            else if(128 <= frame[i]){
                width_reg += 256 - frame[i];
            }

            if(frame[i] != 127)
                break;

        }

        return width_reg;
    }
}


bool normal_decode(uint8_t* frame, size_t frame_size, size_t width, bool from_header, colour* pal, std::vector<colour>& raw_image)
{
    size_t i;

    // Skip the header if it exists
    if(from_header)
        i = 11;
    else
        i = 0;
    
    for(; i < frame_size; i++){

        // Regular command
        if(frame[i] <= 127)
        {
            std::cout << i << " regular: " << (int)frame[i] << std::endl;
            
            // Just push the number of pixels specified by the command
            for(size_t j = 1; j < frame[i]+1; j++)
                raw_image.push_back(pal[frame[i+j]]);
            
            i+= frame[i];
        }

        // Transparency command
        else if(128 <= frame[i])
        {
            std::cout << i << " transparency: " << (int)frame[i] << " " << (256 - frame[i]) << std::endl;
            
            // Push (256 - command value) transparent pixels
            for(size_t j = 0; j < 256-frame[i]; j++)
                raw_image.push_back(colour(255, 0, 255, false));
        }

    }

    return true;
}

size_t decode_raw_32(uint8_t* frame, size_t frame_size, colour* pal, std::vector<colour>& raw_image)
{

    for(int i = 0; i < frame_size; i++)
    {
        raw_image.push_back(pal[frame[i]]);
    }

    return 32;
}

size_t Cel_file::get_frame(FILE* cel_file, colour* pal, uint32_t* frame_offsets, size_t frame_num, std::vector<colour>& raw_image, bool is_tile_cel, size_t width_override)
{
    size_t frame_size = frame_offsets[frame_num+1] - frame_offsets[frame_num];
    std::cout << std::endl << "frame 0 size: " << frame_size << std::endl;

    

    // Load frame data
    uint8_t frame[frame_size];
    fseek(cel_file, frame_offsets[frame_num], SEEK_SET);
    fread(&frame[0], 1, frame_size, cel_file);

    // Make sure we're not concatenating onto some other image 
    raw_image.clear();
    

    //print_cel(frame, frame_size);
    //std::cout << std::endl;
   
    size_t width;
    
    if(is_tile_cel)
        width = 32;

    if(is_tile_cel && is_less_than(frame, frame_size))
        decode_less_than(frame, frame_size, pal, raw_image);
    
    else if(is_tile_cel && is_greater_than(frame, frame_size))
        decode_greater_than(frame, frame_size, pal, raw_image);
    
    else
    {
        uint16_t offset;
        bool from_header = false;
        
        // Tile cel frames never have headers 
        if(!is_tile_cel)
        { 
            // The frame has a header which we can use to determine width
            if(frame[0] == 10)
            {
                from_header = true;
                fseek(cel_file, frame_offsets[frame_num]+2, SEEK_SET);
                fread(&offset, 2, 1, cel_file);
            }
            
            width = normal_width(frame, frame_size, from_header, offset);
        }
        
        normal_decode(frame, frame_size, width, from_header, pal, raw_image);
        
        if(is_tile_cel && raw_image.size() != 32*32) // It's a fully opaque raw frame, width 32, from a level tileset
        {
            raw_image.clear();
            decode_raw_32(frame, frame_size, pal, raw_image);
        }
        

    }
    
    fix_image(raw_image, width);
  
    std::cout << "WIDTH used: " << width << std::endl;


    return width;
}


//size_t Cel_file::get_frame(FILE* cel_file, colour* pal, uint32_t* frame_offsets, size_t frame_num, std::vector<colour>& raw_image, bool is_tile_cel, size_t width_override)
size_t Cel_file::get_frame(size_t frame_num, std::vector<colour>& raw_image)
{
    return get_frame(mFile, mPal, mFrame_offsets, frame_num, raw_image, mIs_tile_cel);
}

bool ends_with(const std::string& full, const std::string& end)
{
    return end.size() <= full.size() && full.substr(full.size() - end.size(), end.size()) == end;
}

std::string replace_end(const std::string& old_end, const std::string& new_end, const std::string& original)
{
    std::string retval = original.substr(0, original.size() - old_end.size());
    retval.append(new_end);
    return retval;
}

bool is_tile_cel(const std::string& file_name)
{
    return 
    ends_with(file_name, "l1.cel") ||
    ends_with(file_name, "l2.cel") ||
    ends_with(file_name, "l3.cel") ||
    ends_with(file_name, "l4.cel") ||
    ends_with(file_name, "town.cel");
}

size_t Cel_file::get_num_frames()
{
    return mNum_frames;
}

void get_pallette(std::string filename, colour* pal)
{
    std::string pal_filename;
    if(ends_with(filename, "l1.cel"))
        pal_filename = replace_end("l1.cel", "l1.pal", filename);
    else
        pal_filename = "diablo.pal";

    
    return get_pal(pal_filename, pal);
}

Cel_file::Cel_file(std::string filename)
{
    mFile = fopen(filename.c_str(), "rb");

    mPal = new colour[256];
    //get_pal("diablo.pal", mPal);
    get_pallette(filename, mPal);

    mNum_frames = ::get_num_frames(mFile);

    mFrame_offsets = new uint32_t[mNum_frames+1]; 
    get_frame_offsets(mFile, mFrame_offsets, mNum_frames);
    
    mIs_tile_cel = is_tile_cel(filename);
}


