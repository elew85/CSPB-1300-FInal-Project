/*
main.cpp
CSPB 1300 Image Processing Application

PLEASE FILL OUT THIS SECTION PRIOR TO SUBMISSION

- Your name:
    Eric Lewis

- All project requirements fully met? (YES or NO):
    Yes

- If no, please explain what you could not get to work:
    N/A

- Did you do any optional enhancements? If so, please explain:
    Added as some input validation as well as protection against saving over input file. 
*/

#include <iostream>
#include <vector>
#include <fstream>
#include <cmath>
#include <string> 
using namespace std;

//***************************************************************************************************//
//                                DO NOT MODIFY THE SECTION BELOW                                    //
//***************************************************************************************************//

// Pixel structure
struct Pixel
{
    // Red, green, blue color values
    int red;
    int green;
    int blue;
};

/**
 * Gets an integer from a binary stream.
 * Helper function for read_image()
 * @param stream the stream
 * @param offset the offset at which to read the integer
 * @param bytes  the number of bytes to read
 * @return the integer starting at the given offset
 */ 
int get_int(fstream& stream, int offset, int bytes)
{
    stream.seekg(offset);
    int result = 0;
    int base = 1;
    for (int i = 0; i < bytes; i++)
    {   
        result = result + stream.get() * base;
        base = base * 256;
    }
    return result;
}

/**
 * Reads the BMP image specified and returns the resulting image as a vector
 * @param filename BMP image filename
 * @return the image as a vector of vector of Pixels
 */
vector<vector<Pixel>> read_image(string filename)
{
    // Open the binary file
    fstream stream;
    stream.open(filename, ios::in | ios::binary);

    // Get the image properties
    int file_size = get_int(stream, 2, 4);
    int start = get_int(stream, 10, 4);
    int width = get_int(stream, 18, 4);
    int height = get_int(stream, 22, 4);
    int bits_per_pixel = get_int(stream, 28, 2);

    // Scan lines must occupy multiples of four bytes
    int scanline_size = width * (bits_per_pixel / 8);
    int padding = 0;
    if (scanline_size % 4 != 0)
    {
        padding = 4 - scanline_size % 4;
    }

    // Return empty vector if this is not a valid image
    if (file_size != start + (scanline_size + padding) * height)
    {
        return {};
    }

    // Create a vector the size of the input image
    vector<vector<Pixel>> image(height, vector<Pixel> (width));

    int pos = start;
    // For each row, starting from the last row to the first
    // Note: BMP files store pixels from bottom to top
    for (int i = height - 1; i >= 0; i--)
    {
        // For each column
        for (int j = 0; j < width; j++)
        {
            // Go to the pixel position
            stream.seekg(pos);

            // Save the pixel values to the image vector
            // Note: BMP files store pixels in blue, green, red order
            image[i][j].blue = stream.get();
            image[i][j].green = stream.get();
            image[i][j].red = stream.get();

            // We are ignoring the alpha channel if there is one

            // Advance the position to the next pixel
            pos = pos + (bits_per_pixel / 8);
        }

        // Skip the padding at the end of each row
        stream.seekg(padding, ios::cur);
        pos = pos + padding;
    }

    // Close the stream and return the image vector
    stream.close();
    return image;
}

/**
 * Sets a value to the char array starting at the offset using the size
 * specified by the bytes.
 * This is a helper function for write_image()
 * @param arr    Array to set values for
 * @param offset Starting index offset
 * @param bytes  Number of bytes to set
 * @param value  Value to set
 * @return nothing
 */
void set_bytes(unsigned char arr[], int offset, int bytes, int value)
{
    for (int i = 0; i < bytes; i++)
    {
        arr[offset+i] = (unsigned char)(value>>(i*8));
    }
}

/**
 * Write the input image to a BMP file name specified
 * @param filename The BMP file name to save the image to
 * @param image    The input image to save
 * @return True if successful and false otherwise
 */
