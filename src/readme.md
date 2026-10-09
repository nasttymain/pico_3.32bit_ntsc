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
   
6. 初期化コードを記述する

   **Core0 で駆動する場合**
   - main関数に以下を記述:
     ```cpp
     init_framedata();
     init_dma(); // いくらなんでもこれ関数名変えたほうがいいよなあ
     ```
   **Core1 で駆動する場合**
     - Core1 のユーザー処理に使用する関数を設定
       (Core1 では映像出力のみを駆動するなら設定不要)
       ```cpp
       void core1_user_proc(){
         // ここに任意の処理を記述
         // 関数が終了したら再度頭に戻ってきます。これの外側に while 1 がある
         // 映像を考慮してすぐに終わる処理にする の必要とかは特になく、
         // たぶん IRQ と DMA をブロックしなければ割と何でもやっていいと思う
       }
       core1_loop = &core1_user_proc;
       ```
     - main関数(※Core0) に以下を記述
       ```cpp
       init_video_on_core1();
       ```
