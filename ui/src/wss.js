import _ from 'lodash'
import moment from "moment";

import Noty from "noty";
import Peer from 'simple-peer';

import util from "@/common/util";

const peers = new Map();
class WSS {
  constructor(addr) {
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
  connect_to_friends() {
    // todo: periodically check
  }
  create_peer(p, initiator = false) {
    const s_ip = this.svr_ip;
    console.log(`s_ip=${s_ip}`)
    const sp = new Peer({
      initiator,
      trickle: true,
      config: {
        iceServers: [
          { urls: `stun:${s_ip}` },
          {
            urls: `turn:${s_ip}`,
            username: 'piaoyun',
            credential: 'freego'
          }
        ]
      }
    });
    sp.info = p;
    sp.on('signal', (sig_data) => {
      let data = {
        to: p.id,
        sig_data,
        cmd: initiator ? 'send_sig': 'return_sig'
      };
      console.log(`sp.on('signal'), initiator=${initiator}`);
      this.send(data);
    });
    sp.on('error', (e) => {
      console.log('peer channel error', e);
    });
    sp.on('close', () => {
      console.log('peer channel closed');

    })
    sp.on('connect', () => {
      console.log('peer channel connect');
    });
    sp.on('stream', (stream) => {
      console.log('on peer channel stream');
    });
 
    return sp;
  }
  on_message(evt) {
    try {
      const data = JSON.parse(evt.data)
      switch (data.cmd) {
        case 'peers': {
          // todo: filter with blacklist
          const ps = data.peers;
          _.each(ps, p => {
            const sp = this.create_peer(p, true)
            peers.set(p.id, sp);
          })
          break;
        }
        case 'send_sig': {
          console.log(`send_sig: ${JSON.stringify(data)}`);
          const sp = this.create_peer(data.from)
          sp.signal(data.sig_data);
          peers.set(data.from.id, sp);
          break;
        }
        case 'return_sig': {
          console.log(`return_sig: ${JSON.stringify(data)}`);
          if( peers.has(data.from.id) ){
            peers.get(data.from.id).signal(data.sig_data)
          }
          break;
        }
      }
      // vm.$emit(data.cmd, data);
    } catch (err) {
      console.log(`parse ws message error: `+evt.data)
    }
  }
  on_error(err) {
    console.log(`${this.url} onerror:  ` + JSON.stringify(err))
  }
  on_close() {
    // 5 seconds, reconnect
    setTimeout(this.init, 5 * 1000);
    console.log(`${this.url} onclose`)
  }
  on_open() {
    console.log(`${this.url} onopen`)
    const token = util.randomInt();
    this.send({
      cmd: 'peer_online',
      id: peer_id,
      token
    });
    util.post_local('mount_pub_svr', {
      svr_addr: this.addr,
      id: peer_id,
      token
    })
  }

  send(data) {
    // todo: if closed, reconnect
    // expect data to be object
    this.ws.send(JSON.stringify(data));
  }
  destroy() {
    this.ws = null;
  }
}
export default WSS;