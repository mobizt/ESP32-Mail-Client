/*
 *Mail Client Arduino Library for ESP32, version 1.0.3
 * 
 * March 21, 2019
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

#include "ESP32_MailClient.h"

struct ESP32_MailClient::IMAP_COMMAND_TYPE
{
  static const uint8_t LOGIN = 0;
  static const uint8_t LIST = 1;
  static const uint8_t SELECT = 2;
  static const uint8_t EXAMINE = 3;
  static const uint8_t STATUS = 4;
  static const uint8_t SEARCH = 5;
  static const uint8_t FETCH_BODY_HEADER = 6;
  static const uint8_t FETCH_BODY_MIME = 7;
  static const uint8_t FETCH_BODY_TEXT = 8;
  static const uint8_t FETCH_BODY_ATTACHMENT = 9;
  static const uint8_t LOGOUT = 10;
};

struct ESP32_MailClient::IMAP_HEADER_TYPE
{
  static const uint8_t FROM = 1;
  static const uint8_t TO = 2;
  static const uint8_t CC = 3;
  static const uint8_t SUBJECT = 4;
  static const uint8_t DATE = 5;
  static const uint8_t MSG_ID = 6;
  static const uint8_t CONT_LANG = 7;
  static const uint8_t ACCEPT_LANG = 8;
};

bool ESP32_MailClient::readMail(HTTPClientESP32Ex &http, IMAPData &imapData)
{

  std::string buf;
  std::string command = "$";

  size_t mailIndex = 0;
  int messageDataIndex = 0;
  int partID = 1;
  int _partID = 1;
  bool res = false;
  bool _res = false;

  int bufSize = 50;

  char *_val = new char[bufSize];
  char *_part = new char[bufSize];

  WiFiClient *tcp;

  ReadStatus cbData;

 if (WiFi.status() != WL_CONNECTED)
    WiFi.reconnect();

  //Try to reconnect WiFi if lost connection
  if (WiFi.status() != WL_CONNECTED)
  { 
    uint8_t tryCount = 0;
    WiFi.reconnect();
    while (WiFi.status() != WL_CONNECTED)
    {
      tryCount++;
      delay(50);
      if (tryCount > 60)
        break;
    }
  }

  //If WiFi is not connected, return false
  if (WiFi.status() != WL_CONNECTED)
  {
    _imapStatus = MAIL_CLIENT_STATUS_WIFI_CONNECT_FAIL;

    if (imapData._readCallback)
    {
      
      cbData._info = ESP32_MAIL_STR_53 + imapErrorReasonStr();
      cbData._status = ESP32_MAIL_STR_52;
      cbData._success = false;
      imapData._readCallback(cbData);
    }
    goto out;
  }

  if (imapData._readCallback)
  {

    cbData._info = ESP32_MAIL_STR_50;
    cbData._status = ESP32_MAIL_STR_51;
    cbData._success = false;
    imapData._readCallback(cbData);
  }

  http.http_begin(imapData._host.c_str(), imapData._port, ESP32_MAIL_STR_202, (const char *)NULL);

  if (!http.http_connect())
  {
    _imapStatus = IMAP_STATUS_SERVER_CONNECT_FAILED;

    if (imapData._readCallback)
    {

      cbData._info = ESP32_MAIL_STR_53 + imapErrorReasonStr();
      cbData._status = ESP32_MAIL_STR_52;
      cbData._success = false;
      imapData._readCallback(cbData);
    }
    goto out;
  }

  tcp = http.http_getStreamPtr();

  if (imapData._readCallback)
  {
    cbData._info = ESP32_MAIL_STR_54;
    cbData._status = ESP32_MAIL_STR_55;
    cbData._success = false;
    imapData._readCallback(cbData);
  }

  if (!waitIMAPResponse(http, imapData, cbData, 0))
  {
    _imapStatus = IMAP_STATUS_IMAP_RESPONSE_FAILED;

    if (imapData._readCallback)
    {
      cbData._info = ESP32_MAIL_STR_53 + imapErrorReasonStr();
      cbData._status = ESP32_MAIL_STR_52;
      cbData._success = false;
      imapData._readCallback(cbData);
    }
    goto out;
  }

  if (imapData._readCallback)
  {
    cbData._info = ESP32_MAIL_STR_56;
    cbData._status = ESP32_MAIL_STR_57;
    cbData._success = false;
    imapData._readCallback(cbData);
  }

  tcp->print(ESP32_MAIL_STR_130);
  tcp->print(imapData._loginEmail.c_str());
  tcp->print(ESP32_MAIL_STR_131);
  tcp->println(imapData._loginPassword.c_str());

  if (!waitIMAPResponse(http, imapData, cbData, IMAP_COMMAND_TYPE::LOGIN, ESP32_MAIL_STR_132))
  {
    _imapStatus = IMAP_STATUS_LOGIN_FAILED;
    if (imapData._readCallback)
    {
      cbData._info = ESP32_MAIL_STR_53 + imapErrorReasonStr();
      cbData._status = ESP32_MAIL_STR_52;
      cbData._success = false;
      imapData._readCallback(cbData);
    }
    goto out;
  }

  if (imapData._fetchUID == "")
  {

    if (imapData._readCallback)
    {
      cbData._info = ESP32_MAIL_STR_58;
      cbData._status = ESP32_MAIL_STR_59;
      cbData._success = false;
      imapData._readCallback(cbData);
    }

    tcp->println(ESP32_MAIL_STR_133);

    if (!waitIMAPResponse(http, imapData, cbData, IMAP_COMMAND_TYPE::LIST, ESP32_MAIL_STR_132, ESP32_MAIL_STR_134))
    {
      _imapStatus = IMAP_STATUS_BAD_COMMAND;
      if (imapData._readCallback)
      {
        cbData._info = ESP32_MAIL_STR_53 + imapErrorReasonStr();
        cbData._status = ESP32_MAIL_STR_52;
        cbData._success = false;
        imapData._readCallback(cbData);
      }
      cbData.empty();
    }

    if (imapData._readCallback)
    {

      cbData._info = ESP32_MAIL_STR_60;
      cbData._success = false;
      imapData._readCallback(cbData);

      for (size_t i = 0; i < imapData._folders.size(); i++)
      {
        cbData._info = imapData._folders[i];
        cbData._status = "";
        cbData._success = false;
        imapData._readCallback(cbData);
      }

      cbData._info = ESP32_MAIL_STR_61 + imapData._currentFolder + ESP32_MAIL_STR_97;
      cbData._status = "";
      cbData._success = false;
      imapData._readCallback(cbData);
    }
  }

  tcp->print(ESP32_MAIL_STR_135);
  tcp->print(imapData._currentFolder.c_str());
  tcp->println(ESP32_MAIL_STR_136);

  if (!waitIMAPResponse(http, imapData, cbData, IMAP_COMMAND_TYPE::EXAMINE, ESP32_MAIL_STR_132, ESP32_MAIL_STR_134))
  {

    _imapStatus = IMAP_STATUS_BAD_COMMAND;
    if (imapData._readCallback)
    {
      cbData._info = ESP32_MAIL_STR_53 + imapErrorReasonStr();
      cbData._status = ESP32_MAIL_STR_52;
      cbData._success = false;
      imapData._readCallback(cbData);
    }
    goto out;
  }

  if (imapData._fetchUID == "")
  {

    if (imapData._readCallback)
    {

      cbData._info = ESP32_MAIL_STR_62 + imapData._nextUID;
      cbData._status = "";
      cbData._success = false;
      imapData._readCallback(cbData);

      cbData._info = ESP32_MAIL_STR_63;
      memset(_val, 0, bufSize);
      itoa(imapData._totalMessage, _val, 10);
      cbData._info += _val;
      cbData._status = "";
      cbData._success = false;
      imapData._readCallback(cbData);

      cbData._info = ESP32_MAIL_STR_64;
      cbData._status = "";
      cbData._success = false;
      imapData._readCallback(cbData);

      for (size_t i = 0; i < imapData._flag.size(); i++)
      {
        cbData._info = imapData._flag[i];
        cbData._status = "";
        cbData._success = false;
        imapData._readCallback(cbData);
      }

      cbData._info = ESP32_MAIL_STR_65;
      cbData._status = "";
      cbData._success = false;
      imapData._readCallback(cbData);

      cbData._info = ESP32_MAIL_STR_66;
      cbData._status = ESP32_MAIL_STR_67;
      cbData._success = false;
      imapData._readCallback(cbData);
    }
  }

  imapData._msgNum.clear();
  imapData._uidSearch = false;
  imapData._msgID.clear();
  imapData._contentLanguage.clear();
  imapData._acceptLanguage.clear();

  if (imapData._fetchUID == "")
  {

    if (imapData._searchCriteria != "")
    {

      if (imapData._searchCriteria.find(ESP32_MAIL_STR_137) != std::string::npos)
      {
        imapData._uidSearch = true;
        command += ESP32_MAIL_STR_138;
      }
      command += ESP32_MAIL_STR_139;

      for (size_t i = 0; i < imapData._searchCriteria.length(); i++)
      {
        if (imapData._searchCriteria[i] != ' ' && imapData._searchCriteria[i] != '\r' && imapData._searchCriteria[i] != '\n' && imapData._searchCriteria[i] != '$')
          buf.append(1, imapData._searchCriteria[i]);

        if (imapData._searchCriteria[i] == ' ')
        {
          if (imapData._uidSearch && buf == ESP32_MAIL_STR_140)
            buf.clear();

          if (buf != ESP32_MAIL_STR_141 && buf != "")
          {
            command += ESP32_MAIL_STR_131;
            command += buf;
          }

          buf.clear();
        }
      }

      if (buf.length() > 0)
      {
        command += ESP32_MAIL_STR_131;
        command += buf;
      }

      tcp->println(command.c_str());

      std::string().swap(command);

      if (!waitIMAPResponse(http, imapData, cbData, IMAP_COMMAND_TYPE::SEARCH, ESP32_MAIL_STR_132, ESP32_MAIL_STR_134, 1))
      {
        _imapStatus = IMAP_STATUS_BAD_COMMAND;
        if (imapData._readCallback)
        {
          cbData._info = ESP32_MAIL_STR_53 + imapErrorReasonStr();
          cbData._status = ESP32_MAIL_STR_52;
          cbData._success = false;
          imapData._readCallback(cbData);
        }
        goto out;
      }

      if (imapData._readCallback)
      {

        cbData._info = ESP32_MAIL_STR_68;
        memset(_val, 0, bufSize);
        itoa(imapData._emailNumMax, _val, 10);
        cbData._info += _val;
        cbData._status = "";
        cbData._success = false;
        imapData._readCallback(cbData);

        if (imapData._msgNum.size() > 0)
        {

          cbData._info = ESP32_MAIL_STR_69;
          memset(_val, 0, bufSize);
          itoa(imapData._searchCount, _val, 10);
          cbData._info += _val;
          cbData._info += ESP32_MAIL_STR_70;
          cbData._status = "";
          cbData._success = false;
          imapData._readCallback(cbData);

          cbData._info = ESP32_MAIL_STR_71;
          memset(_val, 0, bufSize);
          itoa(imapData._msgNum.size(), _val, 10);
          cbData._info += _val;
          cbData._info += ESP32_MAIL_STR_70;
          cbData._status = "";
          cbData._success = false;
          imapData._readCallback(cbData);
        }
        else
        {
          cbData._info = ESP32_MAIL_STR_72;
          cbData._status = "";
          cbData._success = false;
          imapData._readCallback(cbData);
        }
      }
    }
    else
    {
      memset(_val, 0, bufSize);
      itoa(imapData._totalMessage, _val, 10);

      imapData._msgNum.push_back(_val);
      if (imapData._readCallback)
      {
        cbData._info = ESP32_MAIL_STR_73;
        cbData._status = "";
        cbData._success = false;
        imapData._readCallback(cbData);
      }
    }
  }
  else
  {

    imapData._msgNum.push_back(imapData._fetchUID);
  }

  for (int i = 0; i < imapData._msgNum.size(); i++)
  {

    if (imapData._readCallback)
    {

      cbData._info = ESP32_MAIL_STR_74;
      memset(_val, 0, bufSize);
      itoa(i + 1, _val, 10);
      cbData._info += _val;

      cbData._status = "";
      if (imapData._uidSearch || imapData._fetchUID != "")
        cbData._info += ESP32_MAIL_STR_75;
      else
        cbData._info += ESP32_MAIL_STR_76;

      cbData._info += imapData._msgNum[i];
      cbData._status = ESP32_MAIL_STR_77;
      cbData._success = false;
      imapData._readCallback(cbData);
    }

    imapData._date.push_back(std::string());
    imapData._subject.push_back(std::string());
    imapData._subject_charset.push_back(std::string());
    imapData._from.push_back(std::string());
    imapData._from_charset.push_back(std::string());
    imapData._to.push_back(std::string());
    imapData._to_charset.push_back(std::string());
    imapData._cc.push_back(std::string());
    imapData._attachmentCount.push_back(0);
    imapData._totalAttachFileSize.push_back(0);
    imapData._downloadedByte.push_back(0);
    imapData._messageDataCount.push_back(0);
    imapData._error.push_back(false);
    imapData._errorMsg.push_back(std::string());
    imapData._cc_charset.push_back(std::string());
    imapData._msgID.push_back(std::string());
    imapData._acceptLanguage.push_back(std::string());
    imapData._contentLanguage.push_back(std::string());

    std::vector<messageBodyData> d = std::vector<messageBodyData>();

    imapData._messageDataInfo.push_back(d);

    if (imapData._uidSearch || imapData._fetchUID != "")
      tcp->print(ESP32_MAIL_STR_142);
    else
      tcp->print(ESP32_MAIL_STR_143);

    tcp->print(imapData._msgNum[i].c_str());
    tcp->println(ESP32_MAIL_STR_144);

    if (!waitIMAPResponse(http, imapData, cbData, IMAP_COMMAND_TYPE::FETCH_BODY_HEADER, ESP32_MAIL_STR_145, "", 0, mailIndex))
    {
      if (imapData._fetchUID == "")
        _imapStatus = IMAP_STATUS_IMAP_RESPONSE_FAILED;
      else
        _imapStatus = IMAP_STATUS_BAD_COMMAND;

      if (imapData._readCallback)
      {
        cbData._info = ESP32_MAIL_STR_53 + imapErrorReasonStr();
        cbData._status = ESP32_MAIL_STR_52;
        cbData._success = false;
        imapData._readCallback(cbData);
      }
      goto out;
    }

    if (!imapData._headerOnly)
    {

      messageDataIndex = 0;
      partID = 1;
      _partID = 1;
      res = false;
      _res = false;

      do
      {

        if (imapData._uidSearch || imapData._fetchUID != "")
          tcp->print(ESP32_MAIL_STR_142);
        else
          tcp->print(ESP32_MAIL_STR_143);

        tcp->print(imapData._msgNum[i].c_str());
        tcp->print(ESP32_MAIL_STR_147);
        tcp->print(partID);
        tcp->println(ESP32_MAIL_STR_148);

        memset(_part, 0, bufSize);
        memset(_val, 0, bufSize);
        itoa(partID, _val, 10);
        strcpy(_part, _val);

        res = waitIMAPResponse(http, imapData, cbData, IMAP_COMMAND_TYPE::FETCH_BODY_MIME, "", "", 0, mailIndex, messageDataIndex, _part);
        if (res)
        {
          if (imapData._messageDataCount[mailIndex] < messageDataIndex + 1)
          {
            imapData._messageDataInfo[mailIndex].push_back(messageBodyData());
            imapData._messageDataCount[mailIndex]++;
          }

          if (imapData._messageDataInfo[mailIndex][messageDataIndex]._contentType == "")
            continue;

          if (imapData._messageDataInfo[mailIndex][messageDataIndex]._contentType.find(ESP32_MAIL_STR_149) != std::string::npos)
          {
            do
            {

              if (imapData._uidSearch || imapData._fetchUID != "")
                tcp->print(ESP32_MAIL_STR_142);
              else
                tcp->print(ESP32_MAIL_STR_143);

              tcp->print(imapData._msgNum[i].c_str());
              tcp->print(ESP32_MAIL_STR_147);
              tcp->print(partID);
              tcp->print(".");
              tcp->print(_partID);
              tcp->println(ESP32_MAIL_STR_148);

              memset(_part, 0, bufSize);
              memset(_val, 0, bufSize);
              itoa(partID, _val, 10);
              strcpy(_part, _val);
              strcat(_part, ".");
              memset(_val, 0, bufSize);
              itoa(_partID, _val, 10);
              strcat(_part, _val);

              _res = waitIMAPResponse(http, imapData, cbData, IMAP_COMMAND_TYPE::FETCH_BODY_MIME, "", "", 0, mailIndex, messageDataIndex, _part);

              if (_res)
              {
                messageDataIndex++;
                _partID++;
              }

            } while (_res);
          }
          else
          {
            messageDataIndex++;
          }
          partID++;
        }

      } while (res);

      if (imapData._saveHTMLMsg || imapData._saveTextMsg || imapData._downloadAttachment)
      {

        if (!_sdOk)
        {
          _sdOk = sdTest();
          delay(200);
        }

        if (_sdOk)
          if (!SD.exists(imapData._savePath.c_str()))
            createDirs(imapData._savePath);
      }

      if (imapData._messageDataInfo[mailIndex].size() > 0)
      {
        if (imapData._attachmentCount[mailIndex] > 0 && imapData._readCallback)
        {
          cbData._info = ESP32_MAIL_STR_78;
          memset(_val, 0, bufSize);
          itoa(imapData._attachmentCount[mailIndex], _val, 10);
          cbData._info += _val;
          cbData._info += ESP32_MAIL_STR_79;
          cbData._status = "";
          cbData._success = false;
          imapData._readCallback(cbData);

          for (int j = 0; j < imapData._messageDataInfo[mailIndex].size(); j++)
          {
            if (imapData._messageDataInfo[mailIndex][j]._disposition == ESP32_MAIL_STR_153)
            {

              cbData._info = imapData._messageDataInfo[mailIndex][j]._filename;
              cbData._info += ESP32_MAIL_STR_83;
              memset(_val, 0, bufSize);
              itoa(imapData._messageDataInfo[mailIndex][j]._size, _val, 10);
              cbData._info += _val;
              cbData._info += ESP32_MAIL_STR_82;
              cbData._status = "";
              cbData._success = false;
              imapData._readCallback(cbData);
            }
          }

          if (imapData._downloadAttachment && _sdOk)
          {
            cbData._info = ESP32_MAIL_STR_80;
            cbData._status = ESP32_MAIL_STR_81;
            cbData._success = false;
            imapData._readCallback(cbData);
          }
        }

        for (int j = 0; j < imapData._messageDataInfo[mailIndex].size(); j++)
        {

          if (imapData._messageDataInfo[mailIndex][j]._disposition == "")
          {

            if (!imapData._textFormat && imapData._messageDataInfo[mailIndex][j]._contentType != ESP32_MAIL_STR_154)
              continue;

            if (!imapData._htmlFormat && imapData._messageDataInfo[mailIndex][j]._contentType != ESP32_MAIL_STR_155)
              continue;

            if (imapData._uidSearch || imapData._fetchUID != "")
              tcp->print(ESP32_MAIL_STR_142);
            else
              tcp->print(ESP32_MAIL_STR_143);

            tcp->print(imapData._msgNum[i].c_str());
            tcp->print(ESP32_MAIL_STR_147);
            tcp->print(imapData._messageDataInfo[mailIndex][j]._part.c_str());
            tcp->println(ESP32_MAIL_STR_156);

            if (!waitIMAPResponse(http, imapData, cbData, IMAP_COMMAND_TYPE::FETCH_BODY_TEXT, ESP32_MAIL_STR_145, "", imapData._message_buffer_size, mailIndex, j))
            {
              _imapStatus = IMAP_STATUS_IMAP_RESPONSE_FAILED;
              if (imapData._readCallback)
              {
                cbData._info = ESP32_MAIL_STR_53 + imapErrorReasonStr();
                cbData._status = ESP32_MAIL_STR_52;
                cbData._success = false;
                imapData._readCallback(cbData);
              }
            }
          }
          else if (imapData._messageDataInfo[mailIndex][j]._disposition == ESP32_MAIL_STR_153 && _sdOk)
          {

            if (imapData._downloadAttachment)
            {
              if (imapData._messageDataInfo[mailIndex][j]._size <= imapData._attacement_max_size)
              {

                if (_sdOk)
                {

                  if (j < imapData._messageDataInfo[mailIndex].size() - 1)
                    if (imapData._messageDataInfo[mailIndex][j + 1]._size > imapData._attacement_max_size)
                      imapData._downloadedByte[mailIndex] += imapData._messageDataInfo[mailIndex][j + 1]._size;

                  if (imapData._uidSearch || imapData._fetchUID != "")
                    tcp->print(ESP32_MAIL_STR_142);
                  else
                    tcp->print(ESP32_MAIL_STR_143);

                  tcp->print(imapData._msgNum[i].c_str());
                  tcp->print(ESP32_MAIL_STR_147);
                  tcp->print(imapData._messageDataInfo[mailIndex][j]._part.c_str());
                  tcp->println(ESP32_MAIL_STR_156);

                  if (!waitIMAPResponse(http, imapData, cbData, IMAP_COMMAND_TYPE::FETCH_BODY_ATTACHMENT, ESP32_MAIL_STR_145, "", imapData._message_buffer_size, mailIndex, j))
                  {
                    _imapStatus = IMAP_STATUS_IMAP_RESPONSE_FAILED;
                    if (imapData._readCallback)
                    {
                      cbData._info = ESP32_MAIL_STR_53 + imapErrorReasonStr();
                      cbData._status = ESP32_MAIL_STR_52;
                      cbData._success = false;
                      imapData._readCallback(cbData);
                    }
                  }

                  delay(1);
                }
              }
              else
              {
                if (j = imapData._messageDataInfo[mailIndex].size() - 1)
                  imapData._downloadedByte[mailIndex] += imapData._messageDataInfo[mailIndex][j]._size;
              }
            }
          }
        }
      }

      if (_sdOk)
        SD.end();

      _sdOk = false;
    }

    if (imapData._readCallback)
    {

      cbData._info = ESP32_MAIL_STR_84;
      memset(_val, 0, bufSize);
      itoa(ESP.getFreeHeap(), _val, 10);
      cbData._info += _val;
      cbData._status = "";
      cbData._success = false;
      imapData._readCallback(cbData);
    }

    mailIndex++;
  }

  if (imapData._readCallback)
  {

    cbData._info = ESP32_MAIL_STR_85;
    cbData._status = ESP32_MAIL_STR_86;
    cbData._success = false;
    imapData._readCallback(cbData);
  }

  tcp->println(ESP32_MAIL_STR_146);
  if (!waitIMAPResponse(http, imapData, cbData, 0))
  {
    _imapStatus = IMAP_STATUS_BAD_COMMAND;
    if (imapData._readCallback)
    {
      cbData._info = ESP32_MAIL_STR_53 + imapErrorReasonStr();
      cbData._status = ESP32_MAIL_STR_52;
      cbData._success = false;
      imapData._readCallback(cbData);
    }
    goto out;
  }

  if (imapData._readCallback)
  {

    cbData._info = ESP32_MAIL_STR_98;
    cbData._status = ESP32_MAIL_STR_96;
    cbData._success = true;
    imapData._readCallback(cbData);
  }

  if (http.http_connected())
  {
    tcp->stop();
    tcp->~WiFiClient();
  }

  cbData.empty();
  delete[] _val;
  delete[] _part;
  std::string().swap(command);
  std::string().swap(buf);

  return true;

out:

  if (http.http_connected())
  {
    tcp->stop();
    tcp->~WiFiClient();
  }

  cbData.empty();
  delete[] _val;
  delete[] _part;
  std::string().swap(command);
  std::string().swap(buf);

  return false;
}

void ESP32_MailClient::createDirs(std::string dirs)
{
  std::string dir = "";
  int count = 0;
  for (int i = 0; i < dirs.length(); i++)
  {
    dir.append(1, dirs[i]);
    count++;
    if (dirs[i] == '/')
    {
      if (dir.length() > 0)
        SD.mkdir(dir.substr(0, dir.length() - 1).c_str());
      count = 0;
    }
  }
  if (count > 0)
    SD.mkdir(dir.c_str());
  std::string().swap(dir);
}

bool ESP32_MailClient::sdTest()
{

  if (!SD.begin())
    return false;

  File file = SD.open(ESP32_MAIL_STR_204, FILE_WRITE);
  if (!file)
    return false;

  if (!file.write(32))
    return false;
  file.close();

  file = SD.open(ESP32_MAIL_STR_204);
  if (!file)
    return false;

  while (file.available())
  {
    if (file.read() != 32)
      return false;
  }
  file.close();

  SD.remove(ESP32_MAIL_STR_204);

  return true;
}

bool ESP32_MailClient::sendMail(HTTPClientESP32Ex &http, SMTPData &smtpData)
{
  _smtpStatus = 0;
  std::string buf;
  std::string buf2;
  int bufSize = 50;
  char *_val = new char[bufSize];
  SendStatus cbData;
  cbData._info = ESP32_MAIL_STR_120;
  cbData._success = false;

  WiFiClient *tcp = http.http_getStreamPtr();

 

  if (smtpData._sendCallback)
    smtpData._sendCallback(cbData);


  if (WiFi.status() != WL_CONNECTED)
    WiFi.reconnect();

  //Try to reconnect WiFi if lost connection
  if (WiFi.status() != WL_CONNECTED)
  {
    uint8_t tryCount = 0;
    WiFi.reconnect();
    while (WiFi.status() != WL_CONNECTED)
    {
      tryCount++;
      delay(50);
      if (tryCount > 60)
        break;
    }
  }

  //If WiFi is not connected, return false
  if (WiFi.status() != WL_CONNECTED)
  {
    _smtpStatus = MAIL_CLIENT_STATUS_WIFI_CONNECT_FAIL;

    if (smtpData._sendCallback)
    {

      cbData._info = ESP32_MAIL_STR_53 + smtpErrorReasonStr();
      cbData._success = false;
      smtpData._sendCallback(cbData);
    }
    goto failed;
  }

  http.http_begin(smtpData._host.c_str(), smtpData._port, ESP32_MAIL_STR_202, (const char *)NULL);

  if (!http.http_connect())
  {
    _smtpStatus = SMTP_STATUS_SERVER_CONNECT_FAILED;
    if (smtpData._sendCallback)
    {
      cbData._info = ESP32_MAIL_STR_53 + smtpErrorReasonStr();
      cbData._success = false;
      smtpData._sendCallback(cbData);
    }
    goto failed;
  }

  if (smtpData._sendCallback)
  {
    cbData._info = ESP32_MAIL_STR_121;
    cbData._success = false;
    smtpData._sendCallback(cbData);
  }

  if (waitSMTPResponse(http) != 220)
  {
    _smtpStatus = SMTP_STATUS_SMTP_RESPONSE_FAILED;
    if (smtpData._sendCallback)
    {
      cbData._info = ESP32_MAIL_STR_53 + smtpErrorReasonStr();
      cbData._success = false;
      smtpData._sendCallback(cbData);
    }
    goto failed;
  }

  if (smtpData._sendCallback)
  {
    cbData._info = ESP32_MAIL_STR_122;
    cbData._success = false;
    smtpData._sendCallback(cbData);
  }

  tcp->println(ESP32_MAIL_STR_5);

  if (waitSMTPResponse(http) != 250)
  {
    _smtpStatus = SMTP_STATUS_IDENTIFICATION_FAILED;
    if (smtpData._sendCallback)
    {
      cbData._info = ESP32_MAIL_STR_53 + smtpErrorReasonStr();
      cbData._success = false;
      smtpData._sendCallback(cbData);
    }
    goto failed;
  }

  if (smtpData._sendCallback)
  {
    cbData._info = ESP32_MAIL_STR_122;
    cbData._success = false;
    smtpData._sendCallback(cbData);
  }

  tcp->println(ESP32_MAIL_STR_6);

  if (waitSMTPResponse(http) != 250)
  {
    _smtpStatus = SMTP_STATUS_AUTHEN_NOT_SUPPORT;
    if (smtpData._sendCallback)
    {
      cbData._info = ESP32_MAIL_STR_53 + smtpErrorReasonStr();
      cbData._success = false;
      smtpData._sendCallback(cbData);
    }
    goto failed;
  }

  if (smtpData._sendCallback)
  {
    cbData._info = ESP32_MAIL_STR_123;
    cbData._success = false;
    smtpData._sendCallback(cbData);
  }

  tcp->println(ESP32_MAIL_STR_4);

  if (waitSMTPResponse(http) != 334)
  {
    _smtpStatus = SMTP_STATUS_AUTHEN_FAILED;
    if (smtpData._sendCallback)
    {
      cbData._info = ESP32_MAIL_STR_53 + smtpErrorReasonStr();
      cbData._success = false;
      smtpData._sendCallback(cbData);
    }
    goto failed;
  }

  if (smtpData._sendCallback)
  {
    cbData._info = ESP32_MAIL_STR_124;
    cbData._success = false;
    smtpData._sendCallback(cbData);
  }

  tcp->println(base64_encode_string((const unsigned char *)smtpData._loginEmail.c_str(), smtpData._loginEmail.length()).c_str());

  if (waitSMTPResponse(http) != 334)
  {
    _smtpStatus = SMTP_STATUS_USER_LOGIN_FAILED;
    if (smtpData._sendCallback)
    {
      cbData._info = ESP32_MAIL_STR_53 + smtpErrorReasonStr();
      cbData._success = false;
      smtpData._sendCallback(cbData);
    }
    goto failed;
  }

  tcp->println(base64_encode_string((const unsigned char *)smtpData._loginPassword.c_str(), smtpData._loginPassword.length()).c_str());

  if (waitSMTPResponse(http) != 235)
  {
    _smtpStatus = SMTP_STATUS_PASSWORD_LOGIN_FAILED;
    if (smtpData._sendCallback)
    {
      cbData._info = ESP32_MAIL_STR_53 + smtpErrorReasonStr();
      cbData._success = false;
      smtpData._sendCallback(cbData);
    }
    goto failed;
  }

  if (smtpData._sendCallback)
  {
    cbData._info = ESP32_MAIL_STR_125;
    cbData._success = false;
    smtpData._sendCallback(cbData);
  }

  if (smtpData._priority > 0 && smtpData._priority <= 5)
  {
    memset(_val, 0, bufSize);
    itoa(smtpData._priority, _val, 10);

    buf2 += ESP32_MAIL_STR_17;
    buf2 += _val;
    buf2 += ESP32_MAIL_STR_34;

    if (smtpData._priority == 1)
    {
      buf2 += ESP32_MAIL_STR_18;
      buf2 += ESP32_MAIL_STR_21;
    }
    else if (smtpData._priority == 3)
    {
      buf2 += ESP32_MAIL_STR_19;
      buf2 += ESP32_MAIL_STR_22;
    }
    else if (smtpData._priority == 5)
    {
      buf2 += ESP32_MAIL_STR_20;
      buf2 += ESP32_MAIL_STR_23;
    }
  }

  buf2 += ESP32_MAIL_STR_10;

  if (smtpData._fromName.length() > 0)
    buf2 += smtpData._fromName;

  buf2 += ESP32_MAIL_STR_14;
  buf2 += smtpData._senderEmail;
  buf2 += ESP32_MAIL_STR_15;
  buf2 += ESP32_MAIL_STR_34;

  buf += ESP32_MAIL_STR_8;
  buf += ESP32_MAIL_STR_14;
  buf += smtpData._senderEmail;
  buf += ESP32_MAIL_STR_15;
  tcp->println(buf.c_str());

  if (waitSMTPResponse(http) != 250)
  {
    _smtpStatus = SMTP_STATUS_SEND_HEADER_FAILED;
    if (smtpData._sendCallback)
    {
      cbData._info = ESP32_MAIL_STR_53 + smtpErrorReasonStr();
      cbData._success = false;
      smtpData._sendCallback(cbData);
    }
    goto failed;
  }

  for (uint8_t i = 0; i < smtpData._recipient.size(); i++)
  {

    if (i == 0)
    {
      buf2 += ESP32_MAIL_STR_11;
      buf2 += ESP32_MAIL_STR_14;
      buf2 += smtpData._recipient[i];
      buf2 += ESP32_MAIL_STR_15;
    }
    else
    {
      buf2 += ESP32_MAIL_STR_13;
      buf2 += smtpData._recipient[i];
      buf2 += ESP32_MAIL_STR_15;
    }

    if (i == smtpData._recipient.size() - 1)
      buf2 += ESP32_MAIL_STR_34;

    buf.clear();

    buf += ESP32_MAIL_STR_9;
    buf += ESP32_MAIL_STR_14;
    buf += smtpData._recipient[i];
    buf += ESP32_MAIL_STR_15;

    tcp->println(buf.c_str());

    if (waitSMTPResponse(http) != 250)
    {
      _smtpStatus = SMTP_STATUS_SEND_HEADER_FAILED;
      if (smtpData._sendCallback)
      {
        cbData._info = ESP32_MAIL_STR_53 + smtpErrorReasonStr();
        cbData._success = false;
        smtpData._sendCallback(cbData);
      }
      goto failed;
    }
  }

  for (uint8_t i = 0; i < smtpData._cc.size(); i++)
  {

    if (i == 0)
    {
      buf2 += ESP32_MAIL_STR_12;
      buf2 += ESP32_MAIL_STR_14;
      buf2 += smtpData._cc[i];
      buf2 += ESP32_MAIL_STR_15;
    }
    else
    {
      buf2 += ESP32_MAIL_STR_13;
      buf2 += smtpData._cc[i];
      buf2 += ESP32_MAIL_STR_15;
    }

    if (i == smtpData.ccCount() - 1)
      buf2 += ESP32_MAIL_STR_34;

    buf.clear();

    buf += ESP32_MAIL_STR_9;
    buf += ESP32_MAIL_STR_14;
    buf += smtpData._cc[i];
    buf += ESP32_MAIL_STR_15;
    tcp->println(buf.c_str());

    if (waitSMTPResponse(http) != 250)
    {
      _smtpStatus = SMTP_STATUS_SEND_HEADER_FAILED;
      if (smtpData._sendCallback)
      {
        cbData._info = ESP32_MAIL_STR_53 + smtpErrorReasonStr();
        cbData._success = false;
        smtpData._sendCallback(cbData);
      }
      goto failed;
    }
  }

  for (uint8_t i = 0; i < smtpData._bcc.size(); i++)
  {

    buf.clear();
    buf += ESP32_MAIL_STR_9;
    buf += ESP32_MAIL_STR_14;
    buf += smtpData._bcc[i];
    buf += ESP32_MAIL_STR_15;
    tcp->println(buf.c_str());

    if (waitSMTPResponse(http) != 250)
    {
      _smtpStatus = SMTP_STATUS_SEND_HEADER_FAILED;
      if (smtpData._sendCallback)
      {
        cbData._info = ESP32_MAIL_STR_53 + smtpErrorReasonStr();
        cbData._success = false;
        smtpData._sendCallback(cbData);
      }
      goto failed;
    }
  }

  if (smtpData._sendCallback)
  {
    cbData._info = ESP32_MAIL_STR_126;
    cbData._success = false;
    smtpData._sendCallback(cbData);
  }

  tcp->println(ESP32_MAIL_STR_16);

  if (waitSMTPResponse(http) != 354)
  {
    _smtpStatus = SMTP_STATUS_SEND_BODY_FAILED;
    if (smtpData._sendCallback)
    {
      cbData._info = ESP32_MAIL_STR_53 + smtpErrorReasonStr();
      cbData._success = false;
      smtpData._sendCallback(cbData);
    }
    goto failed;
  }

  tcp->print(buf2.c_str());

  tcp->print(ESP32_MAIL_STR_24);
  tcp->println(smtpData._subject.c_str());
  tcp->print(ESP32_MAIL_STR_3);
  tcp->print(ESP32_MAIL_STR_1);
  tcp->print(ESP32_MAIL_STR_2);
  tcp->print(ESP32_MAIL_STR_35);

  buf.clear();

  set_message_header(buf, smtpData._message, smtpData._htmlFormat);

  tcp->print(buf.c_str());

  if (smtpData._attach._index > 0)
  {
    cbData._info = ESP32_MAIL_STR_127;
    cbData._success = false;
    smtpData._sendCallback(cbData);
  }

  for (uint8_t i = 0; i < smtpData._attach._index; i++)
  {
    if (smtpData._attach._type[i] == 0)
    {

      cbData._info = smtpData._attach._filename[i];
      cbData._success = false;
      smtpData._sendCallback(cbData);

      buf.clear();
      set_attachment_header(i, buf, smtpData._attach);
      tcp->print(buf.c_str());
      send_base64_encode_data(tcp, smtpData._attach._buf[i].front(), smtpData._attach._size[i]);
      tcp->print(ESP32_MAIL_STR_34);
    }
    else
    {
      if (!_sdOk)
        _sdOk = sdTest();

      if (!_sdOk)
        continue;

      if (SD.exists(smtpData._attach._filename[i].c_str()))
      {
        cbData._info = smtpData._attach._filename[i];
        cbData._success = false;
        smtpData._sendCallback(cbData);

        buf.clear();
        set_attachment_header(i, buf, smtpData._attach);
        tcp->print(buf.c_str());
        send_base64_encode_file(tcp, smtpData._attach._filename[i].c_str());
        tcp->print(ESP32_MAIL_STR_34);
      }
    }
  }

  tcp->print(ESP32_MAIL_STR_33);
  tcp->print(ESP32_MAIL_STR_2);
  tcp->print(ESP32_MAIL_STR_33);
  tcp->print(ESP32_MAIL_STR_37);

  if (smtpData._sendCallback)
  {
    cbData._info = ESP32_MAIL_STR_128;
    cbData._success = false;
    smtpData._sendCallback(cbData);
  }

  if (waitSMTPResponse(http) != 250)
  {
    _smtpStatus = SMTP_STATUS_SEND_BODY_FAILED;
    if (smtpData._sendCallback)
    {
      cbData._info = ESP32_MAIL_STR_53 + smtpErrorReasonStr();
      cbData._success = false;
      smtpData._sendCallback(cbData);
    }
    goto failed;
  }

  if (smtpData._sendCallback)
  {
    cbData._info = ESP32_MAIL_STR_129;
    cbData._success = true;
    smtpData._sendCallback(cbData);
  }

  if (http.http_connected())
  {
    tcp->stop();
    tcp->~WiFiClient();
  }

  cbData.empty();
  std::string().swap(buf);
  std::string().swap(buf2);
  delete[] _val;

  return true;

failed:

  if (http.http_connected())
  {
    tcp->stop();
    tcp->~WiFiClient();
  }

  cbData.empty();
  std::string().swap(buf);
  std::string().swap(buf2);
  delete[] _val;
  return false;
}

String ESP32_MailClient::smtpErrorReason()
{
  std::string res = "";
  switch (_smtpStatus)
  {
  case SMTP_STATUS_SERVER_CONNECT_FAILED:
    res = ESP32_MAIL_STR_38;
  case SMTP_STATUS_SMTP_RESPONSE_FAILED:
    res = ESP32_MAIL_STR_39;
  case SMTP_STATUS_IDENTIFICATION_FAILED:
    res = ESP32_MAIL_STR_41;
  case SMTP_STATUS_AUTHEN_NOT_SUPPORT:
    res = ESP32_MAIL_STR_42;
  case SMTP_STATUS_AUTHEN_FAILED:
    res = ESP32_MAIL_STR_43;
  case SMTP_STATUS_USER_LOGIN_FAILED:
    res = ESP32_MAIL_STR_44;
  case SMTP_STATUS_PASSWORD_LOGIN_FAILED:
    res = ESP32_MAIL_STR_47;
  case SMTP_STATUS_SEND_HEADER_FAILED:
    res = ESP32_MAIL_STR_48;
  case SMTP_STATUS_SEND_BODY_FAILED:
    res = ESP32_MAIL_STR_49;
  default:
    res = "";
  }
  return res.c_str();
}

std::string ESP32_MailClient::smtpErrorReasonStr()
{
  std::string res = "";
  switch (_smtpStatus)
  {
  case SMTP_STATUS_SERVER_CONNECT_FAILED:
    res = ESP32_MAIL_STR_38;
  case SMTP_STATUS_SMTP_RESPONSE_FAILED:
    res = ESP32_MAIL_STR_39;
  case SMTP_STATUS_IDENTIFICATION_FAILED:
    res = ESP32_MAIL_STR_41;
  case SMTP_STATUS_AUTHEN_NOT_SUPPORT:
    res = ESP32_MAIL_STR_42;
  case SMTP_STATUS_AUTHEN_FAILED:
    res = ESP32_MAIL_STR_43;
  case SMTP_STATUS_USER_LOGIN_FAILED:
    res = ESP32_MAIL_STR_44;
  case SMTP_STATUS_PASSWORD_LOGIN_FAILED:
    res = ESP32_MAIL_STR_47;
  case SMTP_STATUS_SEND_HEADER_FAILED:
    res = ESP32_MAIL_STR_48;
  case SMTP_STATUS_SEND_BODY_FAILED:
    res = ESP32_MAIL_STR_49;
  case MAIL_CLIENT_STATUS_WIFI_CONNECT_FAIL:
    res = ESP32_MAIL_STR_221;
  default:
    res = "";
  }
  return res;
}

String ESP32_MailClient::imapErrorReason()
{
  std::string res = "";
  switch (_imapStatus)
  {
  case IMAP_STATUS_SERVER_CONNECT_FAILED:
    res = ESP32_MAIL_STR_38;
  case IMAP_STATUS_IMAP_RESPONSE_FAILED:
    res = ESP32_MAIL_STR_40;
  case IMAP_STATUS_LOGIN_FAILED:
    res = ESP32_MAIL_STR_45;
  case IMAP_STATUS_BAD_COMMAND:
    res = ESP32_MAIL_STR_46;
  case MAIL_CLIENT_STATUS_WIFI_CONNECT_FAIL:
    res = ESP32_MAIL_STR_221;
  default:
    res = "";
  }
  return res.c_str();
}


std::string ESP32_MailClient::imapErrorReasonStr()
{
  std::string res = "";
  switch (_imapStatus)
  {
  case IMAP_STATUS_SERVER_CONNECT_FAILED:
    res = ESP32_MAIL_STR_38;
  case IMAP_STATUS_IMAP_RESPONSE_FAILED:
    res = ESP32_MAIL_STR_40;
  case IMAP_STATUS_LOGIN_FAILED:
    res = ESP32_MAIL_STR_45;
  case IMAP_STATUS_BAD_COMMAND:
    res = ESP32_MAIL_STR_46;
  case MAIL_CLIENT_STATUS_WIFI_CONNECT_FAIL:
    res = ESP32_MAIL_STR_221;
  default:
    res = "";
  }
  return res;
}

void ESP32_MailClient::set_message_header(string &header, string &message, bool htmlFormat)
{
  header += ESP32_MAIL_STR_33;
  header += ESP32_MAIL_STR_2;
  header += ESP32_MAIL_STR_34;
  if (!htmlFormat)
    header += ESP32_MAIL_STR_27;
  else
    header += ESP32_MAIL_STR_28;

  header += ESP32_MAIL_STR_29;
  header += ESP32_MAIL_STR_34;

  header += message;
  header += ESP32_MAIL_STR_34;
  header += ESP32_MAIL_STR_34;
}

void ESP32_MailClient::set_attachment_header(uint8_t index, std::string &header, attachmentData &attach)
{

  header += ESP32_MAIL_STR_33;
  header += ESP32_MAIL_STR_2;
  header += ESP32_MAIL_STR_34;

  header += ESP32_MAIL_STR_25;

  if (attach._mime_type[index].length() == 0)
    header += ESP32_MAIL_STR_32;
  else
    header += attach._mime_type[index];

  header += ESP32_MAIL_STR_26;

  std::string filename(attach._filename[index]);

  size_t found = filename.find_last_of("/\\");

  if (found != std::string::npos)
  {
    filename.clear();
    filename += attach._filename[index].substr(found + 1);
  }

  header += filename;
  header += ESP32_MAIL_STR_36;

  header += ESP32_MAIL_STR_30;
  header += filename;
  header += ESP32_MAIL_STR_36;

  header += ESP32_MAIL_STR_31;
  header += ESP32_MAIL_STR_34;

  std::string().swap(filename);
}

int ESP32_MailClient::waitSMTPResponse(HTTPClientESP32Ex &http)
{

  long dataTime = millis();

  char c = 0;
  std::string lineBuf = "";
  int lfCount = 0;
  size_t p1 = 0;
  bool completeResp = false;

  int resCode = -1000;

  WiFiClient *tcp = http.http_getStreamPtr();

  while (tcp->connected() && !tcp->available() && millis() - dataTime < http.tcpTimeout)
    delay(1);

  dataTime = millis();
  if (tcp->connected() && tcp->available())
  {
    while (tcp->available())
    {
      c = tcp->read();
      lineBuf.append(1, c);

      if (lineBuf.find(ESP32_MAIL_STR_158) != std::string::npos || lineBuf.find(ESP32_MAIL_STR_159) != std::string::npos)
        completeResp = true;

      if (c == '\n')
      {
        dataTime = millis();
        if (lfCount == 0)
        {
          p1 = lineBuf.find(" ");
          if (p1 != std::string::npos)
            resCode = atoi(lineBuf.substr(0, p1).c_str());
        }
        lineBuf.clear();
        lfCount++;
      }

      if (millis() - dataTime > http.tcpTimeout || completeResp)
        break;
    }
  }

  std::string().swap(lineBuf);
  return resCode;
}

bool ESP32_MailClient::waitIMAPResponse(HTTPClientESP32Ex &http, IMAPData &imapData, ReadStatus &cbData, uint8_t imapCommandType, string endResp1, string endResp2, int maxChar, int mailIndex, int messageDataIndex, std::string part)
{

  long dataTime = millis();

  char c = 0;
  std::string lineBuf = "";
  std::string msgNumBuf = "";
  std::string filepath = "";
  std::string hpath = "";
  int bufSize = 100;
  char *dest = new char[bufSize];
  char *buf = new char[bufSize];

  int readCount = 0;
  int lfCount = 0;
  int charCount = 0;
  size_t p1 = 0;
  size_t p2 = 0;
  size_t payloadLength = 0;
  size_t outputLength;

  bool completeResp = false;
  bool validResponse = false;
  bool downloadReq = false;
  size_t currentDownloadByte = 0;

  uint8_t headerType = 0;

  File file;
  int reportState = 0;

  if (imapCommandType == IMAP_COMMAND_TYPE::LIST)
    std::vector<std::string>().swap(imapData._folders);

  if (endResp1.length() == 0 && endResp2.length() == 0)
    completeResp = true;

  WiFiClient *tcp = http.http_getStreamPtr();

  while (tcp->connected() && !tcp->available() && millis() - dataTime < http.tcpTimeout)
    delay(1);

  dataTime = millis();
  if (tcp->connected() && tcp->available())
  {
    while (tcp->available() || !completeResp)
    {
      yield();

      c = tcp->read();

      lineBuf.append(1, c);

      if (endResp1.length() > 0 && endResp2.length() > 0)
      {
        if (lineBuf.find(endResp1) != std::string::npos && lineBuf.find(endResp2) != std::string::npos)
          completeResp = true;
      }
      else if (endResp1.length() > 0 && endResp2.length() == 0)
      {
        if (lineBuf.find(endResp1) != std::string::npos)
          completeResp = true;
      }

      if (lineBuf.find(ESP32_MAIL_STR_158) != std::string::npos || lineBuf.find(ESP32_MAIL_STR_159) != std::string::npos)
      {
        if (imapData._error.size() > 0 && mailIndex > -1)
        {

          if (imapCommandType == IMAP_COMMAND_TYPE::FETCH_BODY_TEXT || imapCommandType == IMAP_COMMAND_TYPE::FETCH_BODY_ATTACHMENT)
          {
            imapData._error[mailIndex] = true;
            imapData._errorMsg[mailIndex].clear();
            imapData._errorMsg[mailIndex] = lineBuf;
          }
        }

        validResponse = false;
        completeResp = true;
        //Let loop til end of stream
        continue;
      }

      if (validResponse && imapCommandType == IMAP_COMMAND_TYPE::FETCH_BODY_TEXT && lfCount > 0)
      {

        if (payloadLength > 0 && charCount < payloadLength - 1)
        {

          if (imapData._messageDataInfo[mailIndex][messageDataIndex]._transfer_encoding != ESP32_MAIL_STR_160)
          {
            if (charCount < maxChar)
              imapData._messageDataInfo[mailIndex][messageDataIndex]._text.append(1, c);

            if (imapData._saveHTMLMsg || imapData._saveTextMsg)
            {

              if (!imapData._messageDataInfo[mailIndex][messageDataIndex]._sdFileOpenWrite)
              {
                imapData._messageDataInfo[mailIndex][messageDataIndex]._sdFileOpenWrite = true;

                if (_sdOk)
                {
                  downloadReq = true;

                  filepath.clear();

                  filepath = imapData._savePath;
                  filepath += ESP32_MAIL_STR_202;
                  filepath += imapData._msgNum[mailIndex];

                  if (!SD.exists(filepath.c_str()))
                    createDirs(filepath);

                  if (!imapData._headerSaved)
                    hpath = filepath + ESP32_MAIL_STR_203;

                  if (imapData._messageDataInfo[mailIndex][messageDataIndex]._contentType == ESP32_MAIL_STR_155)
                  {
                    if (imapData._saveDecodedText)
                      filepath += ESP32_MAIL_STR_161;
                    else
                      filepath += ESP32_MAIL_STR_162;
                  }
                  else if (imapData._messageDataInfo[mailIndex][messageDataIndex]._contentType == ESP32_MAIL_STR_154)
                  {
                    if (imapData._saveDecodedHTML)
                      filepath += ESP32_MAIL_STR_163;
                    else
                      filepath += ESP32_MAIL_STR_164;
                  }

                  file = SD.open(filepath.c_str(), FILE_WRITE);
                }
                else
                {

                  if (imapData._messageDataCount[mailIndex] == messageDataIndex + 1)
                  {
                    imapData._messageDataInfo[mailIndex][messageDataIndex]._error = true;
                    imapData._messageDataInfo[mailIndex][messageDataIndex]._downloadError.clear();
                    imapData._messageDataInfo[mailIndex][messageDataIndex]._downloadError = ESP32_MAIL_STR_89;
                  }
                }
              }
              if (_sdOk)
                file.write(c);
            }
          }

          charCount++;
        }
      }

      if (c == '\n')
      {
        dataTime = millis();
        trim(lineBuf);
        string tmp = lineBuf;
        std::transform(tmp.begin(), tmp.end(), tmp.begin(), ::tolower);
        if (lfCount == 0)
        {
          if (imapCommandType == IMAP_COMMAND_TYPE::FETCH_BODY_TEXT ||
              imapCommandType == IMAP_COMMAND_TYPE::FETCH_BODY_MIME ||
              imapCommandType == IMAP_COMMAND_TYPE::FETCH_BODY_HEADER ||
              imapCommandType == IMAP_COMMAND_TYPE::FETCH_BODY_ATTACHMENT)
          {

            p1 = lineBuf.find(ESP32_MAIL_STR_165);
            if (p1 != std::string::npos)
              validResponse = true;
          }

          p1 = lineBuf.find(ESP32_MAIL_STR_166);
          if (p1 != std::string::npos)
            validResponse = true;
        }

        p1 = lineBuf.find(ESP32_MAIL_STR_211);
        if (p1 != std::string::npos)
          validResponse = true;

        if (imapCommandType == IMAP_COMMAND_TYPE::FETCH_BODY_MIME && lfCount > 0)
        {
          if (payloadLength > 0 && validResponse)
          {

            if (imapData._messageDataCount[mailIndex] < messageDataIndex + 1)
            {
              imapData._messageDataInfo[mailIndex].push_back(messageBodyData());
              imapData._messageDataCount[mailIndex]++;
            }

            p1 = tmp.find(ESP32_MAIL_STR_167);
            if (p1 != std::string::npos)
            {

              p2 = lineBuf.find(";", p1 + strlen(ESP32_MAIL_STR_167));
              if (p2 != std::string::npos)
              {

                if (validSubstringRange(p1 + strlen(ESP32_MAIL_STR_167), p2 - p1 - strlen(ESP32_MAIL_STR_167), lineBuf))
                  imapData._messageDataInfo[mailIndex][messageDataIndex]._contentType = lineBuf.substr(p1 + strlen(ESP32_MAIL_STR_167), p2 - p1 - strlen(ESP32_MAIL_STR_167));

                p1 = tmp.find(ESP32_MAIL_STR_168, p2);
                if (p1 != std::string::npos)
                {
                  p2 = lineBuf.find(ESP32_MAIL_STR_136, p1 + strlen(ESP32_MAIL_STR_168));
                  if (p2 != std::string::npos)
                  {

                    if (validSubstringRange(p1 + strlen(ESP32_MAIL_STR_168), p2 - p1 - strlen(ESP32_MAIL_STR_168), lineBuf))
                      imapData._messageDataInfo[mailIndex][messageDataIndex]._charset = lineBuf.substr(p1 + strlen(ESP32_MAIL_STR_168), p2 - p1 - strlen(ESP32_MAIL_STR_168));
                  }
                }
                else if (tmp.find(ESP32_MAIL_STR_169, p2) != std::string::npos)
                {
                  p1 = tmp.find(ESP32_MAIL_STR_169, p2);

                  imapData._messageDataInfo[mailIndex][messageDataIndex]._charset = lineBuf.substr(p1 + strlen(ESP32_MAIL_STR_169));
                }

                p1 = tmp.find(ESP32_MAIL_STR_170, p2);
                if (p1 != std::string::npos)
                {
                  p2 = lineBuf.find(ESP32_MAIL_STR_136, p1 + strlen(ESP32_MAIL_STR_170));
                  if (p2 != std::string::npos)
                  {

                    if (validSubstringRange(p1 + strlen(ESP32_MAIL_STR_170), p2 - p1 - strlen(ESP32_MAIL_STR_170), lineBuf))
                      imapData._messageDataInfo[mailIndex][messageDataIndex]._name = lineBuf.substr(p1 + strlen(ESP32_MAIL_STR_170), p2 - p1 - strlen(ESP32_MAIL_STR_170));
                  }
                }
                else if (tmp.find(ESP32_MAIL_STR_171, p2) != std::string::npos)
                {
                  p1 = tmp.find(ESP32_MAIL_STR_171, p2);
                  imapData._messageDataInfo[mailIndex][messageDataIndex]._name = lineBuf.substr(p1 + strlen(ESP32_MAIL_STR_171));
                }
              }
            }

            p1 = tmp.find(ESP32_MAIL_STR_172);
            if (p1 != std::string::npos)
            {
              p2 = lineBuf.find(ESP32_MAIL_STR_173, p1 + strlen(ESP32_MAIL_STR_172));

              if (p2 != std::string::npos)
              {
                if (validSubstringRange(p1 + strlen(ESP32_MAIL_STR_172), p2 - p1 - strlen(ESP32_MAIL_STR_172), lineBuf))
                  imapData._messageDataInfo[mailIndex][messageDataIndex]._transfer_encoding = lineBuf.substr(p1 + strlen(ESP32_MAIL_STR_172), p2 - p1 - strlen(ESP32_MAIL_STR_172));
              }
              else
              {
                imapData._messageDataInfo[mailIndex][messageDataIndex]._transfer_encoding = lineBuf.substr(p1 + strlen(ESP32_MAIL_STR_172));
              }
            }

            p1 = tmp.find(ESP32_MAIL_STR_174);
            if (p1 != std::string::npos)
            {
              p2 = lineBuf.find(ESP32_MAIL_STR_173, p1 + strlen(ESP32_MAIL_STR_174));

              if (p2 != std::string::npos)
              {
                if (validSubstringRange(p1 + strlen(ESP32_MAIL_STR_174), p2 - p1 - strlen(ESP32_MAIL_STR_174), lineBuf))
                  imapData._messageDataInfo[mailIndex][messageDataIndex]._descr = lineBuf.substr(p1 + strlen(ESP32_MAIL_STR_174), p2 - p1 - strlen(ESP32_MAIL_STR_174));
              }
              else
                imapData._messageDataInfo[mailIndex][messageDataIndex]._descr = lineBuf.substr(p1 + strlen(ESP32_MAIL_STR_174));
            }

            p1 = tmp.find(ESP32_MAIL_STR_175);
            if (p1 != std::string::npos)
            {
              p2 = lineBuf.find(";", p1 + strlen(ESP32_MAIL_STR_175));

              if (p2 != std::string::npos)
              {
                if (validSubstringRange(p1 + strlen(ESP32_MAIL_STR_175), p2 - p1 - strlen(ESP32_MAIL_STR_175), lineBuf))
                  imapData._messageDataInfo[mailIndex][messageDataIndex]._disposition = lineBuf.substr(p1 + strlen(ESP32_MAIL_STR_175), p2 - p1 - strlen(ESP32_MAIL_STR_175));
              }
              else
                imapData._messageDataInfo[mailIndex][messageDataIndex]._disposition = lineBuf.substr(p1 + strlen(ESP32_MAIL_STR_175));

              if (imapData._messageDataInfo[mailIndex][messageDataIndex]._disposition == ESP32_MAIL_STR_153)
                imapData._attachmentCount[mailIndex]++;
            }

            if (imapData._messageDataInfo[mailIndex][messageDataIndex]._disposition != "")
            {

              p1 = tmp.find(ESP32_MAIL_STR_176);
              if (p1 != std::string::npos)
              {
                p2 = lineBuf.find(ESP32_MAIL_STR_136, p1 + strlen(ESP32_MAIL_STR_176));

                if (p2 != std::string::npos)
                {
                  if (validSubstringRange(p1 + strlen(ESP32_MAIL_STR_176), p2 - p1 - strlen(ESP32_MAIL_STR_176), lineBuf))
                    imapData._messageDataInfo[mailIndex][messageDataIndex]._filename = lineBuf.substr(p1 + strlen(ESP32_MAIL_STR_176), p2 - p1 - strlen(ESP32_MAIL_STR_176));
                }
              }
              else if (tmp.find(ESP32_MAIL_STR_177) != std::string::npos)
              {

                p1 = tmp.find(ESP32_MAIL_STR_177);
                imapData._messageDataInfo[mailIndex][messageDataIndex]._filename = lineBuf.substr(p1 + strlen(ESP32_MAIL_STR_177));
              }

              p1 = tmp.find(ESP32_MAIL_STR_178);
              if (p1 != std::string::npos)
              {
                p2 = lineBuf.find(";", p1 + strlen(ESP32_MAIL_STR_178) + 1);
                if (p2 != std::string::npos)
                {

                  if (validSubstringRange(p1 + strlen(ESP32_MAIL_STR_178), p2 - p1 - strlen(ESP32_MAIL_STR_178), lineBuf))
                    imapData._messageDataInfo[mailIndex][messageDataIndex]._size = atoi(lineBuf.substr(p1 + strlen(ESP32_MAIL_STR_178), p2 - p1 - strlen(ESP32_MAIL_STR_178)).c_str());
                  imapData._totalAttachFileSize[mailIndex] += imapData._messageDataInfo[mailIndex][messageDataIndex]._size;
                }
                else
                {

                  imapData._messageDataInfo[mailIndex][messageDataIndex]._size = atoi(lineBuf.substr(p1 + strlen(ESP32_MAIL_STR_178)).c_str());
                  imapData._totalAttachFileSize[mailIndex] += imapData._messageDataInfo[mailIndex][messageDataIndex]._size;
                }
              }

              p1 = tmp.find(ESP32_MAIL_STR_179);
              if (p1 != std::string::npos)
              {
                p2 = lineBuf.find(ESP32_MAIL_STR_136, p1 + strlen(ESP32_MAIL_STR_179));
                if (p2 != std::string::npos)
                {

                  if (validSubstringRange(p1 + strlen(ESP32_MAIL_STR_179), p2 - p1 - strlen(ESP32_MAIL_STR_179), lineBuf))
                    imapData._messageDataInfo[mailIndex][messageDataIndex]._creation_date = lineBuf.substr(p1 + strlen(ESP32_MAIL_STR_179), p2 - p1 - strlen(ESP32_MAIL_STR_179));
                }
              }
              else if (tmp.find(ESP32_MAIL_STR_180) != std::string::npos)
              {
                p1 = tmp.find(ESP32_MAIL_STR_180);

                imapData._messageDataInfo[mailIndex][messageDataIndex]._creation_date = lineBuf.substr(p1 + strlen(ESP32_MAIL_STR_180));
              }

              p1 = tmp.find(ESP32_MAIL_STR_181);
              if (p1 != std::string::npos)
              {
                p2 = lineBuf.find(ESP32_MAIL_STR_136, p1 + strlen(ESP32_MAIL_STR_181));
                if (p2 != std::string::npos)
                {
                  if (validSubstringRange(p1 + strlen(ESP32_MAIL_STR_181), p2 - p1 - strlen(ESP32_MAIL_STR_181), lineBuf))
                    imapData._messageDataInfo[mailIndex][messageDataIndex]._modification_date = lineBuf.substr(p1 + strlen(ESP32_MAIL_STR_181), p2 - p1 - strlen(ESP32_MAIL_STR_181));
                }
              }
              else if (tmp.find(ESP32_MAIL_STR_182) != std::string::npos)
              {

                p1 = tmp.find(ESP32_MAIL_STR_182);
                imapData._messageDataInfo[mailIndex][messageDataIndex]._modification_date = lineBuf.substr(p1 + strlen(ESP32_MAIL_STR_182));
              }
            }

            imapData._messageDataInfo[mailIndex][messageDataIndex]._part = part;
          }
        }

        if (imapCommandType == IMAP_COMMAND_TYPE::SEARCH && lfCount == 0)
        {
          int max = imapData._emailNumMax;
          if (!imapData._recentSort)
            max = max - 1;

          for (size_t i = 0; i < lineBuf.length(); i++)
          {
            if (lineBuf[i] == ' ')
            {
              if (msgNumBuf != ESP32_MAIL_STR_183 && msgNumBuf != ESP32_MAIL_STR_141 && imapData._msgNum.size() <= max)
              {
                imapData._msgNum.push_back(msgNumBuf);

                if (imapData._msgNum.size() > imapData._emailNumMax && imapData._recentSort)
                  imapData._msgNum.erase(imapData._msgNum.begin());
                imapData._searchCount++;
              }

              msgNumBuf.clear();
            }
            else if (lineBuf[i] != '\r' && lineBuf[i] != '\n')
            {
              msgNumBuf.append(1, lineBuf[i]);
            }
          }

          if (msgNumBuf.length() > 0 && msgNumBuf != ESP32_MAIL_STR_183 && msgNumBuf != ESP32_MAIL_STR_141 && imapData._msgNum.size() <= max)
          {
            imapData._msgNum.push_back(msgNumBuf);
            imapData._searchCount++;

            if (imapData._msgNum.size() > imapData._emailNumMax && imapData._recentSort)
              imapData._msgNum.erase(imapData._msgNum.begin());
          }

          if (imapData._recentSort)
            std::sort(imapData._msgNum.begin(), imapData._msgNum.end(), compFunc);
        }
        else if (imapCommandType == IMAP_COMMAND_TYPE::FETCH_BODY_HEADER)
        {

          uint8_t _headerType = 0;

          p1 = tmp.find(ESP32_MAIL_STR_184);
          if (p1 != std::string::npos)
          {
            headerType = IMAP_HEADER_TYPE::FROM;
            _headerType = IMAP_HEADER_TYPE::FROM;

            string from;

            p2 = lineBuf.find(ESP32_MAIL_STR_173, p1 + strlen(ESP32_MAIL_STR_184));
            if (p2 != std::string::npos)
            {
              if (validSubstringRange(p1 + strlen(ESP32_MAIL_STR_184), p2 - p1 - strlen(ESP32_MAIL_STR_184), lineBuf))
                from = lineBuf.substr(p1 + strlen(ESP32_MAIL_STR_184), p2 - p1 - strlen(ESP32_MAIL_STR_184));
            }
            else
              from = lineBuf.substr(p1 + strlen(ESP32_MAIL_STR_184));

            if (from[0] == '=' && from[1] == '?')
            {
              p1 = from.find("?", 2);

              if (p1 != std::string::npos)
              {
                if (validSubstringRange(2, p1 - 2, lineBuf))
                  imapData._from_charset[mailIndex] = from.substr(2, p1 - 2);
              }
            }
            else
              imapData._from_charset[mailIndex] = "";

            memset(dest, 0, bufSize);
            RFC2047Decoder.rfc2047Decode(dest, from.c_str(), bufSize);
            imapData._from[mailIndex] = dest;
            std::string().swap(from);
          }

          p1 = tmp.find(ESP32_MAIL_STR_185);
          if (p1 != std::string::npos)
          {
            headerType = IMAP_HEADER_TYPE::TO;
            _headerType = IMAP_HEADER_TYPE::TO;

            string to;
            p2 = lineBuf.find(ESP32_MAIL_STR_173, p1 + 1);
            if (p2 != std::string::npos)
            {
              if (validSubstringRange(p1 + strlen(ESP32_MAIL_STR_185), p2 - p1 - strlen(ESP32_MAIL_STR_185), lineBuf))
                to = lineBuf.substr(p1 + strlen(ESP32_MAIL_STR_185), p2 - p1 - strlen(ESP32_MAIL_STR_185));
            }
            else
              to = lineBuf.substr(p1 + strlen(ESP32_MAIL_STR_185));

            if (to[0] == '=' && to[1] == '?')
            {
              p1 = to.find("?", 2);

              if (p1 != std::string::npos)
              {
                if (validSubstringRange(2, p1 - 2, lineBuf))
                  imapData._to_charset[mailIndex] = to.substr(2, p1 - 2);
              }
            }
            else
              imapData._to_charset[mailIndex] = "";

            memset(dest, 0, bufSize);
            RFC2047Decoder.rfc2047Decode(dest, to.c_str(), bufSize);
            imapData._to[mailIndex] = dest;
            std::string().swap(to);
          }

          p1 = tmp.find(ESP32_MAIL_STR_186);
          if (p1 != std::string::npos)
          {
            headerType = IMAP_HEADER_TYPE::CC;
            _headerType = IMAP_HEADER_TYPE::CC;

            string cc;
            p2 = lineBuf.find(ESP32_MAIL_STR_173, p1 + 1);
            if (p2 != std::string::npos)
            {
              if (validSubstringRange(p1 + strlen(ESP32_MAIL_STR_186), p2 - p1 - strlen(ESP32_MAIL_STR_186), lineBuf))
                cc = lineBuf.substr(p1 + strlen(ESP32_MAIL_STR_186), p2 - p1 - strlen(ESP32_MAIL_STR_186));
            }
            else
              cc = lineBuf.substr(p1 + strlen(ESP32_MAIL_STR_186));

            if (cc[0] == '=' && cc[1] == '?')
            {
              p1 = cc.find("?", 2);

              if (p1 != std::string::npos)
              {
                if (validSubstringRange(2, p1 - 2, lineBuf))
                  imapData._cc_charset[mailIndex] = cc.substr(2, p1 - 2);
              }
            }
            else
              imapData._cc_charset[mailIndex] = "";

            memset(dest, 0, bufSize);
            RFC2047Decoder.rfc2047Decode(dest, cc.c_str(), bufSize);
            imapData._cc[mailIndex] = dest;
            std::string().swap(cc);
          }

          p1 = tmp.find(ESP32_MAIL_STR_187);
          if (p1 != std::string::npos)
          {
            headerType = IMAP_HEADER_TYPE::SUBJECT;
            _headerType = IMAP_HEADER_TYPE::SUBJECT;

            p2 = lineBuf.find(ESP32_MAIL_STR_173, p1 + 1);

            string subject;
            memset(dest, 0, bufSize);
            if (p2 != std::string::npos)
            {
              if (validSubstringRange(p1 + strlen(ESP32_MAIL_STR_187), p2 - p1 - strlen(ESP32_MAIL_STR_187), lineBuf))
                subject = lineBuf.substr(p1 + strlen(ESP32_MAIL_STR_187), p2 - p1 - strlen(ESP32_MAIL_STR_187));
            }
            else
              subject = lineBuf.substr(p1 + strlen(ESP32_MAIL_STR_187));

            if (subject[0] == '=' && subject[1] == '?')
            {
              p1 = subject.find("?", 2);
              if (p1 != std::string::npos)
              {
                if (validSubstringRange(2, p1 - 2, lineBuf))
                  imapData._subject_charset[mailIndex] = subject.substr(2, p1 - 2);
              }
            }
            else
              imapData._subject_charset[mailIndex] = "";

            memset(dest, 0, bufSize);
            RFC2047Decoder.rfc2047Decode(dest, subject.c_str(), bufSize);
            imapData._subject[mailIndex] = dest;
            std::string().swap(subject);
          }
          p1 = tmp.find(ESP32_MAIL_STR_188);
          if (p1 != std::string::npos)
          {
            headerType = IMAP_HEADER_TYPE::DATE;
            _headerType = IMAP_HEADER_TYPE::DATE;

            p2 = lineBuf.find(ESP32_MAIL_STR_173, p1 + 1);
            if (p2 != std::string::npos)
            {
              if (validSubstringRange(p1 + strlen(ESP32_MAIL_STR_188), p2 - p1 - strlen(ESP32_MAIL_STR_188), lineBuf))
                imapData._date[mailIndex] = lineBuf.substr(p1 + strlen(ESP32_MAIL_STR_188), p2 - p1 - strlen(ESP32_MAIL_STR_188));
            }
            else
              imapData._date[mailIndex] = lineBuf.substr(p1 + strlen(ESP32_MAIL_STR_188));
          }

          p1 = tmp.find(ESP32_MAIL_STR_189);
          if (p1 != std::string::npos)
          {
            headerType = IMAP_HEADER_TYPE::MSG_ID;
            _headerType = IMAP_HEADER_TYPE::MSG_ID;

            p2 = lineBuf.find(ESP32_MAIL_STR_173, p1 + 1);
            if (p2 != std::string::npos)
            {
              if (validSubstringRange(p1 + strlen(ESP32_MAIL_STR_189), p2 - p1 - strlen(ESP32_MAIL_STR_189), lineBuf))
                imapData._msgID[mailIndex] = lineBuf.substr(p1 + strlen(ESP32_MAIL_STR_189), p2 - p1 - strlen(ESP32_MAIL_STR_189));
            }
            else
              imapData._msgID[mailIndex] = lineBuf.substr(p1 + strlen(ESP32_MAIL_STR_189));
          }

          p1 = tmp.find(ESP32_MAIL_STR_190);
          if (p1 != std::string::npos)
          {
            headerType = IMAP_HEADER_TYPE::ACCEPT_LANG;
            _headerType = IMAP_HEADER_TYPE::ACCEPT_LANG;

            p2 = lineBuf.find(ESP32_MAIL_STR_173, p1 + 1);
            if (p2 != std::string::npos)
            {
              if (validSubstringRange(p1 + strlen(ESP32_MAIL_STR_190), p2 - p1 - strlen(ESP32_MAIL_STR_190), lineBuf))
                imapData._acceptLanguage[mailIndex] = lineBuf.substr(p1 + strlen(ESP32_MAIL_STR_190), p2 - p1 - strlen(ESP32_MAIL_STR_190));
            }
            else
              imapData._acceptLanguage[mailIndex] = lineBuf.substr(p1 + strlen(ESP32_MAIL_STR_190));
          }

          p1 = tmp.find(ESP32_MAIL_STR_191);
          if (p1 != std::string::npos)
          {
            headerType = IMAP_HEADER_TYPE::CONT_LANG;
            _headerType = IMAP_HEADER_TYPE::CONT_LANG;

            p2 = lineBuf.find(ESP32_MAIL_STR_173, p1 + 1);
            if (p2 != std::string::npos)
            {
              if (validSubstringRange(p1 + strlen(ESP32_MAIL_STR_191), p2 - p1 - strlen(ESP32_MAIL_STR_191), lineBuf))
                imapData._contentLanguage[mailIndex] = lineBuf.substr(p1 + strlen(ESP32_MAIL_STR_191), p2 - p1 - strlen(ESP32_MAIL_STR_191));
            }
            else
              imapData._contentLanguage[mailIndex] = lineBuf.substr(p1 + strlen(ESP32_MAIL_STR_191));
          }

          if (_headerType == 0 && lineBuf != ESP32_MAIL_STR_192 && lineBuf.find(ESP32_MAIL_STR_145) == std::string::npos)
          {
            if (headerType == IMAP_HEADER_TYPE::FROM)
            {
              memset(dest, 0, bufSize);
              RFC2047Decoder.rfc2047Decode(dest, lineBuf.c_str(), bufSize);
              imapData._from[mailIndex] += dest;
            }
            else if (headerType == IMAP_HEADER_TYPE::TO)
            {
              memset(dest, 0, bufSize);
              RFC2047Decoder.rfc2047Decode(dest, lineBuf.c_str(), bufSize);
              imapData._to[mailIndex] += dest;
            }
            else if (headerType == IMAP_HEADER_TYPE::CC)
            {
              memset(dest, 0, bufSize);
              RFC2047Decoder.rfc2047Decode(dest, lineBuf.c_str(), bufSize);
              imapData._cc[mailIndex] += dest;
            }
            else if (headerType == IMAP_HEADER_TYPE::SUBJECT)
            {
              memset(dest, 0, bufSize);
              RFC2047Decoder.rfc2047Decode(dest, lineBuf.c_str(), bufSize);
              imapData._subject[mailIndex] += dest;
            }
          }
        }
        else if (imapCommandType == IMAP_COMMAND_TYPE::FETCH_BODY_TEXT ||
                 imapCommandType == IMAP_COMMAND_TYPE::FETCH_BODY_MIME ||
                 imapCommandType == IMAP_COMMAND_TYPE::FETCH_BODY_HEADER ||
                 imapCommandType == IMAP_COMMAND_TYPE::FETCH_BODY_ATTACHMENT)
        {
          if (lfCount == 0)
          {
            p1 = lineBuf.find_last_of(ESP32_MAIL_STR_193);
            if (p1 != std::string::npos)
            {
              p2 = lineBuf.find(ESP32_MAIL_STR_194, p1 + 1);
              if (p2 != std::string::npos)
                if (validSubstringRange(p1 + 1, p2 - p1 - 1, lineBuf))
                  payloadLength = atoi(lineBuf.substr(p1 + 1, p2 - p1 - 1).c_str());
            }
          }
        }

        if (imapCommandType == IMAP_COMMAND_TYPE::LIST)
        {
          p1 = lineBuf.find(ESP32_MAIL_STR_195);
          p2 = lineBuf.find(ESP32_MAIL_STR_196);

          if (p1 != std::string::npos && p2 == std::string::npos)
          {
            p2 = lineBuf.find_last_of(ESP32_MAIL_STR_136);
            if (p2 != std::string::npos)
            {
              p1 = lineBuf.find_last_of(ESP32_MAIL_STR_136, p2 - 1);
              if (p1 != std::string::npos)
              {
                if (validSubstringRange(p1 + 1, p2 - p1 - 1, lineBuf))
                  imapData._folders.push_back(lineBuf.substr(p1 + 1, p2 - p1 - 1));
              }
            }
          }
        }

        if (imapCommandType == IMAP_COMMAND_TYPE::SELECT || imapCommandType == IMAP_COMMAND_TYPE::EXAMINE)
        {

          p1 = lineBuf.find(ESP32_MAIL_STR_197);
          if (p1 != std::string::npos)
          {
            p1 = lineBuf.find(ESP32_MAIL_STR_198);
            if (p1 != std::string::npos)
            {
              p2 = lineBuf.find(ESP32_MAIL_STR_192);
              if (p2 != std::string::npos)
              {
                string tmp;
                if (validSubstringRange(p1 + 1, p2 - p1 - 1, lineBuf))
                  tmp = lineBuf.substr(p1 + 1, p2 - p1 - 1).c_str();
                msgNumBuf.clear();

                for (size_t i = 0; i < tmp.length(); i++)
                {
                  if (tmp[i] != '\\' && tmp[i] != ' ' && tmp[i] != '\r' && tmp[i] != '\n')
                    msgNumBuf.append(1, tmp[i]);

                  if (tmp[i] == ' ')
                  {
                    imapData._flag.push_back(msgNumBuf);
                    msgNumBuf.clear();
                  }
                }
                if (msgNumBuf.length() > 0)
                {
                  imapData._flag.push_back(msgNumBuf);
                }

                std::string().swap(tmp);
              }
            }
          }

          p2 = lineBuf.find(ESP32_MAIL_STR_199);
          if (p2 != std::string::npos)
          {
            if (validSubstringRange(2, p2 - 2, lineBuf))
              imapData._totalMessage = atoi(lineBuf.substr(2, p2 - 2).c_str());
          }

          p1 = lineBuf.find(ESP32_MAIL_STR_200);
          if (p1 != std::string::npos)
          {
            p2 = lineBuf.find(ESP32_MAIL_STR_156, p1 + 10);
            if (p2 != std::string::npos)
            {
              if (validSubstringRange(p1 + 10, p2 - p1 - 10, lineBuf))
                imapData._nextUID = lineBuf.substr(p1 + 10, p2 - p1 - 10);
            }
          }
        }

        if (validResponse && imapCommandType == IMAP_COMMAND_TYPE::FETCH_BODY_TEXT && lfCount > 0 && (charCount < maxChar || imapData._saveHTMLMsg || imapData._saveTextMsg))
        {

          if (imapData._messageDataInfo[mailIndex][messageDataIndex]._transfer_encoding == ESP32_MAIL_STR_160)
          {

            unsigned char *decoded = base64_decode_char((const unsigned char *)lineBuf.c_str(), lineBuf.length(), &outputLength);

            if (decoded)
            {
              if (charCount < maxChar)
                imapData._messageDataInfo[mailIndex][messageDataIndex]._text.append((char *)decoded, outputLength);

              if (imapData._saveHTMLMsg || imapData._saveTextMsg)
              {

                if (!imapData._messageDataInfo[mailIndex][messageDataIndex]._sdFileOpenWrite)
                {

                  imapData._messageDataInfo[mailIndex][messageDataIndex]._sdFileOpenWrite = true;

                  if (_sdOk)
                  {

                    downloadReq = true;

                    filepath.clear();
                    filepath += imapData._savePath;
                    filepath += ESP32_MAIL_STR_202;
                    filepath += imapData._msgNum[mailIndex];

                    if (!SD.exists(filepath.c_str()))
                      createDirs(filepath);

                    if (!imapData._headerSaved)
                      hpath = filepath + ESP32_MAIL_STR_203;

                    if (imapData._messageDataInfo[mailIndex][messageDataIndex]._contentType == ESP32_MAIL_STR_155)
                    {
                      if (imapData._saveDecodedText)
                        filepath += ESP32_MAIL_STR_161;
                      else
                        filepath += ESP32_MAIL_STR_162;
                    }
                    else if (imapData._messageDataInfo[mailIndex][messageDataIndex]._contentType == ESP32_MAIL_STR_154)
                    {
                      if (imapData._saveDecodedHTML)
                        filepath += ESP32_MAIL_STR_163;
                      else
                        filepath += ESP32_MAIL_STR_164;
                    }

                    file = SD.open(filepath.c_str(), FILE_WRITE);
                  }
                  else
                  {
                    if (imapData._messageDataCount[mailIndex] == messageDataIndex + 1)
                    {
                      imapData._messageDataInfo[mailIndex][messageDataIndex]._error = true;
                      imapData._messageDataInfo[mailIndex][messageDataIndex]._downloadError.clear();
                      imapData._messageDataInfo[mailIndex][messageDataIndex]._downloadError = ESP32_MAIL_STR_89;
                    }
                  }
                }

                if (_sdOk)
                {
                  if (imapData._messageDataInfo[mailIndex][messageDataIndex]._contentType == ESP32_MAIL_STR_155 && imapData._saveDecodedText ||
                      imapData._messageDataInfo[mailIndex][messageDataIndex]._contentType == ESP32_MAIL_STR_154 && imapData._saveDecodedHTML)
                    file.write((const uint8_t *)decoded, outputLength);
                  else
                    file.write((const uint8_t *)lineBuf.c_str(), lineBuf.length());
                }
              }

              free(decoded);
            }
          }
        }

        if (validResponse && imapCommandType == IMAP_COMMAND_TYPE::FETCH_BODY_ATTACHMENT && lfCount > 0)
        {

          if (imapData._messageDataInfo[mailIndex][messageDataIndex]._transfer_encoding == ESP32_MAIL_STR_160)
          {

            if (!imapData._messageDataInfo[mailIndex][messageDataIndex]._sdFileOpenWrite)
            {

              imapData._messageDataInfo[mailIndex][messageDataIndex]._sdFileOpenWrite = true;

              if (_sdOk)
              {

                downloadReq = true;

                filepath.clear();
                filepath += imapData._savePath;
                filepath += ESP32_MAIL_STR_202;
                filepath += imapData._msgNum[mailIndex];

                if (!SD.exists(filepath.c_str()))
                  createDirs(filepath);

                filepath += ESP32_MAIL_STR_202;

                filepath += imapData._messageDataInfo[mailIndex][messageDataIndex]._filename;
                file = SD.open(filepath.c_str(), FILE_WRITE);
              }
              else
              {
                if (imapData._messageDataCount[mailIndex] == messageDataIndex + 1)
                {
                  imapData._messageDataInfo[mailIndex][messageDataIndex]._error = true;
                  imapData._messageDataInfo[mailIndex][messageDataIndex]._downloadError.clear();
                  imapData._messageDataInfo[mailIndex][messageDataIndex]._downloadError = ESP32_MAIL_STR_89;
                }
              }
            }

            if (_sdOk)
            {
              unsigned char *decoded = base64_decode_char((const unsigned char *)lineBuf.c_str(), lineBuf.length(), &outputLength);

              if (decoded)
              {
                file.write((const uint8_t *)decoded, outputLength);
                yield();

                if (imapData._downloadReport)
                {
                  imapData._downloadedByte[mailIndex] += outputLength;
                  currentDownloadByte += outputLength;

                  if (imapData._messageDataInfo[mailIndex][messageDataIndex]._size == 0)
                  {
                    if (payloadLength > 36)
                    {
                      imapData._messageDataInfo[mailIndex][messageDataIndex]._size = base64DecodeSize(lineBuf, payloadLength - (payloadLength / 36));
                      imapData._totalAttachFileSize[mailIndex] += imapData._messageDataInfo[mailIndex][messageDataIndex]._size;
                    }
                  }

                  int p = 0;

                  if (imapData._totalAttachFileSize[mailIndex] > 0)
                    p = 100 * imapData._downloadedByte[mailIndex] / imapData._totalAttachFileSize[mailIndex];

                  if (p % 5 == 0 & p <= 100)
                  {
                    bool success = (p == 100);
                    if (imapData._readCallback && reportState != -1)
                    {
                      memset(buf, 0, bufSize);
                      itoa(p, buf, 10);

                      std::string dl = ESP32_MAIL_STR_90 + imapData._messageDataInfo[mailIndex][messageDataIndex]._filename + ESP32_MAIL_STR_91 + buf + ESP32_MAIL_STR_92;

                      if (imapData._readCallback)
                      {
                        cbData._info = dl;
                        cbData._status = dl;
                        cbData._success = false;
                        imapData._readCallback(cbData);
                      }
                    }
                    reportState = -1;
                  }
                  else
                    reportState = 0;
                }

                free(decoded);
              }
            }
          }
        }

        lineBuf.clear();
        lfCount++;
        std::string().swap(tmp);
      }

      if (millis() - dataTime > http.tcpTimeout)
        break;

      readCount++;
    }

    if (imapData._error.size() > 0 && mailIndex > -1)
    {
      if (validResponse && !imapData._error[mailIndex])
      {
        imapData._errorMsg[mailIndex].clear();
        imapData._errorMsg[mailIndex] = "";
      }
    }

    if (millis() - dataTime > http.tcpTimeout)
    {

      if (downloadReq)
      {
        if (imapData._messageDataCount[mailIndex] == messageDataIndex + 1)
        {
          imapData._messageDataInfo[mailIndex][messageDataIndex]._error = true;
          imapData._messageDataInfo[mailIndex][messageDataIndex]._downloadError.clear();
          imapData._messageDataInfo[mailIndex][messageDataIndex]._downloadError = ESP32_MAIL_STR_93;
        }
      }
      else
      {

        if (imapData._error.size() > 0 && mailIndex > -1)
        {
          imapData._error[mailIndex] = true;
          imapData._errorMsg[mailIndex].clear();
          if (WiFi.status() != WL_CONNECTED)
            imapData._errorMsg[mailIndex] = ESP32_MAIL_STR_94;
          else
            imapData._errorMsg[mailIndex] = ESP32_MAIL_STR_95;
        }
      }
    }
  }

  if (validResponse && (imapCommandType == IMAP_COMMAND_TYPE::FETCH_BODY_ATTACHMENT || imapCommandType == IMAP_COMMAND_TYPE::FETCH_BODY_TEXT) && messageDataIndex != -1)
  {
    if (imapData._messageDataInfo[mailIndex][messageDataIndex]._sdFileOpenWrite)
      file.close();
  }

  if (validResponse && imapCommandType == IMAP_COMMAND_TYPE::FETCH_BODY_ATTACHMENT && imapData._messageDataInfo[mailIndex][messageDataIndex]._size != currentDownloadByte)
  {
    imapData._messageDataInfo[mailIndex][messageDataIndex]._size = currentDownloadByte;
  }

  if (hpath != "")
  {

    file = SD.open(hpath.c_str(), FILE_WRITE);

    file.print(ESP32_MAIL_STR_99);
    file.println(imapData._date[mailIndex].c_str());

    file.print(ESP32_MAIL_STR_100);
    if (imapData._uidSearch)
      file.println(imapData._msgNum[mailIndex].c_str());
    else
      file.println();

    file.print(ESP32_MAIL_STR_101);
    file.println(imapData._msgNum[mailIndex].c_str());

    file.print(ESP32_MAIL_STR_102);
    file.println(imapData._acceptLanguage[mailIndex].c_str());

    file.print(ESP32_MAIL_STR_103);
    file.println(imapData._contentLanguage[mailIndex].c_str());

    file.print(ESP32_MAIL_STR_104);
    file.println(imapData._from[mailIndex].c_str());

    file.print(ESP32_MAIL_STR_105);
    file.println(imapData._from_charset[mailIndex].c_str());

    file.print(ESP32_MAIL_STR_106);
    file.println(imapData._to[mailIndex].c_str());

    file.print(ESP32_MAIL_STR_107);
    file.println(imapData._to_charset[mailIndex].c_str());

    file.print(ESP32_MAIL_STR_108);
    file.println(imapData._cc[mailIndex].c_str());

    file.print(ESP32_MAIL_STR_109);
    file.println(imapData._cc_charset[mailIndex].c_str());

    file.print(ESP32_MAIL_STR_110);
    file.println(imapData._subject[mailIndex].c_str());

    file.print(ESP32_MAIL_STR_111);
    file.println(imapData._subject_charset[mailIndex].c_str());

    file.print(ESP32_MAIL_STR_112);
    file.println(imapData._messageDataInfo[mailIndex][messageDataIndex]._charset.c_str());

    if (imapData._attachmentCount[mailIndex] > 0)
    {

      file.print(ESP32_MAIL_STR_113);
      file.println(imapData._attachmentCount[mailIndex]);

      for (int j = 0; j < imapData._attachmentCount[mailIndex]; j++)
      {
        file.print(ESP32_MAIL_STR_114);
        file.println(j + 1);

        file.print(ESP32_MAIL_STR_115);
        file.println(imapData.getAttachmentFileName(mailIndex, j));

        file.print(ESP32_MAIL_STR_116);
        file.println(imapData.getAttachmentName(mailIndex, j));

        file.print(ESP32_MAIL_STR_117);
        file.println(imapData.getAttachmentFileSize(mailIndex, j));

        file.print(ESP32_MAIL_STR_118);
        file.println(imapData.getAttachmentType(mailIndex, j));

        file.print(ESP32_MAIL_STR_119);
        file.println(imapData.getAttachmentCreationDate(mailIndex, j));
      }
    }

    delete[] buf;
    delete[] dest;

    file.close();
    imapData._headerSaved = true;
  }

  std::string().swap(lineBuf);
  std::string().swap(msgNumBuf);
  std::string().swap(filepath);
  std::string().swap(hpath);

  return validResponse;
}

bool ESP32_MailClient::validSubstringRange(int start, int length, std::string str)
{
  return start + length < str.length() && length > 0 && start >= 0;
}

double ESP32_MailClient::base64DecodeSize(std::string lastBase64String, int length)
{
  double result = 0;
  int padding = 0;
  if (lastBase64String != "")
  {

    if (lastBase64String[lastBase64String.length() - 1] == '=' && lastBase64String[lastBase64String.length() - 2] == '=')
    {
      padding = 2;
    }
    else if (lastBase64String[lastBase64String.length() - 1] == '=')
      padding = 1;
  }
  result = (ceil(length / 4) * 3) - padding;
  return result;
}

inline std::string ESP32_MailClient::trim(std::string &str)
{
  str.erase(0, str.find_first_not_of(' ')); //prefixing spaces
  str.erase(str.find_last_not_of(' ') + 1); //surfixing spaces
  str.erase(std::remove(str.begin(), str.end(), '\r'), str.end());
  str.erase(std::remove(str.begin(), str.end(), '\n'), str.end());
  return str;
}

unsigned char *ESP32_MailClient::base64_decode_char(const unsigned char *src, size_t len, size_t *out_len)
{

  unsigned char *out, *pos, block[4], tmp;
  size_t i, count, olen;
  int pad = 0;
  size_t extra_pad;

  unsigned char *dtable = new unsigned char[256];

  memset(dtable, 0x80, 256);

  for (i = 0; i < sizeof(base64_table) - 1; i++)
    dtable[base64_table[i]] = (unsigned char)i;
  dtable['='] = 0;

  count = 0;
  for (i = 0; i < len; i++)
  {
    if (dtable[src[i]] != 0x80)
      count++;
  }

  if (count == 0)
    return NULL;
  extra_pad = (4 - count % 4) % 4;

  olen = (count + extra_pad) / 4 * 3;
  pos = out = (unsigned char *)malloc(olen);
  if (out == NULL)
    return NULL;

  count = 0;
  for (i = 0; i < len + extra_pad; i++)
  {
    unsigned char val;

    if (i >= len)
      val = '=';
    else
      val = src[i];
    tmp = dtable[val];
    if (tmp == 0x80)
      continue;

    if (val == '=')
      pad++;
    block[count] = tmp;
    count++;
    if (count == 4)
    {
      *pos++ = (block[0] << 2) | (block[1] >> 4);
      *pos++ = (block[1] << 4) | (block[2] >> 2);
      *pos++ = (block[2] << 6) | block[3];
      count = 0;
      if (pad)
      {
        if (pad == 1)
          pos--;
        else if (pad == 2)
          pos -= 2;
        else
        {
          /* Invalid padding */
          free(out);
          return NULL;
        }
        break;
      }
    }
  }

  *out_len = pos - out;
  delete[] dtable;
  return out;
}

