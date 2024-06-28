// Sandbox_ImageToSound.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <vector>
#include <cmath>
#include <memory>
#include <chrono>
#include <algorithm>

#include <fftw3.h>
#include <raylib.h>
#include <sndfile.h>
#include "pffft.h"

// Mungkin perlu di lakukan Hann window sebelum di IFFT

// Karena saat ambil frame, perlu dilakukan hann, lalu di fft, lalu diambil output amplitude freq.
// ambil amplitue freq, ifft, tambah frame, ohh mungkin tidak perlu hann windowing?

// Convert image to grayscale
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

void image_to_frequency_data(const Image& image, fftw_complex* freq_data, size_t n) {
    Color* pixels = LoadImageColors(image);
    size_t pixel_count = image.width * image.height;

    for (size_t i = 0; i < n; i++) {
        if (i < pixel_count) {
            // Assuming grayscale image; take the red channel as intensity
            freq_data[i][0] = (double)pixels[i].r / 255.0;
        }
        else {
            freq_data[i][0] = 0.0;
        }
        freq_data[i][1] = 0.0; // Imaginary part
    }

    UnloadImageColors(pixels); // Free the image colors array
}

void ifft_calculation(fftw_complex* in, fftw_complex* out, size_t n) {
    //fftw_plan plan = fftw_plan_dft_1d(n, in, out, FFTW_BACKWARD, FFTW_ESTIMATE);
    fftw_plan plan = fftw_plan_dft_1d(n, in, out, FFTW_BACKWARD, FFTW_MEASURE);
    fftw_execute(plan);
    fftw_destroy_plan(plan);

    // Normalize the IFFT output
    for (size_t i = 0; i < n; i++) {
        out[i][0] /= n;
        out[i][1] /= n;
    }
}

void normalize_audio_data(fftw_complex* data, std::vector<short>& audio_data, size_t n) {
    double max_val = 0.0;
    for (size_t i = 0; i < n; i++) {
        if (std::abs(data[i][0]) > max_val) {
            max_val = std::abs(data[i][0]);
        }
    }
    double scale = 32767.0 / max_val;

    for (size_t i = 0; i < n; i++) {
        audio_data[i] = static_cast<short>(data[i][0] * scale);
    }
}

void save_as_wav(const std::vector<short>& audio_data, const std::string& filename, int sample_rate) {
    SF_INFO sfinfo;
    sfinfo.channels = 1;
    sfinfo.samplerate = sample_rate;
    sfinfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;

    SNDFILE* outfile = sf_open(filename.c_str(), SFM_WRITE, &sfinfo);
    if (!outfile) {
        std::cerr << "Error opening output file: " << sf_strerror(outfile) << std::endl;
        return;
    }

    sf_write_short(outfile, audio_data.data(), audio_data.size());
    sf_close(outfile);
}

int nearest_power_of_2(int n) {
    if (n < 1) return 1;

    int lowerPower = 1;
    while ((lowerPower << 1) <= n) {
        lowerPower <<= 1;
    }

    int higherPower = lowerPower << 1;

    if ((n - lowerPower) < (higherPower - n)) {
        return lowerPower;
    }
    else {
        return higherPower;
    }
}

//int main(void) {
//    const int screenWidth = 800;
//    const int screenHeight = 600;
//
//    InitWindow(screenWidth, screenHeight, "Image to Audio");
//
//    Image image = LoadImage("resources/Image/screenshotTirakat.png");
//    if (image.data == nullptr) {
//        std::cerr << "Failed to load image" << std::endl;
//        CloseWindow();
//        return -1;
//    }
//
//
//    // Allocate memory for grayscale data
//    unsigned char* grayscale_data = new unsigned char[image.width * image.height];
//
//    // Convert image to grayscale
//    image_to_grayscale(image, grayscale_data, image.width * image.height);
//
//    image.data = grayscale_data;
//    if (ExportImage(image, "test.png")) {
//        std::cout << "berhasil" << std::endl;
//    } else std::cout << "gagal" << std::endl;
//
//    // Use smart pointers for FFTW memory management
//    std::unique_ptr<fftw_complex[]> freq_data(static_cast<fftw_complex*>(fftw_malloc(sizeof(fftw_complex) * N)));
//    std::unique_ptr<fftw_complex[]> time_data(static_cast<fftw_complex*>(fftw_malloc(sizeof(fftw_complex) * N)));
//
//    image_to_frequency_data(image, freq_data.get(), N);
//    ifft_calculation(freq_data.get(), time_data.get(), N);
//
//
//    std::vector<short> audio_data(N);
//    normalize_audio_data(time_data.get(), audio_data, N);
//
//    save_as_wav(audio_data, "output.wav", 44100);
//
//    // FFTW memory is automatically freed by smart pointers
//
//    UnloadImage(image);  // Unload image data after processing
//
//    CloseWindow();
//
//    return 0;
//}


