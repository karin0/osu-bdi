#ifndef DISERVER_HPP
#define DISERVER_HPP

#include "config.hpp"
#include "utils.hpp"

#include <QFileSystemWatcher>
#include <QLabel>
#include <QSet>
#include <QWebSocketServer>
#include <QWidget>
#include <QSvgWidget>

QT_BEGIN_NAMESPACE
namespace Ui {
    class DIServer;
}
QT_END_NAMESPACE

typedef QWebSocketServer WSServer;

class DIServer : public QWidget {
    Q_OBJECT

  public:
    DIServer(QWidget *, Config *);
    ~DIServer();

  private:
    Ui::DIServer *ui;

    Config *conf;
    PortValidator port_validator;
    QTranslator *translator;
    bool retranslated;

    QWebSocketServer serv;
    QString songs_path;
    port_t port;

    QString msg_beatmaps_songs;
    QSet<QWebSocket *> clients;
    QSet<int> beatmaps_songs;
    QFileSystemWatcher watcher_songs;

    QMutex mtx_clients, mtx_beatmaps_songs, mtx_scan_songs, mtx_msg_beatmaps_songs, mtx_watcher_songs,
        mtx_update_songs_path, mtx_update_port;

    void set_port(int);
    void set_songs_path(const QString &);

    void update_port(int);
    void update_port_without_refresh(int);
    void update_songs_path(const QString &);
    void update_songs_path_without_refresh(const QString &);
    void update_lang(const QString &);

    void watch_songs();
    void start();
    bool listen();
    void close();

    void init_ui(const QString &);
    void destruct_ui();

    void refresh_info();

    bool is_songs_path_valid, is_listening;
    void refresh_songs_tip();
    void refresh_port_tip();

    void refresh_port();
    void refresh_songs_path();

  private slots:
    void lang_changed_handler(int);
    void songs_changed_handler(const QString &);
    void connected_handler();
    void disconnected_handler();

    void on_port_edit_editingFinished();
    void on_songs_edit_editingFinished();
    void on_port_set_default_button_clicked();
    void on_songs_browse_button_clicked();
    void on_songs_detect_button_clicked();
    void on_about_button_clicked();
    void on_get_userscript_button_clicked();
};

#endif  // DISERVER_HPP
