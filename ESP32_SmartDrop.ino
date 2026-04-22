#define BLYNK_TEMPLATE_ID "TMPL6GbyVKj-w"
#define BLYNK_TEMPLATE_NAME "SMART LOCKER"
#define BLYNK_AUTH_TOKEN "_o6Ia7lY_DOrAuohyrUMSFR1lsIK7YTn"


#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <ESP_Mail_Client.h>
#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <time.h>
#include <esp_system.h>
// ===== WIFI + BLYNK =====
char auth[] = "_o6Ia7lY_DOrAuohyrUMSFR1lsIK7YTn";
char ssid[] = "Quandang";
char pass[] = "minhquan123";

// ===== EMAIL =====
#define SMTP_HOST "smtp.gmail.com"
#define SMTP_PORT 465
#define AUTHOR_EMAIL "quanminh.work2404@gmail.com"
#define AUTHOR_PASSWORD "efjokdyvlblyxwdh"

String recipientEmail = "";
String recipientEmail2 = "";

// ===== LCD =====
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ===== BUZZER =====
#define BUZZER 4

// ===== RELAY =====
#define RELAY1 16
#define RELAY2 17

// ===== IR SENSOR =====
#define IR_LOCKER1 18
#define IR_LOCKER2 19

// ===== KEYPAD =====
const byte ROWS = 4;
const byte COLS = 4;

char keys[ROWS][COLS] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};

byte rowPins[ROWS] = { 26, 25, 33, 32 };
byte colPins[COLS] = { 13, 12, 14, 27 };

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// ===== SMTP =====
SMTPSession smtp;

// ===== SYSTEM =====
String otpSys = "123456";
String otpSys2 = "123456";
String otpUserInput = "******";
int inputCounterPinOut = 0;

bool isFullLocker1 = 0;
bool isFullLocker2 = 0;
int SecondRemain = 0;
bool fkgKeyPress = 0;
bool flgSysWarning = 0;
int locker = 0;
unsigned long otpTime = 0;
#define OTP_TIMEOUT 60000

bool isValidGmail(String email) {
  email.trim();  // bỏ khoảng trắng

  int atIndex = email.indexOf('@');

  // phải có '@' và không nằm đầu/cuối
  if (atIndex <= 0 || atIndex >= email.length() - 1) {
    return false;
  }

  String localPart = email.substring(0, atIndex);
  String domainPart = email.substring(atIndex + 1);

  // check domain
  if ((domainPart != "gmail.com") && (domainPart != "hcmut.edu.vn")) {
    return false;
  }

  // local part không rỗng
  if (localPart.length() == 0) {
    return false;
  }

  return true;
}

// ===== BLYNK nhận EMAIL =====
BLYNK_WRITE(V0) {
  String inputEmail = param.asString();

  if (isValidGmail(inputEmail)) {
    recipientEmail = inputEmail;
    Serial.println("Email OK");
  } else {
    Serial.println("Email sai dinh dang");
  }
}
BLYNK_WRITE(V2) {
  int SendOTP = param.asInt();

  if (SendOTP) {
    if (isValidGmail(recipientEmail)) {
      otpSys = genOTP();

      lcd.clear();
      lcd.print("OTP Sending...");
      sendEmail(1);
      lcd.clear();
      lcd.print("OTP Send Done");
    } else {
      Serial.println("Email sai dinh dang");
    }
  }
}
BLYNK_WRITE(V1) {
  String inputEmail = param.asString();

  if (isValidGmail(inputEmail)) {
    recipientEmail2 = inputEmail;
    Serial.println("Email OK");
  } else {
    Serial.println("Email sai dinh dang");
  }
}
BLYNK_WRITE(V3) {
  int SendOTP = param.asInt();

  if (SendOTP) {
    if (isValidGmail(recipientEmail2)) {
      otpSys2 = genOTP();

      lcd.clear();
      lcd.print("OTP Sending...");
      sendEmail(2);
      lcd.clear();
      lcd.print("OTP Send Done");
    } else {
      Serial.println("Email sai dinh dang");
    }
  }
}
// ===== BEEP =====
void beep(int t) {
  digitalWrite(BUZZER, HIGH);
  delay(t);
  digitalWrite(BUZZER, LOW);
  delay(t);
}

