// Incognito.cpp : This file contains the 'main' function. Program execution begins and ends there.
// Author : Ukhem Fahmi Thoriqul Haq
// Title  : Incognito : Image to Audio Converter
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
#include <cctype>
#include <filesystem>

#include <raylib.h>
#include <fftw3.h>
#include <sndfile.h>


// Declare the GetKeyState function
extern "C" short __stdcall GetKeyState(int nVirtKey);   // https://github.com/raysan5/raylib/issues/1217#issuecomment-1872377579

bool IsCapsLockActive() {
    return (GetKeyState(0x14) & 0x0001) != 0;
}

#define FONT_LOC_Sofia_Sans_Condensed_MED       {"resources/Fonts/Sofia_Sans_Condensed/static/SofiaSansCondensed-Medium.ttf"}
#define ICON_INCOGNITO                          {"resources/Icons/Incognito.png"}
#define ICON_CHECK_LOCK                         {"resources/Icons/Check.png"}

#define VERYDARKGRAY                            {50, 50, 50, 255}

void UpdateDrawUI();

void InputTextBox(Rectangle& input_text_place);

void LoadSetup(int new_width, int new_height);

void InitializedIcons();

void InitializedFonts();

void OutputFolderTest();

void DrawTextMine(Rectangle& panel, std::string& text, int align, float size, Color color, Color fillColor);

void DrawRectangleLinesExCustom(Rectangle rec, float lineThick, Color color);

void ImageToAudio(const Image image, const std::string filename, int audioFormat, int audioEncoder);

struct ImageSize {
    float width{};
    float height{};
};

struct Frame {
    float y{};
    float h{};
};

struct ButtonPadThickness {
    const int value{};
    bool isChosen{};
    bool isHover{};
    const Color colorPressed{ 79, 100, 166, 255 };
    const Color colorNormal{ 180, 180, 180, 255 };

    ButtonPadThickness(const int value, bool isChosen) : value(value), isChosen(isChosen) {}

    void chooseThisButton() {
        isChosen = true;
    }

    void resetChosen() {
        isChosen = false;
    }

    int getValue() {
        return value;
    }

    Color getColorButton() const {
        if (isChosen || isHover) return colorPressed;
        else return colorNormal;
    }

    Color getColorText() const {
        return isChosen ? WHITE : BLACK;
    }
};

struct ButtonPadColor {
    const Color color{};
    bool isChosen{};
    bool isHover{};

    ButtonPadColor(const Color color, bool isChosen) : color(color), isChosen(isChosen) {};

    void chooseThisButton() {
        isChosen = true;
    }

    void resetChosen() {
        isChosen = false;
    }

    Color getColorButton() const {
        return color;
    }

    Color getColorIcon() const {
        if (isChosen) return Fade(BLUE, 1.0F);
        else if (isHover) return Fade(BLUE, 0.3F);
        else return Fade(BLACK, 0.0F);
    }
};

enum PadPlace {
    BOTTOM,
    TOP_BOTTOM
};

struct ButtonPadPlace {
    const int place{};
    std::string text{};
    bool isChosen{};
    bool isHover{};
    const Color colorPressed{ 79, 100, 166, 255 };
    const Color colorNormal{ 180, 180, 180, 255 };

    ButtonPadPlace(const int place, const std::string text, bool isChosen) : place(place), text(text), isChosen(isChosen) {};

    void chooseThisButton() {
        isChosen = true;
    }

    void resetChosen() {
        isChosen = false;
    }

    int getPadPlace() {
        return place;
    }

    std::string getPadText() {
        return text;
    }

    Color getColorButton() const {
        if (isChosen || isHover) return colorPressed;
        else return colorNormal;
    }

    Color getColorText() const {
        return isChosen ? WHITE : BLACK;
    }
};

enum AudioFormat {
    WAV = SF_FORMAT_WAV,
    FLAC = SF_FORMAT_FLAC,
    OGG = SF_FORMAT_OGG
};

struct ButtonAudioFormat {
    const int format{};
    const int encoder{};
    std::string textDisplay{};
    std::string textFormat{};
    bool isChosen{};
    bool isHover{};
    const Color colorPressed{ 79, 100, 166, 255 };
    const Color colorNormal{ 180, 180, 180, 255 };

    ButtonAudioFormat
    (
        const int format, const int encoder, const std::string textDisplay, const std::string textFormat, bool isChosen
    ) :
        format(format), encoder(encoder), textDisplay(textDisplay), textFormat(textFormat), isChosen(isChosen) {};

    void chooseThisButton() {
        isChosen = true;
    }

    void resetChosen() {
        isChosen = false;
    }

    int getAudioFormat() {
        return format;
    }

    int getAudioEncoder() {
        return encoder;
    }

    std::string getFormatDisplay() const {
        return textDisplay;
    }

    std::string getOutputFormatting() const {
        return textFormat;
    }

    Color getColorButton() const {
        if (isChosen || isHover) return colorPressed;
        else return colorNormal;
    }

    Color getColorText() const {
        return isChosen ? WHITE : BLACK;
    }
};

