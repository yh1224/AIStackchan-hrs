#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

#include <utility>

#include "lib/ChatGptClient.h"
#include "lib/utils.h"

/// GPT model
static const char *CHATGPT_MODEL = "gpt-3.5-turbo";

/// Chat API URL
static const char *CHAT_URL = "https://api.openai.com/v1/chat/completions";

/// size for request/response
static const size_t CONTENT_MAX_SIZE = 16 * 1024;

/// certificate for https://api.openai.com
/// Baltimore CyberTrust Root, valid until Wed Jan 01 2025, size: 1379 bytes
static const char *rootCACert = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIDzTCCArWgAwIBAgIQCjeHZF5ftIwiTv0b7RQMPDANBgkqhkiG9w0BAQsFADBa\n" \
"MQswCQYDVQQGEwJJRTESMBAGA1UEChMJQmFsdGltb3JlMRMwEQYDVQQLEwpDeWJl\n" \
"clRydXN0MSIwIAYDVQQDExlCYWx0aW1vcmUgQ3liZXJUcnVzdCBSb290MB4XDTIw\n" \
"MDEyNzEyNDgwOFoXDTI0MTIzMTIzNTk1OVowSjELMAkGA1UEBhMCVVMxGTAXBgNV\n" \
"BAoTEENsb3VkZmxhcmUsIEluYy4xIDAeBgNVBAMTF0Nsb3VkZmxhcmUgSW5jIEVD\n" \
"QyBDQS0zMFkwEwYHKoZIzj0CAQYIKoZIzj0DAQcDQgAEua1NZpkUC0bsH4HRKlAe\n" \
"nQMVLzQSfS2WuIg4m4Vfj7+7Te9hRsTJc9QkT+DuHM5ss1FxL2ruTAUJd9NyYqSb\n" \
"16OCAWgwggFkMB0GA1UdDgQWBBSlzjfq67B1DpRniLRF+tkkEIeWHzAfBgNVHSME\n" \
"GDAWgBTlnVkwgkdYzKz6CFQ2hns6tQRN8DAOBgNVHQ8BAf8EBAMCAYYwHQYDVR0l\n" \
"BBYwFAYIKwYBBQUHAwEGCCsGAQUFBwMCMBIGA1UdEwEB/wQIMAYBAf8CAQAwNAYI\n" \
"KwYBBQUHAQEEKDAmMCQGCCsGAQUFBzABhhhodHRwOi8vb2NzcC5kaWdpY2VydC5j\n" \
"b20wOgYDVR0fBDMwMTAvoC2gK4YpaHR0cDovL2NybDMuZGlnaWNlcnQuY29tL09t\n" \
"bmlyb290MjAyNS5jcmwwbQYDVR0gBGYwZDA3BglghkgBhv1sAQEwKjAoBggrBgEF\n" \
"BQcCARYcaHR0cHM6Ly93d3cuZGlnaWNlcnQuY29tL0NQUzALBglghkgBhv1sAQIw\n" \
"CAYGZ4EMAQIBMAgGBmeBDAECAjAIBgZngQwBAgMwDQYJKoZIhvcNAQELBQADggEB\n" \
"AAUkHd0bsCrrmNaF4zlNXmtXnYJX/OvoMaJXkGUFvhZEOFp3ArnPEELG4ZKk40Un\n" \
"+ABHLGioVplTVI+tnkDB0A+21w0LOEhsUCxJkAZbZB2LzEgwLt4I4ptJIsCSDBFe\n" \
"lpKU1fwg3FZs5ZKTv3ocwDfjhUkV+ivhdDkYD7fa86JXWGBPzI6UAPxGezQxPk1H\n" \
"goE6y/SJXQ7vTQ1unBuCJN0yJV0ReFEQPaA1IwQvZW+cwdFD19Ae8zFnWSfda9J1\n" \
"CZMRJCQUzym+5iPDuI9yP+kHyCREU3qzuWFloUwOxkgAyXVjBYdwRVKD05WdRerw\n" \
"6DEdfgkfCv4+3ao8XnTSrLE=\n" \
"-----END CERTIFICATE-----\n" \
"";

ChatGptClient::ChatGptClient(String apiKey) : _apiKey(std::move(apiKey)) {}

