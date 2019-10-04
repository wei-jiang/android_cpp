<template>
  <div class="peer-chat">
    <audio id="pack_audio"></audio>
    <div v-if="tp === null">{{$t('peer-closed')}}</div>
    <div v-else class="player">
      <div class="title">
        <div @click="$router.replace({name: 'connected'})" class="small material-icons">keyboard_backspace</div>
        <div class="peer-name">
          <img class="small-avatar" :src="tp.avatar" />
          <div>{{tp.nickname}}</div>
        </div>

        <div @click.stop="add_friend()">
          <i class="small material-icons">favorite_border</i>
          &nbsp;&nbsp;
        </div>
        <div @click.stop="block_it()">
          <i class="small material-icons">block</i>
        </div>
      </div>

      <div class="chat">
        <div class="chat-log">
          <audio id="rt_audio" autoplay/>
          <video id="rt_video" controls autoplay/>
          <video id="rt_video_local"/>
          <!-- include variable components -->
          <div v-for="l in chat_logs">
            <ChatText v-if="l.type == 'text'" :log="l" :peer="tp"></ChatText>
            <ChatImage v-else-if="l.type == 'image'" :log="l" :peer="tp"></ChatImage>
            <ChatAudio v-else-if="l.type == 'audio'" :log="l" :peer="tp" @play_this="play_this"></ChatAudio>
            <ChatVideo v-else-if="l.type == 'video'" :log="l" :peer="tp"></ChatVideo>
            <ChatOthers v-else-if="l.type == 'others'" :log="l" :peer="tp"></ChatOthers>
            <ChatStream v-else-if="l.type == 'stream'" :log="l" :peer="tp"></ChatStream>
            <div v-else>不支持的聊天类型</div>
          </div>
        </div>
        <div class="chat-input">
          <div
            v-if="!record_voice"
            class="small material-icons"
            :class="{gray: audio_busy}"
            @click="show_record_btn()"
          >record_voice_over</div>
          <div
            v-if="record_voice"
            class="small material-icons"
            @click="record_voice = !record_voice"
          >keyboard</div>
          <textarea
            v-if="!record_voice"
            @input="auto_height($event)"
            rows="1"
            class="ci"
            wrap="soft"
            placeholder="input content"
            v-model="chat_content"
          ></textarea>
          <div
            v-if="record_voice"
            class="push-speak"
            @touchend.prevent.stop="stop_record($event)"
            @touchstart.prevent.stop="start_record($event)"
          >按住说话</div>
          <div v-if="chat_content" class="small material-icons" @click="send_text_msg(p)">send</div>
          <div
            v-if="chat_content == '' && !additional"
            class="small material-icons"
            @click="show_additional()"
          >add_circle_outline</div>
          <div
            v-if="additional"
            class="small material-icons"
            @click="additional = false"
          >remove_circle_outline</div>
        </div>
        <div class="additional" v-if="additional">
          <div class="send-file">
            <div @click.prevent="open_file('image/*')">
              <div class="small material-icons">photo</div>
              <div>相册</div>
            </div>
            <div @click="capture_image()">
              <div class="small material-icons">photo_camera</div>
              <div>拍照</div>
            </div>
            <div @click="capture_video()">
              <div class="small material-icons">perm_camera_mic</div>
              <div>摄像</div>
            </div>
            <div @click.prevent="open_file('*/*')">
              <div class="small material-icons">insert_drive_file</div>
              <div>文件</div>
            </div>
          </div>
          <div class="realtime">
            <div @click="audio_chat()" :class="{'btn-error':audio_streaming}">
              <div class="small material-icons" >phone</div>
              <div>{{!audio_streaming ? '语音聊天' : '挂断'}}</div>
            </div>
            <div @click="video_chat()" :class="{'btn-error':video_streaming}">
              <div class="small material-icons">videocam</div>
              <div>{{!video_streaming ? '视频聊天' : '挂断'}}</div>
            </div>
            <div @click.prevent="request_proxy()">
              <div class="small material-icons">vpn_key</div>
              <div>请求代理</div>
            </div>
          </div>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