std::string ESP32_MailClient::base64_encode_string(const unsigned char *src, size_t len)
{
  unsigned char *out, *pos;
  const unsigned char *end, *in;

  size_t olen;

  olen = 4 * ((len + 2) / 3); /* 3-byte blocks to 4-byte */

  if (olen < len)
    return std::string(); /* integer overflow */

  std::string outStr;
  outStr.resize(olen);
  out = (unsigned char *)&outStr[0];

  end = src + len;
  in = src;
  pos = out;

  while (end - in >= 3)
  {
    *pos++ = base64_table[in[0] >> 2];
    *pos++ = base64_table[((in[0] & 0x03) << 4) | (in[1] >> 4)];
    *pos++ = base64_table[((in[1] & 0x0f) << 2) | (in[2] >> 6)];
    *pos++ = base64_table[in[2] & 0x3f];
    in += 3;
    yield();
  }

  if (end - in)
  {
    *pos++ = base64_table[in[0] >> 2];
    if (end - in == 1)
    {
      *pos++ = base64_table[(in[0] & 0x03) << 4];
      *pos++ = '=';
    }
    else
    {
      *pos++ = base64_table[((in[0] & 0x03) << 4) | (in[1] >> 4)];
      *pos++ = base64_table[(in[1] & 0x0f) << 2];
    }
    *pos++ = '=';
  }

  return outStr;
}