bool write_image(string filename, const vector<vector<Pixel>>& image)
{
    // Get the image width and height in pixels
    int width_pixels = image[0].size();
    int height_pixels = image.size();

    // Calculate the width in bytes incorporating padding (4 byte alignment)
    int width_bytes = width_pixels * 3;
    int padding_bytes = 0;
    padding_bytes = (4 - width_bytes % 4) % 4;
    width_bytes = width_bytes + padding_bytes;

    // Pixel array size in bytes, including padding
    int array_bytes = width_bytes * height_pixels;

    // Open a file stream for writing to a binary file
    fstream stream;
    stream.open(filename, ios::out | ios::binary);

    // If there was a problem opening the file, return false
    if (!stream.is_open())
    {
        return false;
    }

    // Create the BMP and DIB Headers
    const int BMP_HEADER_SIZE = 14;
    const int DIB_HEADER_SIZE = 40;
    unsigned char bmp_header[BMP_HEADER_SIZE] = {0};
    unsigned char dib_header[DIB_HEADER_SIZE] = {0};

    // BMP Header
    set_bytes(bmp_header,  0, 1, 'B');              // ID field
    set_bytes(bmp_header,  1, 1, 'M');              // ID field
    set_bytes(bmp_header,  2, 4, BMP_HEADER_SIZE+DIB_HEADER_SIZE+array_bytes); // Size of BMP file
    set_bytes(bmp_header,  6, 2, 0);                // Reserved
    set_bytes(bmp_header,  8, 2, 0);                // Reserved
    set_bytes(bmp_header, 10, 4, BMP_HEADER_SIZE+DIB_HEADER_SIZE); // Pixel array offset

    // DIB Header
    set_bytes(dib_header,  0, 4, DIB_HEADER_SIZE);  // DIB header size
    set_bytes(dib_header,  4, 4, width_pixels);     // Width of bitmap in pixels
    set_bytes(dib_header,  8, 4, height_pixels);    // Height of bitmap in pixels
    set_bytes(dib_header, 12, 2, 1);                // Number of color planes
    set_bytes(dib_header, 14, 2, 24);               // Number of bits per pixel
    set_bytes(dib_header, 16, 4, 0);                // Compression method (0=BI_RGB)
    set_bytes(dib_header, 20, 4, array_bytes);      // Size of raw bitmap data (including padding)                     
    set_bytes(dib_header, 24, 4, 2835);             // Print resolution of image (2835 pixels/meter)
    set_bytes(dib_header, 28, 4, 2835);             // Print resolution of image (2835 pixels/meter)
    set_bytes(dib_header, 32, 4, 0);                // Number of colors in palette
    set_bytes(dib_header, 36, 4, 0);                // Number of important colors

    // Write the BMP and DIB Headers to the file
    stream.write((char*)bmp_header, sizeof(bmp_header));
    stream.write((char*)dib_header, sizeof(dib_header));

    // Initialize pixel and padding
    unsigned char pixel[3] = {0};
    unsigned char padding[3] = {0};

    // Pixel Array (Left to right, bottom to top, with padding)
    for (int h = height_pixels - 1; h >= 0; h--)
    {
        for (int w = 0; w < width_pixels; w++)
        {
            // Write the pixel (Blue, Green, Red)
            pixel[0] = image[h][w].blue;
            pixel[1] = image[h][w].green;
            pixel[2] = image[h][w].red;
            stream.write((char*)pixel, 3);
        }
        // Write the padding bytes
        stream.write((char *)padding, padding_bytes);
    }

    // Close the stream and return true
    stream.close();
    return true;
}

//***************************************************************************************************//
//                                DO NOT MODIFY THE SECTION ABOVE                                    //
//***************************************************************************************************//


//
// YOUR FUNCTION DEFINITIONS HERE
//

/** Helper function - validate output file name
 * @param1 - name of input file name
 * @return - output file name if != input file name
*/ 

string validate_file_name(string input_file_name)
    {
        bool valid = false; 
        string output_file_name; 
        do
        {
            cout << "Enter output file name: " << endl;
            cin >> output_file_name; 
            if (input_file_name != output_file_name)
            {
                if (output_file_name.length() > 4 && 
                    output_file_name.substr(output_file_name.length() - 4) == ".bmp")
                {
                    valid = true; 
                }
                else cout << "\nMust save as .bmp file format." << endl; 
            }
            else { cout << "\nCannot save over input file name! Try again... " << endl; }
        }
        while (!valid);
               
        return output_file_name; 
    }


