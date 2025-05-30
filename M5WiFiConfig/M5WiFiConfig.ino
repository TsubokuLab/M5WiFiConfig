/*
*******************************************************************************
* M5WiFiConfig.ino - Arduino IDE用 WiFi設定システム（M5Unified対応）
* 
* M5Stack用高機能WiFi設定システム - タッチUI・QRコード・キャプティブポータル対応
* 
* 主な機能:
* - タッチスクリーン対応の直感的UI
* - QRコード表示によるワンタップ接続
* - キャプティブポータル（自動リダイレクト）
* - 日本語フォント対応の見やすい表示
* - LEDフィードバック付きタッチ操作
* - MDNS対応（settings.localでアクセス可能）
* - セキュリティ強化（APパスワード設定）
* 
* 使用方法:
* 1. config.h でアプリ情報・カラーテーマをカスタマイズ
* 2. Arduino IDEでM5Stack-Core2ボードを選択
* 3. コンパイル・アップロード
* 4. 初回起動時は設定モード、以降は自動WiFi接続
* 
* 必要ライブラリ:
* - M5Unified (最新版推奨)
* - WiFi (ESP32標準)
* - WebServer (ESP32標準) 
* - DNSServer (ESP32標準)
* - Preferences (ESP32標準)
* - ESPmDNS (ESP32標準)
* 
* 操作方法:
* - タッチスクリーン: 画面下部の仮想ボタンをタッチ
* - 物理ボタン: BtnA=情報表示、BtnC=再起動
* - QRコード: WiFi接続とWeb設定画面への直接アクセス
* 
* カスタマイズ:
* - config.h: アプリ名・作者情報・カラーテーマ・各種設定
* - styles.h: WebUIのCSS・フッター・ページ生成ロジック
*******************************************************************************
*/

#include <M5Unified.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiClient.h>
#include <DNSServer.h>
#include <WebServer.h>
#include <Preferences.h>

// 設定ファイルの読み込み
#include "config.h"
#include "styles.h"

// グローバル変数
String ssidList;
String wifi_ssid;
String wifi_password;
int networkCount = 0;                     // スキャンしたネットワーク数

int width = 320;
int height = 240;
M5Canvas canvas(&M5.Display);            // 画面全体描画用キャンバス

// WiFi監視用変数
unsigned long lastWiFiCheck = 0;         // 最後のWiFiチェック時刻
boolean isConnect = false;               // 接続状態フラグ
String connectionStatus = "初期化中";     // 接続状態メッセージ
int reconnectCount = 0;

DNSServer dnsServer;
WebServer webServer(WEB_SERVER_PORT);
Preferences preferences;

// モード
enum DeviceMode {
    SETUP_MODE,
    APP_MODE,
    SETTING_MODE
};
DeviceMode deviceMode = SETUP_MODE;

// URLデコード関数
String urlDecode(String input) {
    String s = input;
    s.replace("%20", " ");
    s.replace("+", " ");
    s.replace("%21", "!");
    s.replace("%22", "\"");
    s.replace("%23", "#");
    s.replace("%24", "$");
    s.replace("%25", "%");
    s.replace("%26", "&");
    s.replace("%27", "\'");
    s.replace("%28", "(");
    s.replace("%29", ")");
    s.replace("%2C", ",");
    s.replace("%2E", ".");
    s.replace("%2F", "/");
    s.replace("%3A", ":");
    s.replace("%3B", ";");
    s.replace("%3C", "<");
    s.replace("%3D", "=");
    s.replace("%3E", ">");
    s.replace("%3F", "?");
    s.replace("%40", "@");
    return s;
}

