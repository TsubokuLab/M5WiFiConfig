/*
*******************************************************************************
* config.h - 設定ファイル
* 
* WiFi設定システムのカスタマイズ可能な項目
* このファイルを編集することで、アプリケーションをカスタマイズできます
*******************************************************************************
*/

#ifndef CONFIG_H
#define CONFIG_H

// ===== 基本設定 =====
#define APP_TITLE "WiFi設定サンプル"                    // アプリケーションタイトル
#define APP_VERSION "v0.0.1"                           // アプリケーションバージョン
#define AP_SSID "Device-Setup"                         // アクセスポイント名
#define AP_PASS "12345678"                             // アクセスポイントパスワード
#define AUTHOR_NAME "YourName"                         // 作者名
#define AUTHOR_URL "https://x.com/kohack_v"            // 作者URL

// ===== シリアル通信設定 =====
#define ENABLE_SERIAL_DEBUG true                     // シリアルデバッグ有効/無効
#define SERIAL_BAUD_RATE 115200                      // シリアル通信速度

// ===== ネットワーク設定 =====
#define AP_IP_ADDR IPAddress(192, 168, 4, 1)           // アクセスポイントIP
#define WEB_SERVER_PORT 80                             // Webサーバーポート
#define DNS_SERVER_PORT 53                             // DNSサーバーポート
#define DNS_DOMAIN "device"                            // ローカルドメイン(http://device.local等でアクセスできる)
#define WIFI_CONNECTION_TIMEOUT 20                     // WiFi接続タイムアウト回数（500ms/回）
#define WIFI_CHECK_INTERVAL 5000                       // WiFi接続確認間隔（ミリ秒）
#define WIFI_RECONNECT_ATTEMPTS 10                     // 再接続試行回数

// ===== UI設定 =====
#define CONTAINER_MAX_WIDTH "480px"                     // 設定画面最大幅
#define BORDER_RADIUS "20px"                            // 角の丸み
#define BUTTON_PADDING "16px"                           // ボタンパディング
#define INPUT_PADDING "16px"                            // 入力フィールドパディング

// ===== M5Stack LCD設定 =====
#define SCREEN_BRIGHTNESS 255
#define FONT_SCALE 1.0                                        // フォントサイズ調整用
#define LCD_TEXT_COLOR M5.Display.color565(0, 0, 0)           // LCD文字色（黒）
#define LCD_BG_COLOR M5.Display.color565(255, 255, 255)       // LCD背景色（白）
#define BTN_BG_COLOR M5.Display.color565(70, 93, 106)         // ボタン背景色（ネイビー）
#define BTN_TEXT_COLOR M5.Display.color565(255, 255, 255)     // ボタンテキスト色（白）
#define SETUP_COLOR M5.Display.color565(195, 87, 87)  	      // セットアップ画面ヘッダー色（赤）
#define APP_COLOR M5.Display.color565(52, 200, 190)  	        // アプリ画面ヘッダー色（緑）
#define SETTING_COLOR M5.Display.color565(116, 86, 177)  	    // 設定画面ヘッダー色（紫）
#define CONNECT_COLOR M5.Display.color565(68, 220, 140)  	    // 接続時（緑）
#define DISCONNECT_COLOR M5.Display.color565(195, 87, 87)  	  // 切断時（赤）

// ===== カラーテーマ設定 =====
// テーマ1: ブルー・パープル（デフォルト）
#define THEME_PRIMARY_START "#667eea"
#define THEME_PRIMARY_END "#764ba2"
#define THEME_SECONDARY_START "#667eea"
#define THEME_SECONDARY_END "#764ba2"
#define THEME_DANGER_START "#ef4444"
#define THEME_DANGER_END "#dc2626"

// テーマ2: グリーン・ティール（下記をアンコメントして使用）
/*
#define THEME_PRIMARY_START "#10b981"
#define THEME_PRIMARY_END "#059669"
#define THEME_SECONDARY_START "#14b8a6"
#define THEME_SECONDARY_END "#0d9488"
#define THEME_DANGER_START "#ef4444"
#define THEME_DANGER_END "#dc2626"
*/

// テーマ3: オレンジ・レッド（下記をアンコメントして使用）
/*
#define THEME_PRIMARY_START "#f97316"
#define THEME_PRIMARY_END "#ea580c"
#define THEME_SECONDARY_START "#f59e0b"
#define THEME_SECONDARY_END "#d97706"
#define THEME_DANGER_START "#ef4444"
#define THEME_DANGER_END "#dc2626"
*/

// テーマ4: ピンク・パープル（下記をアンコメントして使用）
/*
#define THEME_PRIMARY_START "#ec4899"
#define THEME_PRIMARY_END "#be185d"
#define THEME_SECONDARY_START "#a855f7"
#define THEME_SECONDARY_END "#7c3aed"
#define THEME_DANGER_START "#ef4444"
#define THEME_DANGER_END "#dc2626"
*/

#endif // CONFIG_H