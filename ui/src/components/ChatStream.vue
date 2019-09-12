<template>
  <div class="chat-stream">
    <div class="dt">{{log.dt}}</div>
    <div :class="log.dir == 0 ? 'right': 'left'" :id="log.uuid">
      <img class="small-avatar" :src="log.dir == 0 ? me.avatar: peer.avatar">
      <div class="video-icon" :class="log.dir == 0 ? 'chat-to': 'chat-from'">
        <!-- status: requesting, reject, handshake, streaming, closed -->
        <div v-if="log.status == 'requesting'">申请通话中</div>
        <div v-else-if="log.status == 'reject'">{{log.reason}}</div>
        <div v-else-if="log.status == 'handshake'">建立连接中</div>
        <div v-else-if="log.status == 'streaming'">
          <div>{{log.sub_type=='audio'?'语音通话中':'视频通话中'}}</div>
        </div>
        <div v-else-if="log.status == 'closed'">通话结束(时长:{{span}})</div>
      </div>
    </div>
  </div>
</template>

<script>
import moment from "moment";
import util from "@/common/util";
export default {
  name: 'ChatStream',
  props: {
    log: Object,
    peer: Object
  },
  watch: { 
    // should not use an arrow function to define a watcher 
    log: function(newVal, oldVal) { // watch it
    
    },

  },
  created: async function() {
    
  },
  destroyed() {

  },
  mounted() {
    this.me = db.user.findOne({});
  },
  data() {
    return {
      me: {
        nickname: "",
        avatar: "",
        signature: "",
      }
    };
  },
  computed: {
    span() {
      if(this.log.end && this.log.start){
        let diff = moment(this.log.end).diff( moment(this.log.start) );
        return moment.utc(diff).format("HH:mm:ss");
      }      
    }
  },
  methods: {
    
  }
}
</script>
<style scoped>
.chat-stream{
  width: 100%;
  flex-flow: column;
  align-items: center;
  margin-bottom: 1em;
}
.video-icon{
  display: flex;
  flex-flow: column;
  align-items: center;
  padding: 0.5em;
}
.dt{
  font-size: 0.9rem;
  color: gray;
}
.left{
  display: flex;
  align-items: center;
}
.right{
  /* border: 1px solid red; */
  display: flex;
  /* fifo */
  /* justify-content: flex-end; */
  flex-flow: row-reverse;
  align-items: center;
}
.small-avatar {
  width: 2rem;
  height: 2rem;
}
.chat-from {
  word-wrap: break-word;
  margin: 0.5em 0.6em;

  max-width: 80%;
	position: relative;
	background: #82ebf5;
	border-radius: .4em;
}
video{
  width: 100%;
}
.chat-from:after {
	content: '';
	position: absolute;
	left: 0;
	top: 50%;
	width: 0;
	height: 0;
	border: 0.563em solid transparent;
	border-right-color: #82ebf5;
	border-left: 0;
	border-bottom: 0;
	margin-top: -0.281em;
	margin-left: -0.562em;
}
.chat-to {
  word-wrap: break-word;
  margin: 0.5em 0.6em;
  max-width: 80%;
	position: relative;
	background: #82ebf5;
	border-radius: .4em;
}
img{
  border-radius: 1em;
  bottom: 0;
}

.chat-to:after {
	content: '';
	position: absolute;
	right: 0;
	top: 50%;
	width: 0;
	height: 0;
	border: 0.625em solid transparent;
	border-left-color: #00aabb;
	border-right: 0;
	border-bottom: 0;
	margin-top: -0.312em;
	margin-right: -0.625em;
}
</style>
<i18n>
{
  "en": {
    "hello": "Hello i18n in SFC!"
  }
}
</i18n>
