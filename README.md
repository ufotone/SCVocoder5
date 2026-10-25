# SCVocoder5

KORG drumlogue用のボコーダーと専用キャリア・シンセです。

## 収録ユニット

- `SC Vocoder 5 v0.5.1`：Master FX用ボコーダー
- `AGE Carrier v0.1.1`：MULTIトラック用SAW和音シンセ

## インストール

1. Releasesから配布ZIPをダウンロードして展開します。
2. drumlogueをUSBマスストレージ・モードで起動します。
3. 次のファイルを本体へコピーします。

|ファイル|コピー先|
|---|---|
|`sc_vocoder5.drmlgunit`|`Units/MasterFXs/`|
|`age_carrier.drmlgunit`|`Units/Synths/`|

4. drumlogueを通常起動します。

## 基本的な使い方

1. MULTIトラックのUSERシンセに`AGE Carrier`を選びます。
2. USB MIDIキーボードを接続し、コードを演奏します。
3. AUDIO INへマイクを接続します。
4. マイク音声をMaster FXのサイドチェインへ送ります。
5. Master FXに`SC Vocoder 5`を選びます。
6. 最初は次の設定で試します。

|パラメーター|推奨値|
|---|---:|
|RANGE|AGE|
|MOD GAIN|+35dB|
|UNVOICED|20%|
|CHORUS|35%|
|MIX|100%|
|VOICE DRY|0%|

`MONITOR`を`CARRIER`にするとシンセ音、`MODULATOR`にするとマイク音声、`VOCODER`にするとボコーダー音を確認できます。

## SC Vocoder 5

### Page 1

|パラメーター|範囲|初期値|説明|
|---|---:|---:|---|
|FORMANT|−12〜+12|0|声のフォルマント位置を上下します。|
|RELEASE|1〜500ms|100ms|声が止まった後の音の残り方を調整します。|
|RANGE|FULL / AGE|AGE|分析する周波数帯域を選びます。通常はAGEを使用します。|
|MIX|0〜100%|100%|原音とボコーダー音の割合を調整します。|

### Page 2

|パラメーター|範囲|初期値|説明|
|---|---:|---:|---|
|ATTACK|1〜50ms|4ms|声への反応速度を調整します。|
|MOD GAIN|−24〜+50dB|+35dB|マイク入力の音量を調整します。|
|OUTPUT|−24〜+12dB|0dB|最終出力音量を調整します。|
|UNVOICED|0〜100%|0%|「し」「す」などの子音成分を加えます。20%前後から試してください。|

### Page 3

|パラメーター|選択肢／範囲|初期値|説明|
|---|---:|---:|---|
|INPUT|SIDECHAIN / L/R SPLIT|SIDECHAIN|入力方法を選びます。L/R SPLITでは左がキャリア、右がモジュレーターです。|
|MONITOR|VOCODER / CARRIER / MODULATOR|VOCODER|確認する信号を選びます。|
|VOICE DRY|0〜100%|0%|加工前のマイク音声を加えます。|
|WIDTH|0〜100%|100%|ボコーダー音のステレオ幅を調整します。|

### Page 4

|パラメーター|範囲|初期値|説明|
|---|---:|---:|---|
|CHORUS|0〜100%|35%|ボコーダー音に広がりを加えます。|

## AGE Carrier

6音ポリフォニックのSAW和音シンセです。鍵盤を押している間、ボコーダー用のキャリア音を出力します。

|パラメーター|範囲|初期値|説明|
|---|---:|---:|---|
|ATTACK|1〜200ms|5ms|音の立ち上がりを調整します。|
|DECAY|1〜1000ms|80ms|サステイン音量へ移る時間を調整します。|
|RELEASE|5〜2000ms|120ms|鍵盤を離した後の余韻を調整します。|
|LEVEL|−24〜0dB|0dB|キャリアの出力音量を調整します。|

## 音が出ない場合

- `MONITOR=MODULATOR`でマイク音声を確認します。
- `MONITOR=CARRIER`でAGE Carrierの音を確認します。
- AUDIO INとサイドチェインの設定を確認します。
- マイクの出力レベルと`MOD GAIN`を確認します。
- 異常が続く場合はユニットを外し、drumlogueを再起動します。

## License

MIT License。KORGテンプレート由来部分にはKORGのBSD 3-Clause Licenseが適用されます。