void ESP32_MailClient::send_base64_encode_data(WiFiClient *tcp, const unsigned char *src, size_t len)
{

  const unsigned char *end, *in;

  size_t olen;

  olen = 4 * ((len + 2) / 3); /* 3-byte blocks to 4-byte */

  if (olen < len)
    return; /* integer overflow */

  end = src + len;
  in = src;

  size_t chunkSize = 512;
  size_t byteAdd = 0;
  size_t byteSent = 0;

  unsigned char *buf = new unsigned char[chunkSize];
  memset(buf, 0, chunkSize);

  while (end - in >= 3)
  {
    buf[byteAdd++] = base64_table[in[0] >> 2];
    buf[byteAdd++] = base64_table[((in[0] & 0x03) << 4) | (in[1] >> 4)];
    buf[byteAdd++] = base64_table[((in[1] & 0x0f) << 2) | (in[2] >> 6)];
    buf[byteAdd++] = base64_table[in[2] & 0x3f];

    if (len > chunkSize)
    {
      if (byteAdd >= chunkSize)
      {
        byteSent += byteAdd;
        tcp->write(buf, byteAdd);
        memset(buf, 0, sizeof buf);
        byteAdd = 0;
      }
    }

    in += 3;
    yield();
  }

  if (byteAdd > 0)
    tcp->write(buf, byteAdd);

  if (end - in)
  {
    memset(buf, 0, sizeof buf);
    byteAdd = 0;

    buf[byteAdd++] = base64_table[in[0] >> 2];
    if (end - in == 1)
    {
      buf[byteAdd++] = base64_table[(in[0] & 0x03) << 4];
      buf[byteAdd++] = '=';
    }
    else
    {
      buf[byteAdd++] = base64_table[((in[0] & 0x03) << 4) | (in[1] >> 4)];
      buf[byteAdd++] = base64_table[(in[1] & 0x0f) << 2];
    }
    buf[byteAdd++] = '=';

    tcp->write(buf, byteAdd);
    memset(buf, 0, chunkSize);
  }
  delete[] buf;
}

