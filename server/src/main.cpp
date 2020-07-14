#include <diserver.hpp>
#include <utils.hpp>

#include <QApplication>
#include <QSharedMemory>
#include <QLocale>

#ifdef Q_OS_WIN
#include <QFont>
#include <QFontDatabase>
#endif

int main(int argc, char **argv) {
    QApplication a(argc, argv);

    QSharedMemory sm;
    sm.setKey("DIServer");
    if (!sm.create(1)) {
        QMessageBox::critical(nullptr, QStringLiteral("DIServer"), QObject::tr("Another instance is running."));
        a.exit();
        return 0;
    }

    QLocale locale;

#ifdef Q_OS_WIN
    if (locale.language() == QLocale::Chinese) {
        QFont font;
        QFontDatabase fdb;
        font = fdb.font("Microsoft Yahei UI", font.styleName(), font.pointSize());
        if (font.family() != "Microsoft Yahei UI")
            font = fdb.font("微软雅黑", font.styleName(), font.pointSize());
        a.setFont(font);
    }
#endif

    Config conf(locale);
    a.installTranslator(&conf.translator());

    DIServer w(nullptr, &conf);
    w.show();
    return a.exec();
}
