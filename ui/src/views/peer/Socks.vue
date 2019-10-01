<template>
  <div class="proxy">
    <div class="remote-proxy">
      <div class="title">
        <div>远程代理信息</div>
      </div>
      <div class="remote-info">
        <div v-if="remote_proxy_user">
          <div class="user">
            <img class="small-avatar" :src="remote_proxy_user.avatar" />
            <div class="peer-name">{{remote_proxy_user.nickname}}</div>
            <div>{{short_it(remote_proxy_user.signature)}}</div>
          </div>
          <div class="local-addr" v-if="window.remote_socks_addr">
            <div>远程socks5地址：</div>
            <div class="green">{{window.remote_socks_addr}}</div>
            <div>pac地址：</div>
            <div class="green">{{`${window.http_addr}remote.pac`}}</div>

            <div>当前本地远程代理连接数：{{lscc}}</div>
          </div>
          <div v-else>未连入局域网</div>         
        </div>
        <div v-else>
          <div>当前未设置远程代理用户。</div>
          <div class="tip"><i class="green">提示：</i>可在与玩家私聊界面中请求他/她作为代理！</div>
        </div>
      </div>
    </div>
    <div class="proxy-users">
      <div class="title tooltip">
        <span class="tooltiptext">当前使用您手机作为代理的用户，及其打开的连接数</span>
        <div class="bubble">{{usr_count}}</div>
        <div class="caption">当前代理用户</div>
        <div class="tt" @click.stop="toggle_tooltip">&#10067;</div>
      </div>
      <div>
        <div v-for="u in users" class="user">
          <img class="small-avatar" :src="u.avatar" />
          <div class="peer-name">{{u.nickname}}</div>
          <div><span class="bubble">{{u.count}}</span>个连接</div>
          <div @click.stop="chat_with(u)">
            <i class="small material-icons green">chat</i>
            &nbsp;&nbsp;
          </div>
        </div>
      </div>
    </div>
  </div>
</template>

<script>
import util from "@/common/util";
export default {
  name: "proxy",
  props: {
    msg: String
  },
  created: async function() {
    this.$root.$on("noty_proxy_info", this.refresh);
    this.$root.$on("remote_proxy_changed", this.remote_proxy_changed);
  },
  destroyed() {
    this.$root.$off("noty_proxy_info", this.refresh);
    this.$root.$off("remote_proxy_changed", this.remote_proxy_changed);
  },
  mounted() {
    this.remote_proxy_changed();
  },
  data() {
    return {
      remote_proxy_id: '',
      users: [],
      lscc: 0
    };
  },
  computed: {
    remote_proxy_user() {
      if(this.remote_proxy_id){
        return peers.get(this.remote_proxy_id).usr;
      }
    },
    usr_count(){
      return this.users.length;
    }
  },
  methods: {
    short_it(str){
      return util.truncate(str, 9);
    },
    remote_proxy_changed(){
      this.remote_proxy_id = socks_pid;
      
    },
    chat_with(u) {
      this.$router.push({ name: "peer-chat", params: { tp: u } });
    },
    toggle_tooltip(event) {
      $(event.target)
        .siblings(".tooltiptext")
        .toggle();
    },
    refresh(data) {
      // {"6dca4b03f180588b468036b1eff907a4":11,"cmd":"noty_proxy_info"}
      this.lscc = data.local_socks_cnn_count;
      delete data.cmd;
      delete data.local_socks_cnn_count;
      this.users = [];
      for (let [k, v] of Object.entries(data)) {
        let u = peers.get(k).usr;
        if (u) {
          this.users.push({
            id: k,
            count: v,
            nickname: u.nickname,
            avatar: u.avatar,
            signature: u.signature
          });
        }
      }
    },

  }
};
</script>

<!-- Add "scoped" attribute to limit CSS to this component only -->
<style scoped>
.remote-proxy, .proxy-users{
  width: 100%;
}
.proxy-users{
  margin-top: 1em;
}
.caption{
  flex: 1;
}
.bubble{
  background: #666;
  border-radius: 50%;
  color: #fff;
  display: inline-block;
  font-weight: bold;
  line-height: 2.1em;
  text-align: center;
  min-width: 2.1em; 
}
.user {
  background-color: rgb(192, 206, 218);
  border: 1px lightcyan inset;
  width: 100%;
  display: flex;
  align-items: center;
  margin: 0.7em 0.5em 0.5em 0;
  /* align-items: center; */
}
.local-addr {
  text-align: left;
}
.remote-info{
  border: 2px darkcyan inset;
}
.small-avatar {
  width: 2rem;
  height: 2rem;
}
.tooltiptext{
  font-size: 0.9rem;
}
.title {
  position: relative;
  margin-top: 0.5em;
  background-color: lightgray;
  width: 100%;
  font-size: 1.1rem;
  text-align: center;
  display: flex;
  align-items : center;
  justify-content: center;
}

.peer-name {
  flex: 1;
}

.tt {
  /* margin-left: auto; */
  /* float: right; */
  right: 0.2em;
  margin-right: 0.2em;
  font-size: 1.5rem;
  cursor: pointer;
}
.tip{
  font-size: 0.9em;
}
.proxy {
  display: flex;
  /* justify-content: space-between; */
  align-items: center;
  flex-flow: column;
  /* overflow: hidden; */
  font-weight: 700;
}
</style>