/**
 * Ask to the ChatGPT and get answer
 *
 * @param text question
 * @param roles roles
 * @param history chat history
 * @param onReceiveContent callback on receive content (use stream if present)
 * @return answer (nullptr: failure)
 * @throws ChatGptClientError
 */
String ChatGptClient::chat(
        const String &text, const std::vector<String> &roles, const std::deque<String> &history,
        const std::function<void(const String &)> &onReceiveContent) {
    DynamicJsonDocument requestDoc{CONTENT_MAX_SIZE};
    DynamicJsonDocument responseDoc{CONTENT_MAX_SIZE};
    requestDoc["model"] = CHATGPT_MODEL;
    if (onReceiveContent != nullptr) {
        requestDoc["stream"] = true;
    }
    JsonArray messages = requestDoc.createNestedArray("messages");
    // Append roles to request parameters
    for (auto &&role: roles) {
        JsonObject newMessage = messages.createNestedObject();
        newMessage["role"] = "system";
        newMessage["content"] = role;
    }
    // Append chat history to request parameters
    for (int i = 0; i < history.size(); i++) {
        JsonObject newMessage = messages.createNestedObject();
        newMessage["role"] = (i % 2 == 0) ? "user" : "assistant";
        newMessage["content"] = history[i];
    }
    // Append question to request parameters
    JsonObject newMessage = messages.createNestedObject();
    newMessage["role"] = "user";
    newMessage["content"] = text;

    if (onReceiveContent != nullptr) {
        std::stringstream ss;
        _httpPost(CHAT_URL, jsonEncode(requestDoc), [&](const String &data) {
            // Handle server-sent event
            // https://developer.mozilla.org/en-US/docs/Web/API/Server-sent_events/Using_server-sent_events#event_stream_format
            if (data == "[DONE]") {
                return;
            }
            auto error = deserializeJson(responseDoc, data.c_str());
            if (error != DeserializationError::Ok) {
                Serial.printf("ERROR: Failed to deserialize JSON: %s\n", error.c_str());
                throw ChatGptClientError("Failed to deserialize JSON");
            }
            const char *content = responseDoc["choices"][0]["delta"]["content"];
            if (content != nullptr) {
                onReceiveContent(content);
                ss << content;
            }
        });
        return String{ss.str().c_str()};
    } else {
        auto result = _httpPost(CHAT_URL, jsonEncode(requestDoc), nullptr);
        auto error = deserializeJson(responseDoc, result.c_str());
        if (error != DeserializationError::Ok) {
            Serial.printf("ERROR: Failed to deserialize JSON: %s\n", error.c_str());
            throw ChatGptClientError("Failed to deserialize JSON");
        }
        auto content = responseDoc["choices"][0]["message"]["content"];
        if (content == nullptr) {
            throw ChatGptClientError("No content");
        }
        return content.as<String>();
    }
}

/**
 * Read chunk
 *
 * @param stream WifiClient stream
 * @param onReceiveChunk callback on receive chunk
 * @return true: success, false: failure
 */
static bool readChunk(WiFiClient *stream, const std::function<bool(const char *)> &onReceiveChunk) {
    char buf[16];
    while (true) {
        // Read chunk size
        int chunkSize;
        int pos = 0;
        while (true) {
            delay(10);
            auto available = stream->available();
            if (available == 0) {
                continue;
            }
            auto c = stream->read();
            if (c < 0) {
                Serial.printf("readChunk: read failed (%d)\n", c);
                return false;
            }
            buf[pos++] = (char) c;
            if (pos > 8) {  // hex 6 桁まで
                Serial.println("readChunk: Invalid chunk size (too long)");
                return false;
            }
            if (pos >= 2 && buf[pos - 2] == '\r' && buf[pos - 1] == '\n') {
                buf[pos - 2] = '\0';
                //Serial.printf("readChunk: chunkSize=%s\n", buf);
                char *endp;
                chunkSize = strtol((char *) buf, &endp, 16);
                if (endp != (char *) &buf[pos - 2]) {
                    Serial.printf("readChunk: Invalid chunk size: %s\n", buf);
                    return -1;
                }
                //Serial.printf("readChunk: chunkSize=%d\n", chunkSize);
                break;
            }
        }
        if (chunkSize == 0) {
            //Serial.println("readChunk: stream end");
            break;
        }

        // Read chunk
        auto chunkData = std::unique_ptr<char>((char *) malloc(chunkSize + 1));
        char *p = chunkData.get();
        int rest = chunkSize;
        while (rest > 0) {
            delay(10);
            auto len = stream->readBytes(p, rest);
            p += len;
            rest -= (int) len;
        }
        *p = '\0';
        //Serial.printf("readChunk: chunkData=[[[%s]]]\n", chunkData.get());

        // Skip chunk delimiter
        auto len = stream->readBytes(buf, 2);
        if (len != 2 || buf[0] != '\r' || buf[1] != '\n') {
            Serial.println("readChunk: Invalid chunk delimiter");
            return false;
        }

        if (!onReceiveChunk((const char *) chunkData.get())) {
            return false;
        }
    }
    return true;
}