// Webサーバー開始関数
void startWebServer() {
    if (deviceMode == SETUP_MODE) {
        // 設定モード: アクセスポイントモード
        if (ENABLE_SERIAL_DEBUG) {
            Serial.print("Webサーバー開始: ");
            Serial.println(WiFi.softAPIP());
        }

        dnsServer.start(DNS_SERVER_PORT, "*", WiFi.softAPIP());
        
        // WiFi設定ページ
        webServer.on("/settings", []() {
            String s = "<h1>📶 WiFi設定</h1>";
            s += "<div class='info'>利用可能なネットワークを選択してパスワードを入力してください</div>";
            s += "<form method='get' action='setap'>";
            s += "<div style='display:flex;align-items:center;gap:10px;margin-bottom:8px;'>";
            s += "<label style='font-weight:bold;color:#374151;flex:1;'>ネットワークを選択:</label>";
            s += "<button type='button' onclick='refreshNetworks()' class='btn' style='width:auto;padding:8px 16px;margin:0;font-size:14px;'>🔄 更新</button>";
            s += "</div>";
            s += "<select id='networkSelect' name='ssid' style='margin-bottom:20px;'>" + ssidList + "</select>";
            s += "<label style='display:block;margin-bottom:8px;font-weight:bold;color:#374151;'>パスワード:</label>";
            s += "<input name='pass' type='password' placeholder='ネットワークパスワードを入力' maxlength='64'>";
            s += "<button type='submit' class='btn'>🔗 接続設定を保存</button></form>";
            
            // JavaScriptでAjax更新機能を追加
            s += "<script>";
            s += "function refreshNetworks() {";
            s += "  var btn = event.target;";
            s += "  btn.innerHTML = '更新中...';";
            s += "  btn.disabled = true;";
            s += "  fetch('/refresh-networks')";
            s += "    .then(response => response.json())";
            s += "    .then(data => {";
            s += "      var select = document.getElementById('networkSelect');";
            s += "      select.innerHTML = data.networks;";
            s += "      btn.innerHTML = '🔄 更新';";
            s += "      btn.disabled = false;";
            s += "      console.log('ネットワークリストを更新しました');";
            s += "    })";
            s += "    .catch(error => {";
            s += "      console.error('エラー:', error);";
            s += "      btn.innerHTML = '🔄 更新';";
            s += "      btn.disabled = false;";
            s += "      alert('ネットワークリストの更新に失敗しました');";
            s += "    });";
            s += "}";
            s += "</script>";
            
            webServer.send(200, "text/html", makePage("WiFi設定", s));
        });
        
        // ネットワークリスト更新処理
        webServer.on("/refresh-networks", []() {
            if (ENABLE_SERIAL_DEBUG) {
                Serial.println("🔄 ネットワークリストの更新リクエストを受信");
            }
            
            // ネットワークリストを更新
            updateNetworkList();
            
            // HTMLタグをJSON用にエスケープ
            String escapedSSIDList = ssidList;
            escapedSSIDList.replace("\\", "\\\\");  // バックスラッシュをエスケープ
            escapedSSIDList.replace("\"", "\\\"");    // ダブルクォートをエスケープ
            
            // JSONレスポンスを返す
            String jsonResponse = "{\"networks\":\"" + escapedSSIDList + "\",\"count\":" + String(networkCount) + "}";
            webServer.send(200, "application/json", jsonResponse);
            
            if (ENABLE_SERIAL_DEBUG) {
                Serial.println("✅ ネットワークリスト更新完了");
            }
        });
        
        // 設定保存処理
        webServer.on("/setap", []() {
            String ssid = urlDecode(webServer.arg("ssid"));
            String pass = urlDecode(webServer.arg("pass"));
            
            if (ENABLE_SERIAL_DEBUG) {
                Serial.printf("SSID: %s\n", ssid.c_str());
                Serial.printf("パスワード設定完了\n");
            }
            
            preferences.putString("WIFI_SSID", ssid);
            preferences.putString("WIFI_PASSWD", pass);
            
            String s = "<h1>✅ 設定完了</h1>";
            s += "<div class='success'>WiFi設定が保存されました。<br>デバイスが自動的に再起動され接続を開始します。</div>";
            s += "<script>setTimeout(function(){document.body.innerHTML='<div style=\"text-align:center;padding:50px;background:white;border-radius:20px;\"><h2>🔄 再起動中...</h2><p>しばらくお待ちください</p></div><button type='button' id='close' class='btn'>❌ 閉じる</button>';}, 2000);</script>";
            s += "<script>const closeBtn = document.getElementById('close');closeBtn.addEventListener('click', function () {  window.close(); });</script>";
            webServer.send(200, "text/html", makePage("設定完了", s));
            delay(2000);
            ESP.restart();
        });
        
        // メインページ（設定モード）
        webServer.onNotFound([]() {
            String s = "<h1>📱 " + String(APP_TITLE) + "</h1>";
            s += "<div class='info'>";
            s += "WiFi接続設定を開始します。<br>";
            s += "アクセスポイント名: <strong>" + String(AP_SSID) + "</strong><br>";
            s += "設定用IP: <strong>" + WiFi.softAPIP().toString() + "</strong>";
            s += "</div>";
            s += "<a href='/settings' class='btn'>⚙️ WiFi設定を開始</a>";
            webServer.send(200, "text/html", makePage("セットアップ", s));
        });
    } else {
        // 通常モード: WiFi接続済み
        if (ENABLE_SERIAL_DEBUG) {
            Serial.print("Webサーバー開始: ");
            Serial.println(WiFi.localIP());
        }
        
        // ステータスページ
        webServer.on("/", []() {
            String s = "<h1>✅ 接続中</h1>";
            s += "<div class='info'>";
            s += "WiFiネットワーク: <strong>" + WiFi.SSID() + "</strong><br>";
            s += "IPアドレス: <strong>" + WiFi.localIP().toString() + "</strong><br>";
            s += "信号強度: <strong>" + String(WiFi.RSSI()) + " dBm</strong><br>";
            s += "稼働時間: <strong>" + String(millis() / 1000) + " 秒</strong><br>";
            s += "</div>";
            s += "<a href='/reset' class='btn btn-danger'>🔄 設定をリセット</a>";
            webServer.send(200, "text/html", makePage("稼働中", s));
        });
        
        // 設定リセット処理
        webServer.on("/reset", []() {
            String s = "<h1>🔄 リセット完了</h1>";
            s += "<div class='success'>WiFi設定をリセットしました。<br>デバイス再起動後、本体の指示に従って接続設定を行って下さい。</div>";
            webServer.send(200, "text/html", makePage("リセット", s));
            resetSettings();
        });
    }
    webServer.begin();
}

