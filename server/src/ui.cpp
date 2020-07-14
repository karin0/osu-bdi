#include <diserver.hpp>
#include <utils.hpp>
#include "ui_diserver.h"

#include <QDir>
#include <QFileDialog>
#include <QRect>
#include <QLocale>
#include <QFont>
#include <QDesktopServices>
#include <QUrl>

static void set_tip(QSvgWidget *l, const QByteArray &data, const QString &tip) {
    l->load(data);
    l->setToolTip(tip);
}

static void init_tip(QSvgWidget *l) {
    l->load(warning_svg);
    l->setFixedSize(25, 25);
}

void DIServer::init_ui(const QString &lang) {
    setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);

    ui->setupUi(this);

    constexpr int pw = 710, ph = 140, side = 30;

#ifndef Q_OS_WIN
    int row, column, row_span, column_span, browse_index;
    for (int i = 0, n = ui->grid_layout->count(); i < n; ++i) {
        auto *item = ui->grid_layout->itemAt(i);
        if (!item)
            continue;
        const auto *widget = item->widget();
        if (widget == ui->songs_detect_button) {
            ui->grid_layout->getItemPosition(i, &row, &column, &row_span, &column_span);
            ui->grid_layout->takeAt(i);
        } else if (widget == ui->songs_browse_button)
            browse_index = i;
    }
    ui->grid_layout->takeAt(browse_index);
    ui->songs_detect_button->setVisible(false);
    ui->grid_layout->addWidget(ui->songs_browse_button, row, column);
#endif

    init_tip(ui->songs_tip_svg);
    init_tip(ui->port_tip_svg);

    setFixedSize(pw, ph);
    const int gw = pw - side, gh = ph - side;
    ui->gridLayoutWidget->setGeometry(QRect((pw - gw + 1) / 2.0, (ph - gh + 1) / 2.0, gw, gh));

    ui->port_edit->setValidator(&port_validator);

    bool index_set = false;
    const QVector<QPair<QString, QString>> &langs = get_languages();
    for (int i = 0; i < langs.length(); ++i) {
        dbg(translator);
        ui->lang_combo->addItem(langs[i].second, langs[i].first);
        if (!index_set && langs[i].first.toLower() == lang) {
            ui->lang_combo->setCurrentIndex(i);
            index_set = true;
        }
    }

    connect(ui->lang_combo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &DIServer::lang_changed_handler);
}

void DIServer::destruct_ui() {
    if (retranslated)
        delete translator;
    delete ui;
}

void DIServer::refresh_port() {
    ui->port_edit->setText(QString::number(port));
}

void DIServer::refresh_songs_path() {
    ui->songs_edit->setText(QDir::toNativeSeparators(songs_path));
}

void DIServer::refresh_songs_tip() {
    if (!is_songs_path_valid)
        set_tip(ui->songs_tip_svg, error_svg, tr("Invalid path"));
    else if (int s; (s = beatmaps_songs.size()))
        set_tip(ui->songs_tip_svg, ok_svg, tr("%1 beatmap(s) detected").arg(QString::number(s)));
    else
        set_tip(ui->songs_tip_svg, warning_svg, tr("No beatmaps detected"));
}

void DIServer::refresh_port_tip() {
    if (!is_listening)
        set_tip(ui->port_tip_svg, error_svg, tr("Failed to bind to this port"));
    else if (port == port_default)
        set_tip(ui->port_tip_svg, ok_svg, QString());
    else
        set_tip(ui->port_tip_svg, warning_svg, tr("A custom port must be configured on the web page"));
}

void DIServer::on_port_edit_editingFinished() {
    dbg("port edited");
    int port_new = ui->port_edit->text().toInt();
    dbg("new port:", port_new);
    if (port_new != port)  // do not check it
        update_port_without_refresh(port_new);
}

void DIServer::on_port_set_default_button_clicked() {
    dbg("psd button clicked");
    if (port != port_default)
        update_port(port_default);
}

void DIServer::on_songs_edit_editingFinished() {
    dbg("songs edited");
    QString songs_path_new = ui->songs_edit->text().trimmed();
    if (songs_path_new != songs_path)
        update_songs_path_without_refresh(songs_path_new);
}

void DIServer::on_songs_browse_button_clicked() {
    QString path = QFileDialog::getExistingDirectory(this, tr("Locate \"Songs\" directory from osu! stable installation"), songs_path, QFileDialog::ShowDirsOnly);
    if (!path.isEmpty() && songs_path != path)
        update_songs_path(path);
}

void DIServer::on_songs_detect_button_clicked() {
    if (QString path = detect_songs(); path.isEmpty())
        warning_box(tr("Failed to find osu! stable installation."));
    else if (path != songs_path)
        update_songs_path(path);
}

void DIServer::on_about_button_clicked() {
    QMessageBox::about(this, tr("About") + " DIServer",
                       "Server of <a href=\"https://github.com/karin0/osu-bdi\">osu! Beatmap Downloaded Indicator</a><br /><br />"
                       + tr("Build time: ") + __DATE__ " " __TIME__);
}

void DIServer::on_get_userscript_button_clicked() {
    QDesktopServices::openUrl(QUrl(QStringLiteral("https://greasyfork.org/scripts/407062-osu-beatmap-downloaded-indicator")));
}

void DIServer::lang_changed_handler(int index) {
    QString lang;
    QTranslator *tran = new QTranslator;

    if (index) {
        if (!tran->load(lang = ui->lang_combo->currentData().toString(), QM_DIR)) {
            warning_box("Missing translation file.");
            delete tran;
            return ui->lang_combo->setCurrentIndex(0);
        }
    } else
        lang = "en";

    qApp->removeTranslator(translator);
    if (retranslated)
        delete translator;
    else
        retranslated = true;
    qApp->installTranslator(translator = tran);
    ui->retranslateUi(this);
    refresh_songs_tip();
    refresh_port_tip();
    update_lang(lang);
}