/**
 * Process 1 - Add vignette
 * @param input img, vector of pixels
 * @param output file name
 * @return new image file with vignette applied
*/
vector<vector<Pixel>> add_vignette(const vector<vector<Pixel>>& image, string i_file)
{
    cout << "\nVignette selected\n" << endl;
    string output_file = validate_file_name(i_file); 
    
    int num_rows = image.size(); 
    int num_cols = image[0].size(); 
    vector<vector<Pixel>> new_image(num_rows, vector<Pixel>(num_cols)); 
    for (int i = 0; i < num_rows; i++)
    {
        for (int j = 0; j < num_cols; j++)
        {
            // get original pixel color values
            int red_value = image[i][j].red;
            int green_value = image[i][j].green;
            int blue_value = image[i][j].blue;
            
            // find distance to the center
            double distance = sqrt(pow((j - num_cols/2.0), 2) + pow((i - num_rows/2.0), 2));
            double scaling_factor = (num_rows - distance) / num_rows;
            
            // set pixel values for new image
            new_image[i][j].red = red_value * scaling_factor;
            new_image[i][j].green = green_value * scaling_factor;
            new_image[i][j].blue = blue_value * scaling_factor;
        }
    }
    write_image(output_file, new_image);
    cout << "\nSuccessfully added vignette!" << endl;
    
    
    return new_image;
}

/**
  * Process 2 - Clarendon effect - lights lighter/darks darker
  * @param input image file
  * @param output file name
  * @return new image file
*/
vector<vector<Pixel>> add_clarendon(const vector<vector<Pixel>>& image, string i_file)
{
    cout << "\nAdd Clarendon selected\n" << endl;
    string output_file = validate_file_name(i_file); 
    
    // get scaling factor
    cout << "Enter a scaling factor: ";
    double scaling_factor;
    cin >> scaling_factor; 
    
    int num_rows = image.size(); 
    int num_cols = image[0].size(); 
    
    // var to store new img
    vector<vector<Pixel>> new_image(num_rows, vector<Pixel>(num_cols)); 
    
    for (int i = 0; i < num_rows; i++)
    {
        for (int j = 0; j < num_cols; j++)
        {
            // get original pixel color values
            int red_value = image[i][j].red;
            int green_value = image[i][j].green;
            int blue_value = image[i][j].blue;
            
            //find average of R,G,B values
            double average_value = (red_value + green_value + blue_value)/3.0;
            
            // If cell is light, make it lighter
            if (average_value >= 170)
            {
                new_image[i][j].red = 255 - (255 - red_value)*scaling_factor;
                new_image[i][j].green = 255 - (255 - green_value)*scaling_factor;
                new_image[i][j].blue = 255 - (255 - blue_value)*scaling_factor;
            }
            else if (average_value < 90)
            {
                new_image[i][j].red = red_value*scaling_factor;
                new_image[i][j].green = green_value*scaling_factor;
                new_image[i][j].blue = blue_value*scaling_factor;
            }
            else 
            {
                new_image[i][j].red = red_value;
                new_image[i][j].green = green_value;
                new_image[i][j].blue = blue_value;
            }

        }
    }
    
    // output new_img object as user-provided filename
    write_image(output_file, new_image); 
    cout << "\nSuccessfully added clarendon effect!" << endl;
    
    return new_image;
}

 // 

/**
  * Process 3 - Gray scale
  * @param - input file name
  * @param - output file name
  * @return - output file image, vector of Pixels
*/

vector<vector<Pixel>> gray_scale(const vector<vector<Pixel>>& image, string i_file)
{
    cout << "\nGray scale effect selected\n" << endl;
    string output_file = validate_file_name(i_file); 
    
    // get height and width of original image
    int num_rows = image.size(); 
    int num_cols = image[0].size(); 
    
    // initialize var to store new img
    vector<vector<Pixel>> new_image(num_rows, vector<Pixel>(num_cols)); 
    
    for (int i = 0; i < num_rows; i++)
    {
        for (int j = 0; j < num_cols; j++)
        {
            // get original pixel color values
            int red_value = image[i][j].red;
            int green_value = image[i][j].green;
            int blue_value = image[i][j].blue;
            
            //find average of R,G,B values
            double average_value = (red_value + green_value + blue_value)/3.0;
            
            // set RGB values to average
            new_image[i][j].red = average_value;
            new_image[i][j].green = average_value;
            new_image[i][j].blue = average_value;
        }
    }
    
    // output new_img object as user-provided filename
    write_image(output_file, new_image); 
    cout << "\nSuccessfully added gray scale effect!" << endl;
    
    return new_image;

}

 
/**  Process 4 - Rotates by 90 degrees
 * @param - input file name
 * @param - output file name
 * @return - output image file
*/

