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
- 起動後の再生される局は局リストの６番目です。個人的に都合が良いので

# M5 StickCの場合
- Output Configuration > Select play mp3 output   = Enable PWM outputへ変更
- Output Configuration > Select play mp3 output > PWM Stream Right Output GPIO NUM =25
- Output Configuration > Select play mp3 output > PWM Stream Left Output GPIO NUM =26
- 動作中は赤LEDがONになります
- StickC+SpakerHATで再生可能（音質悪い）。サードボタンで局変更
- この設定で任意のGPIOからオーディオ出力可能なのでECHOでも設定次第で外部スピーカ等をつなげます
- 音質についてはローパスフィルタをつけることで改善可能です

無保証です。自己責任でお楽しみください。外部接続などで場合によっては機器を破損したりする場合もあります。

以下シリアルに出るログのサンプルです
```
21:07:14.924 -> rst:0x1 (POWERON_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
21:07:14.924 -> configsip: 188777542, SPIWP:0xee
21:07:14.924 -> clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
21:07:14.924 -> mode:DIO, clock div:1
21:07:14.924 -> load:0x3fff0030,len:4
21:07:14.924 -> load:0x3fff0034,len:7336
21:07:14.924 -> load:0x40078000,len:13792
21:07:14.970 -> ho 0 tail 12 room 4
21:07:14.970 -> load:0x40080400,len:4560
21:07:14.970 -> entry 0x400806a4
21:07:14.970 -> [0;32mI (31) boot: ESP-IDF v4.1.2-dirty 2nd stage bootloader[0m
21:07:14.970 -> [0;32mI (31) boot: compile time 20:01:57[0m
21:07:14.970 -> [0;32mI (31) boot: chip revision: 1[0m
21:07:14.970 -> [0;32mI (35) boot_comm: chip revision: 1, min. bootloader chip revision: 0[0m
21:07:14.970 -> [0;32mI (42) qio_mode: Enabling default flash chip QIO[0m
21:07:14.970 -> [0;32mI (47) boot.esp32: SPI Speed      : 80MHz[0m
21:07:14.970 -> [0;32mI (52) boot.esp32: SPI Mode       : QIO[0m
21:07:14.970 -> [0;32mI (56) boot.esp32: SPI Flash Size : 4MB[0m
21:07:15.018 -> [0;32mI (61) boot: Enabling RNG early entropy source...[0m
21:07:15.018 -> [0;32mI (66) boot: Partition Table:[0m
21:07:15.018 -> [0;32mI (70) boot: ## Label            Usage          Type ST Offset   Length[0m
21:07:15.018 -> [0;32mI (77) boot:  0 nvs              WiFi data        01 02 00009000 00006000[0m
21:07:15.018 -> [0;32mI (85) boot:  1 phy_init         RF data          01 01 0000f000 00001000[0m
21:07:15.018 -> [0;32mI (92) boot:  2 factory          factory app      00 00 00010000 00300000[0m
21:07:15.018 -> [0;32mI (100) boot: End of partition table[0m
21:07:15.018 -> [0;32mI (104) boot_comm: chip revision: 1, min. application chip revision: 0[0m
21:07:15.064 -> [0;32mI (111) esp_image: segment 0: paddr=0x00010020 vaddr=0x3f400020 size=0x2facc (195276) map[0m
21:07:15.112 -> [0;32mI (177) esp_image: segment 1: paddr=0x0003faf4 vaddr=0x3ffb0000 size=0x00524 (  1316) load[0m
21:07:15.112 -> [0;32mI (178) esp_image: segment 2: paddr=0x00040020 vaddr=0x400d0020 size=0xb8f50 (757584) map[0m
21:07:15.346 -> [0;32mI (407) esp_image: segment 3: paddr=0x000f8f78 vaddr=0x3ffb0524 size=0x02d30 ( 11568) load[0m
21:07:15.346 -> [0;32mI (411) esp_image: segment 4: paddr=0x000fbcb0 vaddr=0x40080000 size=0x00404 (  1028) load[0m
21:07:15.346 -> [0;32mI (414) esp_image: segment 5: paddr=0x000fc0bc vaddr=0x40080404 size=0x162d4 ( 90836) load[0m
21:07:15.393 -> [0;32mI (467) boot: Loaded app from partition at offset 0x10000[0m
21:07:15.393 -> [0;32mI (467) boot: Disabling RNG early entropy source...[0m
21:07:15.393 -> [0;32mI (467) cpu_start: Pro cpu up.[0m
21:07:15.393 -> [0;32mI (471) cpu_start: Application information:[0m
21:07:15.393 -> [0;32mI (476) cpu_start: Project name:     radiko-esp32[0m
21:07:15.439 -> [0;32mI (481) cpu_start: App version:      c495fec-dirty[0m
21:07:15.439 -> [0;32mI (487) cpu_start: Compile time:     Oct  1 2021 20:12:11[0m
21:07:15.439 -> [0;32mI (493) cpu_start: ELF file SHA256:  721aef7b03f6f159...[0m
21:07:15.439 -> [0;32mI (499) cpu_start: ESP-IDF:          v4.1.2-dirty[0m
21:07:15.439 -> [0;32mI (504) cpu_start: Starting app cpu, entry point is 0x40081384[0m
21:07:15.439 -> [0;32mI (0) cpu_start: App cpu up.[0m
21:07:15.439 -> [0;32mI (514) heap_init: Initializing. RAM available for dynamic allocation:[0m
21:07:15.439 -> [0;32mI (521) heap_init: At 3FFAE6E0 len 00001920 (6 KiB): DRAM[0m
21:07:15.487 -> [0;32mI (527) heap_init: At 3FFB8560 len 00027AA0 (158 KiB): DRAM[0m
21:07:15.487 -> [0;32mI (534) heap_init: At 3FFE0440 len 00003AE0 (14 KiB): D/IRAM[0m
21:07:15.487 -> [0;32mI (540) heap_init: At 3FFE4350 len 0001BCB0 (111 KiB): D/IRAM[0m
21:07:15.487 -> [0;32mI (546) heap_init: At 400966D8 len 00009928 (38 KiB): IRAM[0m
21:07:15.487 -> [0;32mI (553) cpu_start: Pro cpu start user code[0m
21:07:15.487 -> [0;32mI (570) spi_flash: detected chip: gd[0m
21:07:15.487 -> [0;32mI (570) spi_flash: flash io: qio[0m
21:07:15.487 -> [0;32mI (570) cpu_start: Starting scheduler on PRO CPU.[0m
21:07:15.533 -> [0;32mI (0) cpu_start: Starting scheduler on APP CPU.[0m
21:07:15.533 -> [0;32mI (621) HTTP_LIVINGSTREAM_EXAMPLE: [ 1 ] Start audio codec chip[0m
21:07:15.533 -> [0;32mI (621) new_codec: new_codec init[0m
21:07:15.533 -> [0;32mI (621) ECHO_BOARD: init audio board[0m
21:07:15.533 -> [0;32mI (631) AUDIO_HAL: Codec mode is 2, Ctrl:1[0m
21:07:15.533 -> [0;32mI (631) HTTP_LIVINGSTREAM_EXAMPLE: [2.0] Create audio pipeline for playback[0m
21:07:15.533 -> [0;32mI (641) HTTP_LIVINGSTREAM_EXAMPLE: [2.1] Create http stream to read data[0m
21:07:15.581 -> [0;32mI (651) HTTP_LIVINGSTREAM_EXAMPLE: [2.2] Create i2s stream to write data to codec chip[0m
21:07:15.581 -> [0;32mI (661) I2S: DMA Malloc info, datalen=blocksize=1200, dma_buf_count=3[0m
21:07:15.581 -> [0;32mI (661) I2S: DMA Malloc info, datalen=blocksize=1200, dma_buf_count=3[0m
21:07:15.581 -> [0;32mI (691) I2S: APLL: Req RATE: 44100, real rate: 44099.988, BITS: 16, CLKM: 1, BCK_M: 8, MCLK: 11289597.000, SCLK: 1411199.625000, diva: 1, divb: 0[0m
21:07:15.581 -> [0;32mI (691) ECHO_V1_0: I2S0, MCLK output by GPIO0[0m
21:07:15.627 -> [0;32mI (691) HTTP_LIVINGSTREAM_EXAMPLE: [2.3] Create aac decoder to decode aac file[0m
21:07:15.627 -> [0;32mI (701) HTTP_LIVINGSTREAM_EXAMPLE: [2.4] Register all elements to audio pipeline[0m
21:07:15.627 -> [0;32mI (711) HTTP_LIVINGSTREAM_EXAMPLE: [2.5] Link it together http_stream-->aac_decoder-->i2s_stream-->[codec_chip][0m
21:07:15.627 -> [0;32mI (721) AUDIO_PIPELINE: link el->rb, el:0x3ffbe328, tag:http, rb:0x3ffc0de8[0m
21:07:15.627 -> [0;32mI (731) AUDIO_PIPELINE: link el->rb, el:0x3ffc0a60, tag:aac, rb:0x3ffc5f30[0m
21:07:15.627 -> [0;32mI (741) HTTP_LIVINGSTREAM_EXAMPLE: [ 3 ] Create and start input key service[0m
21:07:15.673 -> [0;32mI (741) gpio: GPIO[39]| InputEn: 1| OutputEn: 0| OpenDrain: 0| Pullup: 1| Pulldown: 0| Intr:3 [0m
21:07:15.673 -> [0;32mI (751) HTTP_LIVINGSTREAM_EXAMPLE: [ 3 ] Start and wait for Wi-Fi network[0m
21:07:15.673 -> I (781) wifi:wifi driver task: 3ffcb450, prio:23, stack:6656, core=0
21:07:15.673 -> [0;32mI (781) system_api: Base MAC address is not set, read default base MAC address from BLK0 of EFUSE[0m
21:07:15.673 -> [0;32mI (781) system_api: Base MAC address is not set, read default base MAC address from BLK0 of EFUSE[0m
21:07:15.720 -> I (801) wifi:wifi firmware version: 1424aca
21:07:15.720 -> I (801) wifi:config NVS flash: enabled
21:07:15.720 -> I (801) wifi:config nano formating: disabled
21:07:15.720 -> I (801) wifi:Init data frame dynamic rx buffer num: 32
21:07:15.720 -> I (811) wifi:Init management frame dynamic rx buffer num: 32
21:07:15.720 -> I (811) wifi:Init management short buffer num: 32
21:07:15.720 -> I (821) wifi:Init static tx buffer num: 16
21:07:15.768 -> I (821) wifi:Init static rx buffer size: 1600
21:07:15.768 -> I (831) wifi:Init static rx buffer num: 10
21:07:15.768 -> I (831) wifi:Init dynamic rx buffer num: 32
21:07:15.768 -> [0;32mI (831) wifi_init: rx ba win: 6[0m
21:07:15.768 -> [0;32mI (841) wifi_init: tcpip mbox: 32[0m
21:07:15.768 -> [0;32mI (841) wifi_init: udp mbox: 6[0m
21:07:15.768 -> [0;32mI (841) wifi_init: tcp mbox: 6[0m
21:07:15.768 -> [0;32mI (851) wifi_init: tcp tx win: 5744[0m
21:07:15.768 -> [0;32mI (851) wifi_init: tcp rx win: 5744[0m
21:07:15.768 -> [0;32mI (861) wifi_init: tcp mss: 1440[0m
21:07:15.768 -> [0;32mI (861) wifi_init: WiFi IRAM OP enabled[0m
21:07:15.768 -> [0;32mI (871) wifi_init: WiFi RX IRAM OP enabled[0m
21:07:15.815 -> [0;32mI (871) phy_init: phy_version 4660,0162888,Dec 23 2020[0m
21:07:15.862 -> I (961) wifi:mode : sta (50:02:91:9f:43:14)
21:07:17.082 -> I (2171) wifi:new:<11,0>, old:<1,0>, ap:<255,255>, sta:<11,0>, prof:1
21:07:17.644 -> I (2721) wifi:state: init -> auth (b0)
21:07:17.644 -> I (2721) wifi:state: auth -> assoc (0)
21:07:17.644 -> I (2721) wifi:state: assoc -> run (10)
21:07:17.738 -> I (2841) wifi:connected with aterm-938f40-g, aid = 1, channel 11, BW20, bssid = 6c:e4:da:51:37:32
21:07:17.738 -> I (2841) wifi:security: WPA2-PSK, phy: bgn, rssi: -58
21:07:17.738 -> I (2841) wifi:pm start, type: 1
21:07:17.786 -> 
21:07:17.786 -> [0;33mW (2841) PERIPH_WIFI: WiFi Event cb, Unhandle event_base:WIFI_EVENT, event_id:4[0m
21:07:17.786 -> I (2881) wifi:AP's beacon interval = 102400 us, DTIM period = 1
21:07:19.048 -> [0;32mI (4121) esp_netif_handlers: sta ip: 192.168.10.6, mask: 255.255.255.0, gw: 192.168.10.1[0m
21:07:19.048 -> [0;32mI (4121) PERIPH_WIFI: Got ip:192.168.10.6[0m
21:07:19.048 -> [0;32mI (4121) GET: Starting auth1[0m
21:07:20.319 -> [0;32mI (5411) EVT_HTTP: HTTP_EVENT_ON_HEADER, key=Server, value=nginx[0m
21:07:20.319 -> [0;32mI (5411) EVT_HTTP: HTTP_EVENT_ON_HEADER, key=Date, value=Fri, 01 Oct 2021 12:07:18 GMT[0m
21:07:20.319 -> [0;32mI (5421) EVT_HTTP: HTTP_EVENT_ON_HEADER, key=Content-Type, value=text/plain[0m
21:07:20.319 -> [0;32mI (5421) EVT_HTTP: HTTP_EVENT_ON_HEADER, key=Transfer-Encoding, value=chunked[0m
21:07:20.368 -> [0;32mI (5431) EVT_HTTP: HTTP_EVENT_ON_HEADER, key=Connection, value=keep-alive[0m
21:07:20.368 -> [0;32mI (5441) EVT_HTTP: HTTP_EVENT_ON_HEADER, key=X-Radiko-AppType, value=pc[0m
21:07:20.368 -> [0;32mI (5451) EVT_HTTP: HTTP_EVENT_ON_HEADER, key=X-Radiko-AppType2, value=pc[0m
21:07:20.368 -> [0;32mI (5451) EVT_HTTP: HTTP_EVENT_ON_HEADER, key=X-Radiko-Authtoken, value=ZIb867Q6hFLM_mnBggE8Sg[0m
21:07:20.368 -> [0;32mI (5461) EVT_HTTP: GOT TOKEN[0m
21:07:20.368 -> [0;32mI (5471) EVT_HTTP: HTTP_EVENT_ON_HEADER, key=X-Radiko-AuthWait, value=0[0m
21:07:20.368 -> [0;32mI (5471) EVT_HTTP: HTTP_EVENT_ON_HEADER, key=X-Radiko-Delay, value=15[0m
21:07:20.414 -> [0;32mI (5481) EVT_HTTP: HTTP_EVENT_ON_HEADER, key=X-Radiko-KeyLength, value=16[0m
21:07:20.414 -> [0;32mI (5491) EVT_HTTP: GOT LENGTH[0m
21:07:20.414 -> [0;32mI (5491) EVT_HTTP: HTTP_EVENT_ON_HEADER, key=X-Radiko-KeyOffset, value=19[0m
21:07:20.414 -> [0;32mI (5501) EVT_HTTP: GOT OFFSET[0m
21:07:20.414 -> [0;32mI (5501) EVT_HTTP: HTTP_EVENT_ON_HEADER, key=Access-Control-Expose-Headers, value=X-Radiko-AuthToken, X-Radiko-Partialkey, X-Radiko-AppType, X-Radiko-AuthWait, X-Radiko-Delay, X-Radiko-KeyLength, X-Radiko-KeyOffset[0m
21:07:20.459 -> [0;32mI (5521) EVT_HTTP: HTTP_EVENT_ON_HEADER, key=Credentials, value=true[0m
21:07:20.459 -> [0;32mI (5531) EVT_HTTP: Received data: please send a part of key[0m
21:07:20.459 -> [0;32mI (5541) GET: HTTPS Status = 200, content_length = -1[0m
21:07:20.459 -> [0;32mI (5541) GET: Token: ZIb867Q6hFLM_mnBggE8Sg[0m
21:07:20.459 -> [0;32mI (5551) GET: Partial Key: f2fd66c32209da9c[0m
21:07:20.459 -> [0;32mI (5551) EVT_HTTP: HTTP_EVENT_DISCONNECTED[0m
21:07:20.459 -> [0;32mI (5561) GET: Base64 encoded: ZjJmZDY2YzMyMjA5ZGE5Yw==[0m
21:07:20.507 -> [0;32mI (5561) GET: Starting auth2[0m
21:07:21.773 -> [0;32mI (6881) GET: HTTPS Status = 200, content_length = -1[0m
21:07:21.820 -> [0;32mI (6881) GET: Region: JP10[0m
21:07:21.820 -> [0;32mI (6881) EVT_HTTP: HTTP_EVENT_DISCONNECTED[0m
21:07:21.820 -> [0;32mI (6891) GET ST LIST: Allocating xml buffer[0m
21:07:21.820 -> [0;32mI (6891) GET ST LIST: Starting https request[0m
21:07:23.174 -> [0;32mI (8271) GET ST LIST: HTTPS Status = 200, content_length = 24751[0m
21:07:23.174 -> [0;32mI (8281) ELEMDATA: [ID]: TBS[0m
21:07:23.174 -> [0;32mI (8281) ELEMDATA: [NAME]: TBSラジオ[0m
21:07:23.174 -> [0;32mI (8281) ELEMDATA: [ASCII_NAME]: TBS RADIO[0m
21:07:23.222 -> [0;32mI (8281) ELEMDATA: [LOGO_XSMALL]: http://radiko.jp/station/logo/TBS/logo_xsmall.png[0m
21:07:23.222 -> 
21:07:23.222 -> [0;32mI (8291) ELEMDATA: [ID]: QRR[0m
21:07:23.222 -> [0;32mI (8291) ELEMDATA: [NAME]: 文化放送[0m
21:07:23.222 -> [0;32mI (8301) ELEMDATA: [ASCII_NAME]: JOQR BUNKA HOSO[0m
21:07:23.222 -> [0;32mI (8301) ELEMDATA: [LOGO_XSMALL]: http://radiko.jp/station/logo/QRR/logo_xsmall.png[0m
21:07:23.222 -> 
21:07:23.222 -> [0;32mI (8311) ELEMDATA: [ID]: LFR[0m
21:07:23.222 -> [0;32mI (8321) ELEMDATA: [NAME]: ニッポン放送[0m
21:07:23.222 -> [0;32mI (8321) ELEMDATA: [ASCII_NAME]: JOLF NIPPON HOSO[0m
21:07:23.222 -> [0;32mI (8331) ELEMDATA: [LOGO_XSMALL]: http://radiko.jp/station/logo/LFR/logo_xsmall.png[0m
21:07:23.269 -> 
21:07:23.269 -> [0;32mI (8341) ELEMDATA: [ID]: RN1[0m
21:07:23.269 -> [0;32mI (8341) ELEMDATA: [NAME]: ラジオNIKKEI第1[0m
21:07:23.269 -> [0;32mI (8341) ELEMDATA: [ASCII_NAME]: RADIONIKKEI[0m
21:07:23.269 -> [0;32mI (8351) ELEMDATA: [LOGO_XSMALL]: http://radiko.jp/station/logo/RN1/logo_xsmall.png[0m
21:07:23.269 -> 
21:07:23.269 -> [0;32mI (8361) ELEMDATA: [ID]: RN2[0m
21:07:23.269 -> [0;32mI (8361) ELEMDATA: [NAME]: ラジオNIKKEI第2[0m
21:07:23.269 -> [0;32mI (8371) ELEMDATA: [ASCII_NAME]: RADIONIKKEI2[0m
21:07:23.269 -> [0;32mI (8371) ELEMDATA: [LOGO_XSMALL]: http://radiko.jp/station/logo/RN2/logo_xsmall.png[0m
21:07:23.316 -> 
21:07:23.316 -> [0;32mI (8381) ELEMDATA: [ID]: INT[0m
21:07:23.316 -> [0;32mI (8381) ELEMDATA: [NAME]: InterFM897[0m
21:07:23.316 -> [0;32mI (8391) ELEMDATA: [ASCII_NAME]: InterFM897[0m
21:07:23.316 -> [0;32mI (8391) ELEMDATA: [LOGO_XSMALL]: http://radiko.jp/station/logo/INT/logo_xsmall.png[0m
21:07:23.316 -> 
21:07:23.316 -> [0;32mI (8401) ELEMDATA: [ID]: FMT[0m
21:07:23.316 -> [0;32mI (8401) ELEMDATA: [NAME]: TOKYO FM[0m
21:07:23.316 -> [0;32mI (8411) ELEMDATA: [ASCII_NAME]: TOKYO FM[0m
21:07:23.316 -> [0;32mI (8411) ELEMDATA: [LOGO_XSMALL]: http://radiko.jp/station/logo/FMT/logo_xsmall.png[0m
21:07:23.316 -> 
21:07:23.316 -> [0;32mI (8421) ELEMDATA: [ID]: FMJ[0m
21:07:23.316 -> [0;32mI (8421) ELEMDATA: [NAME]: J-WAVE[0m
21:07:23.363 -> [0;32mI (8431) ELEMDATA: [ASCII_NAME]: J-WAVE[0m
21:07:23.363 -> [0;32mI (8431) ELEMDATA: [LOGO_XSMALL]: http://radiko.jp/station/logo/FMJ/logo_xsmall.png[0m
21:07:23.363 -> 
21:07:23.363 -> [0;32mI (8441) ELEMDATA: [ID]: FMGUNMA[0m
21:07:23.363 -> [0;32mI (8451) ELEMDATA: [NAME]: FM GUNMA[0m
21:07:23.363 -> [0;32mI (8451) ELEMDATA: [ASCII_NAME]: FM GUNMA[0m
21:07:23.363 -> [0;32mI (8451) ELEMDATA: [LOGO_XSMALL]: http://radiko.jp/station/logo/FMGUNMA/logo_xsmall.png[0m
21:07:23.363 -> 
21:07:23.363 -> [0;32mI (8461) ELEMDATA: [ID]: JORF[0m
21:07:23.363 -> [0;32mI (8471) ELEMDATA: [NAME]: ラジオ日本[0m
21:07:23.363 -> [0;32mI (8471) ELEMDATA: [ASCII_NAME]: RF RADIO NIPPON[0m
21:07:23.410 -> [0;32mI (8481) ELEMDATA: [LOGO_XSMALL]: http://radiko.jp/station/logo/JORF/logo_xsmall.png[0m
21:07:23.410 -> 
21:07:23.410 -> [0;32mI (8491) ELEMDATA: [ID]: BAYFM78[0m
21:07:23.410 -> [0;32mI (8491) ELEMDATA: [NAME]: bayfm78[0m
21:07:23.410 -> [0;32mI (8491) ELEMDATA: [ASCII_NAME]: bayfm78[0m
21:07:23.410 -> [0;32mI (8501) ELEMDATA: [LOGO_XSMALL]: http://radiko.jp/station/logo/BAYFM78/logo_xsmall.png[0m
21:07:23.410 -> 
21:07:23.410 -> [0;32mI (8511) ELEMDATA: [ID]: NACK5[0m
21:07:23.410 -> [0;32mI (8511) ELEMDATA: [NAME]: NACK5[0m
21:07:23.410 -> [0;32mI (8511) ELEMDATA: [ASCII_NAME]: NACK5[0m
21:07:23.410 -> [0;32mI (8521) ELEMDATA: [LOGO_XSMALL]: http://radiko.jp/station/logo/NACK5/logo_xsmall.png[0m
21:07:23.457 -> 
21:07:23.457 -> [0;32mI (8531) ELEMDATA: [ID]: YFM[0m
21:07:23.457 -> [0;32mI (8531) ELEMDATA: [NAME]: ＦＭヨコハマ[0m
21:07:23.457 -> [0;32mI (8541) ELEMDATA: [ASCII_NAME]: Fm yokohama 84.7[0m
21:07:23.457 -> [0;32mI (8541) ELEMDATA: [LOGO_XSMALL]: http://radiko.jp/station/logo/YFM/logo_xsmall.png[0m
21:07:23.457 -> 
21:07:23.457 -> [0;32mI (8551) ELEMDATA: [ID]: IBS[0m
21:07:23.457 -> [0;32mI (8551) ELEMDATA: [NAME]: LuckyFM 茨城放送[0m
21:07:23.457 -> [0;32mI (8561) ELEMDATA: [ASCII_NAME]: IBS RADIO[0m
21:07:23.457 -> [0;32mI (8561) ELEMDATA: [LOGO_XSMALL]: http://radiko.jp/station/logo/IBS/logo_xsmall.png[0m
21:07:23.503 -> 
21:07:23.503 -> [0;32mI (8571) ELEMDATA: [ID]: HOUSOU-DAIGAKU[0m
21:07:23.503 -> [0;32mI (8581) ELEMDATA: [NAME]: 放送大学[0m
21:07:23.503 -> [0;32mI (8581) ELEMDATA: [ASCII_NAME]: HOUSOU-DAIGAKU[0m
21:07:23.503 -> [0;32mI (8591) ELEMDATA: [LOGO_XSMALL]: http://radiko.jp/station/logo/HOUSOU-DAIGAKU/logo_xsmall.png[0m
21:07:23.503 -> 
21:07:23.503 -> [0;32mI (8601) ELEMDATA: [ID]: JOAK[0m
21:07:23.503 -> [0;32mI (8601) ELEMDATA: [NAME]: NHKラジオ第1（東京）[0m
21:07:23.503 -> [0;32mI (8601) ELEMDATA: [ASCII_NAME]: JOAK[0m
21:07:23.503 -> [0;32mI (8611) ELEMDATA: [LOGO_XSMALL]: http://radiko.jp/station/logo/JOAK/logo_xsmall.png[0m
21:07:23.550 -> 
21:07:23.550 -> [0;32mI (8621) ELEMDATA: [ID]: JOAK-FM[0m
21:07:23.550 -> [0;32mI (8621) ELEMDATA: [NAME]: NHK-FM（東京）[0m
21:07:23.550 -> [0;32mI (8631) ELEMDATA: [ASCII_NAME]: JOAK-FM[0m
21:07:23.550 -> [0;32mI (8631) ELEMDATA: [LOGO_XSMALL]: http://radiko.jp/station/logo/JOAK-FM/logo_xsmall.png[0m
21:07:23.550 -> 
21:07:23.550 -> [0;32mI (8641) RESULTS: [0]: TBS[0m
21:07:23.550 -> [0;32mI (8641) RESULTS: [1]: QRR[0m
21:07:23.550 -> [0;32mI (8651) RESULTS: [2]: LFR[0m
21:07:23.596 -> [0;32mI (8651) RESULTS: [3]: RN1[0m
21:07:23.596 -> [0;32mI (8651) RESULTS: [4]: RN2[0m
21:07:23.596 -> [0;32mI (8661) RESULTS: [5]: INT[0m
21:07:23.596 -> [0;32mI (8661) RESULTS: [6]: FMT[0m
21:07:23.596 -> [0;32mI (8661) RESULTS: [7]: FMJ[0m
21:07:23.596 -> [0;32mI (8671) RESULTS: [8]: FMGUNMA[0m
21:07:23.596 -> [0;32mI (8671) RESULTS: [9]: JORF[0m
21:07:23.596 -> [0;32mI (8671) RESULTS: [10]: BAYFM78[0m
21:07:23.596 -> [0;32mI (8681) RESULTS: [11]: NACK5[0m
21:07:23.596 -> [0;32mI (8681) RESULTS: [12]: YFM[0m
21:07:23.596 -> [0;32mI (8681) RESULTS: [13]: IBS[0m
21:07:23.596 -> [0;32mI (8691) RESULTS: [14]: HOUSOU-DAIGAKU[0m
21:07:23.596 -> [0;32mI (8691) RESULTS: [15]: JOAK[0m
21:07:23.596 -> [0;32mI (8701) RESULTS: [16]: JOAK-FM[0m
21:07:23.644 -> [0;32mI (8701) GET ST LIST: Removing buffer[0m
21:07:23.644 -> [0;32mI (8701) EVT_HTTP: HTTP_EVENT_DISCONNECTED[0m
21:07:23.644 -> [0;32mI (8711) RADIKO: TMP_URL: http://f-radiko.smartstream.ne.jp/INT/_definst_/simul-stream.stream/playlist.m3u8[0m
21:07:23.644 -> [0;33mW (8721) RADIKO: URL: http://f-radiko.smartstream.ne.jp/INT/_definst_/simul-stream.stream/playlist.m3u8[0m
21:07:23.644 -> [0;32mI (8731) RADIKO: Starting get playlist[0m
21:07:23.830 -> [0;32mI (8921) RADIKO: HTTPS Status = 200, content_length = 171[0m
21:07:23.830 -> [0;32mI (8921) RADIKO: Received body: #EXTM3U
21:07:23.830 -> #EXT-X-VERSION:3
21:07:23.830 -> #EXT-X-STREAM-INF:BANDWIDTH=48719,CODECS="mp4a.40.2"
21:07:23.830 -> http://f-radiko.smartstream.ne.jp/INT/_definst_/simul-stream.stream/chunklist_w58807089.m3u8
21:07:23.830 -> [0m
21:07:23.830 -> [0;32mI (8931) RADIKO: Stream url: http://f-radiko.smartstream.ne.jp/INT/_definst_/simul-stream.stream/chunklist_w58807089.m3u8[0m
21:07:23.878 -> [0;32mI (8941) EVT_HTTP: HTTP_EVENT_DISCONNECTED[0m
21:07:23.878 -> [0;32mI (8951) HTTP_LIVINGSTREAM_EXAMPLE: [2.6] Set up  uri (http as http_stream, aac as aac decoder, and default output is i2s)[0m
21:07:23.878 -> [0;32mI (8961) HTTP_LIVINGSTREAM_EXAMPLE: [ 4 ] Set up  event listener[0m
21:07:23.878 -> [0;32mI (8971) HTTP_LIVINGSTREAM_EXAMPLE: [4.1] Listening event from all elements of pipeline[0m
21:07:23.878 -> [0;32mI (8981) HTTP_LIVINGSTREAM_EXAMPLE: [4.2] Listening event from peripherals[0m
21:07:23.878 -> [0;32mI (8981) HTTP_LIVINGSTREAM_EXAMPLE: [ 5 ] Start audio_pipeline[0m
21:07:23.924 -> [0;32mI (8991) AUDIO_ELEMENT: [http-0x3ffbe328] Element task created[0m
21:07:23.924 -> [0;32mI (9001) AUDIO_ELEMENT: [aac-0x3ffc0a60] Element task created[0m
21:07:23.924 -> [0;32mI (9001) AUDIO_ELEMENT: [i2s-0x3ffc06ac] Element task created[0m
21:07:23.924 -> [0;32mI (9011) AUDIO_PIPELINE: Func:audio_pipeline_run, Line:359, MEM Total:135464 Bytes

21:07:23.924 -> [0m
21:07:23.924 -> [0;32mI (9021) AUDIO_ELEMENT: [http] AEL_MSG_CMD_RESUME,state:1[0m
21:07:23.924 -> [0;32mI (9031) AUDIO_ELEMENT: [aac] AEL_MSG_CMD_RESUME,state:1[0m
21:07:23.924 -> [0;32mI (9031) AUDIO_ELEMENT: [i2s] AEL_MSG_CMD_RESUME,state:1[0m
21:07:23.971 -> [0;32mI (9041) I2S_STREAM: AUDIO_STREAM_WRITER[0m
21:07:23.971 -> [0;32mI (9041) AUDIO_PIPELINE: Pipeline started[0m
21:07:23.971 -> [0;32mI (9071) HTTP_STREAM: total_bytes=4787[0m
21:07:23.971 -> [0;32mI (9091) HTTP_STREAM: Live stream URI. Need to be fetched again![0m
21:07:24.019 -> [0;32mI (9111) HTTP_STREAM: total_bytes=30043[0m
21:07:24.019 -> [0;32mI (9111) CODEC_ELEMENT_HELPER: The element is 0x3ffc0a60. The reserve data 2 is 0x0.[0m
21:07:24.019 -> [0;32mI (9111) AAC_DECODER: a new song playing[0m
21:07:24.019 -> [0;32mI (9121) AAC_DECODER: this audio is RAW AAC[0m
21:07:24.019 -> [0;32mI (9131) HTTP_LIVINGSTREAM_EXAMPLE: [ * ] Receive music info from aac decoder, sample_rates=24000, bits=16, ch=2[0m
21:07:24.064 -> [0;32mI (9161) AUDIO_ELEMENT: [i2s] AEL_MSG_CMD_PAUSE[0m
21:07:24.064 -> [0;32mI (9181) I2S: APLL: Req RATE: 24000, real rate: 23999.980, BITS: 16, CLKM: 1, BCK_M: 8, MCLK: 6143995.000, SCLK: 767999.375000, diva: 1, divb: 0[0m
21:07:24.111 -> [0;32mI (9181) AUDIO_ELEMENT: [i2s] AEL_MSG_CMD_RESUME,state:4[0m
21:07:24.111 -> [0;32mI (9191) I2S_STREAM: AUDIO_STREAM_WRITER[0m
21:07:25.282 -> [0;33mW (10381) HTTP_STREAM: No more data,errno:0, total_bytes:30043, rlen = 0[0m
21:07:25.282 -> [0;32mI (10391) AUDIO_ELEMENT: IN-[http] AEL_IO_DONE,0[0m
21:07:25.329 -> [0;32mI (10421) HTTP_STREAM: total_bytes=30021[0m
21:07:30.253 -> [0;33mW (15331) HTTP_STREAM: No more data,errno:0, total_bytes:30021, rlen = 0[0m
21:07:30.253 -> [0;32mI (15341) AUDIO_ELEMENT: IN-[http] AEL_IO_DONE,0[0m
21:07:30.253 -> [0;32mI (15361) HTTP_STREAM: total_bytes=29977[0m
21:07:35.224 -> [0;33mW (20321) HTTP_STREAM: No more data,errno:0, total_bytes:29977, rlen = 0[0m
21:07:35.224 -> [0;32mI (20321) AUDIO_ELEMENT: IN-[http] AEL_IO_DONE,0[0m
21:07:35.271 -> [0;32mI (20351) HTTP_STREAM: total_bytes=30022[0m
21:07:40.232 -> [0;33mW (25321) HTTP_STREAM: No more data,errno:0, total_bytes:30022, rlen = 0[0m
21:07:40.232 -> [0;32mI (25321) AUDIO_ELEMENT: IN-[http] AEL_IO_DONE,0[0m
21:07:40.232 -> [0;32mI (25341) HTTP_STREAM: total_bytes=30029[0m
```
