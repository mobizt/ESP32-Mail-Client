


//Required HTTPClientESP32Ex library, can be installed from https : //github.com/mobizt/HTTPClientESP32Ex

//To use send Email for Gmail, less secure app option should be enabled. https://myaccount.google.com/lesssecureapps?pli=1

//To receive Email for Gmail, IMAP option should be enabled. https://support.google.com/mail/answer/7126229?hl=en


#include <Arduino.h>
#include "ESP32_MailClient.h"
#include "SD.h"

//For demo only
#include "image.h"

#define WIFI_SSID "YOUR_WIFI_SSID"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

//WiFi or HTTP client for internet connection
HTTPClientESP32Ex http;

//The Email Sending data object contains config and data to send
SMTPData smtpData;

//Callback function to get the Email sending status
void sendCallback(SendStatus info);

void setup()
{

  Serial.begin(115200);
  Serial.println();

  Serial.print("Connecting to AP");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(200);
  }

  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println();


  Serial.println("Mounting SD Card...");

  if (SD.begin())
  {

    Serial.println("Preparing attach file...");

    File file = SD.open("/text_file.txt", FILE_WRITE);
    file.print("Hello World!\r\nHello World!");
    file.close();

    file = SD.open("/binary_file.dat", FILE_WRITE);

    static uint8_t buf[512];

    buf[0] = 'H';
    buf[1] = 'E';
    buf[2] = 'A';
    buf[3] = 'D';
    file.write(buf, 4);

    size_t i;
    memset(buf, 0xff, 512);
    for (i = 0; i < 2048; i++)
    {
      file.write(buf, 512);
    }

    buf[0] = 'T';
    buf[1] = 'A';
    buf[2] = 'I';
    buf[3] = 'L';
    file.write(buf, 4);

    file.close();
  }
  else
  {
    Serial.println("SD Card Monting Failed");
  }

  Serial.println();


  Serial.println("Sending email...");

  //Set the Email host, port, account and password
  smtpData.setLogin("smtp.gmail.com", 465, "YOUR_EMAIL_ACCOUNT@gmail.com", "YOUR_EMAIL_PASSWORD");

  //Set the sender name and Email
  smtpData.setSender("ESP32", "info@vibrologic.com");

  //Set Email priority or importance High, Normal, Low or 1 to 5 (1 is highest)
  smtpData.setPriority("High");

  //Set the subject
  smtpData.setSubject("ESP32 SMTP Mail Sending Test");

  //Set the message - normal text or html format
  smtpData.setMessage("<div style=\"color:#ff0000;font-size:20px;\">Hello World! - From ESP32</div>", true);

  //Add recipients, can add more than one recipient
  smtpData.addRecipient("k_suwatchai@hotmail.com");



  //Add attachments, can add the file or binary data from flash memory, file in SD card
  //Data from internal memory
  smtpData.addAttachData("firebase_logo.png", "image/png", (uint8_t *)dummyImageData, sizeof dummyImageData);

  //Add attach files from SD card
  //Comment these two lines, if no SD card connected
  //Two files that previousely created.
  smtpData.addAttachFile("/binary_file.dat");
  smtpData.addAttachFile("/text_file.txt");


  smtpData.setSendCallback(sendCallback);

  //Start sending Email, can be set callback function to track the status
  if (!MailClient.sendMail(http, smtpData))
    Serial.println("Error sending Email, " + MailClient.smtpErrorReason());

  //Clear all data from Email object to free memory
  smtpData.empty();

}

void loop()
{
}

//Callback function to get the Email sending status
void sendCallback(SendStatus msg)
{
  //Print the current status
  Serial.println(msg.info());

  //Do something when complete
  if (msg.success())
  {
    Serial.println("----------------");
  }
}

