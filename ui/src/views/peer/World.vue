<template>
  <div class="world-chat" @click="hide_menu">
    <div class="player">
      <div class="title">
        世界人数：{{peer_size}}
      </div>
      <div class="chat">
        <div class="chat-log">          
          <!-- include variable components -->
          <div v-for="l in chat_logs">
            <ChatText v-if="l.type == 'text'" :log="l" :peer="get_usr_info_by_id(l)" :head_cb="head_cb"></ChatText>
            <div v-else>不支持的聊天类型</div>
          </div>
        </div>
        <div class="chat-input">
          <textarea
            @input="auto_height($event)"
            rows="1"
            class="ci"
            wrap="soft"
            placeholder="input content"
            v-model="chat_content"
          ></textarea>
 
          <div v-if="chat_content" class="small material-icons" @click="send_text_msg()">send</div>
        </div>
      </div>
      <div class="pm">
        <div @click.stop="add_friend()">
          <i class="material-icons">favorite_border</i>
          加为好友
        </div>
        <div @click.stop="block_it()">
          <i class="material-icons">block</i>
          加入黑名单
        </div>
      </div>
    </div>
  </div>
</template>

<script>
import ChatText from "@/components/ChatText.vue";

import util from "@/common/util";
import FTrans from "@/common/file_transfer";

// const chat_record = "chat_record.amr";
const chat_record = "chat_record.mp3";
export default {
  name: "world-chat",
  components: {
    ChatText,
  },
  created: async function() {
    this.$root.$on("peer_changed", this.peer_changed);
    this.$root.$on("world_msg", this.world_msg);
  },
  destroyed() {
    this.$root.$off("peer_changed", this.peer_changed);
    this.$root.$off("world_msg", this.world_msg);

  },
  mounted() {
    this.update_chat_log();
    this.peer_size = peers.size;
  },
  data() {
    return {
      peer_size: 0,
      chat_logs: [],
      chat_content: "",
      target: null,
    };
  },
  computed: {
    aaa() {}
  },
  methods: {
    chat_with() {
      this.$router.push({ name: 'peer-chat', params: { tp: this.target } } );
      this.hide_menu();
    },
    hide_menu(){
      $(`.pm`).hide()
      // console.log(`hide_menu()`);
    },
    head_cb(event, log){
      if(log.dir == 0) return;
      // console.log(`event.clientX=${event.clientX}; event.clientY=${event.clientY}`);
      // console.log(`event.pageX=${event.pageX}; event.pageY=${event.pageY}`);
      // console.log(`$(window).width()=${$(window).width()}`);
      // console.log(`event.screenX=${event.screenX}; event.screenY=${event.screenY}`);
      const mid_w = $(window).width() / 2;
      const mid_h = $(window).height() / 2;
      const menu_w = $(`.pm`).width();
      const menu_h = $(`.pm`).height();
      // console.log(`menu_w=${menu_w}; menu_h=${menu_h}`);
      let pos;
      if(event.pageX < mid_w){
        if(event.pageY < mid_h){
          pos = { left: event.pageX, top: event.pageY };
        } else {
          pos = { left: event.pageX, top: event.pageY - menu_h };
        }
      } else {
        if(event.pageY < mid_h){
          pos = { left: event.pageX - menu_w, top: event.pageY };
        } else {
          pos = { left: event.pageX - menu_w, top: event.pageY - menu_h };
        }
      }   
      // console.log(`pos=${JSON.stringify(pos)}`);
      $(`.pm`).css( pos );  
      $(`.pm`).show()
      event.stopPropagation();
      this.target = this.get_usr_info_by_id(log);
    },
    peer_changed() {
      this.peer_size = peers.size;
    },

    world_msg(data) {
        this.update_chat_log();
    },

    get_usr_info_by_id(log){
      let u = peers.get(log.id);
      if(u && u.usr) return {...u.usr, id: log.id};
    },
    send_text_msg() {
      this.$root.$emit("send_world_msg", {
        id: cli_id,
        type: "text",
        content: this.chat_content
      });
      db.nearby_chat_log.insert({
        id: cli_id,
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
      this.hide_menu();
      if(!this.target) return;
      this.$root.$emit("add_friend", this.target);
      
    },
    block_it() {
      this.hide_menu();
      if(!this.target) return;
      this.$root.$emit("block_it", this.target);     
    },
    update_chat_log() {
      this.chat_logs = db.nearby_chat_log.find({}).reverse();
    }
  }
};
</script>

<!-- Add "scoped" attribute to limit CSS to this component only -->
<style scoped>

.small-avatar {
  width: 2rem;
  height: 2rem;
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

.world-chat {
  width: 100%;
  display: flex;
  /* justify-content: space-between; */
  align-items: center;
  flex-flow: column;
  /* overflow: hidden; */
  font-weight: 700;
  /* outline: 5px solid green; */
}
.pm{
  display: none;
  position: fixed;
  background-color: lightgray;
}
.pm > div + div{
  border-top: 1px outset;
}
.pm > div {
  /* width: 100%; */
  text-align: center;
  padding: 0.8em 1em;
  font-weight: normal;
}
</style>