struct Plug {
    std::string title = "Incognito";
    float h_space = 0.025F;
    Frame header = { 0, 0.05F };
    Frame section1 = { header.h, 0.525F };
    Frame section2 = { section1.h, 0.3F };
    Frame footer = { section2.h, 0.05F };
    float labelSize = 32.5F;
    Color bg_color = { 20,20,20,255 };
    Vector2 mouse_position{};
    Image image_input{};
    Image image_output{};
    Texture2D texture_input{};
    Texture2D texture_output{};
    std::vector<std::string> pad_function{ "THICKNESS", "COLOR", "PLACE" };
    std::vector<ButtonPadThickness> argument_thickness{ {2, 0}, {4, 1}, {6, 0}, {8,0}, {10,0} };
    std::vector<ButtonPadColor> argument_color{ {BLACK, 0}, {DARKGRAY, 0}, {GRAY, 1}, {LIGHTGRAY, 0}, {RAYWHITE, 0} };
    std::vector<ButtonPadPlace> argument_place{ {BOTTOM, "BOTTOM", 1}, {TOP_BOTTOM, "TOP & BOT", 0} };
    std::vector<std::string> convert_function{ "TITLE", "FORMAT", "" };
    //std::vector<ButtonAudioFormat> argument_format{ {WAV, SF_FORMAT_PCM_16, "WAV", ".wav", 1}, {FLAC, SF_FORMAT_PCM_16, "FLAC", ".flac", 0}, {OGG, SF_FORMAT_PCM_16, "MP3", ".mp3", 0}, {OGG, SF_FORMAT_VORBIS, "OGG", ".ogg", 0}};
    std::vector<ButtonAudioFormat> argument_format{ {WAV, SF_FORMAT_PCM_16, "WAV", ".wav", 1}, {FLAC, SF_FORMAT_PCM_16, "FLAC", ".flac", 0} };
    Texture2D TEX_Check{};
    Rectangle flexible_panel_input{};
    Rectangle flexible_panel_output{};
    ImageSize flexible_ratio{};
    int pad_thick{};
    Color pad_color{};
    int pad_place{};
    std::string input_title{};
    std::string outputTitle{};
    int audioFormat{};
    int audioEncoder{};
    bool reload_setup{ true };
    bool notificationON{};
    int notificationLevel{};
};

Plug ImageToSoundPlug{};
static Plug* p = &ImageToSoundPlug;

struct ScreenSize {
    float w{};
    float h{};
};

ScreenSize screen{};
Font font{};

enum TextAlign {
    LEFT,
    CENTER,
    RIGHT
};

enum NotificationSentiment {
    FAILED,
    SUCCESS
};


int main() {

    screen = { 850, 750 };

    SetConfigFlags(FLAG_MSAA_4X_HINT);

    InitWindow((int)screen.w, (int)screen.h, p->title.c_str());
    SetWindowIcon(LoadImage(ICON_INCOGNITO));
    SetTargetFPS(60);

    InitializedFonts();
    InitializedIcons();
    OutputFolderTest();

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(p->bg_color);

        UpdateDrawUI();

        EndDrawing();
    }
}

void OutputFolderTest()
{
    std::string outputPath{ "resources/Output" };
    if (std::filesystem::exists(outputPath) && std::filesystem::is_directory(outputPath)) {
        std::string info{ "[Passed] : Output Directory Exist" };
        TraceLog(LOG_INFO, info.c_str());
    }
    else {
        std::string info{ "[Failed] : Output Directory NOT Exists" };
        TraceLog(LOG_WARNING, info.c_str());

        std::filesystem::create_directory(outputPath);

        info = "Creating resources/Output Directory";
        TraceLog(LOG_INFO, info.c_str());
        if (std::filesystem::exists(outputPath) && std::filesystem::is_directory(outputPath)) {
            std::string info{ "[Passed] : Output Directory Exist" };
            TraceLog(LOG_INFO, info.c_str());
        }
    }
}

void InitializedFonts()
{
    font = LoadFontEx(FONT_LOC_Sofia_Sans_Condensed_MED, 45, 0, 0);
    SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);
}

void InitializedIcons()
{
    Image check_icon = LoadImage(ICON_CHECK_LOCK);
    p->TEX_Check = LoadTextureFromImage(check_icon);
    SetTextureFilter(p->TEX_Check, TEXTURE_FILTER_BILINEAR);
}

