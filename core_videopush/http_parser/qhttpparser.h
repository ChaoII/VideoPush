#pragma once

#include <QHash>
#include <QObject>

struct http_parser;
struct http_parser_settings;
typedef QHash<QByteArray, QByteArray> HeaderHash;

class QHttpParser : public QObject {
Q_OBJECT

public:
    explicit QHttpParser(QObject *parent = nullptr);

    ~QHttpParser();

    //解析请求参数
    bool parserRequest(const QByteArray &buf);

    //解析返回参数
    bool parserResponse(const QByteArray &buf);

    QByteArray url() const;

    QByteArray body() const;

    QByteArray method() const;

    HeaderHash headers() const;

    int statusCode() const;

    QString httpVersion() const;

    QByteArray headerValue(const QByteArray &headerName) const;

private:
    static int MessageBegin(http_parser *parser);

    static int MyUrl(http_parser *parser, const char *at, size_t length);

    static int HeaderField(http_parser *parser, const char *at, size_t length);

    static int HeaderValue(http_parser *parser, const char *at, size_t length);

    static int HeadersComplete(http_parser *parser);

    static int Body(http_parser *parser, const char *at, size_t length);

    static int MessageComplete(http_parser *parser);

private:
    bool m_headersFinish;
    bool m_messageFinish;

    QByteArray m_body;
    QByteArray m_method;
    QByteArray m_currentUrl;
    HeaderHash m_currentHeaders;
    QByteArray m_currentHeaderField;

    http_parser *m_parser;
    http_parser_settings *m_parserSettings;
};

