# SC Vocoder 5 v0.7.0

KORG drumlogueのMaster FX用ボコーダー。v0.5.1のコーラスに加えて、フランジャーとリバーブを搭載しています。ディレイは効果がないので見送りました。フランジャーの音量補正を追加しました。AGE Carrierと組み合わせて使用できます。

追加エフェクトはすべて初期値0%（OFF）です。全17項目です.

## インストール

`sc_vocoder5.drmlgunit` をdrumlogueの `Units/MasterFXs/` にコピーして通常起動します。既存のSC Vocoder 5を更新します。AGE Carrierの更新は不要です。

## 追加エフェクト

|項目|範囲|初期値|内容|
|---|---|---|---|
|FLANGER|0〜100%|0%|フランジャーの量。遅延時間0.5〜4.5ms、固定フィードバック45%。|
|FLG RATE|0〜100%|20%|フランジャーの揺れる速さ。0.05〜1.5Hzに対応。|
|REVERB|0〜100%|0%|ステレオ残響の量。|
|REV TIME|300〜5000ms|1800ms|リバーブの減衰時間の目安。高域は早く減衰します。|

エフェクト順は **CHORUS → FLANGER → REVERB** です。ボコーダー音とUNVOICED成分に追加エフェクトをかけます。MIXで混ぜる原音とVOICE DRYはそのまま加えます。MIX=0%では追加エフェクトも聞こえません。

MONITORのCARRIER / MODULATORは、すべてのエフェクトをバイパスします。VOCODERへ戻す際は追加エフェクトの残響を消去し、過去の音が再生されないようにしています。各追加エフェクトは0%に下げると短くフェードアウトします。再度上げたときは、OFF中に凍結した古い音を再生しません。

フランジャーは量に応じて自動でゲインを補正します。0%では0dB、100%では約+3.01dBです。補正量はFLANGERの値と一緒に滑らかに変化します。フィードバックの書き込み後に補正するため、補正によってループの増幅率は上がりません。OUTPUTの上限は従来の+12dBのままです。

補正は音量低下を抑えるためのもので、フランジャー本来の周波数の谷や音色の変化は残ります。最終出力の安全リミッターも維持しています。

## パラメーターのページ

|ページ|項目1|項目2|項目3|項目4|
|---|---|---|---|---|
|1|FORMANT|RELEASE|RANGE|MIX|
|2|ATTACK|MOD GAIN|OUTPUT|UNVOICED|
|3|INPUT|MONITOR|VOICE DRY|WIDTH|
|4|CHORUS|FLANGER|FLG RATE|REVERB|
|5|REV TIME|—|—|—|

まずREVERB=20%程度で試せます。FLANGERは10〜20%から調整してください。

## 既存パラメーター

FORMANT −12〜+12（0）、RELEASE 1〜500ms（100）、RANGE FULL / AGE（AGE）、MIX 0〜100%（100）、ATTACK 1〜50ms（4）、MOD GAIN −24〜+50dB（+35）、OUTPUT −24〜+12dB（0）、UNVOICED 0〜100%（0）、INPUT SIDECHAIN / L/R SPLIT（SIDECHAIN）、MONITOR VOCODER / CARRIER / MODULATOR（VOCODER）、VOICE DRY 0〜100%（0）、WIDTH 0〜100%（100）、CHORUS 0〜100%（35）。括弧内は初期値です。

## ビルド・検証

```powershell
./build.ps1 -SdkRoot 'C:\path\to\logue-sdk'
```

SDKの `platform/drumlogue/sc-vocoder70` にビルドします。既存ディレクトリーがある場合は上書きせず停止します。

Visual Studio開発者PowerShellでは `./test-host.ps1 -SdkRoot 'C:\path\to\logue-sdk'` でホストテストを実行できます。

詳細はVALIDATION.mdを参照してください。ARMビルドとホストテストは完了しています。今回の変更後の実機での音量感・音質・CPU負荷・安定性は未確認です。

MIT License。KORGテンプレート部分は同梱のLICENSE-KORGによります。tests/vocoder_v051_reference.hは互換性比較用のv0.5.1実装です。