// 保存された設定の復元
boolean restoreConfig() {
    wifi_ssid = preferences.getString("WIFI_SSID", "");
    wifi_password = preferences.getString("WIFI_PASSWD", "");    
    if (wifi_ssid.length() > 0) {
        WiFi.begin(wifi_ssid.c_str(), wifi_password.c_str());
        return true;
    }
    return false;
}

// 再起動処理
void rebootDevice(){
    if(ENABLE_SERIAL_DEBUG) Serial.println("再起動します");
    M5.Display.clear(LCD_BG_COLOR);
    M5.Display.setTextColor(LCD_TEXT_COLOR);
    M5.Display.setTextDatum(MC_DATUM);
    M5.Display.drawString("再起動中...", width/2, height/2);
    M5.Speaker.tone(3000, 500);
    delay(1000);
    ESP.restart();
}

// 設定をクリア
void resetSettings(){
    if(ENABLE_SERIAL_DEBUG) Serial.println("設定をクリア");
    preferences.remove("WIFI_SSID");
    preferences.remove("WIFI_PASSWD");
    rebootDevice();
}

// WiFi接続確認
boolean checkConnection() {
    int count = 0;
    if (ENABLE_SERIAL_DEBUG) Serial.print("WiFi接続確認中");

    // WiFi接続確認時間を記録
    lastWiFiCheck = millis();

    while (count < WIFI_CONNECTION_TIMEOUT) {
        if (WiFi.status() == WL_CONNECTED) {
            if (ENABLE_SERIAL_DEBUG) Serial.println("\n接続成功!");
            return true;
        }
        delay(500);
        if (ENABLE_SERIAL_DEBUG) Serial.print(".");
        count++;
    }
    
    if (ENABLE_SERIAL_DEBUG) Serial.println("接続タイムアウト");
    return false;
}

