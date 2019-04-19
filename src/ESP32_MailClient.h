/*
 *Mail Client Arduino Library for ESP32, version 1.0.4
 * 
 * April 18, 2019
 * 
 * This library allows ESP32 to send Email with/without attachment and receive Email with/without attachment download through SMTP and IMAP servers.
 * 
 * The library supports all ESP32 MCU based modules.
 * 
 * The MIT License (MIT)
 * Copyright (c) 2019 K. Suwatchai (Mobizt)
 * 
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef ESP32_MailClient_H
#define ESP32_MailClient_H

#include <Arduino.h>
#include <HTTPClientESP32Ex.h>
#include <vector>
#include <string>
#include "SD.h"
#include "SPI.h"
#include "RFC2047.h"

#define FORMAT_SPIFFS_IF_FAILED true

static RFC2047 RFC2047Decoder;

using namespace std;

#define SMTP_STATUS_SERVER_CONNECT_FAILED 1
#define SMTP_STATUS_SMTP_RESPONSE_FAILED 2
#define SMTP_STATUS_IDENTIFICATION_FAILED 3
#define SMTP_STATUS_AUTHEN_NOT_SUPPORT 4
#define SMTP_STATUS_AUTHEN_FAILED 5
#define SMTP_STATUS_USER_LOGIN_FAILED 6
#define SMTP_STATUS_PASSWORD_LOGIN_FAILED 7
#define SMTP_STATUS_SEND_HEADER_FAILED 8
#define SMTP_STATUS_SEND_BODY_FAILED 9

#define IMAP_STATUS_SERVER_CONNECT_FAILED 1
#define IMAP_STATUS_IMAP_RESPONSE_FAILED 2
#define IMAP_STATUS_LOGIN_FAILED 3
#define IMAP_STATUS_BAD_COMMAND 4

#define MAIL_CLIENT_STATUS_WIFI_CONNECT_FAIL 100

static const unsigned char base64_table[65] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

class IMAPData;
class ReadStatus;
class SMTPData;
class attachmentData;
class SendStatus;
class messageBodyData;
class DownloadProgress;
class MessageData;

typedef void (*sendStatusCallback)(SendStatus);
typedef void (*readStatusCallback)(ReadStatus);
typedef void (*downloadStatusCallback)(DownloadProgress);

static const char ESP32_MAIL_STR_1[] PROGMEM = "Content-Type: multipart/mixed; boundary=\"";
static const char ESP32_MAIL_STR_2[] PROGMEM = "{BOUNDARY}";
static const char ESP32_MAIL_STR_3[] PROGMEM = "Mime-Version: 1.0\r\n";
static const char ESP32_MAIL_STR_4[] PROGMEM = "AUTH LOGIN";
static const char ESP32_MAIL_STR_5[] PROGMEM = "HELO dude";
static const char ESP32_MAIL_STR_6[] PROGMEM = "EHLO dude";
static const char ESP32_MAIL_STR_7[] PROGMEM = "QUIT";
static const char ESP32_MAIL_STR_8[] PROGMEM = "MAIL FROM:";
static const char ESP32_MAIL_STR_9[] PROGMEM = "RCPT TO:";
static const char ESP32_MAIL_STR_10[] PROGMEM = "From: ";
static const char ESP32_MAIL_STR_11[] PROGMEM = "To: ";
static const char ESP32_MAIL_STR_12[] PROGMEM = "Cc: ";
static const char ESP32_MAIL_STR_13[] PROGMEM = ",<";
static const char ESP32_MAIL_STR_14[] PROGMEM = "<";
static const char ESP32_MAIL_STR_15[] PROGMEM = ">";
static const char ESP32_MAIL_STR_16[] PROGMEM = "DATA";
static const char ESP32_MAIL_STR_17[] PROGMEM = "X-Priority: ";
static const char ESP32_MAIL_STR_18[] PROGMEM = "X-MSMail-Priority: High\r\n";
static const char ESP32_MAIL_STR_19[] PROGMEM = "X-MSMail-Priority: Normal\r\n";
static const char ESP32_MAIL_STR_20[] PROGMEM = "X-MSMail-Priority: Low\r\n";
static const char ESP32_MAIL_STR_21[] PROGMEM = "Importance: High\r\n";
static const char ESP32_MAIL_STR_22[] PROGMEM = "Importance: Normal\r\n";
static const char ESP32_MAIL_STR_23[] PROGMEM = "Importance: Low\r\n";
static const char ESP32_MAIL_STR_24[] PROGMEM = "Subject: ";
static const char ESP32_MAIL_STR_25[] PROGMEM = "Content-Type: ";
static const char ESP32_MAIL_STR_26[] PROGMEM = "; Name=\"";
static const char ESP32_MAIL_STR_27[] PROGMEM = "Content-type: text/plain; charset=us-ascii\r\n";
static const char ESP32_MAIL_STR_28[] PROGMEM = "Content-type: text/html; charset=\"UTF-8\"\r\n";
static const char ESP32_MAIL_STR_29[] PROGMEM = "Content-transfer-encoding: 7bit\r\n";
static const char ESP32_MAIL_STR_30[] PROGMEM = "Content-Disposition: attachment; filename=\"";
static const char ESP32_MAIL_STR_31[] PROGMEM = "Content-transfer-encoding: base64\r\n";
static const char ESP32_MAIL_STR_32[] PROGMEM = "application/octet-stream";
static const char ESP32_MAIL_STR_33[] PROGMEM = "--";
static const char ESP32_MAIL_STR_34[] PROGMEM = "\r\n";
static const char ESP32_MAIL_STR_35[] PROGMEM = "\"\r\n\r\n";
static const char ESP32_MAIL_STR_36[] PROGMEM = "\"\r\n";
static const char ESP32_MAIL_STR_37[] PROGMEM = "\r\n.\r\n";
static const char ESP32_MAIL_STR_38[] PROGMEM = "could not connect to server";
static const char ESP32_MAIL_STR_39[] PROGMEM = "could not handle SMTP server response";
static const char ESP32_MAIL_STR_40[] PROGMEM = "could not handle IMAP server response";
static const char ESP32_MAIL_STR_41[] PROGMEM = "identification failed";
static const char ESP32_MAIL_STR_42[] PROGMEM = "authentication is not support";
static const char ESP32_MAIL_STR_43[] PROGMEM = "authentication failed";
static const char ESP32_MAIL_STR_44[] PROGMEM = "login account is not valid";
static const char ESP32_MAIL_STR_45[] PROGMEM = "could not sigin";
static const char ESP32_MAIL_STR_46[] PROGMEM = "could not parse command";
static const char ESP32_MAIL_STR_47[] PROGMEM = "login password is not valid";
static const char ESP32_MAIL_STR_48[] PROGMEM = "send header failed";
static const char ESP32_MAIL_STR_49[] PROGMEM = "send header failed";
static const char ESP32_MAIL_STR_50[] PROGMEM = "Connecting to IMAP server...";
static const char ESP32_MAIL_STR_51[] PROGMEM = "initialize";
static const char ESP32_MAIL_STR_52[] PROGMEM = "failed";
static const char ESP32_MAIL_STR_53[] PROGMEM = "Error, ";
static const char ESP32_MAIL_STR_54[] PROGMEM = "IMAP server connected, wait for response...";
static const char ESP32_MAIL_STR_55[] PROGMEM = "wait_for_response";
static const char ESP32_MAIL_STR_56[] PROGMEM = "Sign in...";
static const char ESP32_MAIL_STR_57[] PROGMEM = "signin";
static const char ESP32_MAIL_STR_58[] PROGMEM = "Lising folders...";
static const char ESP32_MAIL_STR_59[] PROGMEM = "listing";
static const char ESP32_MAIL_STR_60[] PROGMEM = ":::::::Message folders:::::::";
static const char ESP32_MAIL_STR_61[] PROGMEM = "Reading ";
static const char ESP32_MAIL_STR_62[] PROGMEM = "Predicted next UID: ";
static const char ESP32_MAIL_STR_63[] PROGMEM = "Total Message: ";
static const char ESP32_MAIL_STR_64[] PROGMEM = "::::::::::::Flags::::::::::::";
static const char ESP32_MAIL_STR_65[] PROGMEM = "::::::::::Messages:::::::::::";
static const char ESP32_MAIL_STR_66[] PROGMEM = "Searching messages...";
static const char ESP32_MAIL_STR_67[] PROGMEM = "searching";
static const char ESP32_MAIL_STR_68[] PROGMEM = "Search limit:";
static const char ESP32_MAIL_STR_69[] PROGMEM = "Found ";
static const char ESP32_MAIL_STR_70[] PROGMEM = " messages";
static const char ESP32_MAIL_STR_71[] PROGMEM = "Show ";
static const char ESP32_MAIL_STR_72[] PROGMEM = "Could not found any Email for defined criteria";
static const char ESP32_MAIL_STR_73[] PROGMEM = "Search criteria is not set, fetch the recent message";
static const char ESP32_MAIL_STR_74[] PROGMEM = "Feching message ";
static const char ESP32_MAIL_STR_75[] PROGMEM = ", UID: ";
static const char ESP32_MAIL_STR_76[] PROGMEM = ", Number: ";
static const char ESP32_MAIL_STR_77[] PROGMEM = "fetching";
static const char ESP32_MAIL_STR_78[] PROGMEM = "Attachment (";
static const char ESP32_MAIL_STR_79[] PROGMEM = ")";
static const char ESP32_MAIL_STR_80[] PROGMEM = "Downloading attachments...";
static const char ESP32_MAIL_STR_81[] PROGMEM = "downloading";
static const char ESP32_MAIL_STR_82[] PROGMEM = " bytes";
static const char ESP32_MAIL_STR_83[] PROGMEM = " - ";
static const char ESP32_MAIL_STR_84[] PROGMEM = "Free Heap: ";
static const char ESP32_MAIL_STR_85[] PROGMEM = "Sign out...";
static const char ESP32_MAIL_STR_86[] PROGMEM = "signout";
static const char ESP32_MAIL_STR_87[] PROGMEM = "Finished";
static const char ESP32_MAIL_STR_88[] PROGMEM = "finished";
static const char ESP32_MAIL_STR_89[] PROGMEM = "SD card mount failed";
static const char ESP32_MAIL_STR_90[] PROGMEM = "download ";
static const char ESP32_MAIL_STR_91[] PROGMEM = ", ";
static const char ESP32_MAIL_STR_92[] PROGMEM = "%";
static const char ESP32_MAIL_STR_93[] PROGMEM = "connection timeout";
static const char ESP32_MAIL_STR_94[] PROGMEM = "WiFi connection lost";
static const char ESP32_MAIL_STR_95[] PROGMEM = "no server response";
static const char ESP32_MAIL_STR_96[] PROGMEM = "finished";
static const char ESP32_MAIL_STR_97[] PROGMEM = " folder...";
static const char ESP32_MAIL_STR_98[] PROGMEM = "Finished";
static const char ESP32_MAIL_STR_99[] PROGMEM = "Date: ";
static const char ESP32_MAIL_STR_100[] PROGMEM = "Messsage UID: ";
static const char ESP32_MAIL_STR_101[] PROGMEM = "Messsage ID: ";
static const char ESP32_MAIL_STR_102[] PROGMEM = "Accept Language: ";
static const char ESP32_MAIL_STR_103[] PROGMEM = "Content Language: ";
static const char ESP32_MAIL_STR_104[] PROGMEM = "From: ";
static const char ESP32_MAIL_STR_105[] PROGMEM = "From Charset: ";
static const char ESP32_MAIL_STR_106[] PROGMEM = "To: ";
static const char ESP32_MAIL_STR_107[] PROGMEM = "To Charset: ";
static const char ESP32_MAIL_STR_108[] PROGMEM = "CC: ";
static const char ESP32_MAIL_STR_109[] PROGMEM = "CC Charset: ";
static const char ESP32_MAIL_STR_110[] PROGMEM = "Subject: ";
static const char ESP32_MAIL_STR_111[] PROGMEM = "Subject Charset: ";
static const char ESP32_MAIL_STR_112[] PROGMEM = "Message Charset: ";
static const char ESP32_MAIL_STR_113[] PROGMEM = "Attachment: ";
static const char ESP32_MAIL_STR_114[] PROGMEM = "File Index: ";
static const char ESP32_MAIL_STR_115[] PROGMEM = "Filename: ";
static const char ESP32_MAIL_STR_116[] PROGMEM = "Name: ";
static const char ESP32_MAIL_STR_117[] PROGMEM = "Size: ";
static const char ESP32_MAIL_STR_118[] PROGMEM = "Type: ";
static const char ESP32_MAIL_STR_119[] PROGMEM = "Creation Date: ";
static const char ESP32_MAIL_STR_120[] PROGMEM = "Connecting to SMTP server...";
static const char ESP32_MAIL_STR_121[] PROGMEM = "SMTP server connected, wait for response...";
static const char ESP32_MAIL_STR_122[] PROGMEM = "Identification...";
static const char ESP32_MAIL_STR_123[] PROGMEM = "Authentication...";
static const char ESP32_MAIL_STR_124[] PROGMEM = "Sign in...";
static const char ESP32_MAIL_STR_125[] PROGMEM = "Sending Email header...";
static const char ESP32_MAIL_STR_126[] PROGMEM = "Sending Email body...";
static const char ESP32_MAIL_STR_127[] PROGMEM = "Sending attacments...";
static const char ESP32_MAIL_STR_128[] PROGMEM = "Finalize...";
static const char ESP32_MAIL_STR_129[] PROGMEM = "Finished\r\nEmail sent successfully";
static const char ESP32_MAIL_STR_130[] PROGMEM = "$ LOGIN ";
static const char ESP32_MAIL_STR_131[] PROGMEM = " ";
static const char ESP32_MAIL_STR_132[] PROGMEM = " OK ";
static const char ESP32_MAIL_STR_133[] PROGMEM = "$ LIST \"\" \"*\"";
static const char ESP32_MAIL_STR_134[] PROGMEM = "Success";
static const char ESP32_MAIL_STR_135[] PROGMEM = "$ EXAMINE \"";
static const char ESP32_MAIL_STR_136[] PROGMEM = "\"";
static const char ESP32_MAIL_STR_137[] PROGMEM = "UID ";
static const char ESP32_MAIL_STR_138[] PROGMEM = " UID";
static const char ESP32_MAIL_STR_139[] PROGMEM = " SEARCH";
static const char ESP32_MAIL_STR_140[] PROGMEM = "UID";
static const char ESP32_MAIL_STR_141[] PROGMEM = "SEARCH";
static const char ESP32_MAIL_STR_142[] PROGMEM = "$ UID FETCH ";
static const char ESP32_MAIL_STR_143[] PROGMEM = "$ FETCH ";
static const char ESP32_MAIL_STR_144[] PROGMEM = " BODY.PEEK[HEADER.FIELDS (SUBJECT FROM TO DATE Message-ID Accept-Language Content-Language)]";
static const char ESP32_MAIL_STR_145[] PROGMEM = "$ OK Success";
static const char ESP32_MAIL_STR_146[] PROGMEM = "$ LOGOUT";
static const char ESP32_MAIL_STR_147[] PROGMEM = " BODY.PEEK[";
static const char ESP32_MAIL_STR_148[] PROGMEM = ".MIME]";
static const char ESP32_MAIL_STR_149[] PROGMEM = "multipart/";
static const char ESP32_MAIL_STR_150[] PROGMEM = "$ UID FETCH ";
static const char ESP32_MAIL_STR_151[] PROGMEM = " BODY.PEEK[";
static const char ESP32_MAIL_STR_152[] PROGMEM = ".";
static const char ESP32_MAIL_STR_153[] PROGMEM = "attachment";
static const char ESP32_MAIL_STR_154[] PROGMEM = "text/html";
static const char ESP32_MAIL_STR_155[] PROGMEM = "text/plain";
static const char ESP32_MAIL_STR_156[] PROGMEM = "]";
static const char ESP32_MAIL_STR_157[] PROGMEM = "* ESEARCH";
static const char ESP32_MAIL_STR_158[] PROGMEM = "$ NO ";
static const char ESP32_MAIL_STR_159[] PROGMEM = "$ BAD ";
static const char ESP32_MAIL_STR_160[] PROGMEM = "base64";
static const char ESP32_MAIL_STR_161[] PROGMEM = "/decoded_msg.txt";
static const char ESP32_MAIL_STR_162[] PROGMEM = "/raw_msg.txt";
static const char ESP32_MAIL_STR_163[] PROGMEM = "/decoded_msg.html";
static const char ESP32_MAIL_STR_164[] PROGMEM = "/raw_msg.html";
static const char ESP32_MAIL_STR_165[] PROGMEM = " FETCH ";
static const char ESP32_MAIL_STR_166[] PROGMEM = "* OK ";
static const char ESP32_MAIL_STR_167[] PROGMEM = "content-type: ";
static const char ESP32_MAIL_STR_168[] PROGMEM = "charset=\"";
static const char ESP32_MAIL_STR_169[] PROGMEM = "charset=";
static const char ESP32_MAIL_STR_170[] PROGMEM = "name=\"";
static const char ESP32_MAIL_STR_171[] PROGMEM = "name=";
static const char ESP32_MAIL_STR_172[] PROGMEM = "content-transfer-encoding: ";
static const char ESP32_MAIL_STR_173[] PROGMEM = "\r";
static const char ESP32_MAIL_STR_174[] PROGMEM = "content-description: ";
static const char ESP32_MAIL_STR_175[] PROGMEM = "content-disposition: ";
static const char ESP32_MAIL_STR_176[] PROGMEM = "filename=\"";
static const char ESP32_MAIL_STR_177[] PROGMEM = "filename=";
static const char ESP32_MAIL_STR_178[] PROGMEM = "size=";
static const char ESP32_MAIL_STR_179[] PROGMEM = "creation-date=\"";
static const char ESP32_MAIL_STR_180[] PROGMEM = "creation-date=";
static const char ESP32_MAIL_STR_181[] PROGMEM = "modification-date=\"";
static const char ESP32_MAIL_STR_182[] PROGMEM = "modification-date=";
static const char ESP32_MAIL_STR_183[] PROGMEM = "*";
static const char ESP32_MAIL_STR_184[] PROGMEM = "from: ";
static const char ESP32_MAIL_STR_185[] PROGMEM = "to: ";
static const char ESP32_MAIL_STR_186[] PROGMEM = "cc: ";
static const char ESP32_MAIL_STR_187[] PROGMEM = "subject: ";
static const char ESP32_MAIL_STR_188[] PROGMEM = "date: ";
static const char ESP32_MAIL_STR_189[] PROGMEM = "message-id: ";
static const char ESP32_MAIL_STR_190[] PROGMEM = "accept-language: ";
static const char ESP32_MAIL_STR_191[] PROGMEM = "content-language: ";
static const char ESP32_MAIL_STR_192[] PROGMEM = ")";
static const char ESP32_MAIL_STR_193[] PROGMEM = "{";
static const char ESP32_MAIL_STR_194[] PROGMEM = "}";
static const char ESP32_MAIL_STR_195[] PROGMEM = " LIST ";
static const char ESP32_MAIL_STR_196[] PROGMEM = "\\Noselect";
static const char ESP32_MAIL_STR_197[] PROGMEM = " FLAGS ";
static const char ESP32_MAIL_STR_198[] PROGMEM = "(";
static const char ESP32_MAIL_STR_199[] PROGMEM = " EXISTS";
static const char ESP32_MAIL_STR_200[] PROGMEM = " [UIDNEXT ";
static const char ESP32_MAIL_STR_201[] PROGMEM = "]";
static const char ESP32_MAIL_STR_202[] PROGMEM = "/";
static const char ESP32_MAIL_STR_203[] PROGMEM = "/header.txt";
static const char ESP32_MAIL_STR_204[] PROGMEM = "/esp.32";
static const char ESP32_MAIL_STR_205[] PROGMEM = "high";
static const char ESP32_MAIL_STR_206[] PROGMEM = "High";
static const char ESP32_MAIL_STR_207[] PROGMEM = "normal";
static const char ESP32_MAIL_STR_208[] PROGMEM = "Normal";
static const char ESP32_MAIL_STR_209[] PROGMEM = "low";
static const char ESP32_MAIL_STR_210[] PROGMEM = "Low";
static const char ESP32_MAIL_STR_211[] PROGMEM = "$ OK ";
static const char ESP32_MAIL_STR_212[] PROGMEM = "FLAGS";
static const char ESP32_MAIL_STR_213[] PROGMEM = "BODY";
static const char ESP32_MAIL_STR_214[] PROGMEM = "PEEK";
static const char ESP32_MAIL_STR_215[] PROGMEM = "TEXT";
static const char ESP32_MAIL_STR_216[] PROGMEM = "HEADER";
static const char ESP32_MAIL_STR_217[] PROGMEM = "FIELDS";
static const char ESP32_MAIL_STR_218[] PROGMEM = "[";
static const char ESP32_MAIL_STR_219[] PROGMEM = "]";
static const char ESP32_MAIL_STR_220[] PROGMEM = "MIME";
static const char ESP32_MAIL_STR_221[] PROGMEM = "connection lost";

static bool compFunc(std::string i, std::string j)
{
  return (atoi(i.c_str()) > atoi(j.c_str()));
}

class ESP32_MailClient
{

public:
  /*
     
    Sending Email through SMTP server
      
    @param http - HTTPClientESP32Ex WiFi client.
    @param smtpData - SMTP Data object to hold data and instances.

    @return Boolean type status indicates the success of operation.

  */
  bool sendMail(HTTPClientESP32Ex &http, SMTPData &smtpData);

  /*
  
    Reading Email through IMAP server.
  
    @param http - HTTPClientESP32Ex WiFi client.
    @param imapData - IMAP Data object to hold data and instances.

    @return Boolean type status indicates the success of operation.
  
  */
  bool readMail(HTTPClientESP32Ex &http, IMAPData &imapData);

  /*
  
    Get the Email sending error details.
  
    @return Error details string (String object).
  
  */
  String smtpErrorReason();

  /*
  
    Get the Email reading error details.
  
    @return Error details string (String object).
  
  */
  String imapErrorReason();

  /*
  
    Init SD card with GPIO pins.
  
    @param sck -  SPI Clock pin.
    @param miso - SPI MISO pin.
    @param mosi - SPI MOSI pin.
    @param ss -   SPI Chip/Slave Select pin.

    @return Boolean type status indicates the success of operation.

  */
  bool sdBegin(uint8_t sck, uint8_t miso, uint8_t mosi, uint8_t ss);

  /*
  
    Init SD card with default GPIO pins.

    @return Boolean type status indicates the success of operation.
  
  */
  bool sdBegin(void);

  struct IMAP_COMMAND_TYPE;
  struct IMAP_HEADER_TYPE;

