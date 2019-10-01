<template>
  <div class="world-chat" @click="hide_menu">
    <div class="player">
      <div class="title">
        <div @click="$router.go(-1)" class="small material-icons">keyboard_backspace</div>
        <div class="online-count">服务器人数：{{online_count}} </div>
        <div class="small material-icons transparent">keyboard_backspace</div>
      </div>
      <div class="chat">
        <div class="chat-log">          
          <!-- include variable components -->
          <div v-for="l in chat_logs">
            <ChatWorld v-if="l.type == 'text'" :log="l" :head_cb="head_cb"></ChatWorld>
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
import ChatWorld from "@/components/ChatWorld.vue";

import util from "@/common/util";

export default {
  name: "world-chat",
  components: {
    ChatWorld,
  },
  created: async function() {
    this.$root.$on("wss_destroyed", this.wss_destroyed);
    this.$root.$on("online_count", this.refresh_online_count);
    this.$root.$on("refresh_world_chat", this.update_chat_log);
  },
  destroyed() {
    this.$root.$off("wss_destroyed", this.wss_destroyed);
    this.$root.$off("online_count", this.refresh_online_count);
    this.$root.$off("refresh_world_chat", this.update_chat_log);

  },
  watch: {
    '$route.query.addr': function(newVal, oldVal) {
      console.log(`watch $route.query.addr`)
        this.get_online_count();
    }
  },
  mounted() {
    this.get_online_count();
    this.update_chat_log();
  },
  data() {
    return {
      cd: null,
      addr: null,
      online_count: 0,
      chat_logs: [],
      chat_content: "",
      target: null,
    };
  },
  computed: {
    aaa() {}
  },
  methods: {
    stringify(data){
      return JSON.stringify(data)
    },
    get_online_count(){     
      this.addr = this.$route.query.addr;
      // console.log(`before get_online_count(): ${this.addr}`)
      this.online_count = sss[this.addr] ? sss[this.addr].total : 0;
      // console.log(`after get_online_count(): ${this.addr}`)
    },
    wss_destroyed(addr){
      if(addr == this.addr){
        this.$router.go(-1)
      }
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
    refresh_online_count(data) {
      if(this.addr == data.addr){
        this.online_count = data.total;
      }     
    },

    get_usr_info_by_id(log){
      let u = peers.get(log.id);
      if(u && u.usr) return {...u.usr, id: log.id};
    },
    send_text_msg() {
      if(this.cd) return util.show_warn_top('发言太快，请稍后再试！')
      if(this.chat_content.length >= 30) return util.show_warn_top('消息内容太长！')
      this.cd = true;
      const wss = sss[this.addr];
      if(wss) {
        console.log(`send world msg to: ${this.addr}`)
        wss.send_world_msg(this.chat_content);
      }
      db.world_chat_log.insert({
        id: cli_id,
        type: "text",
        content: this.chat_content,
        dt: util.now_str(),
        dir: 0
      });
      this.update_chat_log();
      this.chat_content = "";
      $("textarea").css("height", "2.0rem");
      setTimeout(()=>{
        this.cd = null;
      }, 130 * 1000);
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
    update_chat_log(addr) {
      if(typeof addr === 'undefined' || addr === this.addr){
        this.chat_logs = db.world_chat_log.find({}).reverse();
        // console.log(`this.chat_logs=${JSON.stringify(this.chat_logs)}`)
      }    
    }
  }
};
</script>

<!-- Add "scoped" attribute to limit CSS to this component only -->
<style scoped>
.transparent{
  opacity: 0;
}
.small-avatar {
  width: 2rem;
  height: 2rem;
}

.title {
  width: 100%;
  display: flex;
  /* justify-content: center; */
  align-items: center; /* center items vertically, in this case */
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
  height: 2rem;
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
  height: calc(100vh - 7.5rem);
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
.online-count{
  flex: 1;
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