void updateNetworkList() {
    // WiFiネットワークスキャン
    networkCount = WiFi.scanNetworks();
    delay(100);
    
    if (ENABLE_SERIAL_DEBUG) {
        Serial.printf("ネットワークリスト更新: %d個のネットワークを検出\n", networkCount);
    }
    
    // ネットワークリスト作成
    ssidList = "";
    for (int i = 0; i < networkCount; ++i) {
        ssidList += "<option value='" + WiFi.SSID(i) + "'>";
        ssidList += WiFi.SSID(i);
        ssidList += " (" + String(WiFi.RSSI(i)) + "dBm)";
        ssidList += (WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " 🔓" : " 🔒";
        ssidList += "</option>";
    }
}

// セットアップモード（アクセスポイントモード）
void setupMode() {
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);
    
    if (ENABLE_SERIAL_DEBUG) Serial.println("ネットワークスキャンを開始");
    
    // ネットワークリスト更新（スキャンも含む）
    updateNetworkList();
    
    // アクセスポイント開始
    delay(100);
    WiFi.softAPConfig(AP_IP_ADDR, AP_IP_ADDR, IPAddress(255, 255, 255, 0));
    WiFi.softAP(AP_SSID, AP_PASS);
    WiFi.mode(WIFI_AP);
    
    // Webサーバー開始
    startWebServer();
    // DNSサーバー開始
    if (MDNS.begin(DNS_DOMAIN)) {
        if (ENABLE_SERIAL_DEBUG) Serial.println("MDNS responder started : http://" + String(DNS_DOMAIN) + ".local");
    }

    if (ENABLE_SERIAL_DEBUG) {
        Serial.printf("\nアクセスポイント開始: \"%s\"\n", AP_SSID);
        Serial.printf("設定URL: http://%s\n", AP_IP_ADDR.toString().c_str());
    }
}

// 仮想ボタンの描画
void drawVirtualButtons(String btnA, String btnB, String btnC) {
    int btm_width = (width + 1) / 3;

    // ボタン背景
    canvas.fillRoundRect(btm_width * 0, height - 40, btm_width - 1, 40, 10, BTN_BG_COLOR);
    canvas.fillRoundRect(btm_width * 1, height - 40, btm_width - 1, 40, 10, BTN_BG_COLOR);
    canvas.fillRoundRect(btm_width * 2, height - 40, btm_width - 1, 40, 10, BTN_BG_COLOR);

    // ボタンラベル
    canvas.setTextColor(BTN_TEXT_COLOR);
    canvas.setFont(&fonts::lgfxJapanGothic_20);
    canvas.setTextSize(1 * FONT_SCALE);
    
    // テキストを中央寄せ
    canvas.setTextDatum(MC_DATUM);
    
    // ボタンAラベル
    canvas.drawString(btnA, width / 6 * 1, height - 20);
    
    // ボタンBラベル
    canvas.drawString(btnB, width / 6 * 3, height - 20);
    
    // ボタンCラベル
    canvas.drawString(btnC, width / 6 * 5, height - 20);
    canvas.setFont(&fonts::lgfxJapanGothic_16);
}

void setup() {
    // M5Unified初期化
    auto cfg = M5.config();
    M5.begin(cfg);

    width = M5.Display.width();
    height = M5.Display.height();

    // ディスプレイ初期設定
    M5.Display.clear();
    M5.Display.setCursor(0, 0);
    M5.Display.setBrightness(SCREEN_BRIGHTNESS);  // 画面の明るさを設定
    M5.Display.setFont(&fonts::lgfxJapanGothic_16);
    M5.Display.setTextColor(TFT_YELLOW);
    M5.Display.setTextSize(1 * FONT_SCALE);

    // キャンバス初期設定
    canvas.setColorDepth(16);
    canvas.createSprite(width, height);
    canvas.clear(LCD_BG_COLOR);
    canvas.setFont(&fonts::lgfxJapanGothic_16);

    // シリアル通信初期化
    if (ENABLE_SERIAL_DEBUG) {
        Serial.begin(SERIAL_BAUD_RATE);
        Serial.println("=== " + String(APP_TITLE) + " 起動 ===");
    }

    // 設定保存領域初期化
    preferences.begin("wifi-config");
    delay(10);

    // LCD表示
    M5.Display.println(String(APP_TITLE) + " - " + String(APP_VERSION));
    M5.Display.println("起動中...");

    // 保存された設定で接続試行
    if (restoreConfig()) {
        if (checkConnection()) {
            // 接続成功: 通常モード
            deviceMode = APP_MODE;
            isConnect = true;
            connectionStatus = "WiFi接続成功";
            M5.Display.println("接続成功 - 稼働モード");
            startWebServer();
            drawDisplay();
            return;
        }
        M5.Display.println("接続失敗 - 設定モード");
    }

    // 設定モードで開始
    deviceMode = SETUP_MODE;
    M5.Display.println("設定モードで開始");
    setupMode();

    drawDisplay();
}

