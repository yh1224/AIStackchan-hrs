#if !defined(LIB_CHATGPT_CLIENT_H)
#define LIB_CHATGPT_CLIENT_H

#include <deque>
#include <vector>
#include <Arduino.h>

class ChatGptClientError : public std::exception {
public:
    explicit ChatGptClientError(const char *msg) : _msg(msg) {};

    const char *what() { return _msg; }

private:
    const char *_msg;
};

class ChatGptHttpError : public ChatGptClientError {
public:
    ChatGptHttpError(int code, const char *msg) : _statusCode(code), ChatGptClientError(msg) {};

    int statusCode() const { return _statusCode; }

private:
    int _statusCode;
};

class ChatGptClient {
public:
    explicit ChatGptClient(String apiKey);

    String chat(
            const String &data, const std::vector<String> &roles, const std::deque<String> &history,
            const std::function<void(const String &)> &onReceiveContent);

private:
    String _apiKey;

    String _httpPost(
            const String &url, const String &body,
            const std::function<void(const String &)> &onReceiveData);
};

#endif // !defined(LIB_CHATGPT_CLIENT_H)