protected:
  int _smtpStatus = 0;
  int _imapStatus = 0;
  bool _sdOk = false;
  bool _sdConfigSet = false;
  uint8_t _sck, _miso, _mosi, _ss;

  std::string smtpErrorReasonStr();
  std::string imapErrorReasonStr();
  void set_message_header(string &header, std::string &message, bool htmlFormat);
  void set_attachment_header(uint8_t index, std::string &header, attachmentData &attach);
  void tcpFlush(WiFiClient *tcp);
  bool validSubstringRange(int start, int length, std::string str);
  double base64DecodeSize(std::string lastBase64String, int length);
  unsigned char *base64_decode_char(const unsigned char *src, size_t len, size_t *out_len);
  std::string base64_encode_string(const unsigned char *src, size_t len);
  void send_base64_encode_data(WiFiClient *tcp, const unsigned char *src, size_t len);
  void send_base64_encode_file(WiFiClient *tcp, const char *filePath);
  int waitSMTPResponse(HTTPClientESP32Ex &http);
  bool waitIMAPResponse(HTTPClientESP32Ex &http, IMAPData &imapData, ReadStatus &cbData, uint8_t imapCommandType = 0, string endResp1 = "", string endResp2 = "", int maxChar = 0, int mailIndex = -1, int messageDataIndex = -1, std ::string part = "");
  inline std::string trim(std::string &str);
  void createDirs(std::string dirs);
  bool sdTest();
};

