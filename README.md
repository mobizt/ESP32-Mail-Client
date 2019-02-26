# Email Client Arduino Library for ESP32 v 1.0.1

This library provides ESP32 to send/receive Email with or without attachment through the SMTP/IMAP mail server. 

The library was test and work well with ESP32s based module.

Copyright (c) 2019 K. Suwatchai (Mobizt).

## Tested Devices

This library works well in the following tested devices.

 * Sparkfun ESP32 Thing
 * NodeMCU-32
 * WEMOS LOLIN32
 
## Features

Able to send Email with large file attachment.

Able to receive Emails, download and save messages and attachments to SD card.

Support large file attachment to send Email or download from received Email.

Able to search Email using IMAP command as in RFC 3501 (depending on IMAP server implementation). 

Fetch Email is stricted to accept only message UID.

## Known Issues

Currently unable to connect to some SMTP/IMAP servers e.g. outlook.com due to some bugs in mbedTLS used in ESP32 Arduino core library.


## Dependencies

Required HTTPClientESP32Ex library to be installed. https://github.com/mobizt/HTTPClientESP32Ex

## Prerequisites

To send Email using Gmail outgoing Email service, less secure app option should be enabled. https://myaccount.google.com/lesssecureapps?pli=1

To receive Email using Gmail incoming Email service, IMAP option should be enabled. https://support.google.com/mail/answer/7126229?hl=en


## Supported functions

```
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
    
    ```
    
    ## IMAPData object call for receiving Email.
    
    ```


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


    ## SMTPData object call for sending Email

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


```


## Usages

See the examples




