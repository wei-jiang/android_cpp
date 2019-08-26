import _ from 'lodash'
import moment from "moment";
import util from "@/common/util";
window.CMD = {
    ping: 0,
    pong: 1,
    handshake: 2,
    send_p2p_msg: 3,
};

class PDealer {
    constructor() {
        this.dealers = {}
        this.dealers[CMD.ping] = this.ping.bind(this);
        this.dealers[CMD.pong] = this.pong.bind(this);
        this.dealers[CMD.handshake] = this.handshake.bind(this);
        this.dealers[CMD.send_p2p_msg] = this.handle_p2p_msg.bind(this);
    }

    handle_msg(sp, data) {
        // data in buffer type
        try {
            if(data.length < 2) return;
            const cmd = data.readUInt16BE(0);
            const payload = data.slice(2);
            const d = this.dealers[cmd];
            if(d){
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
    handle_p2p_msg(sp, data) {
        data = JSON.parse(data);
        const chat_log = {
            id: sp.info.id,
            type: data.type,
            content: data.content,
            span: data.span,
            dt: util.now_str(),
            dir: 1,
            nickname: sp.usr.nickname
        };
        db.peer_chat_log.insert(chat_log);
        switch (data.type)
        {
            case 'text':
                util.show_alert_top_tm(`${sp.usr.nickname}: ${data.content}`);
                break;
            case 'audio':
                util.show_alert_top_tm(`${sp.usr.nickname}: 发送语音`);
                break;
        }
        vm.$emit('p2p_msg', chat_log);     
        // console.log('handle_p2p_msg: '+JSON.stringify(data))
    }
}

export default PDealer;