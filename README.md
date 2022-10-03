# mono-to-stereo-gui

## 概要

MS2109系USBキャプチャを使っているユーザー向け[mono-to-stereo](https://github.com/ToadKing/mono-to-stereo)のGUIで操作できる版。

![image](https://user-images.githubusercontent.com/92087784/193593505-111b864e-fe1a-4d71-8e2c-73be9d952ec4.png)


## インストール方法

Releasesから自分のPCにあったアーキテクチャ(x86/x64/arm/arm64)のZIPをダウンロードして解凍する。

解凍先は書き込めるフォルダであればどこでも良い。

書き込み権限はログと設定ファイルを書き込むために必要。


## 使い方

起動するとタスクトレイに常駐するので、"render"(出力)と"capture"(入力)を選択する。

選択時、両方が有効であればモノラル→ステレオの変換が開始される。


## 問題点

手元の環境では入力/出力の速度が一定でないのか、出力バッファが追いついたり離れすぎたりするので、
閾値を超えたら都度1フレームをスキップしたり、前の1フレームを重複させたりして違和感が減るような処理を入れた。

プログラム自体に問題点があったら [@ndekopon](https://twitter.com/ndekopon) へどうぞ。


## テスト環境

下記環境で検証し、音がある程度問題なく聞こえることを確認。

- OS: Windows10 21H2(19044.2006)
- CPU: AMD Ryzen 5 5600X


## ライセンス

[MIT license](https://choosealicense.com/licenses/mit/)


## 謝辞

音声処理部分は [mono-to-stereo](https://github.com/ToadKing/mono-to-stereo) を参考にしました。