vector<vector<Pixel>> rotate_90(const vector<vector<Pixel>>& image, string o_file)
{       
    // get height and width of original image
    int num_rows = image.size(); 
    int num_cols = image[0].size(); 
    
    // initialize var to store new img; cols & rows swapped!
    vector<vector<Pixel>> new_image(num_cols, vector<Pixel>(num_rows)); 
    
    for (int i = 0; i < num_cols; i++)
    {
        for (int j = 0; j < num_rows; j++)
        {   
            // rotate image 90 degrees
            new_image[i][j] = image[(num_rows - 1) - j][i];
        }
    }
    
    // output new_img object as user-provided filename
    write_image(o_file, new_image); 
    
    return new_image;

}

/** Process 5 - Rotates by multiples of 90 degrees
  * @param - input file name
  * @param - number of times to rotate
  * @param - output file name
  * @return - new image vector<vector<Pixel>>
*/

vector<vector<Pixel>> rotate_90_multiple(const vector<vector<Pixel>>& image, string o_file)
{
    //prompt user for number of times to rotate
    int num_rotations = 0;
    cout << "How many times would you like to rotate this image? ";
    cin >> num_rotations; 
    int angle = num_rotations*90;
    
    // initialize var to store new img
    vector<vector<Pixel>> new_image; 
    
    // based on above, conditions to choose how many times to use rotate func
    if ( angle % 90 != 0 ) { cout << "Angle must be a multiple of 90 degrees." << endl; }
    else if ( angle % 360 == 0 ) { new_image = image; }
    else if ( angle % 360 == 90 ) { new_image = rotate_90(image, o_file); }
    else if ( angle % 360 == 180 ) { new_image = rotate_90(rotate_90(image, o_file), o_file); }
    else { new_image = rotate_90(rotate_90(rotate_90(image, o_file), o_file), o_file); }

    // output new_img object as user-provided filename
    write_image(o_file, new_image); 
    cout << "\nSuccessfully rotated image " <<  num_rotations << " times!" << endl;
    
    return new_image;
}

 /** Process 6 - Enlarges in the X and Y directions
  * @param - input file name
  * @param - x scaling factor
  * @param - y scaling factor
  * @param - output file name
  * @return - new image
*/

vector<vector<Pixel>> enlarge(const vector<vector<Pixel>>& image, string i_file)
{
    cout << "\nEnlarge image selected\n" << endl;
    string output_file = validate_file_name(i_file); 

    // get height and width of original image
    int num_rows = image.size(); 
    int num_cols = image[0].size(); 

    // Prompt user for scaling factors
    int x_scaling_factor; 
    int y_scaling_factor;
    cout << "Enter integer to enlarge in X direction: " << endl;
    cin >> x_scaling_factor; 
    cout << "Enter integer to englarge in Y direction: " << endl;
    cin >> y_scaling_factor; 
    
    // initialize var to store new img
    vector<vector<Pixel>> new_image(num_rows*y_scaling_factor, vector<Pixel>(num_cols*x_scaling_factor)); 
    
    for (int i = 0; i < num_rows*y_scaling_factor; i++)
    {
        for (int j = 0; j < num_cols*x_scaling_factor; j++)
        {   
            // write_new image
            new_image[i][j] = image[i/y_scaling_factor][j/x_scaling_factor];
        }
    }
    
    // output new_img object as user-provided filename
    write_image(output_file, new_image); 
    cout << "\nSuccessfully enlarged image!" << endl;
    
    return new_image;
}

/** Process 7 - Convert to high-contrast, B & W only
  * @param - input file name
  * @param - output file name
  * @return new image w/ high-contrast applied
*/

vector<vector<Pixel>> high_contrast(const vector<vector<Pixel>>& image, string i_file)
{
    cout << "\nHigh Contrast selected\n" << endl;
    string output_file = validate_file_name(i_file); 
    
    // Get size of original image
    int num_rows = image.size(); 
    int num_cols = image[0].size(); 

    // initialize new image object
    vector<vector<Pixel>> new_image(num_rows, vector<Pixel>(num_cols)); 


    for (int i = 0; i < num_rows; i++)
    {
        for (int j = 0; j < num_cols; j++)
        {
            // get original pixel color values
            int red_value = image[i][j].red;
            int green_value = image[i][j].green;
            int blue_value = image[i][j].blue;
            
            // average to find gray values
            double gray_value = (red_value + green_value + blue_value) / 3.0;
            
            // set pixel values for new image
            if (gray_value >= 255/2.0)
            {
                new_image[i][j].red = 255;
                new_image[i][j].green = 255;
                new_image[i][j].blue = 255; 
            }
            else 
            {
                new_image[i][j].red = 0; 
                new_image[i][j].green = 0; 
                new_image[i][j].blue = 0;
            }
        }
    }
    write_image(output_file, new_image);
    cout << "\nSuccessfully added high-contrast filter!" << endl;

    return new_image;
}

 /** Process 8 - Lighten image
  * @param - input file name
  * @param - output file name
  * @return - output image
  */
