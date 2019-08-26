import _ from 'lodash'
import moment from "moment";

import Noty from "noty";
import Peer from 'simple-peer';

import util from "@/common/util";
import PDealer from "@/common/peer_dealer";

window.peers = new Map();
// id --> sp --> {info --> {id, ep}, usr }
class WSS extends PDealer {
  constructor(addr) {
    super();
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
  check_friends(ids){
    if(this.connected){
      this.send({
        cmd: 'check_fiends',
        friends: ids
      })
    }
  }

  create_peer(p, initiator = false) {
    const s_ip = this.svr_ip;
    // console.log(`s_ip=${s_ip}`)
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
    sp.passive = !initiator;
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
      peers.delete(sp.info.id)
      // not fired, need custom heartbeat?
      vm.$emit('peer_changed', '');
    })
    sp.on('connect', () => {
      console.log('peer channel connect');
      const ui = db.user.findOne({})
      sp.send_json(CMD.handshake, {
        nickname: ui.nickname,
        avatar: ui.avatar,
        signature: ui.signature
      });
    });
    sp.on('stream', (stream) => {
      console.log('on peer channel stream');
    });
    sp.on('data', data => {
      sp.activity = new Date();
      this.handle_msg(sp, data);
    })
    return sp;
  }
  on_message(evt) {
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
        case 'peers': {          
          let ps = data.peers;
          // filter out blacklist
          ps = ps.filter( p=>!( peers.has(p.id) || db.blacklist.findOne({id: p.id}) ) );
          // ps = _.difference( ps, [...peers.keys()] );
          _.each(ps, p => {
            const sp = this.create_peer(p, true)
            this.postfix_peer(sp, p);
          })
          break;
        }
        case 'send_sig': {
          // console.log(`send_sig: ${JSON.stringify(data)}`);
          // to establish connection, this will be called multiple times, so must be sure only create peer instance one time
          let sp;
          let p = data.from;
          if(peers.has(p.id) && peers.get(p.id).passive ){
            sp = peers.get(p.id)
          } else{
            if( peers.has(p.id) ) peers.get(p.id).destroy();
            sp = this.create_peer(p)
            this.postfix_peer(sp, p)
          }
          sp.signal(data.sig_data);
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
  replenish(){
    const aim = Math.min(this.total, 100);
    if(peers.size < aim){
      this.send({
        cmd: 'need_peers',
        amount: aim - peers.size
      });
    }
  }
  postfix_peer(sp, p){
    sp.info = p;
    sp.send_json = (cmd, j)=> {
      const payload = JSON.stringify(j);
      let buf = Buffer.alloc(2);
      buf.writeUInt16BE(cmd);
      buf = Buffer.concat([buf, Buffer.from(payload)]);
      sp.send(buf) ;
    };
    sp.send_cmd = (cmd)=> {
      let buf = Buffer.alloc(2);
      buf.writeUInt16BE(cmd);
      sp.send(buf) ;
    };
    sp.activity = new Date();
    peers.set(p.id, sp);  //-----------------------------------
    console.log(`add ${p.id} to Map`)
    function send_ping(id){
      setTimeout(()=>{
        // console.log('keep_alive send ping ...')
        if( peers.has(id) && !peers.get(id).passive ){
          peers.get(id).send_cmd(CMD.ping);
          send_ping(id);
        }
      }, 3000);
    }
    send_ping(p.id);
  }
  on_error(err) {
    console.log(`${this.url} onerror:  ` + JSON.stringify(err))
  }
  on_close() {
    this.connected = false;
    if(this.ws){
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
  }
}
export default WSS;