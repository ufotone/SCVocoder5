# SCVocoder5

[English documentation](README_EN.md)

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
2. MIDIキーボードを接続し、後述の手順でMIDIチャンネルを設定します。
3. プリアンプ付きのマイクをAUDIO INへ接続します。
4. `SHIFT`を押しながら`BD`ボタンを押し、AUDIO INの`ROUTE`を`SIDECH`に設定します。
5. Master FXに`SC Vocoder 5`を選びます。
6. キーボードでコードを押さえながらマイクへ話します。
7. 最初は次の設定で試します。

|パラメーター|推奨値|
|---|---:|
|RANGE|AGE|
|MOD GAIN|+35dB|
|UNVOICED|20%|
|CHORUS|35%|
|MIX|100%|
|VOICE DRY|0%|

`MONITOR`を`CARRIER`にするとシンセ音、`MODULATOR`にするとマイク音声、`VOCODER`にするとボコーダー音を確認できます。

## マイクとAUDIO INの設定

マイクは、原則としてプリアンプ付きのセルフパワー・マイク、または外部マイクプリアンプを通したものが必要です。一般的なパッシブ・マイクを直接接続すると、十分な音量を得られないことがあります。電源が必要なマイクには、マイク自身の電池や外部電源を使用してください。

1. マイクやプリアンプの出力を低めにして、drumlogueのAUDIO INへ接続します。
2. `SHIFT`を押しながら`BD`ボタンを押して、AUDIO IN設定を開きます。
3. `ROUTE`を`SIDECH`に設定します。
4. `LEVEL`を低い値から少しずつ上げます。
5. SC Vocoder 5の`MONITOR`を`MODULATOR`にして、マイク音声を確認します。
6. 実際に使う最大の声量で話し、歪まない範囲にマイク出力と`LEVEL`を調整します。
7. `MONITOR`を`VOCODER`へ戻し、必要に応じて`MOD GAIN`を調整します。

音量が足りない場合でも、最初からすべてのゲインを最大にしないでください。マイク出力、AUDIO INの`LEVEL`、`MOD GAIN`を順に上げます。音が割れる、ザラつく、子音で急に大きくなる場合は、前段のマイク出力またはAUDIO INの`LEVEL`を下げてください。`MOD GAIN`を上げすぎると、ノイズや歪みも大きくなります。

## MIDIキーボードの設定

次は、drumlogueのMIDIチャンネル範囲を`1-12`にする例です。この設定ではMULTIトラックをMIDIチャンネル11で演奏します。

1. MIDIキーボードをUSBまたはMIDI INへ接続します。
2. `SHIFT`を押しながら`LIVE`ボタンを押して、GLOBALモードを開きます。
3. ボタン7の`MIDI ROUTING`を開きます。
4. `CH`を`1-12`に設定します。
5. MIDIキーボードの送信チャンネルを`11`に設定します。
6. MULTIトラックで`AGE Carrier`を選び、鍵盤を弾いて音が出ることを確認します。

キーボード側のチャンネル変更方法は、使用しているキーボードの説明書を確認してください。鍵盤を弾いても反応しない場合は、drumlogueとキーボードのMIDIチャンネルが一致しているか確認します。

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

## 困ったとき

- マイク音声が聞こえない場合は、AUDIO INの`ROUTE=SIDECH`、マイクの電源、電池、プリアンプを確認し、`MONITOR=MODULATOR`で確認します。
- AGE Carrierが鳴らない場合は、`MONITOR=CARRIER`にし、MULTIトラックの選択とMIDIチャンネルを確認します。
- マイクだけが聞こえる場合は、`MONITOR=MODULATOR`になっていないか、`VOICE DRY`が上がっていないか確認します。
- シンセ音だけが聞こえる場合は、`MONITOR=CARRIER`になっていないか、AUDIO INが`SIDECH`へ送られているか確認します。
- ボコーダー音が小さい場合は、マイクのプリアンプ、AUDIO INの`LEVEL`、`MOD GAIN`の順に確認します。
- 音が歪む場合は、マイク出力とAUDIO INの`LEVEL`を下げてから、`MOD GAIN`と`OUTPUT`を調整します。
- 異常が続く場合はユニットを外し、drumlogueを再起動します。

## License

MIT License。KORGテンプレート由来部分にはKORGのBSD 3-Clause Licenseが適用されます。
