![GitHub last commit](https://img.shields.io/github/last-commit/UFTHaq/Incognito?style=for-the-badge)
![GitHub top language](https://img.shields.io/github/languages/top/UFTHaq/Incognito?logo=cpp&style=for-the-badge)
![GitHub code size in bytes](https://img.shields.io/github/languages/code-size/UFTHaq/Incognito?style=for-the-badge)
![GitHub Repo stars](https://img.shields.io/github/stars/UFTHaq/Incognito?color=red&style=for-the-badge)
![GitHub Downloads (all assets, all releases)](https://img.shields.io/github/downloads/UFTHaq/incognito/total?style=for-the-badge)
<!-- ![GitHub top language](https://img.shields.io/github/languages/top/UFTHaq/Incognito?label=Raylib&logo=raylib&logoColor=black&style=for-the-badge) -->


![Visual Studio](https://img.shields.io/badge/Visual%20Studio-5C2D91.svg?style=for-the-badge&logo=visual-studio&logoColor=white)
![C](https://img.shields.io/badge/c-%2300599C.svg?style=for-the-badge&logo=c&logoColor=white)
![C++](https://img.shields.io/badge/c++-%2300599C.svg?style=for-the-badge&logo=c%2B%2B&logoColor=white)

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg?style=for-the-badge)](https://opensource.org/licenses/MIT)

# Incognito : Image to Audio Converter
Convert your image to audio

<div align="center">
  <img src="https://github.com/user-attachments/assets/09ef1ec2-6484-4f33-91fb-1071425a74c5" width=180/>
</div>

## Incognito Demo App
https://github.com/user-attachments/assets/b590e59c-df37-4a22-b07a-773628c8b7fb

https://www.youtube.com/watch?v=2cm3MWPysow

### Skill Issue:
- I dont know any build system yet, so sorry for dev.

### Features:
- Drag&Drop PNG & JPG image.
- Setup padding.
  - Pad with the right color and thickness can clean out the spectrogram result in [Tirakat](https://github.com/UFTHaq/Tirakat) app.
- Title input.
- Export to Audio Format (WAV & FLAC).

### Build with:
- C++17 in MSVC Visual Studio C++ 2022

### Libraries:
- Raylib : [https://www.raylib.com](https://www.raylib.com/)
- SFML : [https://www.sfml-dev.org](https://www.sfml-dev.org/)
- FFTW3 : [https://www.fftw.org](https://www.fftw.org/)
- Libsndfile : [https://libsndfile.github.io/libsndfile](https://libsndfile.github.io/libsndfile/)

### Download:
- Windows-x64 : <a href="https://github.com/UFTHaq/Incognito/releases/download/v.1.0.0/Incognito-v1.0.0-win64.zip">
 Incognito-v1.0.0-win64.zip
  </a>

## Changelog
### Since v2.0.0
- Changing UI:
  - Added slider for thickness and color.
  - Added auto pad color button.
- New Features:
  - Support PNG & JPG formats.
  - Auto play audio after successful encoding.
- Bug Fixes: 
  - Fixed windows startup issue (from v1.0.0).
  - Fixed image conversion limitation (from v1.0.0).
- Screenshot:
<div align="center">
  <img src="https://github.com/user-attachments/assets/079a34c3-0a5b-44e2-b557-d0bcb0b6657b" width=600/>
</div>

####  Known Issues / Bugs:
- Not found yet.

<hr></hr>

### Since v1.0.0
- Features:
  - Only support PNG.
- Screenshot:
<div align="center">
  <img src="https://github.com/user-attachments/assets/79f52c14-39c1-41c3-8306-8d3a3e778b8b" width=600/>
</div>

####  Known Issues / Bugs:
- <b>Window startup issue:</b> The window startup is broken when using 2 monitors in extended mode, at least in my setup (laptop ryzen5 Win11 and second monitor). This could be a compatibility issue with Raylib, Windows, or AMD drivers.
  - <b>Workaround:</b> Quick fix is by changing extended mode monitor to (PC screen or Second screen only), open This App, and then its okay to cameback to Extended mode.
- <b>Image conversion limitation:</b> The app has a limitation of converting only one image to audio per session. To convert another image, you will need to exit and restart the app. I am unable to determine what cause this issue.

<br></br>

<p align="center">
  <b>If you consider this page is useful, please leave a star ⭐</b>
</p>
