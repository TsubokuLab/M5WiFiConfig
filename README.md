# M5Stack WiFi設定画面テンプレート

M5Stack Core系デバイス用のWiFi設定システムテンプレートです。  
タッチスクリーンUI、QRコード、Webインターフェースを組み合わせた使いやすいWiFi設定機能画面を実装できます。

## 主要機能

**3つの動作モード**

- **セットアップモード**: 初期WiFi設定用のアクセスポイント機能
- **アプリモード**: メインアプリケーション画面
- **設定モード**: WiFi状態確認と設定管理

**操作方法**
- タッチスクリーン対応の直感的なUI
- 物理ボタン操作にも対応
- QRコードによる設定画面への簡単アクセス

**WiFi管理**
- 自動接続状態監視（5秒間隔）
- 切断時の自動再接続機能
- 接続状態のリアルタイム表示

**Webインターフェース**
- レスポンシブデザインの設定画面
- Ajax対応のネットワークスキャン更新
- 4種類のカラーテーマから選択可能

## 必要環境

**ハードウェア**
- M5Stack Core系(Basic, Core2, CoreS3等)

**開発環境**

- Arduino IDE 1.8.x以降 または Arduino IDE 2.x

**必要ライブラリ**
- M5Unified（最新版推奨）
- WiFi, WebServer, DNSServer, Preferences, ESPmDNS（ESP32標準）

## セットアップ手順

### 1. ライブラリのインストール
Arduino IDEのライブラリマネージャーから「M5Unified」をインストールしてください。

### 2. ボード設定
- ボード：「ESP32 Arduino」>「M5Stack-Core2」を選択
- 通信ポート：適切なCOMポートを選択

### 3. プロジェクトの準備
```
M5WiFiConfig/M5WiFiConfig.ino をArduino IDEで開く
```
※ config.h、styles.h は自動的に読み込まれます

### 4. カスタマイズ（config.h）
基本設定を変更できます：

```cpp
// アプリケーション設定
#define APP_TITLE "あなたのアプリ名"
#define AP_SSID "Device-Setup"
#define AP_PASS "12345678"

// WiFi監視設定
#define WIFI_CHECK_INTERVAL 5000        // 監視間隔（ms）
#define WIFI_RECONNECT_ATTEMPTS 10      // 再接続試行回数

// 色設定
#define APP_COLOR M5.Display.color565(52, 200, 190)
```

### 5. コンパイル・アップロード
Arduino IDEでコンパイル後、M5Stackにアップロードしてください。

## 使用方法

### 初回起動時
1. M5Stackが自動的にセットアップモードで起動
2. 画面に表示されるQRコードでWiFiに接続
3. 設定用QRコードまたは `192.168.4.1` にアクセス
4. WiFiネットワークを選択してパスワードを入力
5. 設定完了後、自動的に再起動

### 通常使用時
- **左タッチ/BtnA**: 設定画面への切り替え
- **中央タッチ/BtnB**: 再起動（設定画面時）
- **右タッチ/BtnC**: 設定初期化（設定画面時）

### 画面表示について
- **画面右上の点滅アイコン**: WiFi接続状態をリアルタイム表示（緑=接続中、赤=切断中）

## アプリ画面のカスタマイズ

`M5WiFiConfig.ino` の `drawMainPage()` 関数でメインアプリ画面をカスタマイズできます：

```cpp
void drawMainPage(){
    // ここにアプリ固有の画面描画処理を実装
    // 例：センサーデータ表示、グラフ描画など
    
    updateConnectionStatusDisplay();  // 接続状態表示
    drawVirtualButtons("設定", "---", "---");  // 仮想ボタン
    canvas.pushSprite(&M5.Display, 0, 0);  // 画面更新
}
```

## カラーテーマの変更

`config.h` で4種類のテーマから選択できます：

```cpp
// デフォルト：ブルー・パープル
#define THEME_PRIMARY_START "#667eea"

// その他のテーマ：対応するコメントアウトを解除
// グリーン・ティール
// オレンジ・レッド
// ピンク・パープル
```

## トラブルシューティング

**コンパイル時のエラー**
- M5Unifiedライブラリがインストールされているか確認
- ボード設定が「M5Stack-Core2」等、使用しているデバイスになっているか確認

**WiFi接続できない**
- パスワードが正しいか確認
- 2.4GHz WiFiネットワークを使用しているか確認
- ルーターの近くで再試行

**設定画面にアクセスできない**
- QRコードを使用するか、直接 `192.168.4.1` にアクセス
- キャプティブポータルが動作しない場合は手動でIPアクセス

## システム仕様

**動作環境**
- 対応機種：M5Stack Core系
- メモリ使用量：約200KB RAM、50KB Flash
- WiFi：2.4GHz 802.11 b/g/n

**ネットワーク**

- アクセスポイント：WPA2-PSK暗号化
- Webサーバー：HTTP/1.1、ポート80
- mDNS：device.local でのアクセス対応

## プロジェクト構成

```
M5WiFiConfig/
├── M5WiFiConfig.ino      # メインプログラム
├── config.h              # 設定定義ファイル
├── styles.h              # WebUI生成ファイル
└── README_Arduino.md     # 詳細マニュアル
```

## ライセンス

MIT License - 商用・個人利用自由

## 応用例

- IoTプロトタイプの基盤として
- センサーデータ収集システム
- リモート制御システム
- 環境モニタリングデバイス
- 学習用WiFi設定システム

---

このテンプレートを基に、あなた独自のM5Stackアプリケーションを開発してください。