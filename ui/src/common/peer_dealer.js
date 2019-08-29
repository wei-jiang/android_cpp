import _ from 'lodash'
import moment from "moment";
import util from "@/common/util";
window.CMD = {
    ping: 0,
    pong: 1,
    handshake: 2,
    send_p2p_msg: 3,
    send_file: 4,
    send_file_resp: 5,
};

class PDealer {
    constructor() {
        this.dealers = {}
        this.dealers[CMD.ping] = this.ping.bind(this);
        this.dealers[CMD.pong] = this.pong.bind(this);
        this.dealers[CMD.handshake] = this.handshake.bind(this);
        this.dealers[CMD.send_p2p_msg] = this.send_p2p_msg.bind(this);
        this.dealers[CMD.send_file] = this.send_file.bind(this);
        this.dealers[CMD.send_file_resp] = this.send_file_resp.bind(this);
    }

    handle_msg(sp, data) {
        // data in buffer type
        try {
            if (data.length < 2) return;
            const cmd = data.readUInt16BE(0);
            const payload = data.slice(2);
            const d = this.dealers[cmd];
            if (d) {
                d(sp, payload);
            }
        } catch (error) {
            console.log('parse peer data error:' + data)
        }
    }
    ping(sp, data) {
        sp.send_cmd(CMD.pong);
    }
    pong(sp, data) {
        // console.log('recieved pong')
    }
    handshake(sp, data) {
        sp.usr = JSON.parse(data);
        vm.$emit('peer_changed', '');
        console.log('handshake done!')
    }
    send_file_resp(sp, data) {
        let res = data.toString();
        // console.log(`send_file_resp: res=${res}`)
        vm.$emit(res, '')
    }
    send_file(sp, data) {
        let i = data.indexOf(0);
        const flag = data[data.length - 1];
        const fn = data.toString('utf8', 0, i);
        let j = data.indexOf(0, i + 1);
        const type = data.toString('utf8', i + 1, j);
        const file_raw = data.slice(j + 1, data.length - 1);
        // console.log(`send_file: fn=${fn}; type=${type}; flag=${flag}`)
        this.write_file(sp, fn, type, flag, file_raw);       
    }
    async write_file(sp, fn, type, flag, buff) {
        let dir = 'mystore/inout/in/';
        if (type.includes('image/')) {
            dir += 'image'
        } else if (type.includes('audio/')) {
            dir += 'audio'
        } else if (type.includes('video/')) {
            dir += 'video'
        } else {
            dir += 'others'
        }
        // console.log(`write_file: dir=${dir}`)
        const dirEntry = await util.create_dir_recursive(dir);
        dirEntry.getFile(
            fn,
            { create: true, exclusive: false },
            fileEntry => {
                fileEntry.createWriter(fileWriter => {
                    fileWriter.onwriteend = ()=> {
                        // console.log(`write ${fn} file successful...`);
                        sp.send_string(CMD.send_file_resp, `${fn}_${flag}`);
                    };
                    fileWriter.onerror = (e)=> {
                        console.log(`write ${fn} file failed: ` + JSON.stringify(e) );
                        // todo: if write error, notify resend it
                        sp.send_string(CMD.send_file_resp, `${fn}_${flag}`);
                    };
                    const dataObj = new Blob([buff], { type });                   
                    if (flag != 0) {
                        try {
                            // console.log("fileWriter=" + JSON.stringify(fileWriter));
                            fileWriter.seek(fileWriter.length);
                        }
                        catch (e) {
                            // console.log("file doesn't exist: " + JSON.stringify(fileWriter));
                        }
                    }
                    fileWriter.write(dataObj);
                });
            },
            err => { }
        );
    }
    send_p2p_msg(sp, data) {
        data = JSON.parse(data);
        const chat_log = {
            id: sp.info.id,
            type: data.type,
            content: data.content,
            span: data.span,
            size: data.size,
            dt: util.now_str(),
            dir: 1,
            nickname: sp.usr.nickname
        };
        db.peer_chat_log.insert(chat_log);
        switch (data.type) {
            case 'text':
                util.show_alert_top_tm(`${sp.usr.nickname}: ${data.content}`);
                break;
            case 'audio':
                util.show_alert_top_tm(`${sp.usr.nickname}: 发送语音`);
                break;
            case 'image':
                util.show_alert_top_tm(`${sp.usr.nickname}: 发送图片`);
                break;
            case 'video':
                util.show_alert_top_tm(`${sp.usr.nickname}: 发送视频`);
                break;
        }
        vm.$emit('p2p_msg', chat_log);
        // console.log('send_p2p_msg: '+JSON.stringify(data))
    }
}

export default PDealer;