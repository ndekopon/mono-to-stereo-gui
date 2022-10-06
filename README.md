# mono-to-stereo-gui

## 概要

MS2109系USBキャプチャを使っているユーザー向け[mono-to-stereo](https://github.com/ToadKing/mono-to-stereo)のGUI操作版。


## インストール方法

Releasesから自分のPCにあったアーキテクチャ(x86/x64/arm/arm64)のZIPをダウンロードして解凍する。

解凍先は、書き込みに管理者権限が必要でないフォルダであればどこでも良い。

(例：`C:\Users\ndekopon\Documents\mono-to-strereo-gui` など)


## 使い方

### 基本

起動するとタスクトレイに常駐する。

右クリックメニューから`capture`(入力)を選択する。

![image](https://user-images.githubusercontent.com/92087784/194231996-b46a864f-a94a-40bd-b599-891eff74331b.png)

右クリックメニューから`render`(出力)を選択する。

![image](https://user-images.githubusercontent.com/92087784/194232196-f818a143-ba69-4ae0-8ee5-bfdd83850a72.png)

選択時、両方が有効であればモノラル→ステレオの変換が開始される。

選択結果は同フォルダの`config.ini`に保存されるので、次回起動時は自動的にステレオ変換が開始される。

終了する際は、右クリックメニューから`exit`を選択する。

![image](https://user-images.githubusercontent.com/92087784/194233720-4f2e5948-6864-4622-9200-efaaebb6f71a.png)


### L/R反転

L/Rが逆に聞こえる場合、右クリックメニューから`reverse channel`を選択する。

![image](https://user-images.githubusercontent.com/92087784/194232915-0b4f5e5a-ab4c-4260-938d-e5b7b19a30a7.png)

### 音量を変える

音量を調節したい場合、右クリックメニューの`volume`から選択する。

![image](https://user-images.githubusercontent.com/92087784/194233329-ad27d0e0-8d90-446a-b61a-72f131c47156.png)


## 問題点

手元の環境では入力/出力の速度が一定でないのか、出力バッファが追いついたり離れすぎたりするので、
閾値を超えたら都度1フレームをスキップしたり、前の1フレームを重複させたりして違和感が減るような処理を入れた。

スキップ・重複を実施した回数はアイコンにカーソルを合わせた際に表示される。

![image](https://user-images.githubusercontent.com/92087784/193987174-92411b22-bf03-4eb9-b830-abafd9ba50cb.png)


ほかに問題点があったら同フォルダに生成されるログとともに [@ndekopon](https://twitter.com/ndekopon) へどうぞ。


## テスト環境

下記環境で検証し、音が途切れることなくステレオで聞こえることを確認。

- OS: Windows10 21H2(19044.2006)
- CPU: AMD Ryzen 5 5600X


## Q&A

- Q. `render`に出力したいデバイスが表示されない
    - A. 出力デバイスはサンプルレートが48000Hzになっていないと恐らく表示されないので、48000Hzに変更する
- Q. 複数MS2109のデバイスがあって同時に使いたい
    - A. デバイス毎にフォルダを作り、それぞれに`mono-to-stereo-gui.exe`を入れて使う


## ライセンス

[MIT license](https://choosealicense.com/licenses/mit/)


## 謝辞

音声処理部分は [mono-to-stereo](https://github.com/ToadKing/mono-to-stereo) を参考に改変した。感謝。