void ESP32_MailClient::send_base64_encode_file(WiFiClient *tcp, const char *filePath)
{

  File file = SD.open(filePath, "r");

  if (!file)
    return;

  size_t chunkSize = 512;
  size_t byteAdd = 0;
  size_t byteSent = 0;

  unsigned char *buf = new unsigned char[chunkSize];
  memset(buf, 0, chunkSize);

  size_t len = file.size();
  size_t fbufIndex = 0;
  unsigned char *fbuf = new unsigned char[3];

  while (file.available())
  {
    memset(fbuf, 0, 3);
    if (len - fbufIndex >= 3)
    {
      file.read(fbuf, 3);

      buf[byteAdd++] = base64_table[fbuf[0] >> 2];
      buf[byteAdd++] = base64_table[((fbuf[0] & 0x03) << 4) | (fbuf[1] >> 4)];
      buf[byteAdd++] = base64_table[((fbuf[1] & 0x0f) << 2) | (fbuf[2] >> 6)];
      buf[byteAdd++] = base64_table[fbuf[2] & 0x3f];

      if (len > chunkSize)
      {
        if (byteAdd >= chunkSize)
        {
          byteSent += byteAdd;
          tcp->write(buf, byteAdd);
          memset(buf, 0, chunkSize);
          byteAdd = 0;
        }
      }
      fbufIndex += 3;

      yield();
    }
    else
    {

      if (len - fbufIndex == 1)
      {
        fbuf[0] = file.read();
      }
      else if (len - fbufIndex == 2)
      {
        fbuf[0] = file.read();
        fbuf[1] = file.read();
      }

      break;
    }
  }

  file.close();

  if (byteAdd > 0)
    tcp->write(buf, byteAdd);

  if (len - fbufIndex > 0)
  {

    memset(buf, 0, chunkSize);
    byteAdd = 0;

    buf[byteAdd++] = base64_table[fbuf[0] >> 2];
    if (len - fbufIndex == 1)
    {
      buf[byteAdd++] = base64_table[(fbuf[0] & 0x03) << 4];
      buf[byteAdd++] = '=';
    }
    else
    {
      buf[byteAdd++] = base64_table[((fbuf[0] & 0x03) << 4) | (fbuf[1] >> 4)];
      buf[byteAdd++] = base64_table[(fbuf[1] & 0x0f) << 2];
    }
    buf[byteAdd++] = '=';

    tcp->write(buf, byteAdd);
  }
  delete[] buf;
  delete[] fbuf;
}

