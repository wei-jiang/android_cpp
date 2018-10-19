import _ from 'lodash'
import moment from "moment";
import adb from "./db";

class WS {
  constructor() {    
    
  }
  init(){
    this.ws=new WebSocket( this.ws_uri() );
    _.bindAll(this, ['on_message', 'on_close', 'on_error', 'on_open'])
    this.ws.onmessage = this.on_message;
    this.ws.onclose = this.on_close;
    this.ws.onerror = this.on_error;
    this.ws.onopen = this.on_open;
  }
  register_ui_evt() {
    vm.$on("notify_seller_status", data => {
      this.emit('notify_seller_status', data)
    });
  }
  on_message(evt){
    const data = JSON.parse(evt.data)
    // console.log(data)
    vm.$emit(data.cmd, data.files);
  }
  on_error(err){
    console.log('onerror', err)
  }
  on_close(){
    console.log('onclose')
  }
  on_open() {
    // this.register_ui_evt()
    console.log('onopen')
  }
  on_update_order_state(data) {
    vm.$emit('update_order_state', data);
  }
  on_refresh_file_list(data) {
    vm.$emit('refresh_file_list', '');
  }
  upload_file(file) {

  }
  send(data) {
    this.ws.send(data);
  }
  ws_uri(){
    let loc = window.location, ws_uri, h = loc.host;
    if (loc.protocol === "https:") {
      ws_uri = "wss:";
    } else if (loc.protocol === "http:"){
      ws_uri = "ws:";
    } else {
      ws_uri = "ws:";
      h = 'localhost:12345';
    }
    ws_uri += "//" + h + "/cpp_channel";
    console.log(ws_uri)
    return ws_uri;
  }
}
export default new WS;