import ChatText from "@/components/ChatText.vue";
import ChatImage from "@/components/ChatImage.vue";
import ChatAudio from "@/components/ChatAudio.vue";
import ChatVideo from "@/components/ChatVideo.vue";
import ChatOthers from "@/components/ChatOthers.vue";
import ChatStream from "@/components/ChatStream.vue";
import util from "@/common/util";
import FTrans from "@/common/file_transfer";
import RTStream from "@/common/rt_stream";

// const chat_record = "chat_record.amr";
const chat_record = "chat_record.mp3";
export default {
  name: "peer-chat",
  components: {
    ChatText,
    ChatImage,
    ChatAudio,
    ChatVideo,
    ChatOthers,
    ChatStream
  },
  props: {
    // for target peer
    tp: Object
  },
  watch: {
    tp: function(newVal, oldVal) {
      // watch it
      // console.log('Prop changed: ', newVal, ' | was: ', oldVal)
      // alert(`watch tp changed to: ${newVal}`)
    },
    audio_busy: function(newVal, oldVal) {
      if(newVal){
        this.record_voice = false;
      }   
    },
  },
  created: async function() {
    this.$root.$on("peer_closed", this.peer_closed);
    this.$root.$on("p2p_msg", this.p2p_msg);
    this.$root.$on("stream_peer_closed", this.stream_peer_closed);
    this.$root.$on('stream_start', this.stream_start);
    this.$root.$on('participate_in_rt_chat', type=>{
      // or use beforeRouteUpdate?
      console.log(`--------------on participate_in_rt_chat, type=${type}-----------------`)
      if (type == "audio") {
        this.audio_streaming = true;
      } else if (type == "video") {
        this.video_streaming = true;
      }
    });
  },
  destroyed() {
    this.$root.$off("peer_closed", this.peer_closed);
    this.$root.$off("p2p_msg", this.p2p_msg);
    this.$root.$off("stream_peer_closed", this.stream_peer_closed);
    this.$root.$off('stream_start', this.stream_start);
    this.$root.$off('participate_in_rt_chat');
  },
  
  mounted() {
    this.update_chat_log();
    console.log(`this.$route.query=${JSON.stringify(this.$route.query)}`)
    if (this.$route.query.stream_type) {
      let stream_type = this.$route.query.stream_type;
      if (stream_type == "audio") {
        this.audio_streaming = true;
      } else if (stream_type == "video") {
        this.video_streaming = true;
      }
    }
    // this.chat_logs = db.peer_chat_log
    //       .chain()
    //       .find({ id: this.tp.id })
    //       .simplesort("dt", true)
    //       .data();
  },
  data() {
    return {
      is_peer_closed: false,
      chat_logs: [],
      chat_content: "",
      record_voice: false,
      additional: false,
      audio: null,
      record_start: 0,
      audio_streaming: false,
      video_streaming: false
    };
  },
  computed: {
    audio_busy() {
      return this.audio_streaming || this.video_streaming;
    }
  },
  methods: {
    request_proxy(){
      const sp = peers.get(this.tp.id);
      sp.send_cmd(CMD.req_proxy);
    },
    stream_start(data){
      const av = $(`#rt_${data.type}`)[0];
      if ('srcObject' in av) {
        av.srcObject = data.stream
      } else {
        av.src = window.URL.createObjectURL(data.stream)
      }
      av.play();
      if(data.type == 'video'){
        const lv = $(`#rt_video_local`)[0];
        lv.muted = 0;
        lv.volume = 0;
        if ('srcObject' in lv) {
          lv.srcObject = window.video_stream
        } else {
          lv.src = window.URL.createObjectURL(window.video_stream)
        }
        lv.play();
        $(`#rt_video_local`).show();
      }
      $(`#rt_${data.type}`).show();
      // <video :src-object.prop.camel="stream">
      // this.stream = window.URL.createObjectURL(stream);
    },
    stream_peer_closed(data) {
      $(`video, audio`).hide();
      $(`#rt_video_local`)[0].pause();
      if (data.id == this.tp.id) {
        if (data.type == "audio") {
          this.audio_streaming = false;
        } else {
          this.video_streaming = false;
        }
      }
    },
    async audio_chat() {
      if (this.audio_streaming) {
        vm.$emit(`close_stream`, this.tp.id);
        this.audio_streaming = false;
        return;
      }
      if (window.audio_stream) {
        new RTStream(this.tp.id, "audio", true);
        this.audio_streaming = true;
      } else {
        try {
          window.audio_stream = await util.get_audio_stream();
          new RTStream(this.tp.id, "audio", true);
          this.audio_streaming = true;
        } catch (error) {
          util.show_warn_top("打开设备失败");
        }
      }
    },
    async video_chat() {
      if (this.video_streaming) {
        vm.$emit(`close_stream`, this.tp.id);
        this.video_streaming = false;
        return;
      }
      if (window.video_stream) {
        new RTStream(this.tp.id, "video", true);
        this.video_streaming = true;
      } else {
        try {
          window.video_stream = await util.get_video_stream();
          new RTStream(this.tp.id, "video", true);
          this.video_streaming = true;
        } catch (error) {
          util.show_warn_top("打开设备失败:" + JSON.stringify(error));
        }
      }
    },
    capture_video() {
      navigator.device.capture.captureVideo(
        async mediaFiles => {
          // only one
          const f = mediaFiles[0];
          const fileEntry = await util.get_fileEntry(f.fullPath);
          const dirEntry = await util.create_dir_recursive(
            "mystore/inout/out/video"
          );
          const fn = `${util.uuid()}${util.get_ext_of_file(f.name)}`;
          fileEntry.copyTo(
            dirEntry,
            fn,
            newEntry => {
              // send file to another peer
              newEntry.file(
                file => {
                  this.send_video(file);
                },
                err => {
                  console.log(`record audio get file from fileEntry failed`);
                }
              );
            },
            err => {
              console.log("copying record failed");
            }
          );
        },
        err => {
          console.log(`captureImage, err=${JSON.stringify(err)}`);
        }
      );
    },
    open_file(type) {
      // type: '*/*' 'image/*'
      cpp.chooseFileByType(type, path => {
        // console.log(`select file paht=${path}`)
        // cpp.openFileByPath(path);
        window.resolveLocalFileSystemURL(
          `file://${path}`,
          fileEntry => {
            fileEntry.file(file => {
              if (type == "image/*") {
                this.send_img(file, fileEntry.toURL());
              } else {
                this.send_file(file, fileEntry.toURL());
              }
            });
            // console.log(`fileEntry.toURL()=${fileEntry.toURL()}`);
            // fileEntry.toURL()=file:///storage/emulated/0/Pictures/Screenshots/Screenshot_20190801-165844.jpg
          },
          () => {
            console.log(`${path} does not exist`);
          }
        );
      });
    },
    peer_closed(id) {
      console.log(`peer_closed(id), id==${id}; this.tp.id=${this.tp.id}`)
      if (this.tp && id == this.tp.id) {
        navigator.notification.alert(
          this.$t("peer-closed"), // message
          () => this.$router.go(-1), // callback
          this.$t("back"), // title
          this.$t("ok") // buttonName
        );
        this.audio_streaming = this.video_streaming = false;
        this.tp = null;
      }
    },
    play_this(data_url) {
      // console.log("in play_this");
      const audio = $("#pack_audio")[0];
      audio.src = data_url;
      audio.load();
      audio.play();
    },
    show_record_btn() {
      if(this.audio_busy) return;
      cpp.requestPermission(
        "android.permission.RECORD_AUDIO",
        () => (this.record_voice = !this.record_voice),
        () => {}
      );
    },
    start_record(e) {
      // console.log(`start_record`);
      const p = this.tp;
      $(e.target).addClass("green");
      if (this.audio) {
        this.audio.release();
      }
      const self = this;
      this.audio = new Media(
        chat_record,
        () => {
          // console.log("recordAudio():Audio Success");
          window.resolveLocalFileSystemURL(
            cordova.file.externalRootDirectory + chat_record,
            async fileEntry => {
              const dirEntry = await util.create_dir_recursive(
                "mystore/inout/out/audio"
              );
              const fn = `${util.uuid()}.mp3`;
              fileEntry.copyTo(
                dirEntry,
                fn,
                newEntry => {
                  let span = new Date().getTime() - self.record_start;
                  span = Math.round(span / 1000);
                  self.put_audio_to_log(fn, span);
                  vm.$once(`${fn}_end`, () => {
                    vm.$emit("send_p2p_msg", {
                      id: p.id,
                      span,
                      type: "audio",
                      content: fn
                    });
                  });
                  // send file to another peer
                  newEntry.file(
                    file => {
                      new FTrans(p.id, file, TransAssort.audio);
                    },
                    err => {
                      console.log(
                        `record audio get file from fileEntry failed`
                      );
                    }
                  );
                },
                err => {
                  console.log("copying record failed");
                }
              );
            },
            () => {}
          );
        },
        err => {
          console.log(
            "recordAudio():Audio Error: " + err.code + "; msg=" + err.message
          );
        }
      );
      this.record_start = new Date().getTime();
      this.audio.startRecord();
    },
    stop_record(e) {
      // console.log(`stop_record`);
      $(e.target).removeClass("green");
      this.audio.stopRecord();
    },
    show_additional() {
      this.additional = !this.additional;
      this.$nextTick(() => {
        // $(".peer-chat").scrollTop($(".peer-chat")[0].clientHeight);
        // $(".player").scrollTop($(".player")[0].clientHeight);
        // $(".chat").scrollTop($(".chat")[0].clientHeight);
        // $(".additional").scrollTop($(".additional")[0].clientHeight);
        // $('.additional').scrollIntoView()
      });
    },
    send_video(file, ab_path) {
      const p = this.tp;
      const size = util.formatFileSize(file.size);
      vm.$once(`${file.name}_end`, () => {
        vm.$emit("send_p2p_msg", {
          id: p.id,
          size,
          type: "video",
          content: file.name
        });
      });
      new FTrans(p.id, file, TransAssort.video);
      db.peer_chat_log.insert({
        id: p.id,
        type: "video",
        size,
        content: ab_path || file.name,
        dt: util.now_str(),
        dir: 0,
        nickname: db.user.findOne({}).nickname
      });
      this.update_chat_log();
    },
    send_file(file, ab_path) {
      const p = this.tp;
      const size = util.formatFileSize(file.size);
      vm.$once(`${file.name}_end`, () => {
        vm.$emit("send_p2p_msg", {
          id: p.id,
          size,
          type: "others",
          content: file.name
        });
      });
      new FTrans(p.id, file, TransAssort.others);
      db.peer_chat_log.insert({
        id: p.id,
        type: "others",
        size,
        content: ab_path || file.name,
        dt: util.now_str(),
        dir: 0,
        nickname: db.user.findOne({}).nickname
      });
      this.update_chat_log();
    },
    send_img(file, ab_path) {
      const p = this.tp;
      const size = util.formatFileSize(file.size);
      vm.$once(`${file.name}_end`, () => {
        vm.$emit("send_p2p_msg", {
          id: p.id,
          size,
          type: "image",
          content: file.name
        });
      });
      new FTrans(p.id, file, TransAssort.image);
      db.peer_chat_log.insert({
        id: p.id,
        type: "image",
        size,
        content: ab_path || file.name,
        dt: util.now_str(),
        dir: 0,
        nickname: db.user.findOne({}).nickname
      });
      this.update_chat_log();
    },
    capture_image() {
      navigator.device.capture.captureImage(
        async mediaFiles => {
          // only one
          const f = mediaFiles[0];
          // do something interesting with the file
          // f={"name":"1566910818956.jpg","localURL":"cdvfile://localhost/persistent/Pictures/1566910818956.jpg","type":"image/jpeg",
          // "lastModified":null,"lastModifiedDate":1566910824000,"size":360868,"start":0,"end":0,"fullPath":"file:///storage/emulated/0/Pictures/1566910818956.jpg"}
          // console.log(`captureImage, f=${JSON.stringify(f)}`);
          // const file = new window.File(
          //   f.name,
          //   f.localURL,
          //   f.type,
          //   f.lastModifiedDate,
          //   f.size
          // );
          const fileEntry = await util.get_fileEntry(f.fullPath);
          const dirEntry = await util.create_dir_recursive(
            "mystore/inout/out/image"
          );
          const fn = `${util.uuid()}${util.get_ext_of_file(f.name)}`;
          fileEntry.copyTo(
            dirEntry,
            fn,
            newEntry => {
              // send file to another peer
              newEntry.file(
                file => {
                  this.send_img(file);
                },
                err => {
                  console.log(`record audio get file from fileEntry failed`);
                }
              );
            },
            err => {
              console.log("copying record failed");
            }
          );
        },
        err => {
          console.log(`captureImage, err=${JSON.stringify(err)}`);
        }
      );
    },
    p2p_msg(data) {
      // console.log(`p2p_msg, data=${JSON.stringify(data)}; tp=${JSON.stringify(this.tp)}`)
      if (this.tp.id == data.id) {
        this.update_chat_log();
      }
    },

    put_audio_to_log(url, span) {
      const p = this.tp;
      db.peer_chat_log.insert({
        id: p.id,
        type: "audio",
        span,
        content: url,
        dt: util.now_str(),
        dir: 0,
        nickname: db.user.findOne({}).nickname
      });
      this.update_chat_log();
    },
    send_text_msg() {
      const p = this.tp;
      // p.chat_logs.unshift(this.chat_content);
      this.$root.$emit("send_p2p_msg", {
        id: p.id,
        type: "text",
        content: this.chat_content
      });
      db.peer_chat_log.insert({
        id: p.id,
        type: "text",
        content: this.chat_content,
        dt: util.now_str(),
        dir: 0,
        nickname: db.user.findOne({}).nickname
      });
      this.update_chat_log();
      this.chat_content = "";
      $("textarea").css("height", "2.0rem");
    },
    auto_height(e) {
      const el = e.target;
      el.style.cssText = "height:auto; padding:0";
      el.style.cssText = "height:" + el.scrollHeight + "px";
      // console.log('in auto_height; el.style.height=' + el.style.height)
    },
    add_friend() {
      if(!this.tp) return;
      this.$root.$emit("add_friend", this.tp);
    },
    block_it() {
      if(!this.tp) return;
      this.$root.$emit("block_it", this.tp);
    },
    update_chat_log() {
      this.chat_logs = db.peer_chat_log.find({ id: this.tp.id }).reverse();
      // this.chat_logs.forEach(l=>{
      //   console.log(`l.content=${l.content}`)
      // })
    }
  }
};
</script>

