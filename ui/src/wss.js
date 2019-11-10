import _ from 'lodash'
import moment from "moment";

import Noty from "noty";
import Peer from 'simple-peer';

import util from "@/common/util";
import PDealer from "@/common/peer_dealer";

window.peers = new Map();
// id --> sp --> { usr --> {nickname, avatar, signiture} }
class WSS extends PDealer {
  constructor(addr) {
    super();
    this.total = 0;
    this.addr = addr;
    this.svr_ip = addr.substring(0, addr.indexOf(':'))
    this.url = `wss://${addr}/`;
    // console.log(`this.url=${this.url}`)
    _.bindAll(this, ['init', 'on_message', 'on_close', 'on_error', 'on_open', 'send'])
    this.init()
  }
  init() {
    this.ws = new WebSocket(this.url);
    this.ws.onmessage = this.on_message;
    this.ws.onclose = this.on_close;
    this.ws.onerror = this.on_error;
    this.ws.onopen = this.on_open;
  }
  check_friends(ids) {
    if (this.connected) {
      this.send({
        cmd: 'check_fiends',
        friends: ids
      })
    }
  }

  create_peer(pid, initiator = false) {
    const s_ip = this.svr_ip;
    // console.log(`s_ip=${s_ip}`)
    const ice_svr = {
      iceServers: [
        { urls: `stun:${s_ip}` },
        {
          urls: `turn:${s_ip}`,
          username: TURN_USER,
          credential: TURN_PASS
        }
      ]
    };
    const sp = new Peer({
      initiator,
      trickle: true,
      config: ice_svr
    });
    sp.ice_svr = ice_svr;
    sp.passive = !initiator;
    sp.on('signal', (sig_data) => {
      let data = {
        to: pid,
        sig_data,
        cmd: initiator ? 'send_sig' : 'return_sig'
      };
      // console.log(`sp.on('signal'), initiator=${initiator}`);
      this.send(data);
    });
    sp.on('error', (e) => {
      console.log('peer channel error', e);
    });
    sp.on('close', () => {
      console.log('peer channel closed');
      peers.delete(sp.pid)
      // not fired, need custom heartbeat?
      vm.$emit('peer_changed', '');
      vm.$emit('peer_closed', sp.pid);
    })
    sp.on('connect', () => {
      console.log('peer channel connect');
      const ui = db.user.findOne({})
      sp.send_json(CMD.handshake, {
        nickname: ui.nickname,
        avatar: ui.avatar,
        signature: ui.signature
      });
      // if(this.my_udp_ep){
      //   sp.send_string(CMD.udp_ep, this.my_udp_ep);
      // }
    });
    sp.on('stream', (stream) => {
      // console.log('on peer channel stream');
    });
    sp.on('data', data => {
      sp.activity = new Date();
      this.handle_msg(sp, data);
    })
    return sp;
  }
  send_world_msg(msg) {
    this.send({
      cmd: 'to_all',
      content: msg
    })
  }
  async on_message(evt) {
    try {
      const data = JSON.parse(evt.data)
      switch (data.cmd) {
        // return order: res_peer_online -> peers -> total
        case 'res_peer_online': {
          // after ws register successfully, then start udp ping
          util.post_local('mount_pub_svr', {
            svr_addr: this.addr,
            id: cli_id,
            token: data.token
          });
          window.TURN_USER = data.TURN_USER;
          window.TURN_PASS = data.TURN_PASS;
          // console.log(`TURN_USER=${TURN_USER} & TURN_PASS=${TURN_PASS}`);
          break;
        }
        case 'your_udp_ep': {
          this.my_udp_ep = data.ep;
          // console.log(`your_udp_ep, got ep=${this.my_udp_ep}`)
          // for (let [pid, sp] of peers) {
          //   if(sp.usr){
          //     sp.send_string(CMD.udp_ep, this.my_udp_ep);
          //   }            
          // }
          break;
        }
        case 'res_friend': {
          if(data.allow){
              db.friends.insert({id: data.from });
              vm.$emit("refresh_friend_list", "");
              util.show_alert_top('添加好友成功');
          } else {
              util.show_alert_top(`请求好友失败：${data.reason}`);
          }
          break;
        }
        case 'req_friend': {
          const usr_settings = db.user.findOne({});
          if (usr_settings.friend == '0') {
            return this.send({
              cmd: 'res_friend',
              to: data.from,
              allow: false,
              reason: '已屏蔽'
            });
          }
          if (usr_settings.friend == '1') {
            if (!db.friends.findOne({ id: data.from })) {
              db.friends.insert({ id: data.from, nickname: data.nickname, signature: data.signature });
              vm.$emit("refresh_friend_list", "");
            }
            return this.send({
              cmd: 'res_friend',
              to: data.from,
              allow: true
            });
          }
          try {
            await util.show_confirm_top(`[${data.nickname}] 请求加为好友<br>个性签名：${data.signature}`, '同意', '拒绝');
            if (!db.friends.findOne({ id: data.from })) {
              db.friends.insert({ id: data.from, nickname: data.nickname, signature: data.signature });
              vm.$emit("refresh_friend_list", "");
            }
            this.send({
              cmd: 'res_friend',
              to: data.from,
              allow: true
            });
          } catch (err) {
            this.send({
              cmd: 'res_friend',
              to: data.from,
              allow: false,
              reason: '已拒绝'
            });
          }
          break;
        }
        case 'too_quick': {
          util.show_alert_top_tm('消息发送太快……');
          break;
        }
        case 'total': {
          this.total = data.total;
          vm.$emit('online_count', {
            addr: this.addr,
            total: data.total
          })
          break;
        }
        case 'to_all': {
          // world chat msg: from, content
          console.log(`收到世界频道消息：${JSON.stringify(data)}`);
          if (db.blacklist.findOne({ id: data.from })) return;
          db.world_chat_log.insert({
            addr: this.addr,
            id: data.from,
            type: "text",
            content: data.content,
            dt: util.now_str(),
            dir: 1
          });
          vm.$emit('refresh_world_chat', this.addr);
          break;
        }
        case 'peers': {
          let ps = data.peers;
          // filter out blacklist
          ps = ps.filter(pid => !(peers.has(pid) || db.blacklist.findOne({ id: pid })));
          // ps = _.difference( ps, [...peers.keys()] );
          _.each(ps, pid => {
            const sp = this.create_peer(pid, true)
            this.postfix_peer(sp, pid);
          })
          break;
        }
        case 'send_sig': {
          // console.log(`send_sig: ${JSON.stringify(data)}`);
          // to establish connection, this will be called multiple times, so must be sure only create peer instance one time
          let sp;
          let pid = data.from;
          if (peers.has(pid) && peers.get(pid).passive) {
            sp = peers.get(pid)
          } else {
            if (db.blacklist.findOne({ id: pid })) return;
            if (peers.has(pid)) peers.get(pid).destroy();
            sp = this.create_peer(pid)
            this.postfix_peer(sp, pid)
          }
          sp.signal(data.sig_data);
          break;
        }
        case 'return_sig': {
          // console.log(`return_sig: ${JSON.stringify(data)}`);
          if (peers.has(data.from)) {
            peers.get(data.from).signal(data.sig_data)
          }
          break;
        }
      }
      // vm.$emit(data.cmd, data);
    } catch (err) {
      console.log(`parse ws message error: ` + evt.data)
    }
  }
  replenish() {
    const aim = Math.min(this.total, 100);
    if (peers.size < aim) {
      this.send({
        cmd: 'need_peers',
        amount: aim - peers.size
      });
    }
  }
  postfix_peer(sp, pid) {
    sp.pid = pid;
    sp.send_string = (cmd, s) => {
      let buf = Buffer.alloc(2);
      buf.writeUInt16BE(cmd);
      buf = Buffer.concat([buf, Buffer.from(s)]);
      sp.send(buf);
    };
    sp.send_json = (cmd, j) => {
      const payload = JSON.stringify(j);
      sp.send_string(cmd, payload);
    };
    sp.send_cmd = (cmd) => {
      let buf = Buffer.alloc(2);
      buf.writeUInt16BE(cmd);
      sp.send(buf);
    };
    sp.send_buff = (cmd, buff) => {
      let buf = Buffer.alloc(2);
      buf.writeUInt16BE(cmd);
      buf = Buffer.concat([buf, buff]);
      sp.send(buf);
    };
    sp.echo = _.throttle(sp.send, 1000);
    sp.activity = new Date();
    peers.set(pid, sp);  //-----------------------------------
    vm.$emit('peer_changed', '');
    console.log(`add ${pid} to Map`)
    function send_ping(id) {
      setTimeout(() => {
        // console.log('keep_alive send ping ...')
        if (peers.has(id) && !peers.get(id).passive) {
          peers.get(id).send_cmd(CMD.ping);
          send_ping(id);
        }
      }, 2000);
    }
    send_ping(pid);
  }
  on_error(err) {
    console.log(`${this.url} onerror:  ` + JSON.stringify(err))
  }
  on_close() {
    this.connected = false;
    if (this.ws) {
      // 5 seconds, reconnect
      setTimeout(this.init, 1 * 1000);
      console.log(`${this.url} onclose`)
    } else {
      console.log(`${this.url} onclose, and not try to reconnect`)
    }
  }
  on_open() {
    this.connected = true;
    console.log(`${this.url} onopen`)
    this.send({
      cmd: 'peer_online',
      id: cli_id,
      token: util.randomInt()
    });
  }

  send(data) {
    // todo: if closed, reconnect
    // expect data to be object
    try {
      this.ws.send(JSON.stringify(data));
    } catch (error) {

    }

  }
  destroy() {
    const t = this.ws;
    this.ws = null;
    // noty udp off these addr
    util.post_local('dismount_pub_svr', {
      svr_addr: this.addr
    });
    t.close();
    vm.$emit('wss_destroyed', this.addr);
  }
}
export default WSS;