void UpdateDraw();
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
            //int new_height = 480;
            int new_height = 400;
            //int new_width = static_cast<int>((static_cast<float>(new_height) / image_ratio_old.height) * image_ratio_old.width);
            int new_width = static_cast<int>((static_cast<float>(new_height) / image_ratio_old.height) * image_ratio_old.width) / 1.0F;
            std::cout << "height: " << new_height << "\nwidth: " << new_width << std::endl;
            Ratio image_ratio_new = { new_width, new_height };

            ImageResize(&image_input, image_ratio_new.width, image_ratio_new.height);
            texture_input = LoadTextureFromImage(image_input);
        }

        UnloadDroppedFiles(dropped_file);

        // make grayscale for texture_output
        image_output = ImageCopy(image_input);
        //ConvertToGrayscale(&image_output);
        //ImageColorGrayscale(&image_output);

        ImageFormat(&image_output, PIXELFORMAT_UNCOMPRESSED_GRAYSCALE);
        //std::vector<uint8_t> gray(image_output.height * image_output.width);
        //for (size_t i = 0; i < image_output.height; i++) {
        //    for (size_t j = 0; j < image_output.width; j++) {
        //        uint8_t col = gray[i * image_output.width + j];     // STILL DOESNT WORK. BELUM COPY DATA IMAGE_OUTPUT
        //        float colf = col / 255.0F;
        //        gray[i * image_output.width + j] = static_cast<uint8_t>(colf * colf * 255);
        //    }
        //}
        ////image_output.data = gray.data();
        //image_output = {
        //    gray.data(),
        //    image_output.height,
        //    image_output.width,
        //    1,
        //    PIXELFORMAT_UNCOMPRESSED_GRAYSCALE
        //};
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

                //float amplitude = (float)image_data[y * width + x];
                //float amplitude = static_cast<float>(image_data[y * width + x]);
                //float amplitude = scale * (float)image_data[y * width + x];
                //float amplitude = scale * image_data[y * width + x] - 1.0;
                //float amplitude = scale * (1.0 - image_data[y * width + x]) - 1.0;

                float amplitude = scale * static_cast<float>(image_data[y * width + x]) / 255;
                //in[y][0] = amplitude * amplitude * amplitude;
                in[y][0] = amplitude * amplitude;
                //in[y][0] = amplitude;
                //in[y][0] = sqrtf(amplitude);
                //in[y][0] = scale * static_cast<float>(image_data[y * width + x]) / 255.0 * hann_coeff;
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



    //{
    //    // PFFFT
    //    int N = height;
    //    PFFFT_Setup* fft_setup = pffft_new_setup(N, PFFFT_REAL);
    //    float* pffft_in = (float*)malloc(sizeof(float) * N);
    //    float* pffft_out = (float*)malloc(sizeof(float) * N);
    //    for (int x = 0; x < width; x++) {
    //        for (int y = 0; y < height; y++) {
    //            pffft_in[y] = scale * static_cast<float>(image_data[y * width + x]) / 255.0;

    //        }

    //        pffft_transform(fft_setup, pffft_in, pffft_out, nullptr, PFFFT_BACKWARD);

    //        for (int y = 0; y < height / 2; y++) {
    //            audio_data.push_back(pffft_out[y]);
    //            pffft_in[y] = 0;
    //            pffft_out[y] = 0;
    //        }

    //        memset(pffft_in, 0, sizeof(float) * N);
    //        memset(pffft_out, 0, sizeof(float) * N);
    //    }

    //    pffft_destroy_setup(fft_setup);
    //    free(pffft_in);
    //    free(pffft_out);
    //}

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
        sfinfo.samplerate = 44100;
        sfinfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
        SNDFILE* sndfile = sf_open(filename, SFM_WRITE, &sfinfo);
        if (!sndfile) {
            std::cerr << "Error opening sound file: " << sf_strerror(sndfile) << std::endl;
            return;
        }
        sf_writef_short(sndfile, short_audio_data.data(), short_audio_data.size());
        sf_close(sndfile);
    }

    //{
    //    // Save audio data to file using libsndfile
    //    int sampleRate = 44100;

    //    SF_INFO sfinfo;
    //    sfinfo.channels = 1;
    //    sfinfo.samplerate = sampleRate;
    //    sfinfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
    //    SNDFILE* sndfile = sf_open(filename, SFM_WRITE, &sfinfo);
    //    if (!sndfile) {
    //        std::cerr << "Error opening sound file: " << sf_strerror(sndfile) << std::endl;
    //        return;
    //    }
    //    sf_writef_double(sndfile, audio_data.data(), audio_data.size());
    //    //sf_writef_double(sndfile, downsampled_audio_data.data(), downsampled_audio_data.size());
    //    sf_close(sndfile);
    //}

}

