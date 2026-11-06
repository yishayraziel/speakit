# Speakit
Speaker, phone and radio simulator by Yishay Raziel. VST3, AU and Standalone. Built with JUCE 8.

## Build
    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release "-DCMAKE_OSX_ARCHITECTURES=arm64;x86_64"
    cmake --build build --config Release --parallel

`third_party/` holds libgsm and Speex. JUCE is downloaded at configure time unless `third_party/JUCE` is present.
`tools/` rebuilds the skin, IR bank and Sample Bay from their sources. Pushing to `main` runs the Mac and Windows builds.