IMAPData::IMAPData() {}
IMAPData::~IMAPData()
{
  empty();
}

void IMAPData::setLogin(const String &host, uint16_t port, const String &loginEmail, const String &loginPassword)
{
  _host.clear();
  _port = port;
  _loginEmail.clear();
  _loginPassword.clear();

  _host = host.c_str();
  _loginEmail = loginEmail.c_str();
  _loginPassword = loginPassword.c_str();
}

void IMAPData::setFolder(const String &folderName)
{
  _currentFolder.clear();
  _currentFolder = folderName.c_str();
}
void IMAPData::setMessageBufferSize(size_t size)
{
  _message_buffer_size = size;
}

void IMAPData::setAttachmentSizeLimit(size_t size)
{
  _attacement_max_size = size;
}

void IMAPData::setSearchCriteria(const String criteria)
{
  _searchCriteria.clear();
  _searchCriteria = criteria.c_str();
}

void IMAPData::setSaveFilePath(const String path)
{
  _savePath.clear();
  if (path.c_str()[0] != '/')
  {
    _savePath = "/";
    _savePath += path.c_str();
  }
  else
    _savePath = path.c_str();
}

void IMAPData::setFechUID(const String fetchUID)
{
  _fetchUID.clear();
  string tmp = fetchUID.c_str();
  std::transform(tmp.begin(), tmp.end(), tmp.begin(), ::toupper);
  if (tmp.find(ESP32_MAIL_STR_140) != std::string::npos || tmp.find(ESP32_MAIL_STR_212) != std::string::npos ||
      tmp.find(ESP32_MAIL_STR_213) != std::string::npos || tmp.find(ESP32_MAIL_STR_214) != std::string::npos || tmp.find(ESP32_MAIL_STR_215) != std::string::npos ||
      tmp.find(ESP32_MAIL_STR_216) != std::string::npos || tmp.find(ESP32_MAIL_STR_217) != std::string::npos || tmp.find(ESP32_MAIL_STR_218) != std::string::npos ||
      tmp.find(ESP32_MAIL_STR_219) != std::string::npos || tmp.find(ESP32_MAIL_STR_220) != std::string::npos)
    _fetchUID = ESP32_MAIL_STR_183;
  else
    _fetchUID = fetchUID.c_str();

  std::string().swap(tmp);
}