// メインのディスプレイ描画処理
void drawDisplay(){
    switch(deviceMode){
        case SETUP_MODE:
            drawSetupPage();
            break;
        case APP_MODE:
            drawMainPage();
            break;
        case SETTING_MODE:
            drawSettingsPage();
            break;
        default:
            break;
    }
}

// WiFi未接続時の初期設定画面
void drawSetupPage(){
    if (ENABLE_SERIAL_DEBUG) Serial.println("セットアップ画面を表示");
    int itemX = 10;
    int itemY = 10;
    int itemHeight = 16 * FONT_SCALE;
    int spacing = 10;
    int qr_size = 80;
    canvas.clear(LCD_BG_COLOR);
    canvas.setTextColor(LCD_TEXT_COLOR);
    canvas.setTextSize(1 * FONT_SCALE);
    canvas.setFont(&fonts::lgfxJapanGothic_16);
    itemY = 125;
    // QRコード描画
    String _setting_url = "http://" + WiFi.softAPIP().toString();
    canvas.qrcode("WIFI:T:WPA;S:" + String(AP_SSID) + ";P:" + AP_PASS + ";H:false;;", 0, height - qr_size, qr_size, 5);
    canvas.setTextDatum(TL_DATUM);
    canvas.drawString("1. WiFiに接続", itemX, itemY);

    _setting_url = "http://" + AP_IP_ADDR.toString();
    canvas.qrcode(_setting_url, width / 2, height - qr_size, qr_size, 5);
    canvas.setTextDatum(TL_DATUM);
    canvas.drawString("2. 設定画面を開く", width / 2, itemY);
    itemY += itemHeight;
    canvas.drawString(_setting_url, width / 2, itemY);

    // ヘッダー
    itemY = 10;
    canvas.setTextDatum(MC_DATUM);
    canvas.fillRoundRect(10, 10, width/2 - 10, 40, 10, SETUP_COLOR);
    itemY += 20;
    canvas.setTextColor(WHITE);
    canvas.setFont(&fonts::lgfxJapanGothic_20);
    canvas.drawString("設定モード", width/4, itemY);
    canvas.setFont(&fonts::lgfxJapanGothic_16);
    canvas.setTextColor(LCD_TEXT_COLOR);
    itemY += 20;
    itemY += spacing;

    canvas.setTextDatum(TL_DATUM);
    canvas.drawString("SSID: " + String(AP_SSID), itemX, itemY);
    itemY += itemHeight;
    canvas.drawString("PASS: " + String(AP_PASS), itemX, itemY);
    //itemY += itemHeight;
    //canvas.drawString("IP: " + WiFi.softAPIP().toString(), itemX, itemY);
    // itemY += itemHeight;
    // canvas.drawString("接続デバイス数: " + String(WiFi.softAPgetStationNum()), itemX, itemY);
    // itemY += itemHeight;
    itemY = 100;
    canvas.setTextDatum(TL_DATUM);
    canvas.setFont(&fonts::lgfxJapanGothic_20);
    canvas.drawString("■WiFi初期設定方法", itemX, itemY);
    canvas.setFont(&fonts::lgfxJapanGothic_16);
    canvas.pushSprite(&M5.Display, 0, 0);
}

