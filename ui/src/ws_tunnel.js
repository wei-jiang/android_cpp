import _ from 'lodash'
import moment from "moment";

import Noty from "noty";

import util from "./common/util";
window.CPP_CMD = {
  NEW_CONNECTION : 0,
  WRITE_BUFFER : 1,
  CLI_WRITE_BUFFER : 2,
  CLOSE_CONNECTION : 3,
  CLOSE_PEER_CNNS : 4
};
class WsTunnel {
  constructor() {
    this.init();
  }
  init() {
    this.ws = new WebSocket(this.ws_uri());
    this.ws.binaryType = 'arraybuffer';
    _.bindAll(this, ['on_message', 'on_close', 'on_error', 'on_open', 'send'])
    this.ws.onmessage = this.on_message;
    this.ws.onclose = this.on_close;
    this.ws.onerror = this.on_error;
    this.ws.onopen = this.on_open;
  }

  on_message(evt) {
    // console.log('WsTunnel on_message, '+ evt.data)
    const data = Buffer.from( evt.data );
    // console.log( data.toString('hex') );
    if(data[1] == 0){
      data.write(cli_id, 2, 32);
      // console.log( data.toString('hex') );
      if(socks_pid){
        const sp = peers.get(socks_pid)
        if(sp){
          return sp.send_buff(CMD.tcp_tunnel, data);
        } 
      }
      data[1] = 1;
      data[0] = CPP_CMD.CLOSE_CONNECTION;
      this.ws.send(data);
    } else{
      const target_id = data.toString('binary', 2, 34);
      const sp = peers.get(target_id)
      if(sp){
        sp.send_buff(CMD.tcp_tunnel, data);
      } else{
        data[1] = 0;
        data[0] = CPP_CMD.CLOSE_CONNECTION;
        this.ws.send(data);
      }
    }
  }
  on_error(err) {
    console.log('WsTunnel onerror: ' + JSON.stringify(err) )
  }
  on_close() {
    this.init()
    console.log('WsTunnel onclose')
  }
  on_open() {
    console.log('WsTunnel onopen')
    // this.send( Buffer.from([1,2,3,4]) );
  }

  send(data) {

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
      h = `localhost:${util.http_port()}`;
    }
    ws_uri += "//" + h + "/rtc_tunnel";
    console.log(ws_uri)
    return ws_uri;
  }
}
export default WsTunnel;