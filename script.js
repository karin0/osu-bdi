// ==UserScript==
// @name         osu! Beatmap Downloaded Indicator
// @namespace    https://github.com/karin0/osu-bdi
// @version      0.1
// @description  Indicate if a beatmap in the beatmap listing is downloaded on the device. An additional local daemon is required.
// @author       karin0
// @icon         https://osu.ppy.sh/favicon.ico
// @include      http*://osu.ppy.sh/beatmapsets*
// @grant        none
// ==/UserScript==

(function () {
    const port_default = '35677', obdi_page = 'https://github.com/karin0/osu-bdi';

    const css = document.createElement('style');
    css.type = 'text/css';
    css.innerText = `
        .di-done {
            filter: brightness(80%) contrast(80%) opacity(20%);
        }
        .di-input {
            width: 6em;
            height: 2.4em;
            margin: auto 0.3em auto auto;
            padding: 10px;
            background-color: hsl(var(--hsl-b2));
            border: 1px solid hsl(var(--hsl-b4));
            -moz-appearance: textfield;
        }
        .di-input::-webkit-inner-spin-button {
            -webkit-appearance: none;
            margin: 0;
        }
        .di-status {
            align-items: center;
            display: flex;
            color: #fff;
        }
    `;

    const map = new Map();
    const set = new Set();

    function get_id(e) {
        if (!e.dataset.diid) {
            const a = e.getElementsByTagName('a')[0];
            if (!a)
                return undefined;
            const href = a.getAttribute('href');
            if (!href)
                return undefined;
            const id = Number(href.substring(href.lastIndexOf('/') + 1));
            if (!id)
                return undefined;
            map[e.dataset.diid = id] = e;
            return id;
        }
        return Number(e.dataset.diid);
    }

    function set_downloaded(e) {
        if (!e)
            return;
        e.classList.add('di-done')
        const i = e.querySelector('i.fa-download');
        if (i) {
            i.classList.remove('fa-download');
            i.classList.add('fa-check-circle');
        }
    }

    function set_undownloaded(e) {
        if (!e)
            return;
        e.classList.remove('di-done')
        const i = e.querySelector('i.fa-check-circle');
        if (i) {
            i.classList.remove('fa-check-circle');
            i.classList.add('fa-download');
        }
    }

    function add(id) {
        if (!set.has(id)) {
            set.add(id);
            set_downloaded(map[id]);
        }
    }

    function remove(id) {
        if (set.has(id)) {
            set.remove(id);
            set_undownloaded(map[id]);
        }
    }

    const port_input = document.createElement('input');
    port_input.type = 'number';
    port_input.min = 1;
    port_input.max = 65535;
    port_input.classList.add('di-input');
    port_input.placeholder = 'obdi Port'

    const stored_port = localStorage.getItem('di_port');
    port_input.value = stored_port ? stored_port : port_default.toString();

    const status = document.createElement('a');
    status.classList.add('di-status');
    status.href = obdi_page;
    status.target = '_blank';

    function on_message(e) {
        let mode;
        console.log('received', e.data);
        for (const s of e.data.split(' ')) {
            const id = Number(s);
            if (id)
                (mode ? add : remove)(id);
            else if (s == '+')
                mode = 1;
            else if (s == '-')
                mode = 0;
            else {
                for (const id of set)
                    set_undownloaded(map[id]);
                set.clear();
            }
        }
    }

    function on_open() {
        status.innerText = 'obdi Connected';
    }

    let socket = null, tryer = 0, tryer_cnt = 0;

    function disconnect() {
        if (socket) {
            socket.onmessage = socket.onopen = socket.onclose = null;
            socket.close()
        }
    }

    function retry(id) {
        if (tryer != id)
            return;
        console.log(id, 'retrying', socket, socket ? socket.readyState : 'qwq');
        const state = socket ? socket.readyState : null;
        if (state == WebSocket.OPEN)
            return tryer = 0;
        if (state != WebSocket.CONNECTING) {
            disconnect();
            socket = new WebSocket('ws://127.0.0.1:' + port_input.value);
            socket.onmessage = on_message;
            socket.onopen = on_open;
            socket.onclose = connect;
        }
        setTimeout(function () { retry(id) }, 1000);
    }

    function connect() {
        if (tryer)
            return;
        status.innerText = 'obdi Disconnected';
        tryer = ++tryer_cnt;
        console.log('connects', tryer);
        retry(tryer);
    }

    port_input.onchange = function () {
        console.log('change to', port_input.value, tryer);
        localStorage.setItem('di_port', port_input.value);
        if (tryer)
            tryer = 0;
        disconnect();
        const port = Number(port_input.value);
        if (0 < port && port < 65536)
            connect();
        else
            status.innerText = 'obdi Disconnected';
    };

    const observer = new MutationObserver(function (muts) {
        for (const mut of muts)
            for (const node of mut.addedNodes)
                for (const e of node.querySelectorAll('div.beatmapsets__item')) {
                    const id = get_id(e);
                    if (id && set.has(id))
                        set_downloaded(e);
                }
    });

    window.addEventListener('load', function () {
        if (document.body.dataset.obdi)
            return;
        document.body.dataset.obdi = 1;

        document.head.appendChild(css);

        const topbar = document.querySelector('div.nav2__colgroup');
        topbar.appendChild(port_input);
        topbar.appendChild(status);

        observer.observe(document.querySelector('div.osu-layout__row'), {
            childList: true, subtree: true
        });
        for (const e of document.querySelectorAll('div.beatmapsets__item'))
            get_id(e);

        connect();
    });
})();