// アプリのメイン画面描画
void drawMainPage(){
    // ここにアプリのメイン画面描画処理などを書く ////////////////////
    // サンプルアプリ画面
    int itemX = 10;
    int itemY = 10;
    int itemHeight = 16 * FONT_SCALE;
    canvas.clear(LCD_BG_COLOR);
    canvas.setTextSize(1 * FONT_SCALE);
    itemY = 10;
    canvas.setTextDatum(MC_DATUM);
    canvas.fillRoundRect(10, 10, width/2 - 10, 40, 10, APP_COLOR);
    itemY += 20;
    canvas.setTextColor(WHITE);
    canvas.setFont(&fonts::lgfxJapanGothic_20);
    canvas.drawString("アプリ画面", width/4, itemY);
    canvas.setFont(&fonts::lgfxJapanGothic_16);
    itemY = 60;
    canvas.setTextDatum(TL_DATUM);
    canvas.setTextColor(LCD_TEXT_COLOR);
    canvas.drawString("<アプリメイン画面サンプル>", itemX, itemY);
    itemY += itemHeight;
    itemY += itemHeight;
    canvas.drawString("サンプルテキスト", itemX, itemY);
    itemY += itemHeight;
    canvas.drawString("サンプルテキスト", itemX, itemY);
    itemY += itemHeight;
    canvas.drawString("サンプルテキスト", itemX, itemY);

    drawConnectionStatus();
    drawVirtualButtons("設定", "---", "---");
    canvas.pushSprite(&M5.Display, 0, 0);
    ///////////////////////////////////////////////////////////////
}

// WiFi未接続時の初期設定画面
void drawSettingsPage(){
    int itemX = 10;
    int itemY = 10;
    int itemHeight = 16 * FONT_SCALE;
    int spacing = 10;
    int qr_size = 80;

    canvas.clear(LCD_BG_COLOR);
    canvas.setTextColor(LCD_TEXT_COLOR);
    canvas.setTextSize(1 * FONT_SCALE);
    canvas.setFont(&fonts::lgfxJapanGothic_16);
    // ヘッダー
    itemY = 10;
    canvas.setTextDatum(MC_DATUM);
    canvas.fillRoundRect(10, 10, width/2 - 10, 40, 10, SETTING_COLOR);
    itemY += 20;   
    canvas.setTextColor(WHITE);
    canvas.setFont(&fonts::lgfxJapanGothic_20);
    canvas.drawString("設定画面", width/4, itemY);
    canvas.setFont(&fonts::lgfxJapanGothic_16);
    canvas.setTextColor(LCD_TEXT_COLOR);
    itemY += 20;
    itemY += spacing;

    canvas.setTextDatum(TL_DATUM);
    canvas.drawString("ネットワーク: " + String(WiFi.SSID()), itemX, itemY);
    itemY += itemHeight;
    canvas.drawString("信号強度: " + String(WiFi.RSSI()) + "dBm", itemX, itemY);
    itemY += itemHeight;
    canvas.drawString("稼働: " + String(millis() / 1000) + " 秒", itemX, itemY);
    itemY += itemHeight;
    canvas.drawString("IPアドレス: " + WiFi.localIP().toString(), itemX, itemY);
    itemY += itemHeight * 2;
    
    String _setting_url = "http://" + WiFi.localIP().toString();
    canvas.qrcode(_setting_url, width - qr_size - spacing, height - 40 - qr_size - spacing, qr_size, 5);
    canvas.setTextDatum(TL_DATUM);
    canvas.drawString("■設定ページ", itemX, itemY);
    itemY += itemHeight;
    canvas.drawString(_setting_url, itemX, itemY);

    drawConnectionStatus();
    drawVirtualButtons("戻る", "再起動", "初期化");
    canvas.pushSprite(&M5.Display, 0, 0);
}

// 接続状態表示の更新
void drawConnectionStatus() {
    uint16_t _c = isConnect ? CONNECT_COLOR : DISCONNECT_COLOR;
    canvas.fillCircle(width / 2 + 16, 30, 6, (millis() % 1000 < 500) ? _c : LCD_BG_COLOR);
    canvas.setTextDatum(ML_DATUM);
    canvas.setTextColor(_c);
    canvas.drawString(connectionStatus, width / 2 + 26, 30);
    canvas.setTextColor(LCD_TEXT_COLOR);
}