void UpdateDrawUI() {
    p->mouse_position = GetMousePosition();

    float pad = 20;
    Rectangle PanelBase{
        0 + pad,
        0 + pad,
        screen.w - (pad * 2),
        screen.h - (pad * 2)
    };
    //DrawRectangleLinesEx(PanelBase, 4, WHITE);

    Rectangle PanelHeader{
        PanelBase.x,
        PanelBase.y,
        PanelBase.width,
        PanelBase.height * p->header.h,
    };
    DrawRectangleLinesEx(PanelHeader, 1.0F, WHITE);

    Rectangle PanelSection1{
        PanelHeader.x,
        PanelHeader.y + (PanelBase.height * (p->section1.y + p->h_space)),
        PanelHeader.width,
        PanelBase.height * p->section1.h
    };
    DrawRectangleLinesExCustom(PanelSection1, 1.0F, WHITE);

    Rectangle PanelSection2{
        PanelSection1.x,
        PanelSection1.y + (PanelBase.height * (p->section2.y + p->h_space)),
        PanelSection1.width,
        PanelBase.height * p->section2.h
    };
    DrawRectangleLinesExCustom(PanelSection2, 1.0F, WHITE);

    Rectangle PanelFooter{
        PanelSection2.x,
        PanelSection2.y + (PanelBase.height * (p->footer.y + p->h_space)),
        PanelSection2.width,
        PanelBase.height * p->footer.h
    };
    DrawRectangleLinesEx(PanelFooter, 1.0F, WHITE);


    // HEADER
    {
        std::string text = "DRAG DROP YOUR IMAGE";
        DrawTextMine(PanelHeader, text, CENTER, 0.9F, WHITE, BLANK);
    }

    // SECTION 1
    {
        Rectangle& panel = PanelSection1;
        float space = 0.01F * panel.width;
        Rectangle rect_l{
            panel.x,
            panel.y,
            panel.width / 2 - space,
            panel.height
        };
        DrawRectangleLinesEx(rect_l, 0.5F, WHITE);

        Rectangle rect_r{
            rect_l.x + rect_l.width + space + space,
            rect_l.y,
            rect_l.width,
            rect_l.height
        };
        DrawRectangleLinesEx(rect_r, 0.5F, WHITE);

        // SECTION 1 LEFT
        {
            float name_space = p->labelSize;
            Rectangle panel_input_image_base{
                rect_l.x,
                rect_l.y,
                rect_l.width,
                rect_l.height - name_space
            };
            DrawRectangleLinesEx(panel_input_image_base, 0.5F, WHITE);

            Rectangle panel_input_image_name{
                panel_input_image_base.x,
                panel_input_image_base.y + panel_input_image_base.height,
                panel_input_image_base.width,
                name_space
            };
            DrawRectangleLinesExCustom(panel_input_image_name, 0.5F, WHITE);

            {
                // INPUT IMAGE TEXT
                std::string text = "INPUT IMAGE";
                DrawTextMine(panel_input_image_name, text, CENTER, 0.9F, WHITE, BLANK);
            }

            if (IsFileDropped()) {
                FilePathList dropped_file = LoadDroppedFiles();

                const char* c_file_path = dropped_file.paths[0];
                std::string cpp_file_path = std::string(c_file_path);

                if (IsFileExtension(c_file_path, ".png") || IsFileExtension(c_file_path, ".jpg")) // I dont know why jpg doesnt work.
                {
                    p->image_input = LoadImage(c_file_path);
                    ImageSize imageOldSize = { (float)p->image_input.width, (float)p->image_input.height };

                    int new_height = 480;
                    int new_width = int((float(new_height) / imageOldSize.height) * imageOldSize.width * 1.1F);
                    std::cout << "height: " << new_height << "\nwidth: " << new_width << std::endl;
                    ImageSize imageNewSize = { (float)new_width, (float)new_height };
                    p->flexible_ratio = imageNewSize;

                    if (imageNewSize.width > imageNewSize.height) {
                        float w = panel_input_image_base.width;
                        float h = imageNewSize.height / imageNewSize.width * panel_input_image_base.width;
                        p->flexible_panel_input = {
                            panel_input_image_base.x + (panel_input_image_base.width - w) / 2,
                            panel_input_image_base.y + (panel_input_image_base.height - h) / 2,
                            w,
                            h
                        };
                    }
                    else {
                        float h = panel_input_image_base.height;
                        float w = imageNewSize.width / imageNewSize.height * panel_input_image_base.height;
                        p->flexible_panel_input = {
                            panel_input_image_base.x + (panel_input_image_base.width - w) / 2,
                            panel_input_image_base.y + (panel_input_image_base.height - h) / 2,
                            w,
                            h
                        };
                    }
                }
                p->reload_setup = true;

                UnloadDroppedFiles(dropped_file);
            }

            if (p->reload_setup) {
                LoadSetup((int)p->flexible_ratio.width, (int)p->flexible_ratio.height);
                p->reload_setup = false;
            }

            if (p->texture_input.height != 0) {
                // Draw input
                {
                    Rectangle source{
                        0,0,(float)p->texture_input.width, (float)p->texture_input.height
                    };
                    Rectangle dest{ p->flexible_panel_input };
                    DrawTexturePro(p->texture_input, source, dest, { 0,0 }, 0, WHITE);
                }
            }

            DrawRectangleLinesExCustom(p->flexible_panel_input, 0.5F, WHITE);

        }

        // SECTION 1 RIGHT
        {
            float name_space = p->labelSize;
            Rectangle panel_input_image_base{
                rect_r.x,
                rect_r.y,
                rect_r.width,
                rect_r.height - name_space
            };
            DrawRectangleLinesEx(panel_input_image_base, 0.5F, WHITE);

            Rectangle panel_input_image_name{
                panel_input_image_base.x,
                panel_input_image_base.y + panel_input_image_base.height,
                panel_input_image_base.width,
                name_space
            };
            DrawRectangleLinesExCustom(panel_input_image_name, 0.5F, WHITE);

            {
                // INPUT IMAGE TEXT
                std::string text = "OUTPUT IMAGE";
                DrawTextMine(panel_input_image_name, text, CENTER, 0.9F, WHITE, BLANK);
            }

            p->flexible_panel_output = {
                panel_input_image_base.x + (panel_input_image_base.width - p->flexible_panel_input.width) / 2,
                p->flexible_panel_input.y,
                p->flexible_panel_input.width,
                p->flexible_panel_input.height
            };

            if (p->texture_input.height != 0) {
                // Draw output
                {
                    Rectangle source{
                        0,0,(float)p->texture_output.width, (float)p->texture_output.height
                    };
                    Rectangle dest{ p->flexible_panel_output };
                    DrawTexturePro(p->texture_output, source, dest, { 0,0 }, 0, WHITE);
                }
            }

            DrawRectangleLinesExCustom(p->flexible_panel_output, 0.5F, WHITE);
        }

    }

    // SECTION 2
    {
        Rectangle& panel = PanelSection2;
        float space = 0.01F * panel.width;
        Rectangle rect_l{
            panel.x,
            panel.y,
            panel.width / 2 - space,
            panel.height
        };
        DrawRectangleLinesEx(rect_l, 0.5F, WHITE);

        Rectangle rect_r{
            rect_l.x + rect_l.width + space + space,
            rect_l.y,
            rect_l.width,
            rect_l.height
        };
        DrawRectangleLinesEx(rect_r, 0.5F, WHITE);

        // SECTION 2 LEFT
        {
            float name_space = p->labelSize;

            Rectangle panel_pad_setup_name{
                rect_l.x,
                rect_l.y,
                rect_l.width,
                name_space
            };
            DrawRectangleLinesExCustom(panel_pad_setup_name, 0.5F, WHITE);

            {
                // INPUT IMAGE TEXT
                std::string text = "PAD SETUP";
                DrawTextMine(panel_pad_setup_name, text, CENTER, 0.9F, WHITE, BLANK);
            }

            Rectangle panel_pad_setup{
                panel_pad_setup_name.x,
                panel_pad_setup_name.y + panel_pad_setup_name.height,
                panel_pad_setup_name.width,
                rect_l.height - name_space
            };
            DrawRectangleLinesEx(panel_pad_setup, 0.5F, WHITE);

            // PAD SETUP
            {
                float space_left = panel_pad_setup.width * 0.3F;

                Rectangle rect_l{
                    panel_pad_setup.x,
                    panel_pad_setup.y,
                    space_left,
                    panel_pad_setup.height
                };
                DrawRectangleLinesExCustom(rect_l, 0.5F, WHITE);

                Rectangle rect_r{
                    rect_l.x + rect_l.width,
                    rect_l.y,
                    panel_pad_setup.width - space_left - 10,
                    rect_l.height
                };
                DrawRectangleLinesExCustom(rect_r, 0.5F, WHITE);

                // PAD FUNCTION
                size_t count = p->pad_function.size();
                for (size_t i = 0; i < count; i++) {
                    Rectangle parameter{
                        rect_l.x,
                        rect_l.y + (i * rect_l.height / count),
                        rect_l.width,
                        rect_l.height / count
                    };
                    DrawRectangleLinesExCustom(parameter, 0.5F, WHITE);

                    std::string text = p->pad_function.at(i);
                    DrawTextMine(parameter, text, LEFT, 0.4F, WHITE, BLANK);

                    Rectangle argument{
                        rect_r.x,
                        parameter.y,
                        rect_r.width,
                        parameter.height
                    };
                    DrawRectangleLinesExCustom(argument, 0.5F, WHITE);

                    if (i == 0)
                    {
                        size_t button_count = p->argument_thickness.size();
                        for (size_t j = 0; j < button_count; j++) {
                            auto& ButtonThick = p->argument_thickness.at(j);

                            Rectangle button_thick_base{
                                argument.x + (j * argument.width / button_count),
                                argument.y,
                                argument.width / button_count,
                                argument.height
                            };
                            DrawRectangleLinesExCustom(button_thick_base, 0.5F, WHITE);

                            float pad = 8;
                            Rectangle ButtonThickness{
                                button_thick_base.x + (pad * 1),
                                button_thick_base.y + (pad * 1.5F),
                                button_thick_base.width - (pad * 2),
                                button_thick_base.height - (pad * 2 * 1.5F),
                            };
                            if (CheckCollisionPointRec(p->mouse_position, ButtonThickness)) {
                                ButtonThick.isHover = true;

                                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                                    for (auto& button : p->argument_thickness) {
                                        button.resetChosen();
                                    }
                                    ButtonThick.chooseThisButton();
                                    p->reload_setup = true;
                                }
                            }
                            else {
                                ButtonThick.isHover = false;
                            }

                            if (ButtonThick.isChosen) p->pad_thick = ButtonThick.getValue();

                            DrawRectangleRounded(ButtonThickness, 0.3F, 10, ButtonThick.getColorButton());
                            std::string text = std::to_string(ButtonThick.getValue());
                            DrawTextMine(ButtonThickness, text, CENTER, 0.9F, ButtonThick.getColorText(), BLANK);
                        }
                    }
                    else if (i == 1)
                    {
                        size_t button_count = p->argument_color.size();
                        for (size_t k = 0; k < button_count; k++) {
                            auto& ButtonColor = p->argument_color.at(k);

                            Rectangle button_color_base{
                                argument.x + (k * argument.width / button_count),
                                argument.y,
                                argument.width / button_count,
                                argument.height
                            };
                            DrawRectangleLinesExCustom(button_color_base, 0.5F, WHITE);

                            float pad = 8;
                            Rectangle ButtonColorness{
                                button_color_base.x + (pad * 1),
                                button_color_base.y + (pad * 1.5F),
                                button_color_base.width - (pad * 2),
                                button_color_base.height - (pad * 2 * 1.5F),
                            };
                            if (CheckCollisionPointRec(p->mouse_position, ButtonColorness)) {
                                ButtonColor.isHover = true;

                                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                                    for (auto& button : p->argument_color) {
                                        button.resetChosen();
                                    }
                                    ButtonColor.chooseThisButton();
                                    p->reload_setup = true;
                                }
                            }
                            else {
                                ButtonColor.isHover = false;
                            }

                            if (ButtonColor.isChosen) p->pad_color = ButtonColor.getColorButton();

                            DrawRectangleRounded(ButtonColorness, 0.2F, 10, ButtonColor.getColorButton());
                            DrawRectangleRoundedLines(ButtonColorness, 0.2F, 10, 1.0F, WHITE);
                            {
                                // DRAW CHECK ICON
                                float icon_size = 100.0F;
                                Rectangle dest = ButtonColorness;
                                Rectangle source = { 0,0, icon_size, icon_size };
                                DrawTexturePro(p->TEX_Check, source, dest, { 0,0 }, 0, ButtonColor.getColorIcon());
                            }

                        }
                    }
                    else if (i == 2)
                    {
                        size_t button_count = p->argument_place.size();
                        for (size_t l = 0; l < button_count; l++) {
                            auto& ButtonPlace = p->argument_place.at(l);

                            Rectangle button_place_base{
                                argument.x + (l * argument.width / button_count),
                                argument.y,
                                argument.width / button_count,
                                argument.height
                            };
                            DrawRectangleLinesExCustom(button_place_base, 0.5F, WHITE);

                            float pad = 8;
                            Rectangle ButtonPlacement{
                                button_place_base.x + (pad * 1),
                                button_place_base.y + (pad * 1.5F),
                                button_place_base.width - (pad * 2),
                                button_place_base.height - (pad * 2 * 1.5F),
                            };
                            if (CheckCollisionPointRec(p->mouse_position, ButtonPlacement)) {
                                ButtonPlace.isHover = true;

                                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                                    for (auto& button : p->argument_place) {
                                        button.resetChosen();
                                    }
                                    ButtonPlace.chooseThisButton();
                                    p->reload_setup = true;
                                }
                            }
                            else {
                                ButtonPlace.isHover = false;
                            }

                            if (ButtonPlace.isChosen) p->pad_place = ButtonPlace.getPadPlace();

                            DrawRectangleRounded(ButtonPlacement, 0.2F, 10, ButtonPlace.getColorButton());
                            std::string text = ButtonPlace.getPadText();
                            DrawTextMine(ButtonPlacement, text, CENTER, 0.7F, ButtonPlace.getColorText(), BLANK);
                        }
                    }
                }
            }
        }

        // SECTION 2 RIGHT
        {
            float name_space = p->labelSize;

            Rectangle panel_convert_name{
                rect_r.x,
                rect_r.y,
                rect_r.width,
                name_space
            };
            DrawRectangleLinesExCustom(panel_convert_name, 0.5F, WHITE);

            {
                // INPUT IMAGE TEXT
                std::string text = "CONVERT TO AUDIO";
                DrawTextMine(panel_convert_name, text, CENTER, 0.9F, WHITE, BLANK);
            }

            Rectangle panel_convert_audio{
                panel_convert_name.x,
                panel_convert_name.y + panel_convert_name.height,
                panel_convert_name.width,
                rect_r.height - name_space
            };
            DrawRectangleLinesEx(panel_convert_audio, 0.5F, WHITE);

            // CONVERT AUDIO
            {
                float space_left = panel_convert_audio.width * 0.25F;

                Rectangle rect_l{
                    panel_convert_audio.x,
                    panel_convert_audio.y,
                    space_left,
                    panel_convert_audio.height
                };
                DrawRectangleLinesExCustom(rect_l, 0.5F, WHITE);

                Rectangle rect_r{
                    rect_l.x + rect_l.width,
                    rect_l.y,
                    panel_convert_audio.width - space_left - 10,
                    rect_l.height
                };
                DrawRectangleLinesExCustom(rect_r, 0.5F, WHITE);

                // CONVERT FUNCTION
                size_t count = p->convert_function.size();
                for (size_t i = 0; i < count; i++) {
                    Rectangle parameter{
                        rect_l.x,
                        rect_l.y + (i * rect_l.height / count),
                        rect_l.width,
                        rect_l.height / count
                    };
                    DrawRectangleLinesExCustom(parameter, 0.5F, WHITE);

                    std::string text = p->convert_function.at(i);
                    DrawTextMine(parameter, text, LEFT, 0.4F, WHITE, BLANK);

                    Rectangle argument{
                        rect_r.x,
                        parameter.y,
                        rect_r.width,
                        parameter.height
                    };
                    DrawRectangleLinesExCustom(argument, 0.5F, WHITE);

                    if (i == 0)
                    {
                        float pad = 8;
                        Rectangle input_text_place{
                            argument.x + (pad * 1),
                            argument.y + (pad * 1.5F),
                            argument.width - (pad * 2),
                            argument.height - (pad * 2 * 1.5F),
                        };
                        DrawRectangleRec(input_text_place, RAYWHITE);

                        InputTextBox(input_text_place);

                        //// CONTOH SEMENTARA
                        //{
                        //    std::string text = "Author_UFTHaq";
                        //    DrawTextMine(input_text_place, text, LEFT, 0.8F, BLACK);
                        //    p->input_title = text;
                        //}


                    }
                    else if (i == 1)
                    {
                        size_t button_count = p->argument_format.size();
                        for (size_t j = 0; j < button_count; j++) {
                            auto& ButtonFormat = p->argument_format.at(j);

                            Rectangle button_format_base{
                                argument.x + (j * argument.width / button_count),
                                argument.y,
                                argument.width / button_count,
                                argument.height
                            };
                            DrawRectangleLinesExCustom(button_format_base, 0.5F, WHITE);

                            float pad = 8;
                            Rectangle ButtonFormatness{
                                button_format_base.x + (pad * 1),
                                button_format_base.y + (pad * 1.5F),
                                button_format_base.width - (pad * 2),
                                button_format_base.height - (pad * 2 * 1.5F),
                            };
                            if (CheckCollisionPointRec(p->mouse_position, ButtonFormatness)) {
                                ButtonFormat.isHover = true;

                                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                                    for (auto& button : p->argument_format) {
                                        button.resetChosen();
                                    }
                                    ButtonFormat.chooseThisButton();
                                }
                            }
                            else {
                                ButtonFormat.isHover = false;
                            }

                            if (ButtonFormat.isChosen) {
                                p->audioFormat = ButtonFormat.getAudioFormat();
                                p->audioEncoder = ButtonFormat.getAudioEncoder();
                                p->outputTitle = "resources/Output/" + p->input_title + ButtonFormat.getOutputFormatting();
                            }

                            DrawRectangleRounded(ButtonFormatness, 0.2F, 10, ButtonFormat.getColorButton());
                            std::string text = ButtonFormat.getFormatDisplay();
                            DrawTextMine(ButtonFormatness, text, CENTER, 0.7F, ButtonFormat.getColorText(), BLANK);
                        }
                    }
                    else if (i == 2)
                    {
                        static bool isHover = false;
                        Color color = { 52, 148,65, 255 };
                        Rectangle export_button_base{
                            argument.x + argument.width / 2,
                            argument.y,
                            argument.width / 2,
                            argument.height
                        };
                        DrawRectangleLinesExCustom(export_button_base, 0.5F, WHITE);

                        float pad = 8;
                        Rectangle ButtonExport{
                            export_button_base.x + (pad * 1),
                            export_button_base.y + (pad * 1.5F),
                            export_button_base.width - (pad * 2),
                            export_button_base.height - (pad * 2 * 1.5F),
                        };

                        if (CheckCollisionPointRec(p->mouse_position, ButtonExport)) {
                            isHover = true;
                            color = Fade(color, 1.0F);

                            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {

                                p->notificationON = true;

                                if (!p->input_title.empty()) {
                                    ImageFlipVertical(&p->image_output);
                                    ImageToAudio(p->image_output, p->outputTitle, p->audioFormat, p->audioEncoder);
                                    ImageFlipVertical(&p->image_output);
                                    std::cout << "Title  : " << p->outputTitle << std::endl;
                                    p->notificationLevel = SUCCESS;
                                }
                                else {
                                    p->notificationLevel = FAILED;
                                }
                            }
                        }
                        else {
                            isHover = false;
                            color = Fade(color, 0.7F);
                        }

                        DrawRectangleRounded(ButtonExport, 0.2F, 10, color);
                        std::string text = "EXPORT";
                        DrawTextMine(ButtonExport, text, CENTER, 0.7F, WHITE, BLANK);
                    }
                }
            }
        }

    }

    // FOOTER
    {
        std::string text = "INCOGNITO : MADE BY UFTHaq WITH RAYLIB";
        DrawTextMine(PanelFooter, text, RIGHT, 0.6F, WHITE, BLANK);

        if (p->notificationON) {
            static float time = 0.0F;
            time += GetFrameTime();

            if (p->notificationLevel == SUCCESS) {
                text = "SUCCESS EXPORT TO " + p->outputTitle;
                DrawTextMine(PanelFooter, text, LEFT, 0.6F, WHITE, DARKGREEN);
            }
            else {
                text = "PLEASE FILL EXPORT TITLE";
                DrawTextMine(PanelFooter, text, LEFT, 0.6F, WHITE, RED);
            }

            if (time > 5.0F) {
                p->notificationON = false;
                time = 0.0F;
            }
        }

    }


    if (IsKeyDown(KEY_LEFT_CONTROL) && IsKeyPressed(KEY_S)) {
        ImageFlipVertical(&p->image_output);
        ExportImage(p->image_output, "output.png");
        //ImageToSound(p->image_output, "output.wav", WAV);
    }

}

