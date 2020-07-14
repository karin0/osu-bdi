#include <diserver.hpp>
#include <utils.hpp>

#include <QDir>
#include <QStringRef>
#include <QWebSocket>

typedef QWebSocket WS;

int to_beatmap_id(const QString &s, QStringRef &ref) {
    int p = 0, l = s.length();
    while (p < l && s[p].isDigit()) ++p;
    return (ref = s.leftRef(p)).toInt();
}

void DIServer::watch_songs() {
    with(mtx_watcher_songs, {
        watcher_songs.removePaths(watcher_songs.directories());
        if ((is_songs_path_valid = !songs_path.isEmpty() && QDir(songs_path).exists())) {
            with(mtx_beatmaps_songs, {
                beatmaps_songs.clear();
                QDir songs(songs_path);
                QStringRef idref;
                with(mtx_msg_beatmaps_songs, {
                    msg_beatmaps_songs = QStringLiteral("+");
                    for (const QString &dir : songs.entryList(QDir::Dirs | QDir::NoDotAndDotDot))
                        if (int id; (id = to_beatmap_id(dir, idref))) {
                            beatmaps_songs.insert(id);
                            (msg_beatmaps_songs += ' ') += idref;
                        }
                    with(mtx_clients, {
                        for (WS *client : clients)
                            client->sendTextMessage(QStringLiteral("/ ") + msg_beatmaps_songs);
                    });
                });
            });
            watcher_songs.addPath(songs_path);
        } else {
            with(mtx_beatmaps_songs, beatmaps_songs.clear());
            with(mtx_msg_beatmaps_songs, msg_beatmaps_songs.clear());
            with(mtx_clients, {
                for (WS *client : clients)
                    client->sendTextMessage(QStringLiteral("/"));
            });
        }
    });
    refresh_songs_tip();
}

void DIServer::songs_changed_handler(const QString &) {
    with(mtx_scan_songs, {
        QDir songs(songs_path);
        QSet<int> beatmaps_new;
        QString msg = "+";
        QStringRef idref;
        bool changed = false;

        with(mtx_msg_beatmaps_songs, {
            msg_beatmaps_songs = "+";
            for (const QString &dir : songs.entryList(QDir::Dirs | QDir::NoDotAndDotDot))
                if (int id; (id = to_beatmap_id(dir, idref))) {
                    beatmaps_new.insert(id);
                    (msg_beatmaps_songs += ' ') += idref;
                    if (!beatmaps_songs.contains(id)) {
                        (msg += ' ') += idref;
                        changed = true;
                    }
                }
        });

        msg += " -";
        for (int id : beatmaps_songs)
            if (!beatmaps_new.contains(id)) {
                (msg += ' ') += QString::number(id);
                changed = true;
            }

        if (changed) {
            with(mtx_beatmaps_songs, beatmaps_songs.swap(beatmaps_new));
            with(mtx_clients, {
                for (WS *client : clients)
                    client->sendTextMessage(msg);
            });
        }
    });
}

void DIServer::start() {
    is_listening = listen();
    refresh_port_tip();
}

void DIServer::close() {
    serv.close();
    with(mtx_clients, clients.clear());
}

bool DIServer::listen() {
    if (!port) {
        if (serv.isListening())
            close();
        return false;
    }
    if (serv.isListening()) {
        if (serv.serverPort() == port)
            return true;
        close();
    }
    if (serv.listen(QHostAddress::LocalHost, port)) {
        connect(&serv, &WSServer::newConnection, this, &DIServer::connected_handler);
        return true;
    }
    return false;
}

void DIServer::connected_handler() {
    WS *client = serv.nextPendingConnection();
    with(mtx_msg_beatmaps_songs, client->sendTextMessage(msg_beatmaps_songs));
    connect(client, &WS::disconnected, this, &DIServer::disconnected_handler);
    with(mtx_clients, clients.insert(client));
}

void DIServer::disconnected_handler() {
    WS *client = qobject_cast<WS *>(sender());
    with(mtx_clients, clients.remove(client));
    client->deleteLater();
}