class messageBodyData
{
public:
  messageBodyData();
  ~messageBodyData();

  friend ESP32_MailClient;
  friend IMAPData;

protected:
  uint8_t _index = 0;
  size_t _size = 0;
  std::string _text = "";
  std::string _filename = "";
  std::string _savePath = "";
  std::string _name = "";
  std::string _disposition = "";
  std::string _contentType = "";
  std::string _descr = "";
  std::string _transfer_encoding = "";
  std::string _creation_date = "";
  std::string _modification_date = "";
  std::string _charset = "";
  std::string _part = "";
  std::string _downloadError = "";
  bool _sdFileOpenWrite = false;
  bool _error = false;
};

class attachmentData
{
public:
  attachmentData();
  ~attachmentData();

  friend ESP32_MailClient;
  friend SMTPData;

protected:
  uint8_t _index;
  std::vector<std::vector<uint8_t *>> _buf;
  std::vector<std::string> _filename;
  std::vector<uint8_t> _id;
  std::vector<uint8_t> _type;
  std::vector<uint16_t> _size;
  std::vector<std::string> _mime_type;

  void add(const String &fileName, const String &mimeType, uint8_t *data, uint16_t size);
  void remove(uint8_t index);
  void free();
  String getFileName(uint8_t index);
  String getMimeType(uint8_t index);
  uint8_t *getData(uint8_t index);
  uint16_t getSize(uint8_t index);
  uint8_t getCount();
  uint8_t getType(uint8_t index);
};

