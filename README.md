# SCVocoder5

## 🌐 Language / 言語

### [日本語](README.md)　|　[English](README_EN.md)

表示言語：**日本語**

---

KORG drumlogue用のボコーダーと専用キャリア・シンセです。

[最新版のダウンロード](https://github.com/ufotone/SCVocoder5/releases/latest)

## 収録ユニット

- `SC Vocoder 5 v0.7.0`：Master FX用ボコーダー
- `AGE Carrier v0.4.0`：MULTIトラック用SAW和音シンセ

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

マイクは、原則としてプリアンプ付きのセルフパワー・マイク、または外部マイクプリアンプを通したものが必要です。一般的なパッシブ・マイクを直接接続しても使えません。電源が必要なマイクには、マイク自身の電池や外部電源を使用してください。

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
|FLANGER|0〜100%|0%|フランジャーの量を調整します。|
|FLG RATE|0〜100%|20%|フランジャーの揺れる速さ（0.05〜1.5Hz）を調整します。|
|REVERB|0〜100%|0%|リバーブの量を調整します。|

### Page 5

|パラメーター|範囲|初期値|説明|
|---|---:|---:|---|
|REV TIME|300〜5000ms|1800ms|リバーブの減衰時間の目安を調整します。|

処理順はCHORUS → FLANGER → REVERBです。FLANGERは量に応じて最大約+3.01dBの自動ゲイン補正を行い、音量低下を抑えます。補正はフィードバックの外側で適用し、最終出力の安全リミッターを維持しています。OUTPUTの上限は従来の+12dBです。追加FXの初期値はOFF。MIXの原音・VOICE DRY・MONITORのCARRIER / MODULATORは追加FXをバイパスします。

## AGE Carrier

v0.4.0は従来のAGE Carrierの改良版です。初期値は6音リアルタイム演奏です。

|MODE|動作|
|---|---|
|LIVE6（初期値）|最大6音の鍵盤演奏をそのまま鳴らします。|
|NOTE|入力単音をルートに、選んだコードを生成します。|
|FIXED|本体でROOT・OCTAVE・CHORDを指定し、単音で和音をトリガーします。|

NOTE / FIXEDでは1和音だけを発音し、新しいトリガーで前の和音を置き換えます。BASSを含め最大4音まで生成します。

- Page 1: ATTACK / DECAY / RELEASE / LEVEL（従来の範囲・初期値・番号を維持）。
- Page 2: CHORD / ROOT / OCTAVE / MODE。
- Page 3: BASS（OFF、またはC〜Bの音名。初期値OFF）。

CHORDはMAJ / MIN / DIM / AUG / SUS2 / SUS4 / 7 / MAJ7 / MIN7 / M7B5 / MIN6 / MINb6 / MAJ6 / DIM7 / MMAJ7 / ADD9 / MADD9 / 9 / MAJ9 / MIN9 / 7SUS4 / 6/9 / M6/9 / 7b5 / 7#5 / M7#5 / 7b9 / 7#9 / 7#11 / 7b13 / 13 / M13 / m13 / 11 / m11 / 7SUS2 / M7S2 / M7S4 / ADD11 / MAD11の40種類です。初期値MAJ。
9th / 6/9系は5度を省略した4音に変更しました。11th / 13thなどの拡張コードも一部省略の4音です。LIVE6は従来の6音演奏を維持します。

BASSで最低音を独立指定できます。構成音なら低音へ移し、構成音でなければ1音追加します。5音になる場合は自然5度、なければルートを省略して最大4音に収めます。
NOTEモードでもBASSは絶対音名です。ROOT・BASSの黒鍵表記はDb / Eb / Gb / Ab / Bbです。設定変更は次のトリガーから反映します。

例の進行はMODE=FIXED、OCTAVE=3、BASS=Gで指定できます。

|コード|ROOT|CHORD|鳴る音|
|---|---|---|---|
|Gm7|G|MIN7|G3–Bb3–D4–F4|
|Gm6|G|MIN6|G3–Bb3–D4–E4|
|Gm(♭6)|G|MINb6|G3–Bb3–D4–Eb4|
|A♭M7/G|Ab|MAJ7|G3–Ab3–C4–Eb4|

各コードはROOT・CHORD・BASSを選んで指定します。進行を鳴らすには表のROOTとCHORDを切り替えます。
詳細は[AGE Carrierの説明](age-carrier/README.md)を参照してください。

表示名はAGE Carrier、ファイル名は `age_carrier.drmlgunit`。元のunit IDと既存8パラメーターの番号を維持しています。

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
