// Sandbox_ImageToSound.cpp : This file contains the 'main' function. Program execution begins and ends there.
// Author : Ukhem Fahmi Thoriqul Haq
// Title  : Image to Sound
// Made   : 27 - 05 - 2024
//

// TODO:
// 1. Add padding in the image data as border, i think border will make the image is more consistent to fft analyzed.
// 
//

#include <iostream>
#include <vector>
#include <cmath>
#include <memory>
#include <chrono>
#include <algorithm>
#include <deque>

#include <fftw3.h>
#include <raylib.h>
#include <sndfile.h>


void UpdateDraw();
void image_to_grayscale(const Image& image, unsigned char* grayscale_data, size_t n);
void ConvertToGrayscale(Image* image);
void ImageToSound(const Image& image, const char* filename);

struct Ratio {
    int width{};
    int height{};
};

Vector2 screen{ 1200, 700 };
Image image_input{};
Image image_output{};
Texture2D texture_input{};
Texture2D texture_output{};


int main() {

    InitWindow(screen.x, screen.y, "Image to Audio");

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground({ 20,20,20,255 });

        UpdateDraw();

        EndDrawing();
    }
}

void image_to_grayscale(const Image& image, unsigned char* grayscale_data, size_t n) {
    Color* pixels = (Color*)image.data;
    size_t pixel_count = image.width * image.height;

    for (size_t i = 0; i < n; i++) {
        if (i < pixel_count) {
            // Convert color to grayscale using luminance formula
            grayscale_data[i] = (unsigned char)(0.299f * pixels[i].r + 0.587f * pixels[i].g + 0.114f * pixels[i].b);
        }
        else {
            grayscale_data[i] = 0; // Set to 0 for padding
        }
    }
}

void ConvertToGrayscale(Image* image) {
    Color* pixels = LoadImageColors(*image);

    for (int i = 0; i < image->width * image->height; i++) {
        unsigned char gray = (unsigned char)(0.299f * pixels[i].r + 0.587f * pixels[i].g + 0.114f * pixels[i].b);
        pixels[i].r = gray;
        pixels[i].g = gray;
        pixels[i].b = gray;
        pixels[i].a = 255; // Ensure alpha is fully opaque
    }

    Image new_image = {
        .data = pixels,
        .width = image->width,
        .height = image->height,
        .mipmaps = 1,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
    };

    *image = new_image;
}

void UpdateDraw() {
    float pad = 50;
    Rectangle panel_base{
        0 + pad,
        0 + pad * 2,
        screen.x - (pad * 2),
        screen.y - (pad * 2 * 2)
    };
    //DrawRectangleLinesEx(panel_base, 5, BLACK);

    //float coef = 0.975F;
    float rect_w_base = panel_base.width / 2;
    if (rect_w_base < panel_base.height) rect_w_base -= (pad / 8);
    else rect_w_base = panel_base.height - (pad / 8);
    float rect_h_base = rect_w_base * 10 / 16;

    Rectangle panel_input{
        panel_base.x,
        panel_base.y + (panel_base.height - rect_h_base) / 2,
        rect_w_base,
        rect_h_base
    };

    Rectangle panel_output{
        panel_base.x + panel_base.width - rect_w_base,
        panel_input.y,
        rect_w_base,
        rect_h_base
    };

    DrawRectangleRec(panel_input, DARKGRAY);
    DrawRectangleRec(panel_output, DARKGRAY);

    if (IsFileDropped()) {
        FilePathList dropped_file = LoadDroppedFiles();

        const char* c_file_path = dropped_file.paths[0];
        std::string cpp_file_path = std::string(c_file_path);

        if (IsFileExtension(c_file_path, ".png") || IsFileExtension(c_file_path, ".jpg")) // I dont know why jpg doesnt work.
        {
            image_input = LoadImage(c_file_path);
            //if (image_input.height)
            Ratio image_ratio_old = { image_input.width, image_input.height };
            //int new_height = nearest_power_of_2(image_input.height);
            //int new_height = (1 << 9);
            int new_height = 480;
            //int new_height = 400;
            //int new_width = static_cast<int>((static_cast<float>(new_height) / image_ratio_old.height) * image_ratio_old.width);
            int new_width = static_cast<int>((static_cast<float>(new_height) / image_ratio_old.height) * image_ratio_old.width) / 1.0F;
            std::cout << "height: " << new_height << "\nwidth: " << new_width << std::endl;
            Ratio image_ratio_new = { new_width, new_height };

            ImageResize(&image_input, image_ratio_new.width, image_ratio_new.height);
            texture_input = LoadTextureFromImage(image_input);

            // TODO: Add padding as border.
            {
                int pad = 5;
                int new_height_bordered = new_height - (pad * 1);
                int new_width_bordered = new_width;

                Image image_process = ImageCopy(image_input);

                //ImageResize(&image_process, new_height_bordered, new_width_bordered);

                Color* pixels = LoadImageColors(image_process);

                std::vector<Color> color_data_input{};
                for (int i = 0; i < new_height_bordered * new_width; ++i) {
                    color_data_input.push_back(pixels[i]);
                }

                //Color pad_color = RAYWHITE;
                Color pad_color = WHITE;
                //Color pad_color = { 20,20,20,255 };
                //Color pad_color = LIGHTGRAY;

                //// PAD ATAS 
                //for (size_t i = 0; i < new_width_bordered * pad; i++) {
                //    color_data_input.insert(color_data_input.begin(), pad_color);
                //}

                // PAD BAWAH
                for (size_t i = 0; i < new_width_bordered * pad; i++) {
                    color_data_input.emplace_back(pad_color);
                }


                Image image_processed = {
                    color_data_input.data(),
                    new_width,
                    new_height,
                    1,
                    PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
                };


                texture_input = LoadTextureFromImage(image_processed);
                image_output = ImageCopy(image_processed);
            }
        }

        UnloadDroppedFiles(dropped_file);

        // make grayscale for texture_output
        //image_output = ImageCopy(image_input);
        //ConvertToGrayscale(&image_output);
        //ImageColorGrayscale(&image_output);
        ImageFormat(&image_output, PIXELFORMAT_UNCOMPRESSED_GRAYSCALE);

        texture_output = LoadTextureFromImage(image_output);
        ImageFlipVertical(&image_output);


    }

    if (texture_input.height != 0) {
        // Draw input
        {
            Rectangle source{
                0,0,(float)texture_input.width, (float)texture_input.height
            };
            Rectangle dest{ panel_input };
            DrawTexturePro(texture_input, source, dest, { 0,0 }, 0, WHITE);
        }


        // Draw output in grayscale
        {
            Rectangle source{
                0,0,(float)texture_output.width, (float)texture_output.height
            };
            Rectangle dest{ panel_output };
            DrawTexturePro(texture_output, source, dest, { 0,0 }, 0, WHITE);
        }
    }

    if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_S)) {
        
        ExportImage(image_output, "output.png");
        ImageToSound(image_output, "output.wav");
    }

}