class IMAPData
{
public:
  IMAPData();
  ~IMAPData();

  /*
    Set the IMAP server login credentials.
  
    @param host - IMAP server e.g. imap.gmail.com.
    @param port - IMAP port e.g. 993 for gmail.
    @param loginEmail - The Email address of account.    
    @param loginPassword - The account password.

    */
  void setLogin(const String &host, uint16_t port, const String &loginEmail, const String &loginPassword);

  /*

    Set the mailbox folder to search or fetch.
    
    @param folderName - Known mailbox folder. Ddefault value is INBOX
  
  */
  void setFolder(const String &folderName);

  /*
  
    Set the maximum message buffer size for text/html result from search or fetch the message.
  
    @param size - The message size in byte.
    
  */
  void setMessageBufferSize(size_t size);

  /*
  
    Set the maximum attachment file size to be downloaded.
    
    @param size - The attachement file size in byte.
  
  */
  void setAttachmentSizeLimit(size_t size);

  /*
    Set the search criteria used in selected mailbox search.

    In case of message UID was set through setFechUID function, search operation will not process, 
    you need to clear message UID by calling imapData.setFechUID("") to clear.

    @param criteria - Search criteria String.

    If folder is not set, the INBOX folder will be used
    
    Example:
    
    "SINCE 10-Feb-2019" will search all messages that received since 10 Feb 2019
    "UID SEARCH ALL" will seach all message which will return the message UID that can be use later for fetch one or more messages.

 
    Search criteria can be consisted these keywords

    ALL - All messages in the mailbox; the default initial key for	ANDing.
    ANSWERED - Messages with the \Answered flag set.
    BCC - Messages that contain the specified string in the envelope structure's BCC field.
    BEFORE - Messages whose internal date (disregarding time and timezone) is earlier than the specified date.
    BODY - Messages that contain the specified string in the body of the	message.
    CC - Messages that contain the specified string in the envelope structure's CC field.
    DELETED - Messages with the \Deleted flag set.
    DRAFT - Messages with the \Draft flag set.
    FLAGGED - Messages with the \Flagged flag set.
    FROM - Messages that contain the specified string in the envelope	structure's FROM field.
    HEADER - Messages that have a header with the specified field-name (as defined in [RFC-2822]) 
    and that contains the specified string	in the text of the header (what comes after the colon).

    If the string to search is zero-length, this matches all messages that have a header line with 
    the specified field-name regardless of	the contents.

    KEYWORD - Messages with the specified keyword flag set.
    LARGER - Messages with an [RFC-2822] size larger than the specified number of octets.
    NEW -  Messages that have the \Recent flag set but not the \Seen flag.
    This is functionally equivalent to "(RECENT UNSEEN)".
    NOT - Messages that do not match the specified search key.
    OLD - Messages that do not have the \Recent flag set.  This is	functionally equivalent to
    "NOT RECENT" (as opposed to "NOT	NEW").
    ON - Messages whose internal date (disregarding time and timezone) is within the specified date.
    OR - Messages that match either search key.
    RECENT - Messages that have the \Recent flag set.
    SEEN - Messages that have the \Seen flag set.
    SENTBEFORE - Messages whose [RFC-2822] Date: header (disregarding time and	timezone) is earlier than the specified date.
    SENTON - Messages whose [RFC-2822] Date: header (disregarding time and timezone) is within the specified date.
    SENTSINCE - Messages whose [RFC-2822] Date: header (disregarding time and timezone) is within or later than the specified date.
    SINCE - Messages whose internal date (disregarding time and timezone) is within or later than the specified date.
    SMALLER - Messages with an [RFC-2822] size smaller than the specified number of octets.
    SUBJECT - Messages that contain the specified string in the envelope structure's SUBJECT field.
    TEXT - Messages that contain the specified string in the header or body of the message.
    TO - Messages that contain the specified string in the envelope structure's TO field.
    UID - Messages with unique identifiers corresponding to the specified unique identifier set.  
    Sequence set ranges are permitted.
    UNANSWERED - Messages that do not have the \Answered flag set.
    UNDELETED - Messages that do not have the \Deleted flag set.
    UNDRAFT - Messages that do not have the \Draft flag set.
    UNFLAGGED - Messages that do not have the \Flagged flag set.
    UNKEYWORD - Messages that do not have the specified keyword flag set.
    UNSEEN - Messages that do not have the \Seen flag set.

  */
  void setSearchCriteria(const String criteria);