vector<vector<Pixel>> lighten_image(const vector<vector<Pixel>>& image, string i_file)
{
    cout << "\nLighten Image selected\n" << endl;
    string output_file = validate_file_name(i_file); 
    
    // get scaling factor
    cout << "Enter a scaling factor: ";
    double scaling_factor;
    cin >> scaling_factor; 
    
    // Get size of original image
    int num_rows = image.size(); 
    int num_cols = image[0].size(); 

    // initialize new image object
    vector<vector<Pixel>> new_image(num_rows, vector<Pixel>(num_cols)); 


    for (int i = 0; i < num_rows; i++)
    {
        for (int j = 0; j < num_cols; j++)
        {
            // get original pixel color values
            int red_value = image[i][j].red;
            int green_value = image[i][j].green;
            int blue_value = image[i][j].blue;
            
            // lighten image to scaling factor
            new_image[i][j].red = (255 - (255 - red_value) * scaling_factor);
            new_image[i][j].green = (255 - (255 - green_value) * scaling_factor);
            new_image[i][j].blue = (255 - (255 - blue_value) * scaling_factor); 
        }
    }
    write_image(output_file, new_image);
    cout << "\nSuccessfully lightened image!" << endl;

    return new_image;
}



 /** Process 9 - Darken image
  * @param - input file name
  * @param - output file name
  * @return - new image
  */
    
vector<vector<Pixel>> darken_image(const vector<vector<Pixel>>& image, string i_file)
{
    // prompt for output file name
    cout << "\nDarken Image selected\n" << endl;
    string output_file = validate_file_name(i_file);
    
    // get scaling factor
    cout << "Enter a scaling factor: ";
    double scaling_factor;
    cin >> scaling_factor; 
    
    // Get size of original image
    int num_rows = image.size(); 
    int num_cols = image[0].size(); 

    // initialize new image object
    vector<vector<Pixel>> new_image(num_rows, vector<Pixel>(num_cols)); 


    for (int i = 0; i < num_rows; i++)
    {
        for (int j = 0; j < num_cols; j++)
        {
            // get original pixel color values
            int red_value = image[i][j].red;
            int green_value = image[i][j].green;
            int blue_value = image[i][j].blue;
            
            // set new pixel to scaling factor
            new_image[i][j].red = red_value * scaling_factor;
            new_image[i][j].green = green_value * scaling_factor;
            new_image[i][j].blue = blue_value * scaling_factor; 
        }
    }
    write_image(output_file, new_image);
    cout << "\nSuccessfully darkened image!" << endl;

    return new_image;
}

/** Process 10 - Convert to only blk, wht, rd, blue, grn
 * @param - input file name
 * @param - output file name
 * @return - output image
 */ 
vector<vector<Pixel>> bwrgb(const vector<vector<Pixel>>& image, string i_file)
{
    cout << "\nB/W/R/G/B selected\n" << endl;
    string output_file = validate_file_name(i_file);
    
    // Get size of original image
    int num_rows = image.size(); 
    int num_cols = image[0].size(); 

    // initialize new image object
    vector<vector<Pixel>> new_image(num_rows, vector<Pixel>(num_cols)); 


    for (int i = 0; i < num_rows; i++)
    {
        for (int j = 0; j < num_cols; j++)
        {
            // get original pixel color values
            int red_value = image[i][j].red;
            int green_value = image[i][j].green;
            int blue_value = image[i][j].blue;
            
            // figure out which color has highest value
            int max_color = red_value;
            if (green_value > max_color)
            {
               max_color = green_value;
            }
            if (blue_value > max_color)
            {
               max_color = blue_value;  
            }
            
            // set new pixel values
            if (red_value + green_value + blue_value >= 550)
            {   
                new_image[i][j].red = 255;
                new_image[i][j].green = 255;
                new_image[i][j].blue = 255;
            }
            else if (red_value + green_value + blue_value <= 150)
            {   
                new_image[i][j].red = 0;
                new_image[i][j].green = 0;
                new_image[i][j].blue = 0;
            }
            else if (max_color == red_value)
            {
                new_image[i][j].red = 255;
                new_image[i][j].green = 0;
                new_image[i][j].blue = 0;
            }
            else if (max_color == green_value)
            {
                new_image[i][j].red = 0;
                new_image[i][j].green = 255;
                new_image[i][j].blue = 0;
            }
            else
            {
                new_image[i][j].red = 0;
                new_image[i][j].green = 0;
                new_image[i][j].blue = 255;
            }
        }
    }
    write_image(output_file, new_image);
    cout << "\nSuccessfully applied B/W/R/G/B to image!" << endl;

    return new_image;
}

