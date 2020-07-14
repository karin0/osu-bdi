#include <utils.hpp>

#include <QByteArray>
#include <QDir>
#include <QSettings>
#include <QWidget>
#include <QPair>

/*
const QString ok_svg = QStringLiteral(":/svgs/red.svg"),
              warning_svg = QStringLiteral(":/svgs/exclamation-triangle.svg"),
              error_svg = QStringLiteral(":/svgs/times.svg");
*/

const QString port_default_str = QStringLiteral("35677"), port_max_str = QStringLiteral("65535");

static QByteArray generate_indicator(const QByteArray &color) {
    static const QByteArray l = "<svg width=\"30\" height=\"30\"><rect x=\"5\" y=\"5\" width=\"20\" height=\"20\" style=\"fill:",
                            r = ";stroke:black;stroke-width:1.5;stroke-opacity:0.8\"/></svg>";
    return l + color + r;
}

const QByteArray ok_svg = generate_indicator("#2ECC71"),
                 warning_svg = generate_indicator("#F1C40F"),
                 error_svg = generate_indicator("#E74C3C");

QVector<QString> language_codes = {"en", "zh_CN", "zh_TW"};

QVector<QPair<QString, QString>> get_languages() {
    QVector<QPair<QString, QString>> langs;
    langs.push_back(qMakePair(QStringLiteral("en"), QObject::tr("English")));
    langs.push_back(qMakePair(QStringLiteral("zh_CN"), QObject::tr("Simplified Chinese")));
    langs.push_back(qMakePair(QStringLiteral("zh_TW"), QObject::tr("Traditional Chinese")));
    return langs;
}

PortValidator::PortValidator(QObject *parent)
    : QIntValidator(port_min, port_max, parent) {}

void PortValidator::fixup(QString &s) const {
    s = s.isEmpty() ? port_default_str : port_max_str;
}

QString detect_songs() {
#ifdef Q_OS_WIN

#define chk(cond) \
    if (cond) return QString();

    QString path = QSettings("HKEY_CLASSES_ROOT\\osu\\shell\\open\\command", QSettings::Registry64Format)
                       .value("Default")
                       .toString();
    chk(path.isEmpty());
    if (path[0] == '"') {
        int p = path.indexOf('"', 1);
        chk(p < 0);
        path = path.mid(1, p - 1);
    } else {
        int p = path.indexOf(' ');
        if (p >= 0)
            path = path.left(p);
    }
    chk(path.isEmpty());
    QFileInfo info(path);
    chk(!info.exists());
    QDir dir = info.dir();
    chk(!dir.cd("Songs"));
    return dir.absolutePath();
#undef chk

#else
    return QString();
#endif
}