  /*
    Set the download folder.
    
    @param path - Path in SD card.

    All text/html message and attachemnts will be saved to message UID folder which created in defined path
    e.g. "/{DEFINED_PATH}/{MESSAGE_UID}/{ATTACHMENT_FILE...}".

  */
  void setSaveFilePath(const String path);

  /*
    
    Specify message UID to fetch or read.
    
    @param fetchUID - The message UID.

    Specify the message UID to fetch (read) only specific message instead of search.

  */
  void setFechUID(const String fetchUID);

  /*

    Enable/disable attachment download.
    
    @param download - Boolean flag to enable/disable attachment download.

  */
  void setDownloadAttachment(bool download);

  /*
    
    Enable/disable html message result.
    
    @param htmlFormat - Boolean flag to enable/disable html message result.

    The default value is false.
  
  */
  void setHTMLMessage(bool htmlFormat);

  /*

    Enable/disable plain text message result.
    
    @param textFormat - Boolean flag to enable/disable plain text message result.

    The default value is true.

  */
  void setTextMessage(bool textFormat);

  /*
    
    Set the maximum message to search.
    
    @param limit - Any number from 0 to 65535.

    The default value is 20.

  */
  void setSearchLimit(uint16_t limit);

  /*

    Enable/disable recent sort result.    
    
    @param recentSort - Boolean flag to enable/disable recent message sort result.

     The default value is true.
   
   */
  void setRecentSort(bool recentSort);