void InputTextBox(Rectangle& input_text_place)
{
    // INPUT BOX
    static bool inputBoxActive = false;
    static int framesCounter = 0;
    int maxInputChars = 20;

    if (CheckCollisionPointRec(p->mouse_position, input_text_place)) {
        SetMouseCursor(MOUSE_CURSOR_IBEAM);
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            inputBoxActive = true;
        }

    }
    else { SetMouseCursor(MOUSE_CURSOR_DEFAULT); }

    if (!CheckCollisionPointRec(p->mouse_position, input_text_place)) {
        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            inputBoxActive = false;
        }
    }

    size_t letterSize = p->input_title.size();
    if (inputBoxActive) {

        framesCounter++;

        int key = GetKeyPressed();

        while (key > 0) {
            // Input KEY Alphabet & numbers
            if (((key >= 'A') && (key <= 'Z')) || ((key >= '0') && (key <= '9')) || (key == '_')) {
                if (letterSize < maxInputChars) {
                    bool isUppercase = (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) ^ IsCapsLockActive();
                    if (key >= 'A' && key <= 'Z') {
                        if (isUppercase) {
                            p->input_title += static_cast<char>(key); // Uppercase
                        }
                        else if (isUppercase == false) {
                            p->input_title += static_cast<char>(tolower(key)); // Lowercase
                        }
                    }
                    else {
                        p->input_title += static_cast<char>(key); // Numbers and underscore remain the same
                    }
                }
            }

            key = GetKeyPressed();
        }

        // Backspace delete chars
        if (IsKeyPressed(KEY_BACKSPACE) && !p->input_title.empty()) {
            p->input_title.pop_back();
        }
        else if (IsKeyDown(KEY_BACKSPACE) && !p->input_title.empty()) {
            static int timeDown_Backspace = 0;
            timeDown_Backspace++;
            if (((timeDown_Backspace) % 15) == 0) p->input_title.pop_back();
        }
    }
    else framesCounter = 0;

    if (inputBoxActive) {
        if (((framesCounter / 30) % 2) == 0) {
            // BLINKING CURSOR
            const char* text = "|";
            float fontSize = input_text_place.height * 0.8F;
            float fontSpace = 0.0F;
            Vector2 text_measure = MeasureTextEx(font, p->input_title.c_str(), fontSize, fontSpace);
            Vector2 text_coor = {
                input_text_place.x + (text_measure.x * 1.1F) + input_text_place.height * 0.3F,
                input_text_place.y + ((input_text_place.height - text_measure.y) / 2) - 0
            };
            DrawTextEx(font, text, text_coor, fontSize, fontSpace, BLACK);
        }
    }

    {
        std::string text{};
        if (p->input_title.empty() && !inputBoxActive) {
            text = "Click Here";
        }
        else {
            text = p->input_title;
        }
        DrawTextMine(input_text_place, text, LEFT, 0.8F, BLACK, BLANK);
    }
}

