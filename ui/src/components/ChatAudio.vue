<template>
  <div class="chat-audio">
    <!-- <p>{{ $t('hello') }}</p> -->
    <div class="dt">{{log.dt}}</div>
    <div v-if="log.received">已接收: {{log.received}}</div>
    <div :class="log.dir == 0 ? 'right': 'left'">      
      <img v-if="log.dir==0 || peer" class="small-avatar" 
        :src="log.dir == 0 ? me.avatar: peer.avatar"
         @click.stop="head_cb && head_cb($event, log)">
      <div v-else class="small material-icons"
       @click.stop="head_cb && head_cb($event, log)">face</div>

      <div class="audio-icon" @click.stop="$emit('play_this', get_url(log))" :class="log.dir == 0 ? 'chat-to': 'chat-from'">       
        <div>{{log.span}}″</div>
        <div class="small material-icons">settings_remote</div>
      </div>
    </div>
    <div v-if="progress != '0%' && log.received == null" class="progressbar">
      <div v-bind:style="{width: progress}"></div>
      <div class="cap">{{`${progress}`}}</div>
    </div>
  </div>
</template>

<script>
// file:///storage/emulated/0/mystore/inout/in/img/8bce8d50-c896-11e9-b0d5-0185c0b36f10.mp3
import util from "@/common/util";
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
    peer: Object,
    head_cb: Function
  },
  watch: { 
    // should not use an arrow function to define a watcher 
    log: function(newVal, oldVal) { // watch it
      // console.log(`log changed, old_content=${oldVal.content}; new_content=${newVal.content}`)
      // console.log(`log changed, this.fn=${this.fn}`)
      const fn = util.get_name_from_path(oldVal.content);
      this.$root.$off(`${fn}_progress`, this.on_trans_progress);
      this.$root.$off(`${fn}_end`, this.on_trans_end);
      this.$root.$on(`${this.fn}_progress`, this.on_trans_progress);
      this.$root.$once(`${this.fn}_end`, this.on_trans_end);
    }
  },
  created: async function() {
    // console.log(`on created, log.content=${this.log.content}`);
    // console.log(`on created, this.fn=${this.fn}`)
    this.$root.$on(`${this.fn}_progress`, this.on_trans_progress);
    this.$root.$once(`${this.fn}_end`, this.on_trans_end);
    // console.log(`on ${this.fn}_progress and ${this.fn}_end event`);
  },
  destroyed() {
    this.$root.$off(`${this.fn}_progress`, this.on_trans_progress);
    this.$root.$off(`${this.fn}_end`, this.on_trans_end);
    // console.log(`off ${this.fn}_progress and ${this.fn}_end event`);
  },
  mounted() {
    // console.log(`on --mounted--, log.content=${this.log.content}`);
    this.me = db.user.findOne({});
  },
  beforeDestroy() {
    
  },
  data() {
    return {
      progress: '0%',
      me: {
        nickname: "",
        avatar: "",
        signature: "",
      }
    };
  },
  computed: {
    fn() {
      return util.get_name_from_path(this.log.content);
    }
  },
  methods: {
    on_trans_progress(progress){
      // console.log(`${this.fn}-- on_trans_progress(progress)=${progress}`);
      this.progress = progress;
      if(progress == '100.00%'){
        setTimeout(()=>{
          this.progress = '0%';
        }, 500);
      }
    },
    on_trans_end(){
      // console.log(`${this.fn}-- on_trans_end()`);
      this.progress = '0%';
    },
    get_url(l){
      if( l.content.startsWith("file://") ) return l.content;
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
@import "../assets/progressbar.css";
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
  flex-flow: wrap;
  align-items: center;
}
.right{
  /* border: 1px solid red; */
  display: flex;
  flex-flow: wrap;
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
