# ts_illumi2019
都立多摩科学技術高校無線工作部で使用するためのイルミネーション制御プログラムです。


## 注意

- 読者はハードウェアの仕様は理解しているものとします。
- このプロジェクトは現在非推奨です。新プロジェクト [illumiGenerator](https://github.com/jj1lis/illumiGenerator)が進行中ですのでそちらも参照してください。

# 使い方

## 定数

ユーザが書き換える必要のある定数は以下の通りです
|定数名|説明|
|------|---|
|DUTY_MAX|PWMの周期。0より大きい範囲|
|DUTY_RATIO|PWMのDuty比。0-DUTY_MAXの間で設定|
|FLUSH_CYCLE|点滅周期(ミリ秒)|
|OUT_MIN|出力ピンの一番下|
|OUT_MAX|出力ピンの一番上|
|SW_MODE|モード切替スイッチのピン|

## 制御
### モード
モードは以下の三つが用意されています。モードの数および名前は変更不可です。
1. xmas
2. mochi
3. oni

### プログラム
[ソースコード](ts_illumi2019.ino)の序盤には以下のような記述が続いています：

```
namespace Common{
    float pin2(float phase){
        return 1.;
    }
    float pin3(float phase){
        ........
        ........
}


namespace Xmas{     //クリスマス
    float pin9(float phase){
        return 1.;
    }
    .......
    .......
    //以下続く
```

それぞれの`namespace`は各モードを表しており、その中の関数`pinX`はX番ピンの動作について記述しています。
`namespace Common`はどのモードにも属さない部分で、各モードで共通の動作をするピン（雪だるまの胴体など）について記述すると、他のすべてのモードでこれが適用されます。

これらを書き換えることによって、自由にピンごとの動作を指定することができます。
このとき、絶対に**各モードと`Common`において記述するピンの番号に重複があってはいけません**。仮に重複があった場合の動作は未定義ですが、恐らくコンパイルエラーが発生するか`Common`の処理が優先されるでしょう。

これより下にあるソースコード部分については**変更しないでください**。ただしユーザが[ソースコード](ts_illumi2019.ino)の内容を理解しており、予想外に発生した不具合を修正する場合は除きます。


# Contact
不具合、不明な点についての問い合わせはこのレポジトリのIssuesのほか、以下の連絡先からもどうぞ。

- mail : rinta.nambuline205@gmail.com
- Twitter : @_SIL1JJ