void LoadSetup(int new_width, int new_height)
{
    // TODO: Add padding as border.
    if (p->reload_setup)
    {
        int pad = p->pad_thick;
        int new_height_bordered = new_height - (pad * 1);
        int new_width_bordered = new_width;

        int pad_replace{};
        if (p->pad_place == BOTTOM) pad_replace = pad;
        else pad_replace = pad * 2;

        ImageResize(&p->image_input, (new_width), new_height - pad_replace);
        p->texture_input = LoadTextureFromImage(p->image_input);

        Image image_process = ImageCopy(p->image_input);

        Color* pixels = LoadImageColors(image_process);

        std::vector<Color> color_data_input{};

        Color pad_color = p->pad_color;

        if (p->pad_place == TOP_BOTTOM)
        { // if pad top & bottom
          // PAD ATAS 
            for (size_t i = 0; i < new_width_bordered * pad; i++) {
                color_data_input.push_back(pad_color);
            }

            for (size_t i = 0; i < (new_height_bordered - pad) * new_width; ++i) {
                color_data_input.push_back(pixels[i]);
            }

            // PAD BAWAH
            for (size_t i = 0; i < new_width_bordered * pad; i++) {
                color_data_input.emplace_back(pad_color);
            }
        }

        if (p->pad_place == BOTTOM)
        { // pad bottom only
            for (size_t i = 0; i < (new_height_bordered)*new_width; ++i) {
                color_data_input.push_back(pixels[i]);
            }

            // PAD BAWAH
            for (size_t i = 0; i < new_width_bordered * pad; i++) {
                color_data_input.emplace_back(pad_color);
            }
        }


        Image image_processed = {
            color_data_input.data(),
            new_width,
            new_height,
            1,
            PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
        };


        p->texture_input = LoadTextureFromImage(image_processed);
        p->image_output = ImageCopy(image_processed);

    }
    ImageFormat(&p->image_output, PIXELFORMAT_UNCOMPRESSED_GRAYSCALE);
    p->texture_output = LoadTextureFromImage(p->image_output);
}