/**
 * Read data of Server-Sent Events
 *
 * @param stream WifiClient stream
 * @param onReceiveData callback on receive data
 * @return true: success, false: failure
 */
static bool readData(WiFiClient *stream, const std::function<void(const std::string &)> &onReceiveData) {
#define SSE_DATA_PREFIX "data: "
#define SSE_DATA_DELIMITER "\n\n"  // TODO: or "\r\n\r\n" ?
    std::stringstream ss;
    return readChunk(stream, [&](const std::string &chunk) {
        ss << chunk;
        //Serial.printf("readData: Read chunk: %d\n", chunk.length());

        while (true) {
            std::string str = ss.str();
            //Serial.printf("readData: str=[[[%s]]] ", str.c_str());
            //for (int i = 0; i < str.length(); i++) {
            //    Serial.printf("%02x", str[i]);
            //}
            //Serial.println();

            auto dataEnd = str.find(SSE_DATA_DELIMITER);
            //Serial.printf("readData: dataEnd=%d\n", dataEnd);
            if (dataEnd == std::string::npos) {
                break;
            }
            std::string nextData = str.substr(dataEnd + strlen(SSE_DATA_DELIMITER));
            std::string oneData = str.substr(strlen(SSE_DATA_PREFIX), dataEnd - strlen(SSE_DATA_PREFIX));
            //Serial.printf("readData: data=[[[%s]]]\n", data.c_str());
            //Serial.printf("readData: tail=[[[%s]]]\n", tail.c_str());
            onReceiveData(oneData);
            ss.clear();
            ss.str(nextData);
        }
        return true;
    });
}

/**
 * HTTP POST
 *
 * @param url URL
 * @param body body
 * @param onReceiveData callback on receive data
 * @return response body
 * @throws ChatGptClientError
 */
String ChatGptClient::_httpPost(
        const String &url, const String &body,
        const std::function<void(const String &)> &onReceiveData) {
    HTTPClient http;
    http.setReuse(false);
    http.setTimeout(60000);
    static const char *headerKeys[] = {"Content-Type", "Transfer-Encoding"};
    http.collectHeaders(headerKeys, 2);

    WiFiClientSecure client;
    client.setCACert(rootCACert);
    if (http.begin(client, url)) {
        http.addHeader("Content-Type", "application/json");
        http.addHeader("Authorization", String("Bearer ") + _apiKey);
        Serial.printf(">>> POST %s\n", url.c_str());
        Serial.printf("%s\n", body.c_str());
        int httpCode = http.POST((uint8_t *) body.c_str(), body.length());
        if (httpCode != HTTP_CODE_OK) {
            Serial.println(HTTPClient::errorToString(httpCode).c_str());
            throw ChatGptHttpError(httpCode, ("HTTP client error: " + String(httpCode)).c_str());
        }

        Serial.printf("<<< %d\n", httpCode);
        String payload;
        if (onReceiveData != nullptr
            && http.header("Content-Type") == "text/event-stream"
            && http.header("Transfer-Encoding") == "chunked") {
            // Receive Event Stream
            std::stringstream ss;
            auto result = readData(http.getStreamPtr(), [&](const std::string &data) {
                onReceiveData(data.c_str());
                ss << data;
            });
            if (!result) {
                throw ChatGptClientError("Failed to receive data");
            }
            payload = String(ss.str().c_str());
        } else {
            payload = http.getString();
        }
        http.end();
        return payload;
    } else {
        throw ChatGptClientError("HTTP begin failed");
    }
}
