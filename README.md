# wonderland.loader

native系/Emscripten系で同じアプリケーションコードでhttp(s)で与えられるURLから
C++標準のstd::future系を用いた非同期処理でデータを取得するライブラリー。

native系はlibcurlpp/libcurlを使用。動作確認は clang++-3.5/g++-4.9 。

Emscripten系はEmscripten APIを使用。動作確認は em++-1.27.1 。

## dependancy

- libcurlpp
    - native 版のhttp(s)スキーマのデータ取得に使用
    - MITライセンス
- libcurl
    - native 版の libcurlpp が依存
    - 修正BSDライセンス
- wonderland.log
    - 試験アプリのロガーとして使用。 `git submodule` で導入可
    - MITライセンス

## License

- [MIT](LICENSE)

## Author

- Usagi Ito <usagi@WonderRabbitProject.net>