void DrawTextMine(Rectangle& panel, std::string& text, int align, float size, Color color, Color fillColor)
{
    float font_size = panel.height * size;
    float font_space = 1.0F;
    Vector2 text_measure = MeasureTextEx(font, text.c_str(), font_size, font_space);
    Vector2 text_coor{};
    if (align == CENTER) {
        text_coor = {
            panel.x + (panel.width - text_measure.x) / 2,
            panel.y + (panel.height - text_measure.y) / 2
        };
    }
    else if (align == LEFT) {
        text_coor = {
            panel.x + panel.height * 0.3F,
            panel.y + (panel.height - text_measure.y) / 2
        };
    }
    else if (align == RIGHT) {
        text_coor = {
            panel.x + panel.width - (text_measure.x + panel.height * 0.3F),
            panel.y + (panel.height - text_measure.y) / 2
        };
    }
    Rectangle fillRect = {
        panel.x,
        panel.y,
        text_measure.x + (panel.height * 0.3F * 2),
        panel.height
    };
    DrawRectangleRec(fillRect, fillColor);
    DrawTextEx(font, text.c_str(), text_coor, font_size, font_space, color);
}

void DrawRectangleLinesExCustom(Rectangle rec, float lineThick, Color color)
{
#ifndef NDEBUG
    DrawRectangleLinesEx(rec, lineThick, color);
#endif // NDEBUG

}

