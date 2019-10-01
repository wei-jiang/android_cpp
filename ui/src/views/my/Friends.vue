<template>
  <div class="friends">
    <div class="favorite" v-for="f in friend_list">
      <div>
        <i class="small material-icons">favorite_border</i>
      </div>
      <div class="peer-info" @click.stop="show_detail">
        <div>{{title(f)}}<i class=" material-icons">info_outline</i></div>       
        <div class="f-info">
          <img v-if="f.avatar" :src="f.avatar">
          <div v-if="f.signature">{{f.signature}}</div>
        </div>
      </div>
      <div v-if="f.online" @click.stop="chat_with(f)">
        <i class="small material-icons">chat</i>
        &nbsp;&nbsp;&nbsp;&nbsp;
      </div>
      <div @click.stop="remove(f)">
        <i class="red small material-icons">remove_circle</i>
      </div>
    </div>
  </div>
</template>

<script>
import _ from 'lodash'
import util from "@/common/util";

export default {
  name: "friends",
  created: function() {
    this.$root.$on("refresh_friend_list", this.refresh);
    this.t = setInterval(this.check_online.bind(this), 1000)
  },
  destroyed() {
    this.$root.$off("refresh_friend_list", this.refresh);
    clearInterval(this.t);
  },
  mounted() {
    this.refresh();
  },
  data() {
    return {
      t: null,
      friend_list: []
    };
  },
  computed: {

  },
  methods: {
    chat_with(p) {
      if(peers.has(p.id)){
        this.$router.push({ name: 'peer-chat', params: { tp: p } } );
      } else {
        util.show_alert_top_tm('对方已离线。')
      }
    },
    check_online(){
      this.friend_list.forEach(f=>{
        if(peers.has(f.id)){
          let sp = peers.get(f.id);
          if(sp.usr){
            Object.assign(f, sp.usr)
            f.online = true;
          }         
        } else {
          f.online = false;
        }
      })
    },
    title(p){
      return p.nickname || util.truncate(p.id, 16)
    },
    refresh(){
      this.friend_list = db.friends.find({}).map(f=>{
        return {...f, online: false};
      });
    },
    show_detail(e){
      $(e.currentTarget).find(`.f-info`).toggle();
    },
    remove(f){
      navigator.notification.confirm(
          `[${f.nickname}]？`, // message
          i=>{
            // the index uses one-based indexing, so the value is 1, 2, 3, etc.
            if(i == 1){
              db.friends.remove(f);
              this.refresh();
            }
          },            
          this.$t('confirm-del'),           // title
          [this.$t('ok'), this.$t('cancel')]     // buttonLabels
      );
      
    },

  }
};
</script>
<style scoped >
.friends{
  width: 100%;
  min-height: 97%;

}
.red {
  color: red;
}
.f-info{
  display: none;
}
.favorite {
  display: flex;
  /* align-items: center; */
  /* text-align: left; */
  background-color: rgb(177, 250, 250);
  margin: 0.3em 0;
  border: 2px inset black;
}
img{
  width: 90%;
}
.peer-info{
  flex: 1;
}

</style>
