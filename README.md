# Radiko for ATOM ECHO
jitenshap/radiko-esp32からのフォークでATOM ECHO用になります。

## Compatibility
基本M5 ATOM ECHO用です。コンフィグでM5 StickCでも動作可能です。

## Install
以下の事前導入が必要です。
- ESP-IDF　(4.1のみでテスト）
- ESP-ADF

## menuconfig
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

# M5 StickCの場合
- Output Configuration > Select play mp3 output   = Enable PWM outputへ変更
- Output Configuration > Select play mp3 output > PWM Stream Right Output GPIO NUM =25
- Output Configuration > Select play mp3 output > PWM Stream Left Output GPIO NUM =26

StickC+SpakerHATで再生可能（音質悪い）。サードボタンで局変更

動作中は赤LEDがONになります