void ImageToAudio(const Image image, const std::string filename, int audioFormat, int audioEncoder) {
    int height = image.height;
    int width = image.width;
    int N = (height * 2);
    //int N = height;
    std::cout << N << std::endl;

    std::vector<double> audio_data{};
    const float scale = 2.0F / 255.0F;

    unsigned char* image_data = (unsigned char*)image.data;

    //{
    //    // FFTW
    //    fftw_complex* in = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * N);
    //    fftw_complex* out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * N);

    //    for (int x = 0; x < width; x++) {
    //        for (int y = 0; y < height; y++) {

    //            float amplitude = scale * static_cast<float>(image_data[y * width + x]) / 255.0F;
    //            //in[y][0] = amplitude * amplitude * amplitude;
    //            //in[y][0] = amplitude * amplitude;
    //            //in[y][0] = amplitude;
    //            in[y][0] = sqrtf(amplitude);
    //            in[y][1] = 0;
    //        }

    //        fftw_plan plan = fftw_plan_dft_1d(N, in, out, FFTW_BACKWARD, FFTW_MEASURE);
    //        fftw_execute(plan);

    //        for (int y = 0; y < height; y++) {
    //            audio_data.push_back(out[y][0]);
    //            in[y][0] = 0;
    //            out[y][0] = 0;
    //        }

    //        fftw_destroy_plan(plan);
    //    }

    //    fftw_free(in);
    //    fftw_free(out);
    //}

    //// Normalize audio data
    //double max_value = *std::max_element(audio_data.begin(), audio_data.end());
    //for (auto& sample : audio_data) {
    //    sample /= max_value;
    //}

    //std::cout << "Original audio size: " << audio_data.size() << std::endl;

    //// Downsampling to make it shorter
    //std::vector<double> downsampled_audio_data{};
    //for (size_t i = 0; i < audio_data.size(); i += 1) {
    //    downsampled_audio_data.push_back(audio_data.at(i));
    //}

    //std::cout << "Downsampled audio size: " << downsampled_audio_data.size() << std::endl;

    //{
    //    std::vector<short> short_audio_data;
    //    short_audio_data.reserve(downsampled_audio_data.size());
    //    for (double sample : downsampled_audio_data) {
    //        // Scale and clamp to valid short integer range (-32768 to 32767)
    //        int scaled_sample = static_cast<int>(sample * 32767.0);
    //        scaled_sample = std::clamp(scaled_sample, -32768, 32767);
    //        short_audio_data.push_back(static_cast<short>(scaled_sample));
    //    }

    //    // Save short audio data using libsndfile
    //    SF_INFO sfinfo;
    //    sfinfo.channels = 1;
    //    sfinfo.samplerate = 48000;
    //    sfinfo.format = audioFormat | audioEncoder;
    //    SNDFILE* sndfile = sf_open(filename.c_str(), SFM_WRITE, &sfinfo);
    //    if (!sndfile) {
    //        std::cerr << "Error opening sound file: " << sf_strerror(sndfile) << std::endl;
    //        return;
    //    }
    //    else TraceLog(LOG_INFO, "I Think Encoding Success...");
    //    sf_writef_short(sndfile, short_audio_data.data(), short_audio_data.size());
    //    sf_close(sndfile);
    //}

    {
        // FFTW
        fftw_complex* in = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * N);
        fftw_complex* out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * N);

        //std::vector<fftw_complex> in(N);
        //std::vector<fftw_complex> out(N);
        //fftw_plan plan = fftw_plan_dft_1d(N, in.data(), out.data(), FFTW_BACKWARD, FFTW_MEASURE);
        fftw_plan plan = fftw_plan_dft_1d(N, in, out, FFTW_BACKWARD, FFTW_MEASURE);


        for (size_t x = 0; x < width; x++) {
            for (size_t y = 0; y < height; y++) {

                float amplitude_l = scale * static_cast<float>(image_data[y * width + x]) / 255.0F;
                //in[y][0] = amplitude * amplitude * amplitude;
                //in[y][0] = amplitude * amplitude;
                //in[y][0] = amplitude;
                in[y][0] = sqrtf(amplitude_l);
                in[y][1] = 0;
            }

            fftw_execute(plan);

            for (size_t y = 0; y < height; y++) {
                audio_data.push_back(out[y][0]);
                audio_data.push_back(out[y][0]);
                in[y][0] = 0;
                out[y][0] = 0;
            }

        }

        fftw_destroy_plan(plan);
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

    // Debug 
    std::cout << "size audio_data : " << audio_data.size() << std::endl;
    std::cout << "size downsampled: " << downsampled_audio_data.size() << std::endl;


    {
        std::vector<short> short_audio_data{};
        short_audio_data.reserve(downsampled_audio_data.size());
        for (auto sample : downsampled_audio_data) {
            // Scale and clamp to valid short integer range (-32768 to 32767)
            int scaled_sample = static_cast<int>(sample * 32767.0);
            scaled_sample = std::clamp(scaled_sample, -32768, 32767);
            short_audio_data.push_back(static_cast<short>(scaled_sample));
        }

        // Save shoty 16 bit audio data using libsndfile
        SF_INFO sfinfo;
        sfinfo.channels = 2;
        sfinfo.samplerate = 48000;
        //sfinfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
        sfinfo.format = audioFormat | audioEncoder;
        SNDFILE* sndfile = sf_open(filename.c_str(), SFM_WRITE, &sfinfo);
        if (!sndfile) {
            std::cerr << "Error opening sound file: " << sf_strerror(sndfile) << std::endl;
            return;
        }

        // Write interleaved stereo samples
        sf_count_t frames_written = sf_writef_short(sndfile, short_audio_data.data(), short_audio_data.size() / 2);
        if (frames_written != static_cast<sf_count_t>(short_audio_data.size() / 2)) {
            std::cerr << "Error writing sound file: " << sf_strerror(sndfile) << std::endl;
        }
        else {
            std::cout << "Encoding successful!" << std::endl;
        }

        sf_close(sndfile);

        short_audio_data.clear();
        short_audio_data.shrink_to_fit();
        std::cout << "short_audio_data size: " << short_audio_data.size() << std::endl;
        if (short_audio_data.size() == 0) TraceLog(LOG_INFO, "DATA CLEAR : short_audio_data");
    }

    audio_data.clear();
    audio_data.shrink_to_fit();
    std::cout << "audio_data size: " << audio_data.size() << std::endl;
    if (audio_data.size() == 0) TraceLog(LOG_INFO, "DATA CLEAR : audio_data");

    downsampled_audio_data.clear();
    downsampled_audio_data.shrink_to_fit();
    std::cout << "downsampled_audio_data size: " << downsampled_audio_data.size() << std::endl;
    if (downsampled_audio_data.size() == 0) TraceLog(LOG_INFO, "DATA CLEAR : downsampled_audio_data");
}