<!-- Add "scoped" attribute to limit CSS to this component only -->
<style scoped>
.send-file,
.realtime {
  display: flex;
  justify-content: space-around;
  background-color: lightgrey;
}
.send-file > div,
.realtime > div {
  margin: 0.5em;
}
.small-avatar {
  width: 2rem;
  height: 2rem;
}
.title {
  width: 100%;
  display: flex;
}
.push-speak {
  flex: 1;
  user-select: none;
  background-color: rgb(228, 224, 224);
  border: 3px outset rgb(90, 90, 90);
}
.green {
  background-color: rgb(138, 211, 138);
}
.peer-name {
  display: flex;
  justify-content: center; /* center items horizontally, in this case */
  align-items: center; /* center items vertically, in this case */
  flex: 1;
}

.player {
  width: 100%;
  display: flex;
  flex-flow: column;
  align-items: center;
  /* text-align: left; */
  background-color: rgb(177, 250, 250);
  margin: 0.3em 0;
  border: 2px inset black;
}
textarea.ci {
  flex: 1;
  width: 100%;
  overflow: auto;
  font-size: 1.1rem;
  max-height: 50vh;
}
.chat {
  /* border: 1px blue solid; */
  display: flex;
  /* align-items: center; */
  flex-flow: column;
  width: 100%;
  height: calc(100vh - 6.5rem);
  overflow-y: auto;
  background-color: rgb(221, 247, 247);
}
.chat-log {
  position: relative;
  overflow-y: auto;
  max-width: 100%;
  flex: 1;
  border: 1px inset;
  display: flex;
  flex-flow: column-reverse;
}
.chat-input {
  margin-top: auto;
  display: flex;
  align-items: flex-end;
}

.peer-chat {
  width: 100%;
  display: flex;
  /* justify-content: space-between; */
  align-items: center;
  flex-flow: column;
  /* overflow: hidden; */
  font-weight: 700;
  /* outline: 5px solid green; */
}

img {
  width: 100%;
}
audio, video {
  display: none;
}
#rt_audio, #rt_video{
  width: 100%;
  position: absolute;
}
#rt_video_local{
  max-width: 250px;
  max-height: 250px;
  position: absolute;
  bottom: 0;
  right: 0;
}

</style>
