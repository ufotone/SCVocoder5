# Validation — AGE Carrier v0.4.0

Windows / Visual C++ 2022、48kHz。

- 従来の6音リアルタイム、リリース、ボイス・スティール、モード切替、240コード波形比較、ピッチベンド、ゲート、全消音、上下限、ラベル: PASS。
- SDK初期化、入出力、全9項目のメタデータと初期値: PASS。
- Gm7 → Gm6 → Gm(b6) → AbM7/Gの構成音・発音・リリースを維持。
- C9 / Cmaj9 / Cm9 / C6/9の5度省略4音を明示的な期待音列と比較: 一致。
- Cmaj9/DbはDb / E / B / Dの4音、C7/DbはDb / C / E / Bbの4音、C7#5/DbはDb / E / G# / Bbの4音を確認。
- 40コード × 12ルート × 13ベース設定 × 2モード = 12,480ボイシングを独立した期待音列で手弾きしたAGE Carrierと比較: 一致。
- すべての期待ボイシングがBASS込みで最大4音であることを確認。
- MIDI 0〜127 × 40コード × 13ベース設定の66,560ケースで有限値と出力範囲を確認。
- LIVE6でCHORD / BASSを無視し、元の6音波形を維持することを確認。

すべてPASS。

コード生成時のレンダーは4音分。LIVE6は6音分を維持。負荷の軽減率は実機未測定です。

ARMビルド: PASS。Cortex-A7 / ARMv7 hard-float / NEON。
text 7,662 bytes / data 1,352 bytes / bss 288 bytes。

age_carrier.drmlgunit SHA-256: D52851774201E935246DF9BBCA15AE029E2BBB950D58ECB3168ACF0DCAF4F4A3

今回のバイナリの実機動作・音質・CPU負荷は未確認です。
