import _ from 'lodash'
import moment from "moment";

import Noty from "noty";
import Peer from 'simple-peer';

import cfg from "./common/config";
import util from "./common/util";
class WS {
  constructor() {

  }
  init() {
    this.connected = null;
    this.ws = new WebSocket(this.ws_uri());
    _.bindAll(this, ['on_message', 'on_close', 'on_error', 'on_open', 'send'])
    this.ws.onmessage = this.on_message;
    this.ws.onclose = this.on_close;
    this.ws.onerror = this.on_error;
    this.ws.onopen = this.on_open;
  }

  on_message(evt) {
    try{
      const data = JSON.parse(evt.data)
      // console.log(evt.data);
      vm.$emit(data.cmd, data);
    }catch(err){
      console.log(JSON.stringify(err))
    }
  }
  on_error(err) {
    console.log('onerror: ' + JSON.stringify(err) )
  }
  on_close() {
    clearInterval(this.connected)
    this.init()
    console.log('onclose')
  }
  on_open() {
    this.connected = setInterval(()=>{
      if (this.ws.readyState == this.ws.OPEN) {  
        this.ws.send('');  
      }
    }, 20*1000);
    console.log('onopen')
  }

  send(data) {
    if (!this.connected) {
      return setTimeout(this.send, 1000, data);
    }
    this.ws.send(data);
  }
  ws_uri() {
    let loc = window.location, ws_uri, h = loc.host;
    if (loc.protocol === "https:") {
      ws_uri = "wss:";
    } else if (loc.protocol === "http:") {
      ws_uri = "ws:";
    } else {
      ws_uri = "ws:";
      h = `localhost:${cfg.svr_port}`;
    }
    ws_uri += "//" + h + "/cpp_channel";
    console.log(ws_uri)
    return ws_uri;
  }
}
export default new WS;