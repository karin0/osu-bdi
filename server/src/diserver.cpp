#include <diserver.hpp>
#include <utils.hpp>
#include "ui_diserver.h"

void DIServer::set_port(int p) {
    port = p;
    refresh_port();
}

void DIServer::set_songs_path(const QString &s) {
    songs_path = s;
    refresh_songs_path();
}

DIServer::DIServer(QWidget *parent, Config *conf)
    : QWidget(parent), ui(new Ui::DIServer), conf(conf), port_validator(PortValidator(this)), translator(&conf->translator()), retranslated(false),
      serv(WSServer("diserv", WSServer::NonSecureMode, this)) {
    QString lang = (*conf)["language"].toString().toLower();
    init_ui(lang);

    set_songs_path((*conf)["songs_path"].toString());
    set_port((*conf)["port"].toInt());

    watch_songs();
    start();
    connect(&watcher_songs, &QFileSystemWatcher::directoryChanged, this, &DIServer::songs_changed_handler);
}

DIServer::~DIServer() {
    destruct_ui();
}

void DIServer::update_port(int p) {
    with(mtx_update_port, {
        set_port(p);
        conf->update("port", p);
        start();
    });
}

void DIServer::update_port_without_refresh(int p) {
    with(mtx_update_port, {
        port = p;
        conf->update("port", p);
        start();
    });
}

void DIServer::update_songs_path(const QString &s) {
    with(mtx_update_songs_path, {
        set_songs_path(s);
        conf->update("songs_path", s);
        watch_songs();
    });
}

void DIServer::update_songs_path_without_refresh(const QString &s) {
    with(mtx_update_songs_path, {
        songs_path = s;
        conf->update("songs_path", s);
        watch_songs();
    });
}

void DIServer::update_lang(const QString &s) {
    conf->update("language", s);
}
