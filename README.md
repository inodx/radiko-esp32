# Radiko for ATOM ECHO
jitenshap/radiko-esp32からのフォークでATOM ECHO用になります。

# 注意事項
無保証です。自己責任でお楽しみください。外部接続などで場合によっては機器を破損したりする場合もあります。
またM5 ATOM ECHOでは「長時間の連続再生は推奨されていませんhttps://www.switch-science.com/catalog/6347/
」。特に初期出荷モデルでは破損する可能性があります。十分ご注意ください。

## Change 10/4
- ステレオの再生が片側チャンネルだったのを、ミックス後１チャンネル再生へ
- ボリュームコントロールが可能になりました。ボタンを長押し（0.5程度）すると音量が下がります。
- ボリュームはある程度下がると元に戻ります。
- デフォルトの音量が-10になっています。
- ECHO用のドライバもおかしかった部分を修正

## Change 10/6
- LEDの制御を外部ライブラリからADF組み込みライブラリへ変更
- ボリューム変更とLEDの明るさが連動するように修正
- その他コード整理

## Compatibility
基本M5 ATOM ECHO用です。コンフィグでM5 StickCでも動作可能です。

## Install
以下の事前導入が必要です。導入方法を検索してからインストールするのをお勧めします。
- ESP-IDF　(4.1のみでテスト）https://github.com/espressif/esp-idf
- ESP-ADF https://github.com/espressif/esp-adf

## menuconfig
![png](https://github.com/inodx/radiko-esp32/blob/main/etc/screenshot1.png)
idf.py menuconfigコマンドから以下の設定を行います

- `Audio HAL` = `Custom audio board`を選択
- `Example Configuration` > `WiFi SSID` and `WiFi Password` = 環境に合わせてください
- `Radiko Configuration` > `auth key` = Radiko初期認証用のコード。配布してよいのか不明なのでRADIKO_AUTHKEY_VALUEで検索してください
- `Component config` > "ESP32-specific" > "Support for external, SPI-connected RAM" = OFF

IDFでコンパイル転送します（idf.py -p COM3 flash -b 115200など環境で異なります）

# Useage
- 起動すると設定のWifiに接続し、Radikoを再生します。
- ボタンを押すと局を変更
- 起動で赤LED,Wifi接続OKで青LED、ストリーミング開始で緑LEDが点灯
- 起動後の再生される局は局リストの６番目です。個人的に都合が良いので

# M5 StickCの場合
- Output Configuration > Select play mp3 output   = Enable PWM outputへ変更
- Output Configuration > Select play mp3 output > PWM Stream Right Output GPIO NUM =25
- Output Configuration > Select play mp3 output > PWM Stream Left Output GPIO NUM =26
- 動作中は赤LEDがONになります
- StickC+SpakerHATで再生可能（音質悪い）。サードボタンで局変更
- この設定で任意のGPIOからオーディオ出力可能なのでECHOでも設定次第で外部スピーカ等をつなげます
- 音質についてはローパスフィルタをつけることで改善可能です



以下シリアルに出るログのサンプルです
https://github.com/inodx/radiko-esp32/blob/main/etc/logsample2.txt