  /*
    
    Assign callback function that return status of message fetching or reading.
    
    @param readCallback - The function that accept readStatusCallback as parameter.
  
  */
  void setReadCallback(readStatusCallback readCallback);

  /*
    
    Enable/disable attachement download progress while fetching or receiving message.
    
    @param report - Boolean flag to enable/disable attachement download progress while fetching or receiving message.

    To get the download status, Callback function should be set through setReadCallback.

  */
  void setDownloadReport(bool report);

  /*
    
    Determine only message header is return when search.
    
  */
  bool isHeaderOnly();

  /*
    
    Get the sender name/Email for selected message from search result.
    
    @param messageIndex - The index of message.

    @return Sender name/Email String.

  */
  String getFrom(uint16_t messageIndex);

  /*
    
    Get the sender name/Email charactor encoding.
    
    @param messageIndex - The index of message.

    @return Sender name/Email charactor encoding which use for decoding.

  */
  String getFromCharset(uint16_t messageIndex);

  /*
    
    Get the recipient name/Email for selected message index from search result.
    
    @param messageIndex - The index of message.

    @return Recipient name/Email String.
  
  */
  String getTo(uint16_t messageIndex);

  /*
    
    Get the recipient name/Email charactor encoding.
    
    @param messageIndex - The index of message.

    @return Recipient name/Email charactor encoding which use in decoding to local language.
  
  */
  String getToCharset(uint16_t messageIndex);

  /*
    
    Get the CC name/Email for selected message index of IMAPData result.
    
    @param messageIndex - The index of message.

    @return CC name/Email String.

  */
  String getCC(uint16_t messageIndex);

  /*
    
    Get the CC name/Email charactor encoding.
    
    @param messageIndex - The index of message.

    @return CC name/Email charactor encoding which use in decoding to local language.
  
  */
  String getCCCharset(uint16_t messageIndex);

  /*
    
    Get the message subject for selected message index from search result.
    
    @param messageIndex - The index of message.

    @return Message subject name/Email String.

  */
  String getSubject(uint16_t messageIndex);

  /*
    
    Get the message subject charactor encoding.
    
    @param messageIndex - The index of message.

    @return Message subject charactor encoding which use in decoding to local language.

  */
  String getSubjectCharset(uint16_t messageIndex);

  /*
    
    Get the html message for selected message index from search result.
    
    @param messageIndex - The index of message.

    @return The html message String or empty String upon the setHTMLMessage was set.

  */
  String getHTMLMessage(uint16_t messageIndex);

  /*
    
    Get the plain text message for selected message index from search result.
    
    @param messageIndex - The index of message.

    @return The plain text message String or empty String upon the setTextMessage was set.

  */
  String getTextMessage(uint16_t messageIndex);

  /*
    
    Get the html message charactor encoding.
    
    @param messageIndex - The index of message.

    @return Html message charactor encoding which use in decoding to local language.

  */
  String getHTMLMessgaeCharset(uint16_t messageIndex);

  /*
    
    Get the text message charactor encoding.
    
    @param messageIndex - The index of message.

    @return The text message charactor encoding which use in decoding to local language.

  */
  String getTextMessgaeCharset(uint16_t messageIndex);

  /*
    
    Get the date of received message for selected message index from search result.
    
    @param messageIndex - The index of message.

    @return The date String.

  */
  String getDate(uint16_t messageIndex);

  /*
    
    Get the message UID for selected message index from search result.
    
    @param messageIndex - The index of message.

    @return UID String that can be use in setFechUID.

  */
  String getUID(uint16_t messageIndex);

  /*

    Get the message number for selected message index from search result.
    
    @param messageIndex - The index of message.

    @return The message number which vary upon search criteria and sorting.

  */
  String getNumber(uint16_t messageIndex);

  /*
    
    Get the message ID for selected message index from search result.
    
    @param messageIndex - The index of message.

    @return The message ID String.

  */
  String getMessageID(uint16_t messageIndex);

  /*
    
    Get the accept language for selected message index from search result.
    
    @param messageIndex - The index of message.

    @return The accept language String.
  
  */
  String getAcceptLanguage(uint16_t messageIndex);

  /*
    
    Get the content language of text or html for selected message index from search result.
    
    @param messageIndex - The index of message.

    @return The content language String.

  */
  String getContentLanguage(uint16_t messageIndex);

  /*
    
    Determine fetch error status for selected message index from search result.
    
    @param messageIndex - The index of message.

    @return Fetch error status.

  */
  bool isFetchMessageFailed(uint16_t messageIndex);

  /*
    Get fetch error reason for selected message index from search result.
    
    @param messageIndex - The index of message.

    @return Fetch error reason String for selected message index.

  */
  String getFetchMessageFailedReason(uint16_t messageIndex);

  /*
    Determine the attachment download error for selected message index from search result.
    
    @param messageIndex - The index of message.

    @return Fetch status.

  */
  bool isDownloadAttachmentFailed(uint16_t messageIndex, size_t attachmentIndex);

  /*
    
    Get the attachment download error reason for selected message index from search result.
    
    @param messageIndex - The index of message.

    @return Download error reason String for selected message index.

  */
  String getDownloadAttachmentFailedReason(uint16_t messageIndex, size_t attachmentIndex);

  /*
    
    Determine the downloaded/saved text message error status for selected message index from search result.
    
    @param messageIndex - The index of message.

    @return Text message download status.

  */
  bool isDownloadMessageFailed(uint16_t messageIndex);

  /*
    
    Get the attachment or message downloadeds error reason for selected message index from search result.
    
    @param messageIndex - The index of message.

    @return Downloaded error reason String for selected message index.
  
  */
  String getDownloadMessageFailedReason(uint16_t messageIndex);

  /*
    
    Assign the download and decode flags for html message download.
    
    @param download - Boolean flag to enable/disable message download.

    @param decoded - Boolean flag to enable/disable html message decoding (support utf8 and base64 encoding).
  
  */
  void saveHTMLMessage(bool download, bool decoded);

