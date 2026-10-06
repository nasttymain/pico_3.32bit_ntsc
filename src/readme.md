# 超簡単なこのプロジェクトの使い方

以下の操作をするだけできみのRaspberry Pi Pico からも アナログ映像信号が出るよ

1. PicoSDK で C++ Project を新規作成する
2. `src` フォルダをそのプロジェクトにコピーして、フォルダ名を `332ntsc` とかいい感じにする
3. Project 直下の `CMakeLists.txt` にある `pico_SDK_init()` の直後に、
   ```cmakelists
   add_subdirectory(332ntsc)
   ```
   ってかく。
4. `CMakeLists.txt` 内の `target_link_libraries` に、
   ```cmakelists
   pico_3bit32_ntsc
   ```
   を追記する。

5. `main.cpp` で、
   ```cpp
   #include <cvbs.hpp>
   ```
   ってかく。