
import moment from "moment";
import _ from 'lodash'
import Peer from 'simple-peer';
import util from "./util";


// rt_chat status: requesting, reject, handshake, streaming, closed
class RTStream {
  // global stream must be available before call on this
  constructor(target_id, type, initiator) {
    this.target_id = target_id;
    this.sp = peers.get(target_id);
    this.initiator = initiator;
    this.uuid = util.uuid();
    this.type = type;
    window.is_streaming = true;
    vm.$once(`peer_closed`, this.on_peer_closed);
    if(initiator){
      this.req_rt_chat();
      this.put_chat_log_status('requesting');
    }else {
      this.participate_rt_chat();
      this.put_chat_log_status('handshake');
    }    
  }
  on_peer_closed(id){
    if(this.target_id == id){
      this.destroy_self();
    }
  }
  put_chat_log_status(status, reason){
    let l = db.peer_chat_log.findOne({uuid: this.uuid});
    if(l){
      l.status = status;
      l.reason = reason;
      db.peer_chat_log.update(l); 
    } else {
      db.peer_chat_log.insert({
        id: this.target_id,
        uuid: this.uuid,
        status,
        type: `stream`,
        sub_type: this.type,
        dt: util.now_str(),
        dir: this.initiator ? 0 : 1,
        nickname: this.initiator ? db.user.findOne({}).nickname : this.sp.usr.nickname
      });
    }   
    vm.$emit('p2p_msg', {id: this.target_id}); 
  }
  participate_rt_chat(){
    this.tm = setTimeout(()=>{
      vm.$emit('stream_peer_closed', {
        id: this.target_id,
        type: this.type
      });
      this.destroy_self()
    }, 5 * 1000);
    this.reg_participant_evt();
    this.create_stream_channel();
  }
  create_stream_channel() {
    const stream = this.type == 'audio' ? window.audio_stream : window.video_stream;
    const sp = new Peer({
      initiator: this.initiator,
      stream,
      trickle: true,
      config: this.sp.ice_svr
    });
    sp.on('signal', (sig_data) => {
      let cmd = this.initiator ? CMD.stream_initiator_sig : CMD.stream_participant_sig;
      // console.log(`stream_sp on(signal), cmd=${cmd}, sig_data=${JSON.stringify(sig_data)}`);
      this.sp.send_json(cmd, sig_data);
    });
    sp.on('error', (e) => {
      console.log('stream_sp peer channel error', e);
    });
    sp.on('close', () => {
      console.log('stream_sp peer channel closed');
      vm.$emit(`close_stream`, '');
      let l = db.peer_chat_log.findOne({uuid: this.uuid});
      l.end = util.now_str();
      this.put_chat_log_status('closed');
      this.sp.stream_sp = null;
      vm.$emit('stream_peer_closed', {
        id: this.target_id,
        type: this.type
      });
    });
    sp.on('connect', () => {
      console.log('stream peer channel connect');
    });
    sp.on('stream', (stream) => {
      console.log('@@@@@@@@@@@@@@@@@@@@@ stream_sp on stream @@@@@@@@@@@@@@@@@@@@@');
      clearTimeout(this.tm);
      let l = db.peer_chat_log.findOne({uuid: this.uuid});
      l.start = util.now_str();
      this.put_chat_log_status('streaming');
      vm.$emit('stream_start', {
        type: this.type,
        stream
      });
    });
    sp.on('data', data => {

    });
    this.sp.stream_sp = sp;
  }
  req_rt_chat() {
    this.sp.send_string(CMD.req_stream_chat, this.type);   
    this.reg_initiator_evt();
  }
  reg_initiator_evt(){
    vm.$once(`close_stream`, this.destroy_self.bind(this) );
    vm.$once(`res_stream_chat`, this.res_stream_chat.bind(this) );
    vm.$on(`stream_participant_sig`, (data) => {
      // console.log(`on stream_participant_sig`)
      this.sp.stream_sp.signal(data);
    });
  }
  res_stream_chat(data){
    if (data.allow) {
      this.put_chat_log_status('handshake');
      console.log(`对方已同意，握手中`)
      this.tm = setTimeout(()=>{
        vm.$emit('stream_peer_closed', {
          id: this.target_id,
          type: this.type
        });
        this.destroy_self()
      }, 5 * 1000);
      this.create_stream_channel();
    } else{
      util.show_alert_top(data.reason);
      this.put_chat_log_status('reject', data.reason);
      vm.$emit('stream_peer_closed', {
        id: this.target_id,
        type: this.type
      });
      this.destroy_self()
    }
  }
  reg_participant_evt(){
    vm.$once(`close_stream`, this.destroy_self.bind(this) );
    vm.$on(`stream_initiator_sig`, (data) => {
      this.sp.stream_sp.signal(data);
    });
  }
  destroy_self(){
    console.log(`destroy_self()----`)
    vm.$off(`close_stream`);
    vm.$off(`res_stream_chat`);
    vm.$off(`stream_initiator_sig`);
    vm.$off(`stream_participant_sig`);
    vm.$off(`peer_closed`, this.on_peer_closed);    
    window.is_streaming = false;
    if(this.type == 'audio' && window.audio_stream){
      window.audio_stream.getTracks().forEach( track => {
        track.stop();
      });
      window.audio_stream = null;
    } else if(this.type == 'video' && window.video_stream) {
      window.video_stream.getTracks().forEach( track => {
        track.stop();
      });
      window.video_stream = null;
    }
    
    if(this.sp.stream_sp){
      this.sp.stream_sp.destroy();
      this.sp.stream_sp = null;
    }
  }
}


export default RTStream;