void IMAPData::setDownloadAttachment(bool download)
{
  _downloadAttachment = download;
}
void IMAPData::setRecentSort(bool recentSort)
{
  _recentSort = recentSort;
}

void IMAPData::setHTMLMessage(bool htmlFormat)
{
  _htmlFormat = htmlFormat;
}
void IMAPData::setTextMessage(bool textFormat)
{
  _textFormat = textFormat;
}
void IMAPData::setHeaderOnly(bool headerOnly)
{
  _headerOnly = headerOnly;
}

void IMAPData::setSearchLimit(uint16_t limit)
{
  _emailNumMax = limit;
}
bool IMAPData::isHeaderOnly()
{
  return _headerOnly;
}

void IMAPData::saveHTMLMessage(bool download, bool decoded)
{
  _saveDecodedHTML = decoded;
  _saveHTMLMsg = download;
}
void IMAPData::saveTextMessage(bool download, bool decoded)
{
  _saveDecodedText = decoded;
  _saveTextMsg = download;
}

void IMAPData::setReadCallback(readStatusCallback readCallback)
{
  _readCallback = readCallback;
}

void IMAPData::setDownloadReport(bool report)
{
  _downloadReport = report;
}

uint16_t IMAPData::getFolderCount()
{
  return _folders.size();
}
String IMAPData::getFolder(uint16_t folderIndex)
{
  if (folderIndex < _folders.size())
    return _folders[folderIndex].c_str();
  return std::string().c_str();
}