//{
//    // FFTW
//    fftw_complex* in = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * N);
//    fftw_complex* out = (fftw_complex*)fftw_malloc(sizeof(fftw_complex) * N);

//    for (int x = 0; x < width; x++) {
//        for (int y = 0; y < height; y++) {

//            float amplitude_l = scale * static_cast<float>(image_data[y * width + x]) / 255.0F;
//            //in[y][0] = amplitude * amplitude * amplitude;
//            //in[y][0] = amplitude * amplitude;
//            //in[y][0] = amplitude;
//            in[y][0] = sqrtf(amplitude_l);
//            in[y][1] = 0;
//        }

//        fftw_plan plan = fftw_plan_dft_1d(N, in, out, FFTW_BACKWARD, FFTW_MEASURE);
//        fftw_execute(plan);

//        for (int y = 0; y < height; y++) {
//            audio_data.push_back(out[y][0]);
//            audio_data.push_back(out[y][0]);
//            in[y][0] = 0;
//            out[y][0] = 0;
//        }

//        fftw_destroy_plan(plan);
//    }

//    fftw_free(in);
//    fftw_free(out);
//}

//// Normalize audio data
//double max_value = *std::max_element(audio_data.begin(), audio_data.end());
//for (auto& sample : audio_data) {
//    sample /= max_value;
//}

//std::cout << "Original audio size: " << audio_data.size() << std::endl;

//// Downsampling to make it shorter
//std::vector<double> downsampled_audio_data{};
//for (size_t i = 0; i < audio_data.size(); i += 1) {
//    downsampled_audio_data.push_back(audio_data.at(i));
//}

//std::cout << "Downsampled audio size: " << downsampled_audio_data.size() << std::endl;

//{
//    std::vector<float> float_audio_data{};
//    float_audio_data.reserve(downsampled_audio_data.size());
//    for (auto sample : downsampled_audio_data) {
//        float_audio_data.push_back(static_cast<float>(sample));
//    }
//    // Save float 32 bit audio data using libsndfile
//    SF_INFO sfinfo;
//    sfinfo.channels = 2;
//    sfinfo.samplerate = 48000;
//    sfinfo.format = SF_FORMAT_WAV | SF_FORMAT_FLOAT;
//    SNDFILE* sndfile = sf_open(filename.c_str(), SFM_WRITE, &sfinfo);
//    if (!sndfile) {
//        std::cerr << "Error opening sound file: " << sf_strerror(sndfile) << std::endl;
//        return;
//    }

//    // Write interleaved stereo samples
//    sf_count_t frames_written = sf_writef_float(sndfile, float_audio_data.data(), float_audio_data.size() / 2);
//    if (frames_written != static_cast<sf_count_t>(float_audio_data.size() / 2)) {
//        std::cerr << "Error writing sound file: " << sf_strerror(sndfile) << std::endl;
//    }
//    else {
//        std::cout << "Encoding successful!" << std::endl;
//    }

//    sf_close(sndfile);
//}

//}



// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
