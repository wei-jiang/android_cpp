<template>
  <div class="audio">
    <div class="fi" v-for="f in files" @click="play_audio(f, $event)" v-bind:id="hash(f.name)">
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
    <div class="audio-player"> 
      <div class="drag-header">
        <div class="prompt">
          <div>{{$t('current-playback')}}: {{cur_audio?cur_audio.name:$t('none')}} </div>
          <div><b>{{$t('drag-it-move')}}</b></div>
        </div>
        {{$t('sequence')}}: {{$t('sequence-tip')}}<br>
        {{$t('repeat-dir')}}: {{$t('repeat-dir-tip')}}<br>
      </div>
      <div>
        <audio id="audio-player" v-if="cur_audio" :src="cur_url" controls autoplay @ended="on_end()"/>
        <div v-if="cur_audio" class="play-type">
          <div @click.stop="set_loop_type(1)">
            <input type="radio" value="1" v-model="play_type">
            <label for="1">{{$t('single')}}</label>
          </div>
          <div @click.stop="set_loop_type(2)">
            <input type="radio" value="2" v-model="play_type">
            <label for="2">{{$t('repeat-one')}}</label>
          </div>
          <div @click.stop="set_loop_type(3)">
            <input type="radio" value="3" v-model="play_type">
            <label for="3">{{$t('sequence')}}</label>
          </div>
          <div @click.stop="set_loop_type(4)">
            <input type="radio" value="4" v-model="play_type">
            <label for="4">{{$t('repeat-dir')}}</label>
          </div>
          
        </div>
        
      </div>
    </div>
  </div>
</template>

<script>
import _ from 'lodash'
import util from "@/common/util";

export default {
  name: "audio",
  created: function() {
    this.$root.$on("update_file_list", this.update_file_list);
  },
  destroyed() {
    this.$root.$off("update_file_list", this.update_file_list);
  },
  mounted() {
    this.files = this.filter_audio(g.files);
    this.draggie = new Draggabilly('.audio-player', {
      containment: '.audio', 
      handle: '.drag-header'
    });
    const ui_set = db.ui.findOne({});
    this.play_type = ui_set.audio_loop_type;
  },
  data() {
    return {
      play_type: 1,
      cur_audio: null,
      files: []
    };
  },
  computed: {
    cur_url() {
      return this.cur_audio ? util.path2url(this.cur_audio.path) : null;
    }
  },
  methods: {
    set_loop_type(type){
      this.play_type = type;
      db.ui.findAndUpdate({}, s => {
        s.audio_loop_type = type;
      });
    },
    hash(name){
      return util.md5(name)
    },
    on_end(){
      switch (parseInt(this.play_type)){
        case 1:{
          // do nothing
          break;
        }
        case 2:{
          $('audio')[0].play();
          break;
        }
        case 3:{
          let i = this.files.indexOf(this.cur_audio);
          if( i >= 0 && i < this.files.length - 1){
            this.cur_audio = this.files[i+1];
            const id = this.hash(this.cur_audio.name)
            $('.fi').removeClass('selected')
            $(`#${id}`).addClass('selected')
          }
          break;
        }
        case 4:{
          let i = this.files.indexOf(this.cur_audio);
          if( i >= 0 ){
            if(i < this.files.length - 1){
              this.cur_audio = this.files[i+1];
            } else{
              this.cur_audio = this.files[0];
            }
            const id = this.hash(this.cur_audio.name)
            $('.fi').removeClass('selected')
            $(`#${id}`).addClass('selected')
          }
          break;
        }  
      }
    },
    formatFileSize(bytes, decimalPoint) {
      return util.formatFileSize(bytes, decimalPoint)
    },
    filter_audio(files){
      let audios = _.filter(files, f=>{
        if( f.type && f.type.includes('audio/') ){
          let audio = new Audio();
          $(audio).on("loadedmetadata", ()=>{
            f.duration = util.toHHMMSS(audio.duration);
            this.files.splice(this.files.indexOf(f), 1, f);
          });
          audio.src = util.path2url(f.path);
          return true;
        }
        return false;
      });
      return audios;
    },
    play_audio(f, e) {
      this.cur_audio = f;
      $('.fi').removeClass('selected')
      $(e.currentTarget).addClass('selected')
    },
    update_file_list(files) {
      this.files = this.filter_audio(files);
    },
  }
};
</script>
<style scoped >
.audio{
  min-height: 97%;
  position: relative;
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