uint16_t IMAPData::getFlagCount()
{
  return _flag.size();
}
String IMAPData::getFlag(uint16_t flagIndex)
{
  if (flagIndex < _flag.size())
    return _flag[flagIndex].c_str();
  return std::string().c_str();
}

size_t IMAPData::totalMessages()
{
  return _totalMessage;
}

size_t IMAPData::searchCount()
{
  return _searchCount;
}

size_t IMAPData::availableMessages()
{
  return _msgNum.size();
}

size_t IMAPData::getAttachmentCount(uint16_t messageIndex)
{
  if (messageIndex < _msgNum.size())
    return _attachmentCount[messageIndex];
  return 0;
}

String IMAPData::getAttachmentFileName(size_t messageIndex, size_t attachmentIndex)
{
  if (messageIndex < _msgNum.size())
  {
    int s = _messageDataInfo[messageIndex].size();
    int id = 0;
    if (s > 0)
    {
      for (int i = 0; i < s; i++)
      {
        if (_messageDataInfo[messageIndex][i]._disposition == ESP32_MAIL_STR_153)
        {
          if (attachmentIndex == id)
            return _messageDataInfo[messageIndex][i]._filename.c_str();
          id++;
        }
      }
    }
    else
      return std::string().c_str();
  }
  else
    return std::string().c_str();
}

String IMAPData::getAttachmentName(size_t messageIndex, size_t attachmentIndex)
{
  if (messageIndex < _msgNum.size())
  {
    int s = _messageDataInfo[messageIndex].size();
    int id = 0;
    if (s > 0)
    {
      for (int i = 0; i < s; i++)
      {
        if (_messageDataInfo[messageIndex][i]._disposition == ESP32_MAIL_STR_153)
        {
          if (attachmentIndex == id)
            return _messageDataInfo[messageIndex][i]._name.c_str();
          id++;
        }
      }
    }
    else
      return std::string().c_str();
  }
  else
    return std::string().c_str();
}
int IMAPData::getAttachmentFileSize(size_t messageIndex, size_t attachmentIndex)
{
  if (messageIndex < _msgNum.size())
  {
    int s = _messageDataInfo[messageIndex].size();
    int id = 0;
    if (s > 0)
    {
      for (int i = 0; i < s; i++)
      {
        if (_messageDataInfo[messageIndex][i]._disposition == ESP32_MAIL_STR_153)
        {
          if (attachmentIndex == id)
            return _messageDataInfo[messageIndex][i]._size;
          id++;
        }
      }
    }
    else
      return 0;
  }
  else
    return 0;
}
String IMAPData::getAttachmentCreationDate(size_t messageIndex, size_t attachmentIndex)
{
  if (messageIndex < _msgNum.size())
  {
    int s = _messageDataInfo[messageIndex].size();
    int id = 0;
    if (s > 0)
    {
      for (int i = 0; i < s; i++)
      {
        if (_messageDataInfo[messageIndex][i]._disposition == ESP32_MAIL_STR_153)
        {
          if (attachmentIndex == id)
            return _messageDataInfo[messageIndex][i]._creation_date.c_str();
          id++;
        }
      }
    }
    else
      return std::string().c_str();
  }
  else
    return std::string().c_str();
}

String IMAPData::getAttachmentType(size_t messageIndex, size_t attachmentIndex)
{
  if (messageIndex < _msgNum.size())
  {
    int s = _messageDataInfo[messageIndex].size();
    int id = 0;
    if (s > 0)
    {
      for (int i = 0; i < s; i++)
      {
        if (_messageDataInfo[messageIndex][i]._disposition == ESP32_MAIL_STR_153)
        {
          if (attachmentIndex == id)
            return _messageDataInfo[messageIndex][i]._contentType.c_str();
          id++;
        }
      }
    }
    else
      return std::string().c_str();
  }
  else
    return std::string().c_str();
}

String IMAPData::getFrom(uint16_t messageIndex)
{
  return _from[messageIndex].c_str();
}

String IMAPData::getFromCharset(uint16_t messageIndex)
{
  return _from_charset[messageIndex].c_str();
}
String IMAPData::getTo(uint16_t messageIndex)
{
  return _to[messageIndex].c_str();
}
String IMAPData::getToCharset(uint16_t messageIndex)
{
  return _to_charset[messageIndex].c_str();
}
String IMAPData::getCC(uint16_t messageIndex)
{
  return _cc[messageIndex].c_str();
}
String IMAPData::getCCCharset(uint16_t messageIndex)
{
  return _cc_charset[messageIndex].c_str();
}

String IMAPData::getSubject(uint16_t messageIndex)
{
  return _subject[messageIndex].c_str();
}
String IMAPData::getSubjectCharset(uint16_t messageIndex)
{
  return _subject_charset[messageIndex].c_str();
}
String IMAPData::getHTMLMessage(uint16_t messageIndex)
{
  return getMessage(messageIndex, true);
}

String IMAPData::getTextMessage(uint16_t messageIndex)
{
  return getMessage(messageIndex, false);
}

String IMAPData::getMessage(uint16_t messageIndex, bool htmlFormat)
{
  if (messageIndex < _msgNum.size())
  {
    int s = _messageDataInfo[messageIndex].size();
    int id = 0;
    if (s > 0)
    {
      for (int i = 0; i < s; i++)
      {
        if (_messageDataInfo[messageIndex][i]._contentType == ESP32_MAIL_STR_155 && !htmlFormat)
          return _messageDataInfo[messageIndex][i]._text.c_str();
        else if (_messageDataInfo[messageIndex][i]._contentType == ESP32_MAIL_STR_154 && htmlFormat)
          return _messageDataInfo[messageIndex][i]._text.c_str();
      }
      return std::string().c_str();
    }
    else
      return std::string().c_str();
  }
  else
    return std::string().c_str();
}

String IMAPData::getHTMLMessgaeCharset(uint16_t messageIndex)
{
  if (messageIndex < _msgNum.size())
  {
    int s = _messageDataInfo[messageIndex].size();
    int id = 0;
    if (s > 0)
    {
      for (int i = 0; i < s; i++)
      {
        if (_messageDataInfo[messageIndex][i]._contentType == ESP32_MAIL_STR_154)
          return _messageDataInfo[messageIndex][i]._charset.c_str();
      }
      return std::string().c_str();
    }
    else
      return std::string().c_str();
  }
  else
    return std::string().c_str();
}

String IMAPData::getTextMessgaeCharset(uint16_t messageIndex)
{
  if (messageIndex < _msgNum.size())
  {
    int s = _messageDataInfo[messageIndex].size();
    int id = 0;
    if (s > 0)
    {
      for (int i = 0; i < s; i++)
      {
        if (_messageDataInfo[messageIndex][i]._contentType == ESP32_MAIL_STR_155)
          return _messageDataInfo[messageIndex][i]._charset.c_str();
      }
      return std::string().c_str();
    }
    else
      return std::string().c_str();
  }
  else
    return std::string().c_str();
}

String IMAPData::getDate(uint16_t messageIndex)
{
  return _date[messageIndex].c_str();
}

