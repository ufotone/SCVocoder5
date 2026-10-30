# Validation — SC Vocoder 5 v0.7.0

## ホストテスト

Windows / Visual C++ 2022、48kHz。

- 既存DSPテスト: 無音、原音、入力ルーティング、MONITOR、ブロック長非依存、コーラス、FULL / AGE、フォルマント、上下限、過大入力、NaN/Inf、リリース減衰。PASS。
- SDK初期化、入出力条件、全17パラメーターのヘッダー・範囲・初期値、設定保持。PASS。
- フランジャー・リバーブがOFFの出力をv0.5.1と比較。一致。
- 各エフェクトの発音変化、同時使用、ブロック長非依存、MIX=0とMONITORバイパス。PASS。
- リバーブの残響時間・減衰、エフェクトOFF→ONで古い音を再生しないこと、リセット。PASS。
- 両FX最大設定・ライブ操作・無入力への移行で有限値、出力上限、減衰。PASS。

広帯域ノイズを用いたフランジャーのステレオRMS（原音比）:

|FLANGER|補正後の音量差|
|---|---|
|25%|−0.23dB|
|50%|−0.49dB|
|75%|−0.64dB|
|100%|−0.47dB|

自動補正はFLANGER=100%で最大+3.01dB。フィードバック書き込み後の出力に適用。音色や入力信号により実際の音量感は異なります。

Vocoderのホスト上のサイズ: 83,264 bytes（v0.6.0の345,456 bytesから262,192 bytes削減）。
両FX最大設定の10秒音声をホストで約0.034秒で処理。ARM実機のCPU負荷測定ではありません。

## ARMビルド

固定digest xiashj/logue-sdk、Cortex-A7 / ARMv7 hard-float / NEON。PASS。

text 10,430 bytes / data 357 bytes / bss 83,280 bytes。

sc_vocoder5.drmlgunit SHA-256: BFC8F928A6641536FDD7EDC6E69765A1BD2D5759BE0B0914DBF1A7F96C2B243E

今回のバイナリの実機動作・音量感・音質・CPU負荷は未確認です。