// ===== ALARM =====
void alarm() {
  for (int i = 0; i < 5; i++) {
    beep(200);
  }
}

// ===== OTP =====
String genOTP() {
  String s = "";
  for (int i = 0; i < 6; i++) s += String(random(0, 10));
  Serial.println("OTP: " + s);
  return s;
}

// ===== OPEN LOCK =====
void openLock(int l) {
  int relay = (l == 1) ? RELAY1 : RELAY2;
  digitalWrite(relay, HIGH);
  beep(100);
  delay(4000);
  digitalWrite(relay, LOW);
}

// ===== SETUP =====
void setup() {
  Serial.begin(115200);

  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  pinMode(IR_LOCKER1, INPUT);
  pinMode(IR_LOCKER2, INPUT);

  lcd.init();
  lcd.backlight();

  lcd.clear();
  lcd.print("Connecting...");

  // Ép xóa bộ nhớ đệm mạng
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);

  Serial.println();
  Serial.println("=================================");
  Serial.print("DANG KET NOI VOI WIFI: ");
  Serial.println(ssid);

  WiFi.begin(ssid, pass);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    attempts++;
    if (attempts > 30) {
      Serial.println("\n[LOI] Khong the ket noi Wi-Fi!");
      lcd.clear();
      lcd.print("WiFi Failed!");
      delay(3000);
      ESP.restart(); 
    }
  }

  Serial.println("\n[THANG CONG] Wi-Fi da ket noi!");
  Serial.print("Dia chi IP: ");
  Serial.println(WiFi.localIP());
  Serial.println("=================================");

  // Chỉ config và connect Blynk, KHÔNG gọi Blynk.begin nữa
  Blynk.config(auth);
  Blynk.connect();

  lcd.clear();
  lcd.print("Ready");
  
  Blynk.syncVirtual(V1); 
  randomSeed(analogRead(0));
  delay(2000);
  fkgKeyPress = 1;
  locker = 0;
  isFullLocker1 = checkLocker(1);
  isFullLocker2 = checkLocker(2);
}
// ===== KẾT THÚC SETUP =====
// ===== LOOP =====
void loop() {
  Blynk.run();

  if (flgSysWarning == 0) {
    char key = keypad.getKey();

    if (key) {
      Serial.println(key);
      fkgKeyPress = 1;
      beep(50);
      if (locker == 0) {
        if (key == 'A' || key == 'B') {
          locker = (key == 'A') ? 1 : 2;  // nẾu locker > 0 thì k được chọn tủ nữa
          isFullLocker1 = checkLocker(1);
          isFullLocker2 = checkLocker(2);
        } else if (key == 'C') {
          isFullLocker1 = checkLocker(1);
          isFullLocker2 = checkLocker(2);
          if ((isFullLocker1 == 0) && (isFullLocker2 == 0)) {
            digitalWrite(RELAY1, HIGH);
            digitalWrite(RELAY2, HIGH);
            beep(100);
            delay(4000);
            digitalWrite(RELAY1, LOW);
            digitalWrite(RELAY2, LOW);
          }
        }
      } else {
        if (key == '*') {
          otpUserInput[inputCounterPinOut] = '*';
          inputCounterPinOut = inputCounterPinOut > 0 ? (inputCounterPinOut - 1) : 0;
        } else if (key == '#') {
          ResetOTP_INPUT();
        } else {
          // input += key;
          // lcd.setCursor(0, 1);
          // lcd.print(input);
        }
      }
    }
    PrintInputFunction(key);
  } else {
    beep(500);
    lcd.setCursor(0, 0);
    lcd.print("UNLOCK After 5'!");
    lcd.setCursor(0, 1);
    lcd.print("===== " + String((int)(SecondRemain / 60)) + ":" + String((int)(SecondRemain % 60)) + "=====");
    SecondRemain = SecondRemain > 0 ? (SecondRemain - 1) : 0;
    if (SecondRemain == 0) {
      flgSysWarning = 0;
      fkgKeyPress = 1;
      locker = 0;
    }
  }
}
