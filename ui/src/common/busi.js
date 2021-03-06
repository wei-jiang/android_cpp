
import moment from "moment";
import _ from 'lodash'
import util from "./util";
import WSS from "@/wss";
import FTrans from "@/common/file_transfer";

window.sss = {}
// this is target socks5 provider peerid
window.socks_pid = '';
// this is target home peerid
window.target_home_pid = '';
class Busi {
    constructor() {
        _.bindAll(this, ['init', 'reg_evt', 'routine']);
        this.need_check_friends = new Date();
    }
    init() {
        this.reg_evt();
        this.go_pub();
        this.routine();
        this.home_tunnel_port = util.http_port() + 100;
        // if(navigator.mediaDevices && navigator.mediaDevices.getSupportedConstraints){
        //     console.log( `navigator.mediaDevices.getSupportedConstraints()=${JSON.stringify(navigator.mediaDevices.getSupportedConstraints())}` );
        // }      
    }
    routine() {
        let span;
        setTimeout(() => {
            for (let [id, sp] of peers) {
                span = moment.duration(moment().diff(moment(sp.activity))).asSeconds();
                // remove peer in blacklist
                const to_be_removed = db.blacklist.findOne({ id })
                if (span > 6 || to_be_removed) {
                    console.log('remove inactive or blocked peer: ' + id)
                    sp.destroy();
                    peers.delete(id);
                    vm.$emit('peer_changed', '');
                }
            }
            // check friends
            span = moment.duration(moment().diff(moment(this.need_check_friends))).asSeconds();
            if (span > 15) {
                const ids = db.friends.find({}).filter(f => !peers.has(f.id)).map(f => f.id);
                if (ids.length > 0) {
                    _.each(sss, (v, k) => {
                        v.check_friends(ids);
                    })
                }
                this.need_check_friends = new Date();
            }
            this.replenish();
            this.routine();
        }, 1500)
    }
    replenish() {
        _.each(sss, (v, k) => {
            v.replenish();
        })
    }
    go_pub() {
        try {
            const addrs = util.ss_addrs();
            // console.log(`go_pub()------------${JSON.stringify(addrs)}`)
            addrs.forEach(s => {
                if (s.enabled) {
                    sss[s.addr] = new WSS(s.addr)
                }
            })
        } catch (err) {
            console.log(`go_pub() exception`)
        }
    }
    reg_evt() {
        vm.$on("open_target_home", async (target_pid) => {
            if(target_pid != target_home_pid){
                if(target_home_pid){
                    // disconnect local cnns
                    const buf = util.get_close_cnns_home_buff(target_home_pid, 1)
                    ws_tunnel.send(buf);
                    // disconnect remote cnns
                    peers.get(target_home_pid).send_cmd(CMD.disconnect_home_cnns);
                }               
                target_home_pid = target_pid;
            }
            const ip = await util.get_ip();
            const url = `http://${ip || location.hostname}:${this.home_tunnel_port}/`;
            // window.open(url)
            vm.$router.replace(({ name: 'phome', params: { url} }))
        });
        vm.$on("start_socks_server_succeed", data => {
            window.remote_proxy_port = parseInt(data.port)+100;
            console.log(`window.remote_proxy_port=${window.remote_proxy_port}`);
        });
        // {"6dca4b03f180588b468036b1eff907a4":11,"cmd":"noty_proxy_info"}
        vm.$on("add_ss", data => {
            const addr = data.addr;
            if (!sss.hasOwnProperty(addr)) {
                sss[addr] = new WSS(addr)
                vm.$emit('ss_changed', {cmd: 'add_ss', addr});
            }
        });
        vm.$on("peer_closed", pid => {
            let buf = util.get_close_cnns_buff(pid, 1);
            if (pid == window.socks_pid) {
                ws_tunnel.send(buf);
                window.socks_pid = "";
                vm.$emit('remote_proxy_changed', socks_pid);
            }
            buf[1] = 0;
            ws_tunnel.send(buf);
            // ////////////////////////
            buf = util.get_close_cnns_home_buff(pid, 1);
            if (pid == window.target_home_pid) {
                ws_tunnel.send(buf);
                window.target_home_pid = "";
            }
            buf[1] = 0;
            ws_tunnel.send(buf);
        });

        vm.$on("del_ss", data => {
            const addr = data.addr;
            if (sss.hasOwnProperty(addr)) {
                sss[addr].destroy();
                delete sss[addr];
                vm.$emit('ss_changed', {cmd: 'del_ss', addr});
            }
        });
        vm.$on("send_p2p_msg", data => {
            // {id, type, content}
            const id = data.id;
            delete data.id;
            peers.get(id).send_json(CMD.send_p2p_msg, data)
        });
        vm.$on("send_neighbor_msg", data => {
            // {id, type, content}
            for (const [k, v] of peers.entries()) {
                v.send_json(CMD.send_neighbor_msg, data)
            }
        });
        vm.$on("send_neighbor_audio", data => {
            // {id, type, content}
            for (const [k, v] of peers.entries()) {
                new FTrans(k, data.file, TransAssort.audio,
                    () => {
                        v.send_json(CMD.send_neighbor_msg, {
                            span: data.span,
                            type: "audio",
                            content: data.content
                        });
                        const l = db.nearby_chat_log.findOne({ content: data.content });
                        l.received += 1;
                        db.nearby_chat_log.update(l);
                        vm.$emit('neighbor_msg', '');
                    });
                // v.send_json(CMD.send_neighbor_msg, data)
            }
        });
        vm.$on("add_friend", data => {
            const it = db.friends.findOne({ id: data.id });
            if (it) {
                // console.log(`add friend: ${JSON.stringify(data)}`);
                util.show_alert_top('已加为好友');
                return;
            }
            navigator.notification.confirm(
                `[${data.nickname || util.truncate(data.id)}]？`, // message
                i => {
                    // the index uses one-based indexing, so the value is 1, 2, 3, etc.
                    if (i == 1) {
                        if( peers.has(data.id) ){
                            peers.get(data.id).send_cmd(CMD.req_friend)
                        } else{
                            const me = db.user.findOne({});
                            data.ws.send({
                                cmd: 'req_friend',
                                nickname: me.nickname,
                                signature: me.signature,
                                to: data.id
                            });
                        }   
                    }
                },
                i18n.t("add-friend"), // title
                [i18n.t("ok"), i18n.t("cancel")] // buttonLabels
            );
        });
        vm.$on("block_it", data => {
            navigator.notification.confirm(
                `[${data.nickname || util.truncate(data.id)}]？`, // message
                i => {
                    // the index uses one-based indexing, so the value is 1, 2, 3, etc.
                    if (i == 1) {
                        let it = db.blacklist.findOne({ id: data.id });
                        if (!it) {
                            db.blacklist.insert(data);
                            vm.$emit("refresh_block_list", "");
                            util.show_alert_top_tm(i18n.t("success"));
                        }
                        it = db.friends.findOne({ id: data.id });
                        if(it){
                            db.friends.remove(it);
                            vm.$emit("refresh_friend_list", "");
                        }
                    }
                },
                i18n.t("block-it"), // title
                [i18n.t("ok"), i18n.t("cancel")] // buttonLabels
            );
        });
    }
}


export default new Busi;