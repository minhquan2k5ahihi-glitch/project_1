int FailOTP_Counter = 0;
// =========================================================================
void configEMAIL() {
  /*  Set the network reconnection option */
  MailClient.networkReconnect(true);

  /** Enable the debug via Serial port
   * 0 for no debugging
   * 1 for basic level debugging
   *
   * Debug port can be changed via ESP_MAIL_DEFAULT_DEBUG_PORT in ESP_Mail_FS.h
   */
  smtp.debug(1);

  /* Set the callback function to get the sending results */
  smtp.callback(smtpCallback);

  /* Declare the Session_Config for user defined session credentials */
  Session_Config config;

  /* Set the session config */
  config.server.host_name = SMTP_HOST;
  config.server.port = SMTP_PORT;
  config.login.email = AUTHOR_EMAIL;
  config.login.password = AUTHOR_PASSWORD;
  config.login.user_domain = "";

  /*
  Set the NTP config time
  For times east of the Prime Meridian use 0-12
  For times west of the Prime Meridian add 12 to the offset.
  Ex. American/Denver GMT would be -6. 6 + 12 = 18
  See https://en.wikipedia.org/wiki/Time_zone for a list of the GMT/UTC timezone offsets
  */
  config.time.ntp_server = F("pool.ntp.org,time.nist.gov");
  config.time.gmt_offset = 3;
  config.time.day_light_offset = 0;
  /* Connect to the server */
  if (!smtp.connect(&config)) {
    ESP_MAIL_PRINTF("Connection error, Status Code: %d, Error Code: %d, Reason: %s", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
    return;
  }

  if (!smtp.isLoggedIn()) {
    Serial.println("\nNot yet logged in.");
  } else {
    if (smtp.isAuthenticated())
      Serial.println("\nSuccessfully logged in.");
    else
      Serial.println("\nConnected with no Auth.");
  }
}
/* Callback function to get the Email sending status */
void smtpCallback(SMTP_Status status) {
  /* Print the current status */
  Serial.println(status.info());

  /* Print the sending result */
  if (status.success()) {
    // ESP_MAIL_PRINTF used in the examples is for format printing via debug Serial port
    // that works for all supported Arduino platform SDKs e.g. AVR, SAMD, ESP32 and ESP8266.
    // In ESP8266 and ESP32, you can use Serial.printf directly.

    Serial.println("----------------");
    ESP_MAIL_PRINTF("Message sent success: %d\n", status.completedCount());
    ESP_MAIL_PRINTF("Message sent failed: %d\n", status.failedCount());
    Serial.println("----------------\n");

    // for (size_t i = 0; i < smtp.sendingResult.size(); i++) {
    //   /* Get the result item */
    //   // SMTP_Result result = smtp.sendingResult.getItem(i);

    //   // In case, ESP32, ESP8266 and SAMD device, the timestamp get from result.timestamp should be valid if
    //   // your device time was synched with NTP server.
    //   // Other devices may show invalid timestamp as the device time was not set i.e. it will show Jan 1, 1970.
    //   // You can call smtp.setSystemTime(xxx) to set device time manually. Where xxx is timestamp (seconds since Jan 1, 1970)

    //   // ESP_MAIL_PRINTF("Message No: %d\n", i + 1);
    //   // ESP_MAIL_PRINTF("Status: %s\n", result.completed ? "success" : "failed");
    //   // ESP_MAIL_PRINTF("Date/Time: %s\n", MailClient.Time.getDateTimeString(result.timestamp, "%B %d, %Y %H:%M:%S").c_str());
    //   // ESP_MAIL_PRINTF("Recipient: %s\n", result.recipients.c_str());
    //   // ESP_MAIL_PRINTF("Subject: %s\n", result.subject.c_str());
    // }
    // Serial.println("----------------\n");

    // // You need to clear sending result as the memory usage will grow up.
    // smtp.sendingResult.clear();
  }
}
// =========================================================================
// ===== SEND MAIL =====
void sendEmail(int lockerSelect) {

  // =========================================================================
  if ((lockerSelect == 1 ? recipientEmail : recipientEmail2) == "") {
    Serial.println("No email");
    lcd.clear();
    lcd.print("No Email!");
    return;
  }
  if (!isValidGmail(lockerSelect == 1 ? recipientEmail : recipientEmail2)) {
    Serial.println("SEND: Email sai dinh dang");
    return;
  }
  MailClient.networkReconnect(true);

  /** Enable the debug via Serial port
   * 0 for no debugging
   * 1 for basic level debugging
   *
   * Debug port can be changed via ESP_MAIL_DEFAULT_DEBUG_PORT in ESP_Mail_FS.h
   */
  smtp.debug(1);

  /* Set the callback function to get the sending results */
  smtp.callback(smtpCallback);

  /* Declare the Session_Config for user defined session credentials */
  Session_Config config;

  /* Set the session config */
  config.server.host_name = SMTP_HOST;
  config.server.port = SMTP_PORT;
  config.login.email = AUTHOR_EMAIL;
  config.login.password = AUTHOR_PASSWORD;
  config.login.user_domain = "";

  /*
  Set the NTP config time
  For times east of the Prime Meridian use 0-12
  For times west of the Prime Meridian add 12 to the offset.
  Ex. American/Denver GMT would be -6. 6 + 12 = 18
  See https://en.wikipedia.org/wiki/Time_zone for a list of the GMT/UTC timezone offsets
  */
  config.time.ntp_server = F("pool.ntp.org,time.nist.gov");
  config.time.gmt_offset = 7;
  config.time.day_light_offset = 0;

  /* Declare the message class */
  SMTP_Message message;
  // lấy thời gian hiện tại
  time_t now = time(nullptr);
  struct tm* t = localtime(&now);

  // SMTP_Message message;
  String subject;
  String body;
  char msgg[64];
  sprintf(msgg, "Thời gian hiện tại: %04d-%02d-%02d %02d:%02d:%02d",
          t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
          t->tm_hour, t->tm_min, t->tm_sec);

  subject = "⚠️ LOCKER OTP";
  body =
    "Dưới đây là OTP để mở tủ của bạn:\r\n"
    "OTP: "
    + String(lockerSelect == 1 ? otpSys : otpSys2) + " => Tủ số " + String(lockerSelect) + " \r\n"
                                                                                           "Vui lòng không chia sẽ với bất cứ ai.\r\n";

  /* Set the message headers */
  message.sender.name = F("SMART LOCKER NOTIFICATION!");
  message.sender.email = AUTHOR_EMAIL;
  message.subject = subject;
  message.addRecipient(F("Admin"), lockerSelect == 1 ? recipientEmail : recipientEmail2);

  // String textMsg = "Wheather Station Alert Rain Gauge";
  // ---- BODY ----
  message.text.content = body.c_str();
  message.text.charSet = "utf-8";
  message.text.transfer_encoding = Content_Transfer_Encoding::enc_7bit;


  message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_low;
  message.response.notify = esp_mail_smtp_notify_success | esp_mail_smtp_notify_failure | esp_mail_smtp_notify_delay;


  /* Connect to the server */
  if (!smtp.connect(&config)) {
    ESP_MAIL_PRINTF("Connection error, Status Code: %d, Error Code: %d, Reason: %s", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
    return;
  }

  if (!smtp.isLoggedIn()) {
    Serial.println("\nNot yet logged in.");
  } else {
    if (smtp.isAuthenticated())
      Serial.println("\nSuccessfully logged in.");
    else
      Serial.println("\nConnected with no Auth.");
  }
  // ESP.wdtFeed();  // feed WDT

  /* Start sending Email and close the session */
  if (!MailClient.sendMail(&smtp, &message)) {
    ESP_MAIL_PRINTF("Error, Status Code: %d, Error Code: %d, Reason: %s", smtp.statusCode(), smtp.errorCode(), smtp.errorReason().c_str());
    lcd.clear();
    lcd.print("Send Fail");
  } else {
    lcd.clear();
    lcd.print("OTP Sent!");
    Serial.println("Sent OK");
  }
}
// =========================================================================
void PrintLCD_OTP(int scr) {
  if (fkgKeyPress) {
    fkgKeyPress = 0;
    if (scr == 0) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Press 'A' or 'B'");
      lcd.setCursor(0, 1);
      lcd.print("Then Enter OTP!");
    } else {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Enter Pin Code:" + String(scr));
      lcd.setCursor(0, 1);
      lcd.print(otpUserInput);
    }
  }
}

bool checkOTP(int lockerSelect) {
  String checkOTP = "";
  checkOTP = lockerSelect == 1 ? otpSys : otpSys2;
  if (otpUserInput == checkOTP) {
    return 1;
  } else {
    return 0;
  }
}
void ResetOTP_INPUT() {
  locker = 0;
  fkgKeyPress = 1;
  inputCounterPinOut = 0;
  otpUserInput = "******";
}
void FailLockAlarm() {

  beep(100);
  delay(500);
  beep(100);
  delay(500);
  FailOTP_Counter++;
  inputCounterPinOut = 0;
  otpUserInput = "******";
  if (FailOTP_Counter == 3) {
    flgSysWarning = 1;
    SecondRemain = 300;
  }
}
// =========================================================================
void PrintInputFunction(char input) {
  if (locker == 0) {
    PrintLCD_OTP(0);
  } else {
    otpTime = millis();
    if (millis() - otpTime > OTP_TIMEOUT) {
      lcd.print("TIMEOUT");
      alarm();
      locker = 0;
      ResetOTP_INPUT();
    }

    if (locker == 1) {
      if (isFullLocker1) {
        if ((input >= '0') && (input <= '9')) {
          if (inputCounterPinOut == 5) {
            otpUserInput[inputCounterPinOut] = input;
            if (checkOTP(1)) {
              lcd.clear();
              lcd.print("Opening Locker 1");
              openLock(1);
              lcd.clear();
              lcd.print("Done");
              otpSys = "";
              delay(2000);
              ResetOTP_INPUT();
            } else {
              lcd.clear();
              lcd.print("Wrong");
              FailLockAlarm();
            }
          } else {
            otpUserInput[inputCounterPinOut] = input;
            inputCounterPinOut++;
          }
        }

      } else {
        lcd.clear();
        lcd.print("Opening Locker 1");
        openLock(1);
        lcd.clear();
        lcd.print("Done");
        otpSys = "";
        delay(2000);
        ResetOTP_INPUT();
      }
    } else if (locker == 2) {
      if (isFullLocker2) {
        if ((input >= '0') && (input <= '9')) {
          if (inputCounterPinOut == 5) {
            otpUserInput[inputCounterPinOut] = input;
            if (checkOTP(2)) {
              lcd.clear();
              lcd.print("Opening Locker 2");
              openLock(2);
              lcd.clear();
              lcd.print("Done");
              otpSys2 = "";
              delay(2000);
              ResetOTP_INPUT();
            } else {
              lcd.clear();
              lcd.print("Wrong");
              FailLockAlarm();
            }
          } else {
            otpUserInput[inputCounterPinOut] = input;
            inputCounterPinOut++;
          }
        }
      } else {
        lcd.clear();
        lcd.print("Opening Locker 2");
        openLock(2);
        lcd.clear();
        lcd.print("Done");
        otpSys2 = "";
        delay(2000);
        ResetOTP_INPUT();
      }
    }
    PrintLCD_OTP(locker);
  }
}
bool checkLocker(int lockerSelect) {
  return lockerSelect == 1 ? (!digitalRead(IR_LOCKER1)) : (!digitalRead(IR_LOCKER2));
}
