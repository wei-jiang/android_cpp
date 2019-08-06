import _ from 'lodash'
import moment from "moment";

import Noty from "noty";
import Peer from 'simple-peer';

import util from "./common/util";
class WSS {
  // vc for vue-conponents
  constructor(addr, vc) {
    this.url = `wss://${addr}/`;
    // console.log(`this.url=${this.url}`)
    this.init()
  }
  init() {
    this.ws = new WebSocket(this.url);
    _.bindAll(this, ['on_message', 'on_close', 'on_error', 'on_open', 'send'])
    this.ws.onmessage = this.on_message;
    this.ws.onclose = this.on_close;
    this.ws.onerror = this.on_error;
    this.ws.onopen = this.on_open;
  }

  on_message(evt) {
    try{
      const data = JSON.parse(evt.data)
      console.log(evt.data);
      vm.$emit(data.cmd, data);
    }catch(err){
      console.log(evt.data)
    }
  }
  on_error(err) {
    console.log(`${this.url} onerror:  `+ JSON.stringify(err) )
  }
  on_close() {
    this.init()
    console.log(`${this.url} onclose`)
  }
  on_open() {
    console.log(`${this.url} onopen`)
    this.send({
      cmd: 'peer_online',
      id: peer_id,
      token: util.randomInt()
    })
  }

  send(data) {
    // expect data to be object
    this.ws.send(JSON.stringify(data) );
  }

}
export default WSS;