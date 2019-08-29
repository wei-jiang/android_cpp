<template>
  <div class="chat-audio">
    <!-- <p>{{ $t('hello') }}</p> -->
    <div class="dt">{{log.dt}}</div>
    <div :class="log.dir == 0 ? 'right': 'left'">
      <img class="small-avatar" :src="log.dir == 0 ? me.avatar: peer.avatar">
      <div class="audio-icon" @click="$emit('play_this', get_url(log))" :class="log.dir == 0 ? 'chat-to': 'chat-from'">
        <div>{{log.span}}″</div>
        <div class="small material-icons">settings_remote</div>
      </div>
    </div>
    
  </div>
</template>

<script>
// file:///storage/emulated/0/mystore/inout/in/img/8bce8d50-c896-11e9-b0d5-0185c0b36f10.mp3
export default {
  name: 'ChatAudio',
  // props: {
//   title: String,
//   likes: Number,
//   isPublished: Boolean,
//   commentIds: Array,
//   author: Object,
//   callback: Function,
//   contactsPromise: Promise // or any other constructor
// }
  props: {
    log: Object,
    peer: Object
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
  methods: {
    get_url(l){
      let url = cordova.file.externalRootDirectory + 'mystore/inout/';
      if(l.dir == 0){
        url += `out/audio/${l.content}`
      } else{
        url += `in/audio/${l.content}`
      }
      // console.log(`in ChatAudio.vue get_url return ${url}`)
      return url;
    },
  }
}
</script>
<style scoped>
.chat-audio{
  width: 100%;
  flex-flow: column;
  align-items: center;
  margin-bottom: 1em;
}
.audio-icon{
  display: flex;
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