void ImageToSound(const Image& image, const char* filename) {
    int height = image.height;
    int width = image.width;
    int N = (height * 2);
    //int N = height;
    std::cout << N << std::endl;

    std::vector<double> audio_data{};
    const double scale = 2.0 / 255.0;

    unsigned char* image_data = (unsigned char*)image.data;

    {
        // FFTW
        fftw_complex* in = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * N);
        fftw_complex* out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * N);

        for (int x = 0; x < width; x++) {
            for (int y = 0; y < height; y++) {

                float amplitude = scale * static_cast<float>(image_data[y * width + x]) / 255;
                //in[y][0] = amplitude * amplitude * amplitude;
                //in[y][0] = amplitude * amplitude;
                //in[y][0] = amplitude;
                in[y][0] = sqrtf(amplitude);
                in[y][1] = 0;
            }

            fftw_plan plan = fftw_plan_dft_1d(N, in, out, FFTW_BACKWARD, FFTW_MEASURE);
            fftw_execute(plan);

            for (int y = 0; y < height; y++) {
                audio_data.push_back(out[y][0]);
                in[y][0] = 0;
                out[y][0] = 0;
            }

            fftw_destroy_plan(plan);
        }

        fftw_free(in);
        fftw_free(out);
    }

    // Normalize audio data
    double max_value = *std::max_element(audio_data.begin(), audio_data.end());
    for (auto& sample : audio_data) {
        sample /= max_value;
    }

    std::cout << "Original audio size: " << audio_data.size() << std::endl;

    // Downsampling to make it shorter
    std::vector<double> downsampled_audio_data{};
    for (size_t i = 0; i < audio_data.size(); i += 1) {
        downsampled_audio_data.push_back(audio_data.at(i));
    }

    std::cout << "Downsampled audio size: " << downsampled_audio_data.size() << std::endl;

    {
        std::vector<short> short_audio_data;
        short_audio_data.reserve(downsampled_audio_data.size());
        for (double sample : downsampled_audio_data) {
            // Scale and clamp to valid short integer range (-32768 to 32767)
            int scaled_sample = static_cast<int>(sample * 32767.0);
            scaled_sample = std::clamp(scaled_sample, -32768, 32767);
            short_audio_data.push_back(static_cast<short>(scaled_sample));
        }

        // Save short audio data using libsndfile
        SF_INFO sfinfo;
        sfinfo.channels = 1;
        sfinfo.samplerate = 48000;
        sfinfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
        SNDFILE* sndfile = sf_open(filename, SFM_WRITE, &sfinfo);
        if (!sndfile) {
            std::cerr << "Error opening sound file: " << sf_strerror(sndfile) << std::endl;
            return;
        }
        sf_writef_short(sndfile, short_audio_data.data(), short_audio_data.size());
        sf_close(sndfile);
    }

}



// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
