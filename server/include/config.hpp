#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <QJsonObject>
#include <QLocale>
#include <QMutex>
#include <QTranslator>

typedef quint16 port_t;

class Config {
  public:
    Config(const QLocale &);

    QTranslator &translator();
    QJsonValue operator[](const QString &) const;
    void update(const QString &, const QJsonValue &);

  private:
    static constexpr const char *conf_fn = "conf.json";

    QJsonObject json;
    QTranslator tran;
    QMutex mtx_update;

    static QJsonDocument read();
    void write() const;

    void set_port(int);
    void set_port_with_check(int);

    static QString parse_songs_path(const QString &);
    QString load_lang(const QString &);
    QString detect_lang(const QLocale &);
};

#endif  // CONFIG_HPP