String IMAPData::getUID(uint16_t messageIndex)
{
  if (_uidSearch)
    return _msgNum[messageIndex].c_str();
  return std::string().c_str();
}

String IMAPData::getNumber(uint16_t messageIndex)
{

  if (!_uidSearch)
    return _msgNum[messageIndex].c_str();

  char *buf = new char[50];
  memset(buf, 0, sizeof buf);
  itoa(messageIndex + 1, buf, 10);
  return buf;
}

String IMAPData::getMessageID(uint16_t messageIndex)
{
  if (messageIndex < _msgNum.size())
    return _msgID[messageIndex].c_str();
  return std::string().c_str();
}

String IMAPData::getAcceptLanguage(uint16_t messageIndex)
{
  if (messageIndex < _msgNum.size())
    return _acceptLanguage[messageIndex].c_str();
  return std::string().c_str();
}
String IMAPData::getContentLanguage(uint16_t messageIndex)
{
  if (messageIndex < _msgNum.size())
    return _contentLanguage[messageIndex].c_str();
  return std::string().c_str();
}

bool IMAPData::isFetchMessageFailed(uint16_t messageIndex)
{
  if (messageIndex < _msgNum.size())
    return _error[messageIndex];
  return false;
}
String IMAPData::getFetchMessageFailedReason(uint16_t messageIndex)
{
  if (messageIndex < _msgNum.size())
    return _errorMsg[messageIndex].c_str();
  return std::string().c_str();
}

bool IMAPData::isDownloadAttachmentFailed(uint16_t messageIndex, size_t attachmentIndex)
{
  if (messageIndex < _msgNum.size())
  {
    int s = _messageDataInfo[messageIndex].size();
    int id = 0;
    if (s > 0)
    {
      for (int i = 0; i < s; i++)
      {
        if (_messageDataInfo[messageIndex][i]._disposition == ESP32_MAIL_STR_153)
        {
          if (attachmentIndex == id)
            return _messageDataInfo[messageIndex][i]._error;
          id++;
        }
      }
    }
    else
      return false;
  }
  else
    return false;
}
String IMAPData::getDownloadAttachmentFailedReason(uint16_t messageIndex, size_t attachmentIndex)
{
  if (messageIndex < _msgNum.size())
  {
    int s = _messageDataInfo[messageIndex].size();
    int id = 0;
    if (s > 0)
    {
      for (int i = 0; i < s; i++)
      {
        if (_messageDataInfo[messageIndex][i]._disposition == ESP32_MAIL_STR_153)
        {
          if (attachmentIndex == id)
            return _messageDataInfo[messageIndex][i]._downloadError.c_str();
          id++;
        }
      }
    }
    else
      return std::string().c_str();
  }
  else
    return std::string().c_str();
}

bool IMAPData::isDownloadMessageFailed(uint16_t messageIndex)
{
  if (messageIndex < _msgNum.size())
  {
    int s = _messageDataInfo[messageIndex].size();
    bool res = false;
    if (s > 0)
    {
      for (int i = 0; i < s; i++)
      {
        if (_messageDataInfo[messageIndex][i]._disposition == "")
        {
          res |= _messageDataInfo[messageIndex][i]._error;
        }
      }

      return res;
    }
    else
      return false;
  }
  else
    return false;
}
String IMAPData::getDownloadMessageFailedReason(uint16_t messageIndex)
{
  if (messageIndex < _msgNum.size())
  {
    int s = _messageDataInfo[messageIndex].size();
    string res = "";
    if (s > 0)
    {
      for (int i = 0; i < s; i++)
      {
        if (_messageDataInfo[messageIndex][i]._disposition == "")
        {
          if (_messageDataInfo[messageIndex][i]._downloadError != "")
            res = _messageDataInfo[messageIndex][i]._downloadError;
        }
      }

      return res.c_str();
    }
    else
      return std::string().c_str();
  }
  else
    return std::string().c_str();
}

void IMAPData::empty()
{
  std::string().swap(_host);
  std::string().swap(_loginEmail);
  std::string().swap(_loginPassword);
  std::string().swap(_currentFolder);
  std::string().swap(_nextUID);
  std::string().swap(_searchCriteria);
  std::vector<std::string>().swap(_date);
  std::vector<std::string>().swap(_subject);
  std::vector<std::string>().swap(_subject_charset);
  std::vector<std::string>().swap(_from);
  std::vector<std::string>().swap(_from_charset);
  std::vector<std::string>().swap(_to);
  std::vector<std::string>().swap(_to_charset);
  std::vector<std::string>().swap(_cc);
  std::vector<std::string>().swap(_cc_charset);
  std::vector<std::string>().swap(_msgNum);
  std::vector<std::string>().swap(_folders);
  std::vector<std::string>().swap(_flag);
  std::vector<std::string>().swap(_msgID);
  std::vector<std::string>().swap(_acceptLanguage);
  std::vector<std::string>().swap(_contentLanguage);
  std::vector<int>().swap(_attachmentCount);
  std::vector<int>().swap(_totalAttachFileSize);
  std::vector<int>().swap(_downloadedByte);
  std::vector<bool>().swap(_error);
  std::vector<std::vector<messageBodyData>>().swap(_messageDataInfo);
  std::vector<std::string>().swap(_errorMsg);
  readStatusCallback _readCallback = NULL;
  downloadStatusCallback _downloadCallback = NULL;
}

messageBodyData::messageBodyData()
{
}
messageBodyData::~messageBodyData()
{
  std::string().swap(_text);
  std::string().swap(_filename);
  std::string().swap(_name);
  std::string().swap(_disposition);
  std::string().swap(_contentType);
  std::string().swap(_descr);
  std::string().swap(_transfer_encoding);
  std::string().swap(_creation_date);
  std::string().swap(_modification_date);
  std::string().swap(_downloadError);
  std::string().swap(_charset);
  std::string().swap(_part);
}

attachmentData::attachmentData() {}
attachmentData::~attachmentData()
{

  std::vector<std::vector<uint8_t *>>().swap(_buf);
  std::vector<std::string>().swap(_filename);
  std::vector<uint8_t>().swap(_id);
  std::vector<uint8_t>().swap(_type);
  std::vector<uint16_t>().swap(_size);
  std::vector<std::string>().swap(_mime_type);
}

void attachmentData::add(const String &fileName, const String &mimeType, uint8_t *data, uint16_t size)
{
  _filename.push_back(fileName.c_str());
  _mime_type.push_back(mimeType.c_str());

  if (size > 0)
  {
    std::vector<uint8_t *> d = std::vector<uint8_t *>();
    d.push_back(data);
    _buf.push_back(d);
    _size.push_back(size);
    _type.push_back(0);
  }
  else
  {
    _buf.push_back(std::vector<uint8_t *>());
    _size.push_back(0);
    _type.push_back(1);
  }

  _id.push_back(_index);
  _index++;
}

void attachmentData::remove(uint8_t index)
{
  _buf.erase(_buf.begin() + index);
  _filename.erase(_filename.begin() + index);
  _type.erase(_type.begin() + index);
  _size.erase(_size.begin() + index);
  _mime_type.erase(_mime_type.begin() + index);
  _id.erase(_id.begin() + index);
}

void attachmentData::free()
{
  std::vector<std::vector<uint8_t *>>().swap(_buf);
  std::vector<std::string>().swap(_filename);
  std::vector<uint8_t>().swap(_id);
  std::vector<uint8_t>().swap(_type);
  std::vector<uint16_t>().swap(_size);
  std::vector<std::string>().swap(_mime_type);
}

String attachmentData::getFileName(uint8_t index)
{
  return _filename[index].c_str();
}

String attachmentData::getMimeType(uint8_t index)
{
  return _mime_type[index].c_str();
}

uint8_t *attachmentData::getData(uint8_t index)
{
  uint8_t *ptr = _buf[index].front();
  return ptr;
}

uint16_t attachmentData::getSize(uint8_t index)
{
  return _size[index];
}

uint8_t attachmentData::getCount()
{
  return _index;
}

uint8_t attachmentData::getType(uint8_t index)
{
  return _type[index];
}

SMTPData::SMTPData() {}

SMTPData::~SMTPData()
{
  empty();
}

void SMTPData::setLogin(const String &host, uint16_t port, const String &loginEmail, const String &loginPassword)
{

  _host.clear();
  _port = port;
  _loginEmail.clear();
  _loginPassword.clear();

  _host = host.c_str();
  _loginEmail = loginEmail.c_str();
  _loginPassword = loginPassword.c_str();
}

void SMTPData::setSender(const String &fromName, const String &senderEmail)
{

  _fromName.clear();
  _senderEmail.clear();

  _fromName += fromName.c_str();
  _senderEmail += senderEmail.c_str();
}

String SMTPData::getFromName()
{
  return _fromName.c_str();
}

String SMTPData::getSenderEmail()
{
  return _senderEmail.c_str();
}

void SMTPData::setPriority(int priority)
{
  _priority = priority;
}
void SMTPData::setPriority(const String &priority)
{
  if (priority == ESP32_MAIL_STR_205 || priority == ESP32_MAIL_STR_206)
    _priority = 1;
  else if (priority == ESP32_MAIL_STR_207 || priority == ESP32_MAIL_STR_208)
    _priority = 3;
  else if (priority == ESP32_MAIL_STR_209 || priority == ESP32_MAIL_STR_210)
    _priority = 5;
}

uint8_t SMTPData::getPriority()
{
  return _priority;
}

void SMTPData::addRecipient(const String &email)
{
  _recipient.insert(_recipient.end(), email.c_str());
}

void SMTPData::removeRecipient(const String &email)
{
  for (uint8_t i = 0; i < _recipient.size(); i++)
    if (_recipient[i].c_str() == email.c_str())
      _recipient.erase(_recipient.begin() + i);
}

void SMTPData::removeRecipient(uint8_t index)
{
  _recipient.erase(_recipient.begin() + index);
}

void SMTPData::clearRecipient()
{
  std::vector<std::string>().swap(_recipient);
}

uint8_t SMTPData::recipientCount()
{
  return _recipient.size();
}

String SMTPData::getRecipient(uint8_t index)
{
  if (index >= _recipient.size())
    return std::string().c_str();
  return _recipient[index].c_str();
}

void SMTPData::setSubject(const String &subject)
{
  _subject = strdup(subject.c_str());
}

String SMTPData::getSubject()
{
  return _subject.c_str();
}

void SMTPData::setMessage(const String &message, bool htmlFormat)
{
  _message.clear();
  _message += message.c_str();
  _htmlFormat = htmlFormat;
}

String SMTPData::getMessage()
{
  return _message.c_str();
}

bool SMTPData::htmlFormat()
{
  return _htmlFormat;
}
void SMTPData::addCC(const String &email)
{
  _cc.push_back(email.c_str());
}

void SMTPData::removeCC(const String &email)
{
  for (uint8_t i = 0; i < _cc.size(); i++)
    if (_cc[i].c_str() == email.c_str())
      _cc.erase(_cc.begin() + i);
}

void SMTPData::removeCC(uint8_t index)
{
  _cc.erase(_cc.begin() + index);
}
void SMTPData::clearCC()
{
  std::vector<std::string>().swap(_cc);
}

uint8_t SMTPData::ccCount()
{
  return _cc.size();
}

String SMTPData::getCC(uint8_t index)
{
  if (index >= _cc.size())
    return std::string().c_str();
  return _cc[index].c_str();
}

void SMTPData::addBCC(const String &email)
{
  _bcc.push_back(email.c_str());
}

void SMTPData::removeBCC(const String &email)
{
  for (uint8_t i = 0; i < _bcc.size(); i++)
    if (_bcc[i].c_str() == email.c_str())
      _bcc.erase(_bcc.begin() + i);
}

void SMTPData::removeBCC(uint8_t index)
{
  _bcc.erase(_bcc.begin() + index);
}

void SMTPData::clearBCC()
{
  std::vector<std::string>().swap(_bcc);
}

uint8_t SMTPData::bccCount()
{
  return _bcc.size();
}

String SMTPData::getBCC(uint8_t index)
{
  if (index >= _bcc.size())
    return std::string().c_str();
  return _bcc[index].c_str();
}

void SMTPData::addAttachData(const String &fileName, const String &mimeType, uint8_t *data, uint16_t size)
{
  _attach.add(fileName, mimeType, data, size);
}

void SMTPData::removeAttachData(const String &fileName)
{
  for (uint8_t i = 0; i < _attach.getCount(); i++)
    if (_attach.getFileName(i) == fileName && _attach.getType(i) == 0)
    {
      _attach.remove(i);
    }
}

void SMTPData::removeAttachData(uint8_t index)
{
  uint8_t id = 0;
  for (uint8_t i = 0; i < _attach.getCount(); i++)
    if (_attach.getType(i) == 0)
    {
      if (id == index)
      {
        _attach.remove(i);
        break;
      }
      id++;
    }
}

uint8_t SMTPData::attachDataCount()
{
  uint8_t count = 0;
  for (uint8_t i = 0; i < _attach.getCount(); i++)
    if (_attach.getType(i) == 0)
      count++;

  return count;
}

void SMTPData::addAttachFile(const String &filePath, const String &mimeType)
{
  _attach.add(filePath, mimeType, NULL, 0);
}

void SMTPData::removeAttachFile(const String &filePath)
{
  for (uint8_t i = 0; i < _attach.getCount(); i++)
    if (_attach.getFileName(i) == filePath && _attach.getType(i) == 1)
    {
      _attach.remove(i);
    }
}

void SMTPData::removeAttachFile(uint8_t index)
{
  uint8_t id = 0;
  for (uint8_t i = 0; i < _attach.getCount(); i++)
    if (_attach.getType(i) == 1)
    {
      if (id == index)
      {
        _attach.remove(i);
        break;
      }
      id++;
    }
}

void SMTPData::clearAttachData()
{
  for (uint8_t i = 0; i < _attach.getCount(); i++)
    if (_attach.getType(i) == 0)
      _attach.remove(i);
}

void SMTPData::clearAttachFile()
{
  for (uint8_t i = 0; i < _attach.getCount(); i++)
    if (_attach.getType(i) == 1)
      _attach.remove(i);
}

void SMTPData::clearAttachment()
{
  _attach.free();
}

uint8_t SMTPData::attachFileCount()
{
  uint8_t count = 0;
  for (uint8_t i = 0; i < _attach.getCount(); i++)
    if (_attach.getType(i) == 1)
      count++;

  return count;
}

void SMTPData::empty()
{
  std::string().swap(_host);
  std::string().swap(_loginEmail);
  std::string().swap(_loginPassword);
  std::string().swap(_fromName);
  std::string().swap(_senderEmail);
  std::string().swap(_subject);
  std::string().swap(_message);
  clearRecipient();
  clearCC();
  clearBCC();
  clearAttachment();
}

void SMTPData::setSendCallback(sendStatusCallback sendCallback)
{
  _sendCallback = sendCallback;
}

ReadStatus::ReadStatus()
{
}
ReadStatus::~ReadStatus()
{
  empty();
}

String ReadStatus::status()
{
  return _status.c_str();
}
String ReadStatus::info()
{
  return _info.c_str();
}

bool ReadStatus::success()
{
  return _success;
}
void ReadStatus::empty()
{
  std::string().swap(_info);
  std::string().swap(_status);
}

SendStatus::SendStatus()
{
}

SendStatus::~SendStatus()
{
  empty();
}

String SendStatus::info()
{
  return _info.c_str();
}
bool SendStatus::success()
{
  return _success;
}
void SendStatus::empty()
{
  std::string().swap(_info);
}

ESP32_MailClient MailClient = ESP32_MailClient();
