#include <config.hpp>
#include <utils.hpp>

#include <QFile>
#include <QIODevice>
#include <QJsonDocument>
#include <QJsonValue>
#include <QLocale>
#include <QObject>

constexpr const char *conf_fn = "conf.json";

void Config::set_port(int p) {
    json["port"] = p;
}

void Config::set_port_with_check(int p) {
    set_port(check_port(p) ? p : port_default);
}

QString Config::parse_songs_path(const QString &s) {
    return s.isEmpty() ? detect_songs() : s;
}

QString Config::load_lang(const QString &s) {
    // if (QString sl = s.toLower(); sl != QStringLiteral("en"))
    QString sl = s.toLower();
    for (auto &s : language_codes)
        if (s.toLower() == sl) {
            if (tran.load(s, QM_DIR))
                return s;
            warning_box(QStringLiteral("Missing translation file."));
            break;
        }
    return QStringLiteral("en");
}

QString Config::detect_lang(const QLocale &locale) {
    return tran.load(locale, QString(), QString(), QM_DIR) ? dbg(233), tran.language() : QStringLiteral("en");
}

void Config::write() const {
    dbg("writing conf");
    if (QFile fp(conf_fn); fp.open(QIODevice::WriteOnly | QIODevice::Text))
        fp.write(QJsonDocument(json).toJson());
    else
        warning_box(QObject::tr("Failed to save the configuration file."));
}

QJsonDocument Config::read() {
    if (QFile fp(conf_fn); fp.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString s = fp.readAll();
        fp.close();
        return QJsonDocument::fromJson(s.toUtf8());
    }
    return QJsonDocument();
}

Config::Config(const QLocale &locale) {
    if (QJsonDocument doc; (doc = read()).isEmpty()) {
        set_port(port_default);
        json["songs_path"] = detect_songs();
        json["language"] = detect_lang(locale);
    } else {
        QJsonObject obj = doc.object();
        QJsonValue port = obj["port"], songs = obj["songs_path"], lang = obj["language"];
        if (port.isDouble())
            set_port_with_check(port.toInt());
        else if (port.isString())
            set_port_with_check(port.toString().toInt());
        else
            set_port(port_default);

        json["songs_path"] = songs.isString() ? parse_songs_path(songs.toString().trimmed()) : detect_songs();
        json["language"] = lang.isString() ? load_lang(lang.toString()) : detect_lang(locale);
    }
    dbg(json);
    write();
}

QJsonValue Config::operator [] (const QString &k) const {
    return json[k];
}

void Config::update(const QString &k, const QJsonValue &v) {
    with(mtx_update, {
        json[k] = v;
        write();
    });
}

QTranslator &Config::translator() {
    return tran;
}
