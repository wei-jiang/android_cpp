<template>
  <div class="peer">
    <div class="player" v-for="p in peer_list">

      <img class="small-avatar" :src="p.avatar">
      <div class="peer-info">         
        <div>{{p.nickname}}</div>     
        <div v-if="p.show_type == 1">
          <img :src="p.avatar">
          <div>{{p.signature}}</div>
        </div>
        <div class="chat" v-else-if="p.show_type == 2">
          <div class="chat-log">
            <div class="chat-bubble" v-for="l in p.chat_logs">{{l}}</div>
          </div>         
          <div class="chat-input">
            <div class="small material-icons">record_voice_over</div>
            <!-- <input id="ci" placeholder="chat content"> -->            
            <textarea @input="auto_height($event)" rows='1' id="ci" wrap="soft" placeholder="input content" v-model="chat_content"></textarea>
            <div class="small material-icons" @click="p.chat_logs.unshift(chat_content); chat_content=''">send</div>
          </div>
        </div>
      </div>
      <div v-if="p.show_type != 0" @click.stop="p.show_type = 0">
        &nbsp;
        <i class="small material-icons">expand_less</i>
        &nbsp;
      </div>
      
      <div v-if="p.show_type == 0" @click.stop="p.show_type = 1">
        <i class="small material-icons">info_outline</i>
        &nbsp;&nbsp;
      </div>      
      <div v-if="p.show_type == 0" @click.stop="p.show_type = 2">
        <i class="small material-icons">chat</i>
        &nbsp;&nbsp;
      </div>
      <div v-if="p.show_type == 0" @click.stop="add_friend(p)">
        <i class="small material-icons">favorite_border</i>
        &nbsp;&nbsp;
      </div>
      <div v-if="p.show_type == 0" @click.stop="block_it(p)">
        <i class="small material-icons">block</i>
      </div>
    </div>
  </div>
</template>

<script>

import util from "@/common/util";
export default {
  name: "Peer",
  props: {
    msg: String
  },
  created: async function() {
    this.$root.$on("peer_changed", this.refresh);
  },
  destroyed() {
    this.$root.$off("peer_changed", this.refresh);
  },
  mounted() {
    this.refresh()
  },
  data() {
    return {
      peer_list: [],
      chat_content: ''
    };
  },
  computed: {
    address() {
      return ``;
    }
  },
  methods: {
    auto_height(e){
      const el = e.target;
      el.style.cssText = 'height:auto; padding:0';
      el.style.cssText = 'height:' + el.scrollHeight + 'px';
      console.log('in auto_height; el.style.height=' + el.style.height)
    },
    add_friend(p){
      navigator.notification.confirm(
          `[${p.nickname}]？`, // message
          i=>{
            // the index uses one-based indexing, so the value is 1, 2, 3, etc.
            if(i == 1){
              const it = db.friends.findOne({id: p.id})
              if(!it){
                db.friends.insert(p)
                this.$root.$emit("add_friend", '');
                util.show_alert_top_tm(this.$t('success'))
              }
            }
          },            
          this.$t('add-friend'),           // title
          [this.$t('ok'), this.$t('cancel')]     // buttonLabels
      );
    },
    block_it(p){
      navigator.notification.confirm(
          `[${p.nickname}]？`, // message
          i=>{
            // the index uses one-based indexing, so the value is 1, 2, 3, etc.
            if(i == 1){
              const it = db.blacklist.findOne({id: p.id})
              if(!it){
                db.blacklist.insert(p)
                this.$root.$emit("add_friend", '');
                util.show_alert_top_tm(this.$t('success'))
              }
            }
          },            
          this.$t('block-it'),           // title
          [this.$t('ok'), this.$t('cancel')]     // buttonLabels
      );
    },
    chat(data) {

    },
    refresh() {
      // console.log('peer refresh')
      const tmp_peer = [];
      for (const [k, v] of peers.entries()) {
        if(v.usr){
          tmp_peer.push({
            id: k,
            show_type: 0,
            nickname: v.usr.nickname,
            avatar: v.usr.avatar,
            signature: v.usr.signature,
            chat_logs: []
          });
        }
      }
      // console.log(`tmp_peer=${JSON.stringify(tmp_peer)}`)
      this.peer_list = tmp_peer;
    },    
    
  }
};
</script>

<!-- Add "scoped" attribute to limit CSS to this component only -->
<style scoped>
.small-avatar {
  width: 2rem;
  height: 2rem;
}
.peer-info{
  flex: 1;
}
.player {
  width: 100%;
  display: flex;
  align-items: center;
  /* text-align: left; */
  background-color: rgb(177, 250, 250);
  margin: 0.3em 0;
  border: 2px inset black;
}
#ci{
  flex: 1;
  width: 100%;
  overflow: auto;
  font-size: 1.1rem;
  max-height: 50vh;
}
.chat{
  display: flex;
  /* align-items: center; */
  flex-flow: column;
  width: 100%;
  height: 70vh;
  background-color: rgb(221, 247, 247);
}
.chat-log{
  max-width: 100%;
  flex: 1;
  border: 1px inset;
  display: flex;
  flex-flow: column-reverse;
}
.chat-input{
  margin-top: auto;
  display: flex;
}
.chat-bubble {
  word-wrap: break-word;
  margin: 0.5em 0.4em;
  left: 50%;
  width: 45%;
	position: relative;
	background: #82ebf5;
	border-radius: .4em;
}

.chat-bubble:after {
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
.peer {
  width: 100%;
  display: flex;
  /* justify-content: space-between; */
  align-items: center;
  flex-flow: column;
  /* overflow: hidden; */
  font-weight: 700;
}
img{
  width: 100%;
}

</style>
