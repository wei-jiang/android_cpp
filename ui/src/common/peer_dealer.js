import _ from 'lodash'
import moment from "moment";
import util from "@/common/util";
import RTStream from "@/common/rt_stream";
window.CMD = {
    ping: 0,
    pong: 1,
    handshake: 2,
    send_p2p_msg: 3,
    send_file: 4,
    send_file_resp: 5,
    req_stream_chat: 6,
    res_stream_chat: 7,
    stream_initiator_sig: 8,
    stream_participant_sig: 9,
    udp_ep: 10,
    tcp_tunnel: 11,
    req_proxy: 12,
    req_proxy_resp: 13,
    disconnect_proxy_cnns: 14,
    send_neighbor_msg: 15,
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

        this.dealers[CMD.req_stream_chat] = this.req_stream_chat.bind(this);
        this.dealers[CMD.res_stream_chat] = this.res_stream_chat.bind(this);
        this.dealers[CMD.stream_initiator_sig] = this.stream_initiator_sig.bind(this);
        this.dealers[CMD.stream_participant_sig] = this.stream_participant_sig.bind(this);
        this.dealers[CMD.udp_ep] = this.udp_ep.bind(this);
        this.dealers[CMD.tcp_tunnel] = this.tcp_tunnel.bind(this);
        this.dealers[CMD.req_proxy] = this.req_proxy.bind(this);
        this.dealers[CMD.req_proxy_resp] = this.req_proxy_resp.bind(this);
        this.dealers[CMD.disconnect_proxy_cnns] = this.disconnect_proxy_cnns.bind(this);
        this.dealers[CMD.send_neighbor_msg] = this.send_neighbor_msg.bind(this);
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
        vm.$emit('peer_changed', sp.pid);
        // for test
        // socks_pid = sp.pid;
        console.log('handshake done!')
    }
    send_neighbor_msg(sp, data) {
        data = JSON.parse(data);
        const chat_log = {
            id: sp.pid,
            type: data.type,
            content: data.content,
            span: data.span,
            size: data.size,
            dt: util.now_str(),
            dir: 1,
            nickname: sp.usr.nickname
        };
        db.nearby_chat_log.insert(chat_log);
        vm.$emit('neighbor_msg', chat_log);
    }
    disconnect_proxy_cnns(sp, data) {
        const buf = util.get_close_cnns_buff(sp.pid, 0);
        ws_tunnel.send(buf);
    }
    async req_proxy(sp, data) {
        const usr_settings = db.user.findOne({});
        if(usr_settings.proxy == '0'){
            return sp.send_json(CMD.req_proxy_resp, {
                allow: false,
                reason: '已屏蔽'
            });
        }
        if(usr_settings.proxy == '1'){
            return sp.send_json(CMD.req_proxy_resp, {
                allow: true
            });
        }
        try {
            await util.show_confirm_top(`[${sp.usr.nickname}] 请求代理`, '同意', '拒绝');
            sp.send_json(CMD.req_proxy_resp, { allow: true });
        } catch (err) {
            // console.log('req_proxy, err='+JSON.stringify(err))
            sp.send_json(CMD.req_proxy_resp, {
                allow: false,
                reason: '已拒绝'
            });
        }
    }
    req_proxy_resp(sp, data) {
        data = JSON.parse(data);
        if(data.allow){
            if(socks_pid){
                // disconnect local cnns
                const buf = util.get_close_cnns_buff(socks_pid, 1)
                ws_tunnel.send(buf);
                // disconnect remote cnns
                peers.get(socks_pid).send_cmd(CMD.disconnect_proxy_cnns);
            }
            socks_pid = sp.pid;
            util.show_alert_top(`请求代理成功，设置远程玩家代理`);
            vm.$emit('remote_proxy_changed', socks_pid);
        } else {
            util.show_alert_top(`请求代理失败：${data.reason}`);
        }
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
        const assort = data[j + 1];
        const file_raw = data.slice(j + 2, data.length - 1);
        // console.log(`send_file: fn=${fn}; type=${type}; flag=${flag}`)
        this.write_file(sp, fn, type, flag, file_raw, assort);
    }
    async write_file(sp, fn, type, flag, buff, assort) {
        let dir = 'mystore/inout/in/';
        if (TransAssort.image == assort) {
            dir += 'image'
        } else if (TransAssort.audio == assort) {
            dir += 'audio'
        } else if (TransAssort.video == assort) {
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
                    fileWriter.onwriteend = () => {
                        // console.log(`write ${fn} file successful...`);
                        sp.send_string(CMD.send_file_resp, `${fn}_${flag}`);
                    };
                    fileWriter.onerror = (e) => {
                        console.log(`write ${fn} file failed: ` + JSON.stringify(e));
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
            id: sp.pid,
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
            case 'others':
                util.show_alert_top_tm(`${sp.usr.nickname}: 发送文件`);
                break;
        }
        vm.$emit('p2p_msg', chat_log);
        // console.log('send_p2p_msg: '+JSON.stringify(data))
    }
    async req_stream_chat(sp, type) {
        type = type.toString();
        if (window.is_streaming) {
            return sp.send_json(CMD.res_stream_chat, {
                allow: false,
                reason: '通话中'
            });
        }
        const usr_settings = db.user.findOne({});
        if(type == 'video' && usr_settings.video_chat == '0'){
            return sp.send_json(CMD.res_stream_chat, {
                allow: false,
                reason: '已屏蔽'
            });
        }
        if(type == 'audio' && usr_settings.audio_chat == '0'){
            return sp.send_json(CMD.res_stream_chat, {
                allow: false,
                reason: '已屏蔽'
            });
        }
        try {
            await util.show_confirm_top(`[${sp.usr.nickname}] 请求${type}聊天`, '同意', '拒绝');
            if (type == 'audio') {
                if (!window.audio_stream) {
                    window.audio_stream = await util.get_audio_stream();
                }
            } else {
                if (!window.video_stream) {
                    window.video_stream = await util.get_video_stream();
                }
            }
            const talk_to = _.clone(sp.usr);
            talk_to.id = sp.pid;
            vm.$router.push({ name: 'peer-chat', params: { tp: talk_to }, query: {stream_type: type} } )
            .catch(err => {});
            new RTStream(sp.pid, type, false);
            sp.send_json(CMD.res_stream_chat, { allow: true });
        } catch (err) {
            console.log('req_stream_chat, err='+JSON.stringify(err))
            sp.send_json(CMD.res_stream_chat, {
                allow: false,
                reason: '已拒绝'
            });
        }
    }
    res_stream_chat(sp, data) {
        data = JSON.parse(data);
        vm.$emit('res_stream_chat', data);
    }
    stream_initiator_sig(sp, data) {
        data = JSON.parse(data);
        vm.$emit('stream_initiator_sig', data);
    }
    stream_participant_sig(sp, data) {
        data = JSON.parse(data);
        vm.$emit('stream_participant_sig', data);
    }
    udp_ep(sp, data) {
        console.log(`in udp_ep, data=${data}`)
        util.post_local('connect_to_peer', {
            id: sp.pid,
            ep: data.toString()
        });
    }
    tcp_tunnel(sp, data) {
        ws_tunnel.send(data)
    }
}

export default PDealer;