  /*
    
    Assign the download and decode flags for plain text message download.
    
    @param download - Boolean flag to enable/disable message download.

    @param decoded - Boolean flag to enable/disable plain text message decoding (support utf8 and base64 encoding).
  
  */
  void saveTextMessage(bool download, bool decoded);

  /*
    
    Determine the mailbox folder count.
    
    @return Folder count number.

  */
  uint16_t getFolderCount();

  /*
    Get the mailbox folder name at selected index.
    
    @param folderIndex - Index of folder.

    @return Folder name String.
    
    Use folder name from this function for fetch or search.

  */
  String getFolder(uint16_t folderIndex);

  /*

    Determin the number of supported flags count.

    @return Flag count number.

  */
  uint16_t getFlagCount();

  /*
    Get the flag name for selected index.
    
    @param folderIndex - Index of folder.

    @return Flag name String.
    
    Use flags from this function for fetch or search.

    */
  String getFlag(uint16_t flagIndex);

  /*
    
    Get the number of message in selected mailbox folder.

    @return Total message number.
  
  */
  size_t totalMessages();

  /*

    Get the number of message from search result.

    @return Search result number.

  */
  size_t searchCount();

  /*

    Get the number of message available from search result which less than search limit.

    @return Available message number.

  */
  size_t availableMessages();

  /*

    Get the number of attachments for selected message index from search result.

    @param messageIndex - Index of message.

    @return Number of attachments
    
  */
  size_t getAttachmentCount(uint16_t messageIndex);

  /*
    
    Get file name of attachment for selected attachment index and message index from search result.

    @param messageIndex - Index of message.
    @param attachmentIndex - Index of attachment.

    @return The attachment file name String at the selected index.

  */
  String getAttachmentFileName(size_t messageIndex, size_t attachmentIndex);

  /*

    Get the name of attachment for selected attachment index and message index from search result.

    @param messageIndex - Index of message.
    @param attachmentIndex - Index of attachment.

    @return The attachment name String at the selected index.

  */
  String getAttachmentName(size_t messageIndex, size_t attachmentIndex);

  /*

    Get attachment file size for selected attachment index and message index from search result.

    @param messageIndex - Index of message.
    @param attachmentIndex - Index of attachment.

    @return The attachment file size in byte at the selected index.

  */
  int getAttachmentFileSize(size_t messageIndex, size_t attachmentIndex);

  /*
    
    Get creation date of attachment for selected attachment index and message index from search result.

    @param messageIndex - Index of message.
    @param attachmentIndex - Index of attachment.

    @return The attachment creation date String at the selected index.

  */
  String getAttachmentCreationDate(size_t messageIndex, size_t attachmentIndex);

  /*
    Get attachment file type for selected attachment index and message index from search result.

    @param messageIndex - Index of message.
    @param attachmentIndex - Index of attachment.

    @return File MIME String at the selected index e.g. image/jpeg.

  */
  String getAttachmentType(size_t messageIndex, size_t attachmentIndex);

  /*

    Clear all IMAPData object data.

  */
  void empty();

  /*

    Clear IMAPData object message data.

  */
  void clearMessageData();

  friend ESP32_MailClient;

private:
  String getMessage(uint16_t messageIndex, bool htmlFormat);
  size_t _totalMessage = 0;
  std::string _host = "";
  uint16_t _port = 993;
  std::string _loginEmail = "";
  std::string _loginPassword = "";
  std::string _currentFolder = "INBOX";
  std::string _nextUID = "";
  std::string _searchCriteria = "ALL*";
  std::string _fetchUID = "";
  std::string _savePath = "";

  bool _downloadAttachment = false;
  bool _recentSort = true;
  bool _htmlFormat = false;
  bool _textFormat = false;
  bool _headerOnly = true;
  bool _uidSearch = false;
  bool _saveHTMLMsg = false;
  bool _saveTextMsg = false;
  bool _saveDecodedHTML = false;
  bool _saveDecodedText = false;
  bool _downloadReport = false;
  bool _headerSaved = false;

  size_t _message_buffer_size = 200;
  size_t _attacement_max_size = 1024 * 1024;
  uint16_t _emailNumMax = 20;
  int _searchCount;
  readStatusCallback _readCallback = NULL;

  std::vector<std::string> _date;
  std::vector<std::string> _subject;
  std::vector<std::string> _subject_charset;
  std::vector<std::string> _from;
  std::vector<std::string> _from_charset;
  std::vector<std::string> _to;
  std::vector<std::string> _to_charset;
  std::vector<std::string> _cc;
  std::vector<std::string> _cc_charset;
  std::vector<std::string> _msgNum;
  std::vector<std::string> _msgID;
  std::vector<std::string> _contentLanguage;
  std::vector<std::string> _acceptLanguage;

  std::vector<std::string> _folders;
  std::vector<std::string> _flag;
  std::vector<int> _attachmentCount;
  std::vector<std::vector<messageBodyData>> _messageDataInfo;
  std::vector<int> _totalAttachFileSize;
  std::vector<int> _downloadedByte;
  std::vector<int> _messageDataCount;
  std::vector<std::string> _errorMsg;
  std::vector<bool> _error;
};

class SMTPData
{
public:
  SMTPData();
  ~SMTPData();

  /*

    Set SMTP server login credentials
    
    @param host - SMTP server e.g. smtp.gmail.com
    @param port - SMTP port.
    @param loginEmail - The account Email.
    @param loginPassword - The account password.

  */
  void setLogin(const String &host, uint16_t port, const String &loginEmail, const String &loginPassword);

  /*
    
    Set Sender info

     @param fromName - Sender's name
     @param senderEmail - Sender's Email.

  */
  void setSender(const String &fromName, const String &senderEmail);

  /*

    Get Sender's name
    
    @return Sender's name String.

  */
  String getFromName();

  /*

    Get Sender's Email
    
    @return Sender's Email String.

  */
  String getSenderEmail();

  /*

    Set Email priority or importance

    @param priority - Number from 1 to 5, 1 for highest, 3 for normal and 5 for lowest priority
  
  */
  void setPriority(int priority);

  /*

    Set Email priority or importance
    
    @param priority - String (High, Normal or Low)
  
  */
  void setPriority(const String &priority);

