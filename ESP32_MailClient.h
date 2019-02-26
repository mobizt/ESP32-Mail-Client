/*
 * ESP32 Mail Client library for Arduino, version 1.0.1
 * 
 * This library provides ESP32 to send and receive email with or without attchament. 
 * 
 * The library was test and work well with ESP32s based module and add support for multiple stream event path.
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

#define SMTP_STATUS_SERVER_CONNECT_FAILED (-1)
#define SMTP_STATUS_SMTP_RESPONSE_FAILED (-2)
#define SMTP_STATUS_IDENTIFICATION_FAILED (-3)
#define SMTP_STATUS_AUTHEN_NOT_SUPPORT (-4)
#define SMTP_STATUS_AUTHEN_FAILED (-5)
#define SMTP_STATUS_USER_LOGIN_FAILED (-6)
#define SMTP_STATUS_PASSWORD_LOGIN_FAILED (-7)
#define SMTP_STATUS_SEND_HEADER_FAILED (-8)
#define SMTP_STATUS_SEND_BODY_FAILED (-9)

#define IMAP_STATUS_SERVER_CONNECT_FAILED (-100)
#define IMAP_STATUS_IMAP_RESPONSE_FAILED (-101)
#define IMAP_STATUS_LOGIN_FAILED (-102)
#define IMAP_STATUS_BAD_COMMAND (-103)

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

static const char STR_1[] PROGMEM = "Content-Type: multipart/mixed; boundary=\"";
static const char STR_2[] PROGMEM = "{BOUNDARY}";
static const char STR_3[] PROGMEM = "Mime-Version: 1.0\r\n";
static const char STR_4[] PROGMEM = "AUTH LOGIN";
static const char STR_5[] PROGMEM = "HELO dude";
static const char STR_6[] PROGMEM = "EHLO dude";
static const char STR_7[] PROGMEM = "QUIT";
static const char STR_8[] PROGMEM = "MAIL FROM:";
static const char STR_9[] PROGMEM = "RCPT TO:";
static const char STR_10[] PROGMEM = "From: ";
static const char STR_11[] PROGMEM = "To: ";
static const char STR_12[] PROGMEM = "Cc: ";
static const char STR_13[] PROGMEM = ",<";
static const char STR_14[] PROGMEM = "<";
static const char STR_15[] PROGMEM = ">";
static const char STR_16[] PROGMEM = "DATA";
static const char STR_17[] PROGMEM = "X-Priority: ";
static const char STR_18[] PROGMEM = "X-MSMail-Priority: High\r\n";
static const char STR_19[] PROGMEM = "X-MSMail-Priority: Normal\r\n";
static const char STR_20[] PROGMEM = "X-MSMail-Priority: Low\r\n";
static const char STR_21[] PROGMEM = "Importance: High\r\n";
static const char STR_22[] PROGMEM = "Importance: Normal\r\n";
static const char STR_23[] PROGMEM = "Importance: Low\r\n";
static const char STR_24[] PROGMEM = "Subject: ";
static const char STR_25[] PROGMEM = "Content-Type: ";
static const char STR_26[] PROGMEM = "; Name=\"";
static const char STR_27[] PROGMEM = "Content-type: text/plain; charset=us-ascii\r\n";
static const char STR_28[] PROGMEM = "Content-type: text/html; charset=\"UTF-8\"\r\n";
static const char STR_29[] PROGMEM = "Content-transfer-encoding: 7bit\r\n";
static const char STR_30[] PROGMEM = "Content-Disposition: attachment; filename=\"";
static const char STR_31[] PROGMEM = "Content-transfer-encoding: base64\r\n";
static const char STR_32[] PROGMEM = "application/octet-stream";
static const char STR_33[] PROGMEM = "--";
static const char STR_34[] PROGMEM = "\r\n";
static const char STR_35[] PROGMEM = "\"\r\n\r\n";
static const char STR_36[] PROGMEM = "\"\r\n";
static const char STR_37[] PROGMEM = "\r\n.\r\n";
static const char STR_38[] PROGMEM = "could not connect to server";
static const char STR_39[] PROGMEM = "could not handle SMTP server response";
static const char STR_40[] PROGMEM = "could not handle IMAP server response";
static const char STR_41[] PROGMEM = "identification failed";
static const char STR_42[] PROGMEM = "authentication is not support";
static const char STR_43[] PROGMEM = "authentication failed";
static const char STR_44[] PROGMEM = "login account is not valid";
static const char STR_45[] PROGMEM = "could not sigin";
static const char STR_46[] PROGMEM = "could not parse command";
static const char STR_47[] PROGMEM = "login password is not valid";
static const char STR_48[] PROGMEM = "send header failed";
static const char STR_49[] PROGMEM = "send header failed";
static const char STR_50[] PROGMEM = "Connecting to IMAP server...";
static const char STR_51[] PROGMEM = "initialize";
static const char STR_52[] PROGMEM = "failed";
static const char STR_53[] PROGMEM = "Error, ";
static const char STR_54[] PROGMEM = "IMAP server connected, wait for response...";
static const char STR_55[] PROGMEM = "wait_for_response";
static const char STR_56[] PROGMEM = "Sign in...";
static const char STR_57[] PROGMEM = "signin";
static const char STR_58[] PROGMEM = "Lising folders...";
static const char STR_59[] PROGMEM = "listing";
static const char STR_60[] PROGMEM = ":::::::Message folders:::::::";
static const char STR_61[] PROGMEM = "Reading ";
static const char STR_62[] PROGMEM = "Predicted next UID: ";
static const char STR_63[] PROGMEM = "Total Message: ";
static const char STR_64[] PROGMEM = "::::::::::::Flags::::::::::::";
static const char STR_65[] PROGMEM = "::::::::::Messages:::::::::::";
static const char STR_66[] PROGMEM = "Searching messages...";
static const char STR_67[] PROGMEM = "searching";
static const char STR_68[] PROGMEM = "Search limit:";
static const char STR_69[] PROGMEM = "Found ";
static const char STR_70[] PROGMEM = " messages";
static const char STR_71[] PROGMEM = "Show ";
static const char STR_72[] PROGMEM = "Could not found any Email for defined criteria";
static const char STR_73[] PROGMEM = "Search criteria is not set, fetch the recent message";
static const char STR_74[] PROGMEM = "Feching message ";
static const char STR_75[] PROGMEM = ", UID: ";
static const char STR_76[] PROGMEM = ", Number: ";
static const char STR_77[] PROGMEM = "fetching";
static const char STR_78[] PROGMEM = "Attachment (";
static const char STR_79[] PROGMEM = ")";
static const char STR_80[] PROGMEM = "Downloading attachments...";
static const char STR_81[] PROGMEM = "downloading";
static const char STR_82[] PROGMEM = " bytes";
static const char STR_83[] PROGMEM = " - ";
static const char STR_84[] PROGMEM = "Free Heap: ";
static const char STR_85[] PROGMEM = "Sign out...";
static const char STR_86[] PROGMEM = "signout";
static const char STR_87[] PROGMEM = "Finished";
static const char STR_88[] PROGMEM = "finished";
static const char STR_89[] PROGMEM = "SD card mount failed";
static const char STR_90[] PROGMEM = "download ";
static const char STR_91[] PROGMEM = ", ";
static const char STR_92[] PROGMEM = "%";
static const char STR_93[] PROGMEM = "connection timeout";
static const char STR_94[] PROGMEM = "WiFi connection lost";
static const char STR_95[] PROGMEM = "no server response";
static const char STR_96[] PROGMEM = "finished";
static const char STR_97[] PROGMEM = " folder...";
static const char STR_98[] PROGMEM = "Finished";
static const char STR_99[] PROGMEM = "Date: ";
static const char STR_100[] PROGMEM = "Messsage UID: ";
static const char STR_101[] PROGMEM = "Messsage ID: ";
static const char STR_102[] PROGMEM = "Accept Language: ";
static const char STR_103[] PROGMEM = "Content Language: ";
static const char STR_104[] PROGMEM = "From: ";
static const char STR_105[] PROGMEM = "From Charset: ";
static const char STR_106[] PROGMEM = "To: ";
static const char STR_107[] PROGMEM = "To Charset: ";
static const char STR_108[] PROGMEM = "CC: ";
static const char STR_109[] PROGMEM = "CC Charset: ";
static const char STR_110[] PROGMEM = "Subject: ";
static const char STR_111[] PROGMEM = "Subject Charset: ";
static const char STR_112[] PROGMEM = "Message Charset: ";
static const char STR_113[] PROGMEM = "Attachment: ";
static const char STR_114[] PROGMEM = "File Index: ";
static const char STR_115[] PROGMEM = "Filename: ";
static const char STR_116[] PROGMEM = "Name: ";
static const char STR_117[] PROGMEM = "Size: ";
static const char STR_118[] PROGMEM = "Type: ";
static const char STR_119[] PROGMEM = "Creation Date: ";
static const char STR_120[] PROGMEM = "Connecting to SMTP server...";
static const char STR_121[] PROGMEM = "SMTP server connected, wait for response...";
static const char STR_122[] PROGMEM = "Identification...";
static const char STR_123[] PROGMEM = "Authentication...";
static const char STR_124[] PROGMEM = "Sign in...";
static const char STR_125[] PROGMEM = "Sending Email header...";
static const char STR_126[] PROGMEM = "Sending Email body...";
static const char STR_127[] PROGMEM = "Sending attacments...";
static const char STR_128[] PROGMEM = "Finalize...";
static const char STR_129[] PROGMEM = "Finished\r\nEmail sent successfully";
static const char STR_130[] PROGMEM = "$ LOGIN ";
static const char STR_131[] PROGMEM = " ";
static const char STR_132[] PROGMEM = " OK ";
static const char STR_133[] PROGMEM = "$ LIST \"\" \"*\"";
static const char STR_134[] PROGMEM = "Success";
static const char STR_135[] PROGMEM = "$ EXAMINE \"";
static const char STR_136[] PROGMEM = "\"";
static const char STR_137[] PROGMEM = "UID ";
static const char STR_138[] PROGMEM = " UID";
static const char STR_139[] PROGMEM = " SEARCH";
static const char STR_140[] PROGMEM = "UID";
static const char STR_141[] PROGMEM = "SEARCH";
static const char STR_142[] PROGMEM = "$ UID FETCH ";
static const char STR_143[] PROGMEM = "$ FETCH ";
static const char STR_144[] PROGMEM = " BODY.PEEK[HEADER.FIELDS (SUBJECT FROM TO DATE Message-ID Accept-Language Content-Language)]";
static const char STR_145[] PROGMEM = "$ OK Success";
static const char STR_146[] PROGMEM = "$ LOGOUT";
static const char STR_147[] PROGMEM = " BODY.PEEK[";
static const char STR_148[] PROGMEM = ".MIME]";
static const char STR_149[] PROGMEM = "multipart/";
static const char STR_150[] PROGMEM = "$ UID FETCH ";
static const char STR_151[] PROGMEM = " BODY.PEEK[";
static const char STR_152[] PROGMEM = ".";
static const char STR_153[] PROGMEM = "attachment";
static const char STR_154[] PROGMEM = "text/html";
static const char STR_155[] PROGMEM = "text/plain";
static const char STR_156[] PROGMEM = "]";
static const char STR_157[] PROGMEM = "* ESEARCH";
static const char STR_158[] PROGMEM = "$ NO ";
static const char STR_159[] PROGMEM = "$ BAD ";
static const char STR_160[] PROGMEM = "base64";
static const char STR_161[] PROGMEM = "/decoded_msg.txt";
static const char STR_162[] PROGMEM = "/raw_msg.txt";
static const char STR_163[] PROGMEM = "/decoded_msg.html";
static const char STR_164[] PROGMEM = "/raw_msg.html";
static const char STR_165[] PROGMEM = " FETCH ";
static const char STR_166[] PROGMEM = "* OK ";
static const char STR_167[] PROGMEM = "content-type: ";
static const char STR_168[] PROGMEM = "charset=\"";
static const char STR_169[] PROGMEM = "charset=";
static const char STR_170[] PROGMEM = "name=\"";
static const char STR_171[] PROGMEM = "name=";
static const char STR_172[] PROGMEM = "content-transfer-encoding: ";
static const char STR_173[] PROGMEM = "\r";
static const char STR_174[] PROGMEM = "content-description: ";
static const char STR_175[] PROGMEM = "content-disposition: ";
static const char STR_176[] PROGMEM = "filename=\"";
static const char STR_177[] PROGMEM = "filename=";
static const char STR_178[] PROGMEM = "size=";
static const char STR_179[] PROGMEM = "creation-date=\"";
static const char STR_180[] PROGMEM = "creation-date=";
static const char STR_181[] PROGMEM = "modification-date=\"";
static const char STR_182[] PROGMEM = "modification-date=";
static const char STR_183[] PROGMEM = "*";
static const char STR_184[] PROGMEM = "from: ";
static const char STR_185[] PROGMEM = "to: ";
static const char STR_186[] PROGMEM = "cc: ";
static const char STR_187[] PROGMEM = "subject: ";
static const char STR_188[] PROGMEM = "date: ";
static const char STR_189[] PROGMEM = "message-id: ";
static const char STR_190[] PROGMEM = "accept-language: ";
static const char STR_191[] PROGMEM = "content-language: ";
static const char STR_192[] PROGMEM = ")";
static const char STR_193[] PROGMEM = "{";
static const char STR_194[] PROGMEM = "}";
static const char STR_195[] PROGMEM = " LIST ";
static const char STR_196[] PROGMEM = "\\Noselect";
static const char STR_197[] PROGMEM = " FLAGS ";
static const char STR_198[] PROGMEM = "(";
static const char STR_199[] PROGMEM = " EXISTS";
static const char STR_200[] PROGMEM = " [UIDNEXT ";
static const char STR_201[] PROGMEM = "]";
static const char STR_202[] PROGMEM = "/";
static const char STR_203[] PROGMEM = "/header.txt";
static const char STR_204[] PROGMEM = "/esp.32";
static const char STR_205[] PROGMEM = "high";
static const char STR_206[] PROGMEM = "High";
static const char STR_207[] PROGMEM = "normal";
static const char STR_208[] PROGMEM = "Normal";
static const char STR_209[] PROGMEM = "low";
static const char STR_210[] PROGMEM = "Low";
static const char STR_211[] PROGMEM = "$ OK ";
static const char STR_212[] PROGMEM = "FLAGS";
static const char STR_213[] PROGMEM = "BODY";
static const char STR_214[] PROGMEM = "PEEK";
static const char STR_215[] PROGMEM = "TEXT";
static const char STR_216[] PROGMEM = "HEADER";
static const char STR_217[] PROGMEM = "FIELDS";
static const char STR_218[] PROGMEM = "[";
static const char STR_219[] PROGMEM = "]";
static const char STR_220[] PROGMEM = "MIME";

static bool compFunc(std::string i, std::string j)
{
  return (atoi(i.c_str()) > atoi(j.c_str()));
}

class ESP32_MailClient
{

public:
  /*
    *
    * Send Email
    * 
    *\param http - HTTPClientESP32Ex WiFi client.
    *\param smtpData - SMTPData contains all information of Email.
    *\return The sending status. True for success sending and False for failed sending.
    */
  bool sendMail(HTTPClientESP32Ex &http, SMTPData &smtpData);

  /*
    *
    * Read Email
    * 
    *\param http - HTTPClientESP32Ex WiFi client.
    *\param imapData - IMAPData that contains all read information.
    *\return The reading return status. True for finished reading and False for failed reading.
    */
  bool readMail(HTTPClientESP32Ex &http, IMAPData &imapData);

  /*
    *Return the Email sending error reason
    *
    *\return error String.
    */
  String smtpErrorReason();

  /*
    * Return the Email receive error reason
    * 
    * \return error String.
    */
  String imapErrorReason();

  struct IMAP_COMMAND_TYPE;
  struct IMAP_HEADER_TYPE;

