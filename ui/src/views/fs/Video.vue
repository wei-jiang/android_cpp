<template>
  <div class="video">
    <div class="fi" v-for="f in files" @click="play_video(f, $event)" >
      <div class="file-desc">
        <div>{{f.name}}</div>
        <div class="file-time">
          <div>{{f.time}}</div>
          <div>{{formatFileSize(f.size)}}</div>
          <div>{{f.duration}}</div>
        </div>
      </div>
    </div>
    <div class="pad-bottom"></div>
    <div class="video-player"> 
      <div class="drag-header">
        <div class="prompt">
          <div>当前播放： {{cur_video?cur_video.name:'无'}} </div>
          <div><b>点此可拖动</b></div>
        </div>
        <p>提示：这里使用浏览器内置播放器播放，有些视频格式不支持,请自行安装vlc等专业播放器播放</p>
      </div>
      <div>
        <video id="video-player" v-if="cur_video" :src="cur_url" controls autoplay />
      </div>
    </div>
  </div>
</template>

<script>
import _ from 'lodash'
import cfg from "@/common/config";
import util from "@/common/util";

export default {
  name: "video",
  created: function() {
    this.$root.$on("update_file_list", this.update_file_list);
  },
  destroyed() {
    this.$root.$off("update_file_list", this.update_file_list);
  },
  mounted() {
    this.files = this.filter_video(g.files);
    this.draggie = new Draggabilly('.video-player', {
      containment: '.video', 
      handle: '.drag-header'
    });
  },
  data() {
    return {
      play_type: 1,
      cur_video: null,
      files: []
    };
  },
  computed: {
    cur_url() {
      return this.cur_video ? util.path2url(this.cur_video.path) : null;
    }
  },
  methods: {
    
    formatFileSize(bytes, decimalPoint) {
      return util.formatFileSize(bytes, decimalPoint)
    },
    filter_video(files){
      let vs = _.filter(files, f=>{
        if( f.type && f.type.includes('video/') ){
          let video = document.createElement("video");
          $(video).on("loadedmetadata", ()=>{
            f.duration = util.toHHMMSS(video.duration);
            this.files.splice(this.files.indexOf(f), 1, f);
          });
          video.src = util.path2url(f.path);
          return true;
        }
        return false;
      });
      return vs;
    },
    play_video(f, e) {
      this.cur_video = f;
      $('.fi').removeClass('selected')
      $(e.currentTarget).addClass('selected')
    },
    update_file_list(files) {
      this.files = this.filter_video(files);
    },
  }
};
</script>
<style scoped >
.video{
  min-height: 97%;
  position: relative;
}
video{
  width: 100%;
}
.drag-header{
  padding: 0 1em;
  color: white;
  background-color: black;
  text-align: left;
  font-size: 0.8rem;
  opacity: 0.79;
}
.prompt{
  display: flex;
  justify-content: space-between;
  text-align: center;
}
b{
  color: red;
}
.play-type{
  display: flex;
  align-items: center;
  background-color: rgb(190, 190, 190);
}
button{
  font-size: 1.5rem;
  border-radius: 1em;
  padding: 0.3em;
}

.file-desc {
  flex: 1;
  margin: 0 0.4em;
}

.file-time {
  display: flex;
  justify-content: space-between;
  font-size: 0.7rem;
}

.fi {
  display: flex;
  align-items: center;
  text-align: left;
  background-color: rgb(177, 250, 250);
  margin: 0.3em 0;
  border: 2px inset black;
}
button:disabled {
  /* background-color: #ccc; */
  color: grey;
}
input[type="radio"]{
  height: 1.3em;
  width: 1.3em;
}
.selected{
  font-size: 1.1rem;
  background-color: aqua;
}

</style>