//void ImageToSound(const Image& image, const char* filename) {
//    std::cout << "Starting ImageToSound..." << std::endl;
//
//    // Start timer
//    auto start = std::chrono::high_resolution_clock::now();
//
//    int width = image.width;
//    int height = image.height;
//    Color* pixels = LoadImageColors(image);
//    int sampleRate = 44100;
//    int duration = 5; // seconds
//    int numSamples = sampleRate * duration;
//    std::vector<double> audioData(numSamples, 0.0);
//
//    for (int y = 0; y < height; ++y) {
//        for (int x = 0; x < width; ++x) {
//            int index = y * width + x;
//            float intensity = pixels[index].r / 255.0f;
//            float frequency = 440.0f + 440.0f * intensity; // Map intensity to frequency range
//            for (int t = 0; t < numSamples; ++t) {
//                audioData[t] += 0.5 * sin(2.0 * PI * frequency * t / sampleRate);
//            }
//        }
//    }
//
//    // Normalize audio data
//    double maxAmplitude = *std::max_element(audioData.begin(), audioData.end());
//    for (double& sample : audioData) {
//        sample /= maxAmplitude;
//    }
//
//    // Save audio data to file using libsndfile
//    SF_INFO sfinfo;
//    sfinfo.channels = 1;
//    sfinfo.samplerate = sampleRate;
//    sfinfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
//    SNDFILE* sndfile = sf_open(filename, SFM_WRITE, &sfinfo);
//    if (!sndfile) {
//        std::cerr << "Error opening sound file: " << sf_strerror(sndfile) << std::endl;
//        return;
//    }
//    sf_write_double(sndfile, audioData.data(), numSamples);
//    sf_close(sndfile);
//
//    // Stop timer and print completion message
//    auto stop = std::chrono::high_resolution_clock::now();
//    auto duration_time = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
//    std::cout << "ImageToSound completed in " << duration_time.count() << " milliseconds." << std::endl;
//}


//Image image = LoadImage("resources/Image/screenshotTirakat.png");


// THE PLAN TO MAKE IMAGE TO SOUND APP
// - Make grayscale image.
// - Load the height data from left to right
// - The loaded height then get modified to make it power of 2, either by zero padding or resize it.
// - put the coef
// - If slighly bigger than 1024 or any other power of 2, then resize it smaller. to make it 1820 x 1024.
// - If smaller than 1024 or any other power of 2, then do zero padding.
// - make 2 complex: in & out
// - use in for ifft by fftw backward. then add the output copy to the vector of audio signal.
// - write raw wav the audio signal using libsndfile in mono channel.
//



// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