protected:
  int _smtpStatus = 0;
  int _imapStatus = 0;
  bool _sdOk = false;
  std::string smtpErrorReasonStr();
  void set_message_header(string &header, std::string &message, bool htmlFormat);
  void set_attachment_header(uint8_t index, std::string &header, attachmentData &attach);
  bool validSubstringRange(int start, int length, std::string str);
  double base64DecodeSize(std::string lastBase64String, int length);
  unsigned char *base64_decode_char(const unsigned char *src, size_t len, size_t *out_len);
  std::string base64_encode_string(const unsigned char *src, size_t len);
  void send_base64_encode_data(WiFiClient *tcp, const unsigned char *src, size_t len);
  void send_base64_encode_file(WiFiClient *tcp, const char *filePath);
  int waitSMTPResponse(HTTPClientESP32Ex &http);
  bool waitIMAPResponse(HTTPClientESP32Ex &http, IMAPData &imapData, ReadStatus &cbData, uint8_t imapCommandType = 0, string endResp1 = "", string endResp2 = "", int maxChar = 0, int mailIndex = -1, int messageDataIndex = -1, std ::string part = "");
  inline std::string trim(std::string &str);
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
    *Set IMAP login credentials
    * 
    *\param host - IMAP server e.g. imap.gmail.com.
    *\param port - IMAP port e.g. 993 for gmail.
    *\param loginEmail - The account Email.
    *\param loginPassword - The account password.
    */
  void setLogin(const String &host, uint16_t port, const String &loginEmail, const String &loginPassword);

  /*
    *Set Folder or path of mailbox
    * 
    *\param folderName - known mailbox folder.
    *
    * The default folder is INBOX
    */
  void setFolder(const String folderName);

  /*
    *Set maximum message buffer size that the plain text or html message can be collected.
    * 
    *\param size - the message size in byte.
    */
  void setMessageBufferSize(size_t size);

  /*
    *Set maximum size of attachment file size that can be download.
    * 
    *\param size - the attachement file size in byte.
    */
  void setAttachmentSizeLimit(size_t size);

  /*
    *Set the search criteria to search in selected mailbox.
    * 
    *\param criteria - the search criteria String.
    *
    * If folder is not set, the INBOX folder will be selected
    * Example:
    * "SINCE 10-Feb-2019" will search all messages that received since 10 Feb 2019
    * "UID SEARCH ALL" will seach all message which will return the message UID that can be use later for fetch one or more messages.
    * 
    *  
    *  Search criteria can be consisted these keywords
    * 
    * ALL - All messages in the mailbox; the default initial key for	ANDing.
    * ANSWERED - Messages with the \Answered flag set.
    * BCC - Messages that contain the specified string in the envelope structure's BCC field.
    * BEFORE - Messages whose internal date (disregarding time and timezone) is earlier than the specified date.
    * BODY - Messages that contain the specified string in the body of the	message.
    * CC - Messages that contain the specified string in the envelope structure's CC field.
    * DELETED - Messages with the \Deleted flag set.
    * DRAFT - Messages with the \Draft flag set.
    * FLAGGED - Messages with the \Flagged flag set.
    * FROM - Messages that contain the specified string in the envelope	structure's FROM field.
    * HEADER - Messages that have a header with the specified field-name (as defined in [RFC-2822]) 
    * and that contains the specified string	in the text of the header (what comes after the colon).  
    * If the string to search is zero-length, this matches all messages that have a header line with 
    * the specified field-name regardless of	the contents.
    * KEYWORD - Messages with the specified keyword flag set.
    * LARGER - Messages with an [RFC-2822] size larger than the specified number of octets.
    * NEW -  Messages that have the \Recent flag set but not the \Seen flag.
    * This is functionally equivalent to "(RECENT UNSEEN)".
    * NOT - Messages that do not match the specified search key.
    * OLD - Messages that do not have the \Recent flag set.  This is	functionally equivalent to
    * "NOT RECENT" (as opposed to "NOT	NEW").
    * ON - Messages whose internal date (disregarding time and timezone) is within the specified date.
    * OR - Messages that match either search key.
    * RECENT - Messages that have the \Recent flag set.
    * SEEN - Messages that have the \Seen flag set.
    * SENTBEFORE - Messages whose [RFC-2822] Date: header (disregarding time and	timezone) is earlier than the specified date.
    * SENTON - Messages whose [RFC-2822] Date: header (disregarding time and timezone) is within the specified date.
    * SENTSINCE - Messages whose [RFC-2822] Date: header (disregarding time and timezone) is within or later than the specified date.
    * SINCE - Messages whose internal date (disregarding time and timezone) is within or later than the specified date.
    * SMALLER - Messages with an [RFC-2822] size smaller than the specified number of octets.
    * SUBJECT - Messages that contain the specified string in the envelope structure's SUBJECT field.
    * TEXT - Messages that contain the specified string in the header or body of the message.
    * TO - Messages that contain the specified string in the envelope structure's TO field.
    * UID - Messages with unique identifiers corresponding to the specified unique identifier set.  
    * Sequence set ranges are permitted.
    * UNANSWERED - Messages that do not have the \Answered flag set.
    * UNDELETED - Messages that do not have the \Deleted flag set.
    * UNDRAFT - Messages that do not have the \Draft flag set.
    * UNFLAGGED - Messages that do not have the \Flagged flag set.
    * UNKEYWORD - Messages that do not have the specified keyword flag set.
    * UNSEEN - Messages that do not have the \Seen flag set.
    */
  void setSearchCriteria(const String criteria);

  /*
    *Set save/download file path.
    * 
    *\param path - the folder or path in SD card.
    *
    * All saved text message files and attachemnts will be save inside and in message UID or message number folder
    */
  void setSaveFilePath(const String path);

  /*
    *Set UID of known message to fetch or read.
    * 
    *\param fetchUID - the message UID, only UID and should not contain any IMAP command and keyword.
    */
  void setFechUID(const String fetchUID);

  /*
    *Set to download attachment when fetch of receive Email.
    * 
    *\param download - True for download attachment.
    */
  void setDownloadAttachment(bool download);

  /*
    *Set to collect the html message.
    * 
    *\param htmlFormat - True to collect html message in IMAPData object result or save in SD card.
    * The default value is false.
    */
  void setHTMLMessage(bool htmlFormat);

  /*
    *Set to collect the plain text message.
    * 
    *\param textFormat - True to collect plain text message in IMAPData object result or save in SD card.
    * The default value is true.
    */
  void setTextMessage(bool textFormat);

  /*
    *Set to collect only message header no message text/html and attachment will read or download.
    * 
    *\param headerOnly - True to collect only header in IMAPData object result.
    * The default value is true.
    */
  void setHeaderOnly(bool headerOnly);

  /*
    *Set the serach result limit when search criteria is set.
    * 
    *\param limit - Any number from 0 to 65535.
    */
  void setSearchLimit(uint16_t limit);

  /*
    *Set the serach result sorted order when search criteria is set.
    * 
    *\param recentSort - True for most recent message first.
    */
  void setRecentSort(bool recentSort);

  /*
    *Set callback function to get the status while fetching or receiving message.
    * 
    *\param readCallback - The function that accept readStatusCallback as parameter.
    */
  void setReadCallback(readStatusCallback readCallback);

  /*
    *Set when attachement download progress is required while fetching or receiving message.
    * 
    *\param report - True for report the progress.
    * Callback function should be set by setReadCallback to accept the download status.
    */
  void setDownloadReport(bool report);

  /*
    *Set when only message header is required in the IMAPData result.
    * 
    *\This can be used when search criteria was set and only each message header was acquired in cluding message UID to be fetch later.
    */
  bool isHeaderOnly();

  /*
    *Get the sender name/Email for selected message index of IMAPData result.
    * 
    *\param messageIndex - The index of message.
    *\return sender name/Email String.
    */
  String getFrom(uint16_t messageIndex);

  /*
    *Get the sender name/Email charactor encoding.
    * 
    *\param messageIndex - The index of message.
    *\return sender name/Email charactor encoding which use in decoding to local language.
    */
  String getFromCharset(uint16_t messageIndex);

  /*
    *Get the recipient name/Email for selected message index of IMAPData result.
    * 
    *\param messageIndex - The index of message.
    *\return recipient name/Email String.
    */
  String getTo(uint16_t messageIndex);

  /*
    *Get the recipient name/Email charactor encoding.
    * 
    *\param messageIndex - The index of message.
    *\return recipient name/Email charactor encoding which use in decoding to local language.
    */
  String getToCharset(uint16_t messageIndex);

  /*
    *Get the CC name/Email for selected message index of IMAPData result.
    * 
    *\param messageIndex - The index of message.
    *\return CC name/Email String.
    */
  String getCC(uint16_t messageIndex);

  /*
    *Get the CC name/Email charactor encoding.
    * 
    *\param messageIndex - The index of message.
    *\return CC name/Email charactor encoding which use in decoding to local language.
    */
  String getCCCharset(uint16_t messageIndex);

  /*
    *Get the message subject for selected message index of IMAPData result.
    * 
    *\param messageIndex - The index of message.
    *\return message subject name/Email String.
    */
  String getSubject(uint16_t messageIndex);

  /*
    *Get the message subject charactor encoding.
    * 
    *\param messageIndex - The index of message.
    *\return message subject charactor encoding which use in decoding to local language.
    */
  String getSubjectCharset(uint16_t messageIndex);

  /*
    *Get the html message for selected message index of IMAPData result.
    * 
    *\param messageIndex - The index of message.
    *\return html message String or empty String upon the setHTMLMessage was set.
    */
  String getHTMLMessage(uint16_t messageIndex);

  /*
    *Get the plain text message for selected message index of IMAPData result.
    * 
    *\param messageIndex - The index of message.
    *\return plain text message String or empty String upon the setTextMessage was set.
    */
  String getTextMessage(uint16_t messageIndex);

  /*
    *Get the html message charactor encoding.
    * 
    *\param messageIndex - The index of message.
    *\return html message charactor encoding which use in decoding to local language.
    */
  String getHTMLMessgaeCharset(uint16_t messageIndex);

  /*
    *Get the text message charactor encoding.
    * 
    *\param messageIndex - The index of message.
    *\return text message charactor encoding which use in decoding to local language.
    */
  String getTextMessgaeCharset(uint16_t messageIndex);

  /*
    *Get the date of received message for selected message index of IMAPData result.
    * 
    *\param messageIndex - The index of message.
    *\return date String.
    */
  String getDate(uint16_t messageIndex);

  /*
    *Get the message UID for selected message index of IMAPData result.
    * 
    *\param messageIndex - The index of message.
    *\return UID String that can be use in setFechUID.
    */
  String getUID(uint16_t messageIndex);

  /*
    *Get the message number for selected message index of IMAPData result.
    * 
    *\param messageIndex - The index of message.
    *\return message number which vary upon search criteria and sorting.
    */
  String getNumber(uint16_t messageIndex);

  /*
    *Get the message ID for selected message index of IMAPData result.
    * 
    *\param messageIndex - The index of message.
    *\return message ID String.
    */
  String getMessageID(uint16_t messageIndex);

  /*
    *Get the accept language for selected message index of IMAPData result.
    * 
    *\param messageIndex - The index of message.
    *\return accept language String.
    */
  String getAcceptLanguage(uint16_t messageIndex);

  /*
    *Get the content language of text or html for selected message index of IMAPData result.
    * 
    *\param messageIndex - The index of message.
    *\return content language String.
    */
  String getContentLanguage(uint16_t messageIndex);

  /*
    *Use when checking the fetch error for selected message index of IMAPData result.
    * 
    *\param messageIndex - The index of message.
    *\return fetch status, True for fetch status for that selected message was failed.
    */
  bool isFetchMessageFailed(uint16_t messageIndex);

  /*
    *Return the fetch error reason for selected message index of IMAPData result.
    * 
    *\param messageIndex - The index of message.
    *\return fetch error reason String for selected message index.
    */
  String getFetchMessageFailedReason(uint16_t messageIndex);

  /*
    *Use when checking the attachment download error for selected message index of IMAPData result.
    * 
    *\param messageIndex - The index of message.
    *\return fetch status, True for error in attachment download for that selected message.
    */
  bool isDownloadAttachmentFailed(uint16_t messageIndex, size_t attachmentIndex);

  /*
    *Return the attachment download error reason for selected message index of IMAPData result.
    * 
    *\param messageIndex - The index of message.
    *\return download error reason String for selected message index.
    */
  String getDownloadAttachmentFailedReason(uint16_t messageIndex, size_t attachmentIndex);

  /*
    *Use when checking the downloaded/saved text message error for selected message index of IMAPData result.
    * 
    *\param messageIndex - The index of message.
    *\return text message download status, True for error in downloading message.
    * The text or html message download can be set by call IMAPData.saveHTMLMessage or IMAPData.saveTextMessage
    */
  bool isDownloadMessageFailed(uint16_t messageIndex);

  /*
    *Return the attachment downloaded/saved text message error reason for selected message index of IMAPData result.
    * 
    *\param messageIndex - The index of message.
    *\return downloaded/saved text message error reason String for selected message index.
    */
  String getDownloadMessageFailedReason(uint16_t messageIndex);

  /*
    *Use when html message was need to be download or save in SD card.
    * 
    *\param save - True for saving html message.
    *\param decoded - True for save in decode html format which support utf8 and base64 encoding.
    */
  void saveHTMLMessage(bool save, bool decoded);

  /*
    *Use when text message was need to be download or save in SD card.
    * 
    *\param save - True for saving text message.
    *\param decoded - True for save in decode text format which support utf8 and base64 encoding.
    */
  void saveTextMessage(bool save, bool decoded);

  /*
    *Return the folder count.
    */
  uint16_t getFolderCount();

  /*
    *Return the folder name or path for selected index.
    * 
    *\param folderIndex - Index of folder.
    *\return folder name or path String that can be use in setFolder call.
    */
  String getFolder(uint16_t folderIndex);

  /*
    *Return number of support flags for mailbox.
    */
  uint16_t getFlagCount();

  /*
    *Return the flag name for selected index.
    * 
    *\param folderIndex - Index of folder.
    *\return flag name that supported by mailbox and can use for search criteria.
    */
  String getFlag(uint16_t flagIndex);

  /*
    *Return number of message in selected mailbox e.g. INBOX.
    */
  size_t totalMessages();

  /*
    *Return number of message from search result.
    */
  size_t searchCount();

  /*
    *Return number of message from search result which not exceed the search limit value that set by setSearchLimit.
    */
  size_t availableMessages();

  /*
    *Return number of attachment for selected message index.
    *\param messageIndex - Index of message.
    *\return number of attachment
    */
  size_t getAttachmentCount(uint16_t messageIndex);

  /*
    *Return file name of attachment for selected attachment index and message index.
    *\param messageIndex - Index of message.
    *\param attachmentIndex - Index of attachment.
    *\return attachment file name String at the selected index
    */
  String getAttachmentFileName(size_t messageIndex, size_t attachmentIndex);

  /*
    *Return name of attachment for selected attachment index and message index.
    *\param messageIndex - Index of message.
    *\param attachmentIndex - Index of attachment.
    *\return attachment name String at the selected index
    */
  String getAttachmentName(size_t messageIndex, size_t attachmentIndex);

  /*
    *Return size in byte of attachment file for selected attachment index and message index.
    *\param messageIndex - Index of message.
    *\param attachmentIndex - Index of attachment.
    *\return attachment file size in byte at the selected index
    */
  int getAttachmentFileSize(size_t messageIndex, size_t attachmentIndex);

  /*
    *Return creation date of attachment for selected attachment index and message index.
    *\param messageIndex - Index of message.
    *\param attachmentIndex - Index of attachment.
    *\return attachment creation date String at the selected index
    */
  String getAttachmentCreationDate(size_t messageIndex, size_t attachmentIndex);

  /*
    *Return type or file MIME of attachment for selected attachment index and message index.
    *\param messageIndex - Index of message.
    *\param attachmentIndex - Index of attachment.
    *\return file MIME String at the selected index e.g. image/jpeg.
    */
  String getAttachmentType(size_t messageIndex, size_t attachmentIndex);

  /*
    *Use when to empty IMAPData object to clear content or no further use.
    */
  void empty();

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
    * Set login credentials to Email object
    * 
    *\param host - SMTP server e.g. smtp.gmail.com
    *\param port - SMTP port.
    *\param loginEmail - The account Email.
    *\param loginPassword - The account password.
    */
  void setLogin(const String &host, uint16_t port, const String &loginEmail, const String &loginPassword);

  /*
     *Set Sender info to Email object
     *
     *\param fromName - Sender's name
     *\param senderEmail - Sender's Email.
    */
  void setSender(const String &fromName, const String &senderEmail);

  /*
    * Get Sender's name from Email object
    * 
    *\return Sender's name String.
    */
  String getFromName();

  /*
    * Get Sender's Email from Email object
    * 
    * \return Sender's Email String.
    */
  String getSenderEmail();

  /*
    * Set Email priority or importance to Email object
    * \param priority - Number from 1 to 5, 1 for highest, 3 for normal and 5 for lowest priority
    */
  void setPriority(int priority);

  /*
    * Set Email priority or importance to Email object
    * 
    * \param priority - String (High, Normal or Low)
    */
  void setPriority(const String &priority);

  /*
    * Get Email priority from Email object
    * \return number represents Email priority (1 for highest, 3 for normal, 5 for low priority).
    */
  uint8_t getPriority();

  /*
    * Add one or more recipient to Email object
    * 
    * \param email - Recipient Email String of one recipient.
    * For add multiple recipients, call addRecipient for each recipient.
    */
  void addRecipient(const String &email);

  /*
    * Remove recipient from Email object
    * 
    * \param email - Recipient Email String.
    */
  void removeRecipient(const String &email);

  /*
    * Remove recipient from Email object
    * 
    * \param index - Index of recipients in Email object that previously added.
    */
  void removeRecipient(uint8_t index);

  /*
    * Clear all recipients in Email object.
    */
  void clearRecipient();

  /*
    * Get one recipient from Email object
    * 
    * \param index - Index of recipients in Email object to get.
    * \return Recipient Email String at the index.
    */
  String getRecipient(uint8_t index);

  /*
    * Get number of recipients in Email object
    * 
    * \return Number of recipients.
    */
  uint8_t recipientCount();

  /*
    * Set the Email subject to Email object
    * 
    * \param subject - The subject.
    */
  void setSubject(const String &subject);

  /*
    * Get the Email subject from Email object
    * 
    * \return Subject String.
    */
  String getSubject();

  /*
    * Set the Email message to Email object
    * 
    * \param message - The message can be in normal text or html format.
    * \param htmlFormat - The html format flag, True for send the message as html format
    */
  void setMessage(const String &message, bool htmlFormat);

  /*
    * Get the message from Email object
    * 
    * \return Message String.
    */
  String getMessage();

  /*
    * Check the message in Email object is being send in html format
    * 
    * \return True for being send message in html format.
    */
  bool htmlFormat();

  /*
    * Add Carbon Copy (CC) Email to Email object
    * 
    * \param email - The CC Email String.
    */
  void addCC(const String &email);

  /*
    * Remove specified Carbon Copy (CC) Email from Email object
    * 
    * \param email - The CC Email String to remove.
    */
  void removeCC(const String &email);

  /*
    * Remove specified Carbon Copy (CC) Email from Email object
    * 
    * \param index - The CC Email index to remove.
    */
  void removeCC(uint8_t index);

  /*
    * Clear all Carbon Copy (CC) Emails from Email object
    */
  void clearCC();

  /*
    * Get Carbon Copy (CC) Email from Email object at specified index
    * 
    * \param index - The CC Email index to get.
    * \return The CC Email string at the index.
    */
  String getCC(uint8_t index);

  /*
    * Get the number of Carbon Copy (CC) Email in Email object
    * 
    * \return Number of CC Emails.
    */
  uint8_t ccCount();

  /*
    * Add Blind Carbon Copy (BCC) Email to Email object
    * 
    * \param email - The BCC Email String.
    */
  void addBCC(const String &email);

  /*
    * Remove specified Blind Carbon Copy (BCC) Email from Email object
    * 
    * \param email - The BCC Email String to remove.
    */
  void removeBCC(const String &email);

  /*
    * Remove specified Blind Carbon Copy (BCC) Email from Email object
    * 
    * \param index - The BCC Email index to remove.
    */
  void removeBCC(uint8_t index);

  /*
    * Clear all Blind Carbon Copy (BCC) Emails from Email object
    */
  void clearBCC();

  /*
    * Get Blind Carbon Copy (BCC) Email from Email object at specified index
    * 
    * \param index - The BCC Email index to get.
    * \return The BCC Email string at the index.
    */
  String getBCC(uint8_t index);

  /*
    * Get the number of Blind Carbon Copy (BCC) Email in Email object
    * 
    * \return Number of BCC Emails.
    */
  uint8_t bccCount();

  /*
    * Add attchement data (binary) from internal memory (flash or ram) to Email object
    * 
    * \param fileName - The file name String that recipient can be saved.
    * \param mimeType - The MIME type of file (image/jpeg, image/png, text/plain...). Can be empty String.
    * \param data - The byte array of data (uint8_t)
    * \param size - The data length in byte.
    */
  void addAttachData(const String &fileName, const String &mimeType, uint8_t *data, uint16_t size);

  /*
    * Remove specified attachment data from Email object
    * 
    * \param fileName - The file name of the attachment data to remove.
    */
  void removeAttachData(const String &fileName);

  /*
    * Remove specified attachment data from Email object
    * 
    * \param index - The index of the attachment data (count only data type attachment) in Email object to remove.
    */
  void removeAttachData(uint8_t index);

  /*
    * Get the number of attachment data in Email object
    * 
    * \return Number of attach data.
    */
  uint8_t attachDataCount();

  /*
    * Add attchement file from SD card to Email object
    * 
    * \param fileName - The file name String that recipient can be saved.
    * \param mimeType - The MIME type of file (image/jpeg, image/png, text/plain...). Can be omitted.
    */
  void addAttachFile(const String &filePath, const String &mimeType = "");

  /*
    * Remove specified attachment file from Email object
    * 
    * \param fileName - The file name of the attachment file to remove.
    */
  void removeAttachFile(const String &filePath);

  /*
    * Remove specified attachment file from Email object
    * 
    * \param index - The index of the attachment file (count only file type attachment) in Email object to remove.
    */
  void removeAttachFile(uint8_t index);

  /*
    * Clear all attachment data from Email object.
    */
  void clearAttachData();

  /*
    * Clear all attachment file from Email object.
    */
  void clearAttachFile();

  /*
    * Clear all attachments (both data and file type attachments) from Email object.
    */
  void clearAttachment();

  /*
    * Get number of attachments (both data and file type attachments) in Email object.
    * 
    * /return Number of all attachemnts in Email object.
    */
  uint8_t attachFileCount();

  /*
    * Clear all data from Email object to free memory.
    */
  void empty();

  /*
    * Set the Email sending status callback function to Email object.
    * 
    * \param sendCallback - The callback function that accept the sendStatusCallback param.
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
