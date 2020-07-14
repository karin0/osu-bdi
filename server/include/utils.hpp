#ifndef UTILS_HPP
#define UTILS_HPP

#include <QDebug>
#include <QMessageBox>
#include <QPair>
#include <QString>
#include <QVector>
#include <QIntValidator>

constexpr int port_default = 35677, port_min = 0, port_max = 65535;
extern const QString port_default_str, port_max_str;

// constexpr const char *ok_tip = u8"\u2705", *warning_tip = u8"\u26a0", *error_tip = u8"\u274c";
extern const QByteArray ok_svg, warning_svg, error_svg;

#define QM_DIR ":/i18n"
#define check_port(p) ((p) <= port_max)
#define warning_box(s) (QMessageBox::warning(nullptr, "DIServer", s))
#define with(mtx, body) \
    mtx.lock();         \
    body;               \
    mtx.unlock();


extern QVector<QString> language_codes;
QVector<QPair<QString, QString>> get_languages();

QString detect_songs();

struct PortValidator : public QIntValidator {
    PortValidator(QObject * = nullptr);
    virtual void fixup(QString &) const;
};

#ifdef QT_DEBUG

struct Debug {
    QDebug *s;
    Debug(QDebug *s)
        : s(s) {}

    template <typename T>
    void f(const T &t) {
        *s << t;
    }
    template <typename T, typename... Ts>
    void f(const T &t, Ts... args) {
        s = &(*s << t);
        f(args...);
    }
};

template <typename... Ts>
void dbg(Ts... args) {
    QDebug d = qDebug();
    Debug(&d).f(args...);
}

#else

#define dbg(...) 0

#endif

#endif  // UTILS_HPP