int main()
{
    // Basic interface for user to select process and enter params including initial image and other args
    cout << "\n*****************************************************" << endl;
    cout << "\n\nEric Lewis\' CSPB 1300 Image Processing Application\n\n" << endl;
    cout << "*****************************************************" << endl;
    
    string input_file; 
    bool valid = false; // bool to validate filename input
    
    while (!valid)   // validate input
    {
        cout << "\nEnter a valid input filename: ";
        cin >> input_file; 
        if (!cin.fail()) // improve this condition check
        { 
            if (input_file.length() > 4 && input_file.substr(input_file.length() - 4) == ".bmp")
            {
                valid = true; cout << "\nFile accepted!" << endl; 
            }
            else { cout << "\nMust be .bmp file format." << endl; } 
        } 
        else { cout << "\nInvalid file. Please try again." << endl; valid = false;}
    }
    
    // save new img file in global scope
    vector<vector<Pixel>> input_img = read_image(input_file); 
    
    string menu = "\n-----------------------------------\n"
        "\nIMAGE PROCESSING MENU\n\n" 
        "0) Change image\n"
        "1) Vignette\n"
        "2) Clarendon\n"
        "3) Gray scale\n"
        "4) Rotate 90 degrees\n"
        "5) Rotate multiples of 90 degrees\n"
        "6) Enlarge\n"
        "7) High Contrast\n"
        "8) Lighten\n"
        "9) Darken\n"
        "10) Black, white, red, green, blue\n"
        "\n-----------------------------------\n"
        "\nEnter numeric menu selection (or Q to quit): \n";

    
    string menu_selection;
    bool menu_select_close = false; 
    
    while (!menu_select_close)
    {
        cout << menu; // prompt user with function menu
        cout << "(Current file: " + input_file + ")\n";
        cin >> menu_selection;
        if (menu_selection == "Q" || menu_selection == "q")
        {
            cout << "Quitting program..." << endl;
            menu_select_close = true; 
            return 1; 
        }
        else if (menu_selection == "0") 
        {
           cout << "\nDid you have another image in mind?\n" << endl;
           bool valid = false; 
           string new_file_name; 
           do
           {
               cout << "Enter new file name: " << endl;
               cin >> new_file_name; 
               if (new_file_name.length() > 4 && 
                   new_file_name.substr(new_file_name.length() - 4) == ".bmp")
               {
                   valid = true; 
               }
                   else cout << "File must be .bmp file format." << endl; 
           }
           while (!valid);
           input_file = new_file_name; 
           input_img = read_image(input_file);
        }
        else if (menu_selection == "1") { add_vignette(input_img, input_file); }
        else if (menu_selection == "2") { add_clarendon(input_img, input_file); }
        else if (menu_selection == "3") { gray_scale(input_img, input_file); } 
        else if (menu_selection == "4") 
        { 
            cout << "\nRotate 90 degrees selected\n" << endl;
            string output_file;
            output_file = validate_file_name(input_file); 
            
            rotate_90(input_img, output_file); 
            cout << "\nSuccessfully rotated image!" << endl;
        }
        else if (menu_selection == "5") 
        { 
            cout << "\nRotate multiple of 90 degrees selected\n" << endl;
            string output_file;
            output_file = validate_file_name(input_file); 
            
            rotate_90_multiple(input_img, output_file); 
        }
        else if (menu_selection == "6") { enlarge(input_img, input_file); }
        else if (menu_selection == "7") { high_contrast(input_img, input_file); }
        else if (menu_selection == "8") { lighten_image(input_img, input_file); }
        else if (menu_selection == "9") { darken_image(input_img, input_file); }
        else if (menu_selection == "10") { bwrgb(input_img, input_file); }
        else 
        {
            cout << menu_selection + " is not a valid menu option. " << endl;
            cout << "Please try again: " << endl;
        }
    }

    
    return 0;
}