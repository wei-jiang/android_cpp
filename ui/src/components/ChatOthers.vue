<template>
  <div class="chat-others">
    <!-- <p>{{ $t('hello') }}</p> -->
    <div class="dt">{{log.dt}}</div>
    <div :class="log.dir == 0 ? 'right': 'left'" @click="show_others(log)">
      <img class="small-avatar" :src="log.dir == 0 ? me.avatar: peer.avatar">
      <div class="others-icon" :class="log.dir == 0 ? 'chat-to': 'chat-from'">
        <div>{{log.size}}</div>
        <div class="fn">{{get_file_name(log.content)}}</div>
      </div>
    </div>
    <div v-if="progress != '0%'" class="progressbar">
      <div v-bind:style="{width: progress}"></div>
      <div class="cap">{{`${progress}`}}</div>
    </div>
  </div>
</template>

<script>
import util from "@/common/util";
export default {
  name: 'ChatVideo',
  props: {
    log: Object,
    peer: Object
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
    this.$root.$on(`${this.fn}_progress`, this.on_trans_progress);
    this.$root.$once(`${this.fn}_end`, this.on_trans_end);
  },
  destroyed() {
    this.$root.$off(`${this.fn}_progress`, this.on_trans_progress);
    this.$root.$off(`${this.fn}_end`, this.on_trans_end);
  },
  mounted() {
    this.me = db.user.findOne({});
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
        }, 1000);
      }
    },
    on_trans_end(){
      // console.log(`${this.fn}-- on_trans_end()`);
      this.progress = '0%';
    },
    show_others(l){
      let path = this.get_url(l);
      if( path.startsWith("file://") ){
        path = path.substring(7);
      }
      path = decodeURI(path);
      console.log(`show_others, path=${path}`)
      cpp.openFileByPath(path);      
    },
    get_url(l){
      if( l.content.startsWith("file://") ) return l.content;
      let url = cordova.file.externalRootDirectory + 'mystore/inout/';
      if(l.dir == 0){
        url += `out/others/${l.content}`
      } else{
        url += `in/others/${l.content}`
      }
      // console.log(`in ChatVideo.vue get_url return ${url}`)
      return url;
    },
    get_file_name(content){
      let fn = content.substring(content.lastIndexOf('/')+1);
      return decodeURI(fn);
    },
  }
}
</script>
<style scoped>
@import "../assets/progressbar.css";
.fn{
  word-break: break-all;
}
.chat-others{
  width: 100%;
  flex-flow: column;
  align-items: center;
  margin-bottom: 1em;
}
.others-icon{
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