  /*

    Get Email priority

    @return number represents Email priority (1 for highest, 3 for normal, 5 for low priority).
  
  */
  uint8_t getPriority();

  /*

    Add one or more recipient
    
    @param email - Recipient Email String of one recipient.

    To add multiple recipients, call addRecipient for each recipient.
  
  */
  void addRecipient(const String &email);

  /*

    Remove recipient
    
    @param email - Recipient Email String.

  */
  void removeRecipient(const String &email);

  /*

    Remove recipient
    
    @param index - Index of recipients in Email object that previously added.

  */
  void removeRecipient(uint8_t index);

  /*
    Clear all recipients.
    */
  void clearRecipient();

  /*

    Get one recipient
    
    @param index - Index of recipients.

    @return Recipient Email String at the index.

  */
  String getRecipient(uint8_t index);

  /*

    Get number of recipients
    
    @return Number of recipients.

  */
  uint8_t recipientCount();

  /*

    Set the Email subject
    
    @param subject - The subject.

  */
  void setSubject(const String &subject);

  /*

    Get the Email subject
    
    @return Subject String.

  */
  String getSubject();

  /*

    Set the Email message
    
    @param message - The message can be in normal text or html format.
    @param htmlFormat - The html format flag, True for send the message as html format
  
  */
  void setMessage(const String &message, bool htmlFormat);

  /*

    Get the message
    
    @return Message String.

  */
  String getMessage();

  /*

    Determine  message is being send in html format
    
    @return Boolean status.

  */
  bool htmlFormat();

  /*

    Add Carbon Copy (CC) Email
    
    @param email - The CC Email String.

  */
  void addCC(const String &email);

  /*

    Remove specified Carbon Copy (CC) Email
    
    @param email - The CC Email String to remove.

  */
  void removeCC(const String &email);

  /*

    Remove specified Carbon Copy (CC) Email
    
    @param index - The CC Email index to remove.
  
  */
  void removeCC(uint8_t index);

  /*

    Clear all Carbon Copy (CC) Emails

  */
  void clearCC();

  /*

    Get Carbon Copy (CC) Email at specified index
    
    @param index - The CC Email index to get.
    @return The CC Email string at the index.

  */
  String getCC(uint8_t index);

  /*

    Get the number of Carbon Copy (CC) Email
    
    @return Number of CC Emails.

  */
  uint8_t ccCount();

  /*
    Add Blind Carbon Copy (BCC) Email
    
    @param email - The BCC Email String.

  */
  void addBCC(const String &email);

  /*

    Remove specified Blind Carbon Copy (BCC) Email
    
    @param email - The BCC Email String to remove.

  */
  void removeBCC(const String &email);

  /*

    Remove specified Blind Carbon Copy (BCC) Email
    
    @param index - The BCC Email index to remove.

  */
  void removeBCC(uint8_t index);

  /*

    Clear all Blind Carbon Copy (BCC) Emails
    
  */
  void clearBCC();

  /*

    Get Blind Carbon Copy (BCC) Email at specified index
    
    @param index - The BCC Email index to get.

    @return The BCC Email string at the index.

  */
  String getBCC(uint8_t index);

  /*

    Get the number of Blind Carbon Copy (BCC) Email
    
    @return Number of BCC Emails.

  */
  uint8_t bccCount();

  /*

    Add attchement data (binary) from internal memory (flash or ram)
    
    @param fileName - The file name String that recipient can be saved.
    @param mimeType - The MIME type of file (image/jpeg, image/png, text/plain...). Can be empty String.
    @param data - The byte array of data (uint8_t)
    @param size - The data length in byte.

  */
  void addAttachData(const String &fileName, const String &mimeType, uint8_t *data, uint16_t size);

  /*

    Remove specified attachment data
    
    @param fileName - The file name of the attachment data to remove.

  */
  void removeAttachData(const String &fileName);

  /*

    Remove specified attachment data
    
    @param index - The index of the attachment data (count only data type attachment) to remove. 
  
  */
  void removeAttachData(uint8_t index);

  /*

    Get the number of attachment data
    
    @return Number of attach data.

  */
  uint8_t attachDataCount();

  /*

    Add attchement file from SD card
    
    @param fileName - The file name String that recipient can be saved.
    @param mimeType - The MIME type of file (image/jpeg, image/png, text/plain...). Can be omitted.

  */
  void addAttachFile(const String &filePath, const String &mimeType = "");

  /*

    Remove specified attachment file from Email object
    
    @param fileName - The file name of the attachment file to remove.

  */
  void removeAttachFile(const String &filePath);

  /*

    Remove specified attachment file
    
    @param index - The index of the attachment file (count only file type attachment) to remove.

  */
  void removeAttachFile(uint8_t index);

  /*

    Clear all attachment data

  */
  void clearAttachData();

  /*

    Clear all attachment file.
  
  */
  void clearAttachFile();

  /*

    Clear all attachments (both data and file type attachments).
  
  */
  void clearAttachment();

  /*

    Get number of attachments (both data and file type attachments).
    
    @return Number of all attachemnts.

  */
  uint8_t attachFileCount();

  /*

    Clear all data from Email object to free memory.

  */
  void empty();

  /*

    Set the Email sending status callback function to Email object.
    
    @param sendCallback - The callback function that accept the sendStatusCallback param.
  
  */
  void setSendCallback(sendStatusCallback sendCallback);

  friend ESP32_MailClient;
  friend attachmentData;

protected:
  int _priority = -1;
  string _loginEmail;
  string _loginPassword;
  string _host;
  uint16_t _port;

  string _fromName;
  string _senderEmail;
  string _subject;
  string _message;
  bool _htmlFormat;
  sendStatusCallback _sendCallback = NULL;

  std::vector<std::string> _recipient;
  std::vector<std::string> _cc;
  std::vector<std::string> _bcc;
  attachmentData _attach;
};

class SendStatus
{
public:
  SendStatus();
  ~SendStatus();
  String info();
  bool success();
  void empty();
  friend ESP32_MailClient;

private:
  std::string _info;
  bool _success;
};

class ReadStatus
{
public:
  ReadStatus();
  ~ReadStatus();
  String status();
  String info();
  bool success();
  void empty();
  friend ESP32_MailClient;

private:
  std::string _status;
  std::string _info;
  bool _success;
};

extern ESP32_MailClient MailClient;

#endif