void changeMode(DeviceMode _mode){
    if(_mode == deviceMode) return;
    switch(_mode){
        case APP_MODE:
            if (ENABLE_SERIAL_DEBUG) Serial.println("設定画面を表示");
            deviceMode = _mode;
            break;
        case SETTING_MODE:
            if (ENABLE_SERIAL_DEBUG) Serial.println("メイン画面を表示");
            deviceMode = _mode;
            break;
        default:
            break;
    }
    drawDisplay();
    M5.Speaker.tone(3000, 100);
    delay(500);
}

void A_Pressed(){
    // ボタンA
    if (ENABLE_SERIAL_DEBUG) Serial.println("A Button Pressed.");
    switch(deviceMode){
        case APP_MODE:
            // 設定画面へ
            changeMode(SETTING_MODE);
            break;
        case SETTING_MODE:
            // アプリメイン画面へ
            changeMode(APP_MODE);
            break;
        default:
            break;
    }
}
void B_Pressed(){
    // ボタンB
    if (ENABLE_SERIAL_DEBUG) Serial.println("B Button Pressed.");
    switch(deviceMode){
        case APP_MODE:
            break;
        case SETTING_MODE:
            // 再起動
            rebootDevice();
            break;
        default:
            break;
    }
}
void C_Pressed(){
    // ボタンC
    if (ENABLE_SERIAL_DEBUG) Serial.println("C Button Pressed.");
    switch(deviceMode){
        case APP_MODE:
            break;
        case SETTING_MODE:
            // 初期化
            M5.Display.clear(LCD_BG_COLOR);
            M5.Display.setTextColor(LCD_TEXT_COLOR);
            M5.Display.setTextDatum(MC_DATUM);
            M5.Display.drawString("設定を初期化します...", width/2, height/2);
            M5.Speaker.tone(3000, 100);
            delay(1000);
            resetSettings();
            break;
        default:
            break;
    }
}

void loop() {
    dnsServer.processNextRequest();
    webServer.handleClient();
    M5.update();

    // タッチスクリーン処理
    auto touch = M5.Touch.getDetail();
    if (touch.isPressed()) {
        int touchX = touch.x;
        int touchY = touch.y;
        if (touchX > 0 && touchX < width / 3 && touchY > height - 40 && touchY < height) {
            A_Pressed();
        }
        if (touchX > width / 3 * 1 && touchX < width / 3 * 2 && touchY > height - 40 && touchY < height) {
            B_Pressed();
        }
        if (touchX > width / 3 * 2 && touchX < width && touchY > height - 40 && touchY < height) {
            C_Pressed();
        }
    }
    if (M5.BtnA.wasPressed()) A_Pressed();
    if (M5.BtnB.wasPressed()) B_Pressed();
    if (M5.BtnC.wasPressed()) C_Pressed();

    // WiFi接続状態の定期監視
    if (deviceMode == APP_MODE || deviceMode == SETTING_MODE){
        unsigned long currentTime = millis();
        if (currentTime - lastWiFiCheck >= WIFI_CHECK_INTERVAL) {
            lastWiFiCheck = currentTime;
            if (WiFi.status() == WL_CONNECTED){
                if(!isConnect && ENABLE_SERIAL_DEBUG) Serial.println("✅ 接続されました");
                isConnect = true;
                connectionStatus = "接続済み";
                reconnectCount = 0;
            }else{
                if (isConnect && ENABLE_SERIAL_DEBUG) Serial.println("❌️ 切断されました");
                isConnect = false;
                connectionStatus = "切断";
                reconnectCount++;

                connectionStatus += ":再接続" + String(reconnectCount) + "回目";
                // 再接続処理
                if (ENABLE_SERIAL_DEBUG) Serial.println("🔄 再接続 " + String(reconnectCount) + "回目");
                WiFi.disconnect();
                WiFi.reconnect();
                // 試行回数を超えたら再起動
                if (reconnectCount > WIFI_RECONNECT_ATTEMPTS) rebootDevice();
            }
            // WiFi接続中ならLEDを点灯
            M5.Power.setLed(isConnect ? 255 : 0);
        }

        //delay(10);
        drawDisplay